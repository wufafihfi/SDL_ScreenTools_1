#pragma once

#include <SDL3/SDL.h>

class App;

class Tray {
public:
    bool Init(App* app);
    void Shutdown();

private:
    SDL_Tray* tray_ = nullptr;
};