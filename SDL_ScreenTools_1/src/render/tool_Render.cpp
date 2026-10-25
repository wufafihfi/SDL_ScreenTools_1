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

    SDL_GetWindowPosition(window, &gData::toolWindowX, &gData::toolWindowY);
    SDL_GetWindowSize(window, &gData::toolWindowW, &gData::toolWindowH);

    return textTool_.Init(app_->toolRenderer(), app_->GetPathStruct()->fontPathStr_.c_str(),20);
}

void ToolRenderer::Shutdown() {
    textTool_.Shutdown();
}

void ToolRenderer::Draw() {
    SDL_Window* window = app_->toolWindow();
    SDL_Renderer* renderer = app_->toolRenderer();

    if (gData::toolFontChangeFlag) {
        gData::toolFontChangeFlag = false;
        gData::toolFontID = textTool_.RegisterFont(
            gData::toolFontFinder.Get(gData::toolFontSelected).fullPath.c_str()
        );
        gData::timeTextSizeChanged = true;
    }

    if(gData::windowPos.x != gData::toolWindowX ||
        gData::windowPos.y != gData::toolWindowY){
        SDL_SetWindowPosition(window,
            gData::windowPos.x - (gData::toolWindowW / 2.0f),
            gData::windowPos.y - (gData::toolWindowH / 2.0f)
        );
    }

    SDL_GetWindowPosition(window, &gData::toolWindowX, &gData::toolWindowY);
    SDL_GetWindowSize(window, &gData::toolWindowW, &gData::toolWindowH);

    if (gData::debugMode) {
        SDL_FRect myRect = { 0, 0, gData::toolWindowW, gData::toolWindowH };
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); // ºìÉ«
        SDL_RenderRect(renderer, &myRect);
    }

    static int TimeW = 0;
    static int TimeH = 0;

    textTool_.BeginBatch();
    textTool_.AddText(GetCurrentTimeString(),
        (gData::toolWindowW - TimeW) / 2.0f,
        (gData::toolWindowH - TimeH) / 2.0f,
        gData::timeTextSize, gData::timeColor,
        gData::toolFontID
    );
    textTool_.FlushBatch();


    SDL_FRect timeRect = { 
        (gData::toolWindowW - TimeW) / 2.0f, (gData::toolWindowH - TimeH) / 2.0f,
        TimeW, TimeH
    };
    if(gData::debugMode){
        SDL_SetRenderDrawColor(renderer, 255, 0, 0, SDL_ALPHA_OPAQUE); // ºìÉ«
        SDL_RenderRect(renderer, &timeRect);
    }

    if ((gData::toolWindowW != timeRect.w + gData::timeTextSize * 3 || gData::toolWindowH != timeRect.h) || gData::timeTextSizeChanged) {
        gData::timeTextSizeChanged = false;
        textTool_.GetTextSize(0, &TimeW, &TimeH);
        SDL_SetWindowSize(window, timeRect.w + gData::timeTextSize * 3, timeRect.h);
    }
}