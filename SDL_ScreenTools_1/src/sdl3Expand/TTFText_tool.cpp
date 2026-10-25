#include "TTFText_tool.h"
#include <cmath>

TTFTextTool::~TTFTextTool() { Shutdown(); }

bool TTFTextTool::Init(SDL_Renderer* renderer, const char* defaultFontPath,
    float defaultPtSize, size_t initialPoolSize) {
    Shutdown();

    if (!renderer || !defaultFontPath || defaultFontPath[0] == '\0') {
        lastError_ = "Init: renderer or fontPath is invalid";
        return false;
    }
    renderer_ = renderer;
    defaultPtSize_ = defaultPtSize > 0.0f ? static_cast<int>(defaultPtSize) : 16;

    // 默认字体注册为 fontId 0
    fontPaths_.clear();
    fontPaths_.emplace_back(defaultFontPath);

    textEngine_ = TTF_CreateRendererTextEngine(renderer_);
    if (!textEngine_) {
        lastError_ = std::string("CreateRendererTextEngine failed: ") + SDL_GetError();
        return false;
    }

    if (!GetFont(0, static_cast<float>(defaultPtSize_))) {
        Shutdown();
        return false;
    }

    if (initialPoolSize == 0) initialPoolSize = 1;
    pool_.reserve(initialPoolSize);
    if (!GrowPool(initialPoolSize)) { Shutdown(); return false; }

    batch_.reserve(initialPoolSize);
    return true;
}

int TTFTextTool::RegisterFont(const char* fontPath) {
    if (!fontPath || fontPath[0] == '\0') {
        lastError_ = "RegisterFont: invalid path";
        return -1;
    }
    fontPaths_.emplace_back(fontPath);
    return static_cast<int>(fontPaths_.size()) - 1;
}

TTF_Font* TTFTextTool::GetFont(int fontId, float ptSize) {
    if (fontId < 0 || fontId >= static_cast<int>(fontPaths_.size())) {
        fontId = 0;  // 非法 ID 回退默认
    }

    FontKey key{ fontId, ptSize };
    auto it = fontCache_.find(key);
    if (it != fontCache_.end()) return it->second;

    if (fontId < 0 || fontId >= static_cast<int>(fontPaths_.size())) {
        lastError_ = "GetFont: invalid fontId";
        return nullptr;
    }

    TTF_Font* font = TTF_OpenFont(fontPaths_[fontId].c_str(), ptSize);
    if (!font) {
        lastError_ = std::string("OpenFont(") + fontPaths_[fontId] + ", "
            + std::to_string(ptSize) + ") failed: " + SDL_GetError();
        return nullptr;
    }
    fontCache_[key] = font;
    return font;
}

void TTFTextTool::CleanupFonts() {
    for (auto& [key, font] : fontCache_) {
        if (font) TTF_CloseFont(font);
    }
    fontCache_.clear();
}

bool TTFTextTool::GrowPool(size_t targetSize) {
    if (targetSize <= pool_.size()) return true;

    TTF_Font* defaultFont = GetFont(0, static_cast<float>(defaultPtSize_));
    if (!defaultFont) return false;

    pool_.reserve(targetSize);
    for (size_t i = pool_.size(); i < targetSize; ++i) {
        TTF_Text* t = TTF_CreateText(textEngine_, defaultFont, "", 0);
        if (!t) {
            lastError_ = std::string("CreateText failed at pool index ")
                + std::to_string(i) + ": " + SDL_GetError();
            return false;
        }
        pool_.push_back(t);
    }
    return true;
}

void TTFTextTool::Shutdown() {
    for (TTF_Text* t : pool_) if (t) TTF_DestroyText(t);
    pool_.clear();
    poolUsed_ = 0;
    batch_.clear();

    if (textEngine_) {
        TTF_DestroyRendererTextEngine(textEngine_);
        textEngine_ = nullptr;
    }
    CleanupFonts();
    fontPaths_.clear();
    renderer_ = nullptr;
}

void TTFTextTool::BeginBatch() {
    batch_.clear();
    poolUsed_ = 0;
}

void TTFTextTool::AddText(const std::string& content, float x, float y,
    int ptSize, SDL_Color color, int fontId) {
    batch_.push_back({ content, std::round(x), std::round(y), ptSize, color, fontId });
}

void TTFTextTool::FlushBatch() {
    if (!textEngine_) return;

    for (const auto& item : batch_) {
        if (poolUsed_ >= pool_.size()) {
            size_t newSize = pool_.size() * 2;
            if (newSize <= pool_.size()) newSize = pool_.size() + 1;
            if (!GrowPool(newSize)) return;
        }

        TTF_Font* font = GetFont(item.fontId, static_cast<float>(item.ptSize));
        if (!font) { ++poolUsed_; continue; }

        TTF_Text* t = pool_[poolUsed_];

        if (!TTF_SetTextFont(t, font)) {
            lastError_ = std::string("SetTextFont failed: ") + SDL_GetError();
            ++poolUsed_; continue;
        }
        if (!TTF_SetTextString(t, item.content.c_str(), 0)) {
            lastError_ = std::string("SetTextString failed: ") + SDL_GetError();
            ++poolUsed_; continue;
        }
        if (!TTF_SetTextColor(t, item.color.r, item.color.g, item.color.b, item.color.a)) {
            lastError_ = std::string("SetTextColor failed: ") + SDL_GetError();
            ++poolUsed_; continue;
        }
        TTF_DrawRendererText(t, item.x, item.y);
        ++poolUsed_;
    }
}

bool TTFTextTool::GetTextSize(int id, int* w, int* h) {
    if (id < 0 || id >= static_cast<int>(pool_.size())) return false;
    TTF_Text* t = pool_[id];
    if (!t) return false;
    return TTF_GetTextSize(t, w, h);
}