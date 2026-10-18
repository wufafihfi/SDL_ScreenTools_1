#include <SDL3/SDL_main.h>

#include "app.h"

int main(int argc, char* argv[]) {
    App app;
    if (!app.Mutex()) {
        return 1;
    }
    if (!app.Init()) {
        app.Shutdown();
        return 1;
    }
    app.Run();
    app.Shutdown();
    return 0;
}