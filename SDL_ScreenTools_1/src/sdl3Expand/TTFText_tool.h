#pragma once
#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <string>
#include <vector>
#include <unordered_map>

class TTFTextTool {
public:
    TTFTextTool() = default;
    ~TTFTextTool();

    // 禁止拷贝，避免 TTF_Text 所有权混乱
    TTFTextTool(const TTFTextTool&) = delete;
    TTFTextTool& operator=(const TTFTextTool&) = delete;

    // 初始化：传入目标渲染器和字体。内部创建文本引擎。
    // 返回 false 时可用 LastError() 查看原因。
    bool Init(SDL_Renderer* renderer, const char* fontPath, float defaultPtSize, size_t initialPoolSize = 32);

    void Shutdown();

    bool IsInitialized() const { return textEngine_ != nullptr; }

    // 开始本帧批次添加
    void BeginBatch();

    void AddText(const std::string& content, float x, float y, int ptSize = 20,
        SDL_Color color = { 255, 255, 255, 255 });

    // 提交本帧批次
    void FlushBatch();

    //BeginBatch + AddText*n + FlushBatch

    // 当前池容量
    size_t PoolSize() const { return pool_.size(); }

    bool GetTextSize(int id, int* w, int* h);

    // 最近一次错误信息
    const std::string& LastError() const { return lastError_; }

private:
    TTF_Font* GetFont(float ptSize);
    void CleanupFonts();
    bool GrowPool(size_t targetSize);

    SDL_Renderer* renderer_ = nullptr;
    TTF_TextEngine* textEngine_ = nullptr;

    std::string fontPath_;
    int defaultPtSize_ = 16.0f;

    std::vector<TTF_Text*> pool_;
    size_t poolUsed_ = 0;

    std::unordered_map<float, TTF_Font*> fontCache_;

    struct BatchItem {
        std::string content;
        float x, y;
        int ptSize;
        SDL_Color color;
    };
    std::vector<BatchItem> batch_;

    std::string lastError_;
};