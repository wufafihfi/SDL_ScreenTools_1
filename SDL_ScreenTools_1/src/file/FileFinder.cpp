#include "FileFinder.h"
#include <SDL3/SDL.h>
#include <algorithm>
#include <cctype>

// ---------- 配置 ----------

void FileFinder::SetRootPath(const std::string& path) {
    rootPath_ = path.empty() ? "." : path;
    InvalidateCache();
}

void FileFinder::SetExtensions(std::vector<std::string> exts) {
    for (auto& e : exts) {
        // 统一转小写便于比较（若区分大小写则保留原样）
        if (!caseSensitive_) {
            std::transform(e.begin(), e.end(), e.begin(),
                           [](unsigned char c) { return (char)std::tolower(c); });
        }
        // 确保以点开头
        if (!e.empty() && e[0] != '.') e.insert(e.begin(), '.');
    }
    extensions_ = std::move(exts);
    InvalidateCache();
}

void FileFinder::SetRecursive(bool recursive) {
    recursive_ = recursive;
}

void FileFinder::SetCaseSensitive(bool sensitive) {
    caseSensitive_ = sensitive;
}

// ---------- 扫描 ----------

bool FileFinder::Rescan() {
    entries_.clear();
    InvalidateCache();

    bool anyScanned = false;

    // 源 1：用户指定目录
    {
        SDL_PathInfo info;
        if (SDL_GetPathInfo(rootPath_.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY) {
            ScanDirectory(rootPath_);
            anyScanned = true;
        }
        else {
            SDL_Log("FileFinder: cannot access root path '%s': %s",
                rootPath_.c_str(), SDL_GetError());
        }
    }

    // 源 2：系统字体目录（仅字体模式）
    if (fontMode_) {
        std::vector<std::string> sysDirs;
        CollectSystemFontDirs(sysDirs);
        for (const auto& d : sysDirs) {
            SDL_PathInfo info;
            if (SDL_GetPathInfo(d.c_str(), &info) && info.type == SDL_PATHTYPE_DIRECTORY) {
                ScanDirectory(d);
                anyScanned = true;
            }
            else {
                // 系统字体目录不存在是正常情况，不刷日志
            }
        }
    }

    InvalidateCache();
    return anyScanned;
}

void FileFinder::SetFontMode(bool enabled) {
    if (fontMode_ == enabled) return;
    fontMode_ = enabled;
    if (fontMode_) {
        MergeFontExtensions();
    }
    InvalidateCache();
}

void FileFinder::MergeFontExtensions() {
    const std::vector<std::string> fontExts = { ".ttf", ".otf", ".ttc" };
    for (const auto& fe : fontExts) {
        std::string e = fe;
        if (!caseSensitive_) {
            std::transform(e.begin(), e.end(), e.begin(),
                [](unsigned char c) { return (char)std::tolower(c); });
        }
        bool exists = false;
        for (const auto& cur : extensions_) {
            if (cur == e) { exists = true; break; }
        }
        if (!exists) extensions_.push_back(e);
    }
}

void FileFinder::CollectSystemFontDirs(std::vector<std::string>& outDirs) const {
    outDirs.clear();
#if defined(_WIN32)
    outDirs.push_back("C:/Windows/Fonts");
    // 用户字体目录（Win10+ 允许非管理员安装字体）
    const char* localAppData = SDL_getenv("LOCALAPPDATA");
    if (localAppData) {
        outDirs.push_back(std::string(localAppData) + "/Microsoft/Windows/Fonts");
    }
#elif defined(__APPLE__)
    outDirs.push_back("/System/Library/Fonts");
    outDirs.push_back("/Library/Fonts");
    const char* home = SDL_getenv("HOME");
    if (home) outDirs.push_back(std::string(home) + "/Library/Fonts");
#else // Linux 及其他 Unix
    outDirs.push_back("/usr/share/fonts");
    outDirs.push_back("/usr/local/share/fonts");
    const char* home = SDL_getenv("HOME");
    if (home) {
        outDirs.push_back(std::string(home) + "/.local/share/fonts");
        outDirs.push_back(std::string(home) + "/.fonts");
    }
#endif
}

namespace {
    // 回调上下文
    struct ScanCtx {
        FileFinder* finder;
        std::string dir;
    };

    SDL_EnumerationResult SDLCALL ScanCallback(void* userdata,
                                               const char* dirname,
                                               const char* fname) {
        auto* ctx = static_cast<ScanCtx*>(userdata);
        FileFinder* self = ctx->finder;

        // 完整路径：SDL 保证 dirname 以分隔符结尾
        std::string fullPath = std::string(dirname) + fname;

        SDL_PathInfo info;
        if (!SDL_GetPathInfo(fullPath.c_str(), &info)) {
            return SDL_ENUM_CONTINUE; // 无法获取信息，跳过
        }

        if (info.type == SDL_PATHTYPE_FILE) {
            self->AddEntry(fullPath);
        } else if (info.type == SDL_PATHTYPE_DIRECTORY && self->recursive_) {
            ScanCtx sub{ self, std::string(dirname) + fname };
            SDL_EnumerateDirectory(fullPath.c_str(), ScanCallback, &sub);
        }
        return SDL_ENUM_CONTINUE;
    }
}

void FileFinder::ScanDirectory(const std::string& dir) {
    ScanCtx ctx{ this, dir };
    if (!SDL_EnumerateDirectory(dir.c_str(), ScanCallback, &ctx)) {
        SDL_Log("FileFinder: failed to enumerate '%s': %s",
                dir.c_str(), SDL_GetError());
    }
}

void FileFinder::AddEntry(const std::string& fullPath) {
    // 取出文件名
    size_t sep = fullPath.find_last_of("/\\");
    std::string fileName = (sep == std::string::npos) ? fullPath : fullPath.substr(sep + 1);
    std::string directory = (sep == std::string::npos) ? "" : fullPath.substr(0, sep);

    if (!MatchExtension(fileName)) return;

    // 去重
    for (const auto& e : entries_) {
        if (e.fullPath == fullPath) return;
    }

    entries_.push_back({ fullPath, fileName, directory });
    InvalidateCache();
}

bool FileFinder::MatchExtension(const std::string& fileName) const {
    if (extensions_.empty()) return true; // 未设置后缀则全部收录

    std::string name = fileName;
    if (!caseSensitive_) {
        std::transform(name.begin(), name.end(), name.begin(),
                       [](unsigned char c) { return (char)std::tolower(c); });
    }

    for (const auto& ext : extensions_) {
        if (name.size() < ext.size()) continue;
        if (name.compare(name.size() - ext.size(), ext.size(), ext) == 0)
            return true;
    }
    return false;
}

// ---------- 查询 ----------

size_t FileFinder::Count() const { return entries_.size(); }
bool FileFinder::Empty() const { return entries_.empty(); }

const FileEntry& FileFinder::Get(size_t index) const {
    static const FileEntry kEmpty{};
    return index < entries_.size() ? entries_[index] : kEmpty;
}

const std::vector<FileEntry>& FileFinder::GetAll() const { return entries_; }

size_t FileFinder::FindByName(const std::string& fileName) const {
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].fileName == fileName) return i;
    }
    return entries_.size();
}

size_t FileFinder::FindByPath(const std::string& fullPath) const {
    for (size_t i = 0; i < entries_.size(); ++i) {
        if (entries_[i].fullPath == fullPath) return i;
    }
    return entries_.size();
}

// ---------- ImGui 缓存 ----------

void FileFinder::InvalidateCache() const {
    cacheDirty_ = true;
}

const char* const* FileFinder::GetNamesCStr() const {
    if (cacheDirty_) {
        nameCache_.clear();
        nameCache_.reserve(entries_.size() + 1);
        for (const auto& e : entries_) {
            nameCache_.push_back(e.fileName.c_str());
        }
        nameCache_.push_back(nullptr); // 结尾
        cacheDirty_ = false;
    }
    return nameCache_.data();
}

int FileFinder::GetNamesCount() const {
    return static_cast<int>(entries_.size());
}

void FileFinder::Clear() {
    entries_.clear();
    InvalidateCache();
}