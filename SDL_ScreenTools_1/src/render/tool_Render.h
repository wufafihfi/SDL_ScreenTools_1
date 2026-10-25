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

    ~ToolRenderer() {
        Shutdown();
    }

private:
    App* app_ = nullptr;

    TTFTextTool textTool_;
};