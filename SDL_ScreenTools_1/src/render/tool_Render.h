#pragma once

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

#include "../sdl3Expand/TTFText_tool.h"

class App;

class ToolRenderer {
public:
    bool Init(App* app);
    void Shutdown();
    void Draw();

private:
    App* app_ = nullptr;

    TTFTextTool textTool_;

    int WindowX = 0;
    int WindowY = 0;
    int WindowW = 0;
    int WindowH = 0;
};