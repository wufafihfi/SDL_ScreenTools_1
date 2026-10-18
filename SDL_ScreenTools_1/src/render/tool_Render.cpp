#include "tool_Render.h"

#include "../app.h"

#include "../globalData.h"

#include <chrono>
#include <ctime>
#include <string>
#include <cstdio>

std::string GetCurrentTimeString() {
    auto now = std::chrono::system_clock::now();
    std::time_t t = std::chrono::system_clock::to_time_t(now);

    std::tm tm{};
    localtime_s(&tm, &t);// Windows

    char buf[32];
    std::strftime(buf, sizeof(buf), "%Y-%m-%d %H:%M:%S", &tm);
    return std::string(buf);
}

bool ToolRenderer::Init(App* app) {
    app_ = app;

    SDL_Window* window = app_->toolWindow();
    SDL_Renderer* renderer = app_->toolRenderer();

    SDL_GetWindowPosition(window, &WindowX, &WindowY);
    SDL_GetWindowSize(window, &WindowW, &WindowH);

    return textTool_.Init(app_->toolRenderer(), app_->GetFontPath()->c_str(),20);
}

void ToolRenderer::Shutdown() {
    textTool_.Shutdown();
}

void ToolRenderer::Draw() {
    SDL_Window* window = app_->toolWindow();
    SDL_Renderer* renderer = app_->toolRenderer();

    if(gData::windowPos.x != WindowX ||
        gData::windowPos.y != WindowY){
        SDL_SetWindowPosition(window,
            gData::windowPos.x - (WindowW / 2.0f),
            gData::windowPos.y - (WindowH / 2.0f)
        );
    }

    SDL_GetWindowPosition(window, &WindowX, &WindowY);
    SDL_GetWindowSize(window, &WindowW, &WindowH);

    //SDL_FRect myRect = { 0, 0, WindowW, WindowH };
    //SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); // ºìÉ«
    //SDL_RenderRect(renderer, &myRect);

    static int TimeW = 0;
    static int TimeH = 0;

    textTool_.BeginBatch();
    textTool_.AddText(GetCurrentTimeString(),
        (WindowW - TimeW) / 2.0f,
        (WindowH - TimeH) / 2.0f,
        gData::timeTextSize, gData::timeColor
    );
    textTool_.FlushBatch();

    textTool_.GetTextSize(0, &TimeW, &TimeH);

    SDL_FRect timeRect = { 
        (WindowW - TimeW) / 2.0f, (WindowH - TimeH) / 2.0f,
        TimeW, TimeH
    };
    //SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); // ºìÉ«
    //SDL_RenderRect(renderer, &timeRect);

    if (WindowW != timeRect.w || WindowH != timeRect.h) {
        SDL_SetWindowSize(window, timeRect.w, timeRect.h);
    }
}