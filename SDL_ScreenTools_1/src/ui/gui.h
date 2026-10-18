#pragma once

#include <SDL3/SDL.h>

class App;

class Gui {
public:
    bool Init(App* app);
    void imguiDraw();
    void sdlDraw();

private:
    App* app_ = nullptr;
};