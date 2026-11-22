#pragma once

#include <SDL3/SDL.h>
#include <SDL3_image/SDL_image.h>

class App;

class Gui {
public:
    bool Init(App* app);
    void Shutdown();
    void imguiDraw();
    void sdlDraw();

    ~Gui() {
        Shutdown();
    }

    SDL_Texture* texture = nullptr;

private:
    void ImguiMainWindow();
    void rightChildWindow_0();
    void rightChildWindow_1();
    void rightChildWindow_2();

    App* app_ = nullptr;
};