#include "tray.h"

#include "../app.h"
#include "../log/LogSystem.h"

//图片
#include "../../assets/icon/icon_data.h"

void trayShowCallback(void* userdata, SDL_TrayEntry* entry) {
    App* app = static_cast<App*>(userdata);
    app->SetUiVisible(true);
}

void trayQuitCallback(void* userdata, SDL_TrayEntry* entry) {
    SDL_Event e;
    e.type = SDL_EVENT_QUIT;
    SDL_PushEvent(&e);
}

void resetToolWindowCallback(void* userdata, SDL_TrayEntry* entry) {
    App* app = static_cast<App*>(userdata);
    app->ResetWindowState();
}

bool Tray::Init(App* app) {
    SDL_IOStream* io = SDL_IOFromMem(const_cast<unsigned char*>(icon_bmp), icon_bmp_len);
    SDL_Surface* icon = SDL_LoadBMP_IO(io, true);
    tray_ = SDL_CreateTray(icon, app->GetTrayTitle()->c_str());
    SDL_TrayMenu* menu = SDL_CreateTrayMenu(tray_);

    SDL_TrayEntry* showEntry = SDL_InsertTrayEntryAt(menu, -1, u8"显示窗口", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(showEntry, trayShowCallback, app);

    SDL_TrayEntry* resetToolWindow = SDL_InsertTrayEntryAt(menu, -1, u8"刷新工具窗口", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(resetToolWindow, resetToolWindowCallback, app);

    SDL_TrayEntry* quitEntry = SDL_InsertTrayEntryAt(menu, -1, u8"退出", SDL_TRAYENTRY_BUTTON);
    SDL_SetTrayEntryCallback(quitEntry, trayQuitCallback, NULL);

    return true;
}

void Tray::Shutdown() {
    if (tray_) {
        SDL_DestroyTray(tray_);
        tray_ = nullptr;
    }
}