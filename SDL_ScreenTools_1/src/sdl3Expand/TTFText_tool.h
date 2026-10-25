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

    TTFTextTool(const TTFTextTool&) = delete;
    TTFTextTool& operator=(const TTFTextTool&) = delete;

    bool Init(SDL_Renderer* renderer, const char* defaultFontPath,
        float defaultPtSize, size_t initialPoolSize = 32);

    // 注册额外字体，返回 fontId（>=0 成功，-1 失败）
    // 第一个注册的字体 ID 为 0，对应 Init 传入的默认字体路径
    int RegisterFont(const char* fontPath);

    void Shutdown();
    bool IsInitialized() const { return textEngine_ != nullptr; }

    void BeginBatch();

    // fontId 默认 0（Init 的默认字体）
    void AddText(const std::string& content, float x, float y, int ptSize = 20,
        SDL_Color color = { 255, 255, 255, 255 }, int fontId = 0);

    void FlushBatch();

    size_t PoolSize() const { return pool_.size(); }
    bool GetTextSize(int id, int* w, int* h);
    const std::string& LastError() const { return lastError_; }

private:
    struct FontKey {
        int fontId;
        float ptSize;
        bool operator==(const FontKey& o) const {
            return fontId == o.fontId && ptSize == o.ptSize;
        }
    };
    struct FontKeyHash {
        size_t operator()(const FontKey& k) const {
            return std::hash<int>()(k.fontId)
                ^ (std::hash<float>()(k.ptSize) << 1);
        }
    };

    TTF_Font* GetFont(int fontId, float ptSize);
    void CleanupFonts();
    bool GrowPool(size_t targetSize);

    SDL_Renderer* renderer_ = nullptr;
    TTF_TextEngine* textEngine_ = nullptr;

    std::vector<std::string> fontPaths_;   // 按 fontId 索引
    int defaultPtSize_ = 16;

    std::vector<TTF_Text*> pool_;
    size_t poolUsed_ = 0;

    std::unordered_map<FontKey, TTF_Font*, FontKeyHash> fontCache_;

    struct BatchItem {
        std::string content;
        float x, y;
        int ptSize;
        SDL_Color color;
        int fontId;
    };
    std::vector<BatchItem> batch_;

    std::string lastError_;
};

/*
// Init 时指定默认字体
textTool_.Init(app_->toolRenderer(), app_->fontPathStr().c_str(), 20.0f);

// 注册额外字体（可选）
int fontBoldId = textTool_.RegisterFont("assets/fonts/msyhbd.ttf");
int fontMonoId = textTool_.RegisterFont("assets/fonts/consola.ttf");

// 绘制时指定字体
textTool_.BeginBatch();
textTool_.AddText(u8"默认字体标题", 10, 10, 24, {255,255,255,255}, 0);
textTool_.AddText(u8"粗体文本",   10, 50, 16, {255,220,0,255}, fontBoldId);
textTool_.AddText(u8"等宽代码",   10, 75, 14, {0,255,0,255},  fontMonoId);
textTool_.FlushBatch();

fontId 0 固定为默认字体。Init 传入的路径就是 fontPaths_[0]，池对象初始也用这个字体创建。后续 RegisterFont 返回 1、2、3……对应新注册的路径。

缓存 key 是 (fontId, ptSize)。同一个字体文件的不同字号、不同字体文件的同一字号，都会各自 TTF_OpenFont 一次，互不干扰。

RegisterFont 只登记路径，不立即打开。真正 TTF_OpenFont 发生在第一次 GetFont 时（FlushBatch 或 GrowPool）。这样注册多个字体不会拖慢 Init。如果你想提前验证路径有效性，可以在 RegisterFont 里立刻 GetFont(fontId, defaultPtSize_) 一次，失败就回滚 fontPaths_.pop_back() 并返回 -1。

defaultPtSize_ 改成了 int。你原来写的是 int defaultPtSize_ = 16.0f;，用浮点字面量初始化整型成员会有警告，这里顺手修正为 = 16。GetFont 的参数仍是 float，调用处用 static_cast<float> 转换，避免隐式转换警告。

GetTextSize 加了边界检查。你原实现里直接 pool_[id]，如果 id 越界会 UB。现在越界返回 false。


*/