#pragma once
#include <string>
#include <vector>
#include <cstddef>

// 单条文件记录
struct FileEntry {
    std::string fullPath;   // 完整路径
    std::string fileName;   // 仅文件名
    std::string directory;  // 所在目录
};

class FileFinder {
public:
    FileFinder() = default;

    // 配置
    void SetRootPath(const std::string& path);           // 扫描根目录
    void SetExtensions(std::vector<std::string> exts);   // 如 {".png", ".jpg"}，含点，大小写不敏感
    void SetRecursive(bool recursive);                   // 是否递归子目录
    void SetCaseSensitive(bool sensitive);               // 后缀匹配是否区分大小写

    // 扫描
    void SetFontMode(bool enabled);
    bool IsFontMode() const { return fontMode_; }
    bool Rescan();                                       // 重新扫描，返回是否成功（目录可访问）

    // 查询
    size_t Count() const;
    bool Empty() const;
    const FileEntry& Get(size_t index) const;
    const std::vector<FileEntry>& GetAll() const;

    // 按文件名查找（返回索引，找不到返回 Count()）
    size_t FindByName(const std::string& fileName) const;
    // 按完整路径查找
    size_t FindByPath(const std::string& fullPath) const;

    // ImGui 下拉菜单支持：返回以 nullptr 结尾的 const char* 数组
    // 注意：内部缓存，Rescan 或修改后自动失效重建
    const char* const* GetNamesCStr() const;
    // 供 ImGui::Combo 使用的 int 计数（不含结尾 nullptr）
    int GetNamesCount() const;

    // 清除
    void Clear();

    void AddEntry(const std::string& fullPath);
    bool recursive_ = true;

private:
    bool MatchExtension(const std::string& fileName) const;
    void ScanDirectory(const std::string& dir);
    void InvalidateCache() const;

    std::string rootPath_ = ".";
    std::vector<std::string> extensions_;
    bool caseSensitive_ = false;

    std::vector<FileEntry> entries_;

    // ImGui 缓存
    mutable std::vector<const char*> nameCache_;
    mutable bool cacheDirty_ = true;

    bool fontMode_ = false;
    void CollectSystemFontDirs(std::vector<std::string>& outDirs) const;
    void MergeFontExtensions();
};

/*
#include "FileFinder.h"
#include "imgui.h"

FileFinder textureFinder;

void InitFinder() {
    textureFinder.SetRootPath("assets/textures");
    textureFinder.SetExtensions({".png", ".jpg"});
    textureFinder.SetRecursive(true);
    textureFinder.SetCaseSensitive(false);
    textureFinder.Rescan();
}

void DrawFinderUI() {
    static int selected = -1;

    // 重新扫描按钮
    if (ImGui::Button("Rescan")) {
        textureFinder.Rescan();
        selected = -1;
    }
    ImGui::SameLine();
    ImGui::Text("Files: %d", textureFinder.GetNamesCount());

    // 下拉菜单：直接喂 const char* 数组
    const char* preview = (selected >= 0 && selected < textureFinder.GetNamesCount())
                          ? textureFinder.Get(selected).fileName.c_str()
                          : "<select file>";

    if (ImGui::BeginCombo("Texture", preview)) {
        for (int i = 0; i < textureFinder.GetNamesCount(); ++i) {
            const bool isSelected = (selected == i);
            // 显示文件名，但可用 Get(i).fullPath 做其它逻辑
            if (ImGui::Selectable(textureFinder.Get(i).fileName.c_str(), isSelected)) {
                selected = i;
            }
            // 悬浮显示完整路径，方便区分同名文件
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", textureFinder.Get(i).fullPath.c_str());
            }
        }
        ImGui::EndCombo();
    }

    if (selected >= 0 && selected < textureFinder.GetNamesCount()) {
        const auto& entry = textureFinder.Get(selected);
        ImGui::Text("Full path: %s", entry.fullPath.c_str());
        ImGui::Text("Directory: %s", entry.directory.c_str());
    }
*/