#include "TTFText_tool.h"
#include <cmath>
#include <utility>

TTFTextTool::~TTFTextTool() {
    Shutdown();
}

bool TTFTextTool::Init(SDL_Renderer* renderer, const char* fontPath, float defaultPtSize, size_t initialPoolSize) {
    Shutdown();

    if (!renderer || !fontPath || fontPath[0] == '\0') {
        lastError_ = "Init: renderer or fontPath is invalid";
        return false;
    }
    renderer_ = renderer;
    fontPath_ = fontPath;
    defaultPtSize_ = defaultPtSize > 0.0f ? defaultPtSize : 16.0f;

    textEngine_ = TTF_CreateRendererTextEngine(renderer_);
    if (!textEngine_) {
        lastError_ = std::string("CreateRendererTextEngine failed: ") + SDL_GetError();
        return false;
    }

    // 预加载默认字号字体（结果存入 fontCache_）
    if (!GetFont(defaultPtSize_)) {
        Shutdown();
        return false;
    }

    if (initialPoolSize == 0) initialPoolSize = 1;
    pool_.reserve(initialPoolSize);
    if (!GrowPool(initialPoolSize)) {
        Shutdown();
        return false;
    }

    batch_.reserve(initialPoolSize);
    return true;
}

TTF_Font* TTFTextTool::GetFont(float ptSize) {
    auto it = fontCache_.find(ptSize);
    if (it != fontCache_.end()) return it->second;

    TTF_Font* font = TTF_OpenFont(fontPath_.c_str(), ptSize);
    if (!font) {
        lastError_ = std::string("OpenFont(") + fontPath_ + ", "
            + std::to_string(ptSize) + ") failed: " + SDL_GetError();
        return nullptr;
    }
    fontCache_[ptSize] = font;
    return font;
}

void TTFTextTool::CleanupFonts() {
    for (auto& [size, font] : fontCache_) {
        if (font) TTF_CloseFont(font);
    }
    fontCache_.clear();
}

bool TTFTextTool::GrowPool(size_t targetSize) {
    if (targetSize <= pool_.size()) return true;

    TTF_Font* defaultFont = GetFont(defaultPtSize_);
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
    for (TTF_Text* t : pool_) {
        if (t) TTF_DestroyText(t);
    }
    pool_.clear();
    poolUsed_ = 0;
    batch_.clear();

    if (textEngine_) {
        TTF_DestroyRendererTextEngine(textEngine_);
        textEngine_ = nullptr;
    }
    CleanupFonts();

    renderer_ = nullptr;
    fontPath_.clear();
}

void TTFTextTool::BeginBatch() {
    batch_.clear();
    poolUsed_ = 0;
}

void TTFTextTool::AddText(const std::string& content, float x, float y, int ptSize,SDL_Color color){
    batch_.push_back({ content, std::round(x), std::round(y), ptSize, color });
}

void TTFTextTool::FlushBatch() {
    if (!textEngine_) return;

    for (const auto& item : batch_) {
        if (poolUsed_ >= pool_.size()) {
            size_t newSize = pool_.size() * 2;
            if (newSize <= pool_.size()) newSize = pool_.size() + 1;
            if (!GrowPool(newSize)) {
                return;
            }
        }

        TTF_Font* font = GetFont(item.ptSize);
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
    TTF_Text* t = pool_[id];
    if (t) {
        TTF_GetTextSize(t, w, h);
        return true;
    }
    return false;
}