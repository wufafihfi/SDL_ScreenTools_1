#include "app.h"
#include "core/tray.h"
#include "ui/gui.h"
#include "render/tool_Render.h"
#include "log/LogSystem.h"
#include "file/dataSaver.h"
#include "globalData.h"

#include "includeIMGUI.hpp"
#include "ImguiStyleAndTheme.h"

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>

App::App() = default;
App::~App() = default;

bool App::Mutex() {
    hMutex = CreateMutex(NULL, TRUE,
        L"screenTools-SDL3-912ef77d9b981a76");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        if (hMutex)
        {
            HANDLE hPipe = CreateFileW(
                L"\\\\.\\pipe\\ScreenTools_SDL3_912ef77d9b981a76_SingleInstance",
                GENERIC_WRITE, 0, nullptr, OPEN_EXISTING, 0, nullptr);
            if (hPipe != INVALID_HANDLE_VALUE) {
                DWORD written;
                WriteFile(hPipe, "1", 1, &written, nullptr);
                CloseHandle(hPipe);
            }
            return false;
        }
    }
    return true;
}

bool App::Init() {
    //参数赋值
    renderTarget_W = gData::renderTarget_W;
    renderTarget_H = gData::renderTarget_H;

    if (!InitPath())    return false;

    LogSystem::getInstance().init(path_.logPath_, LogLevel::DEBUG, true, true, 1000);
    LogSystem::getInstance().setWindowTitle(u8"日志");

    LoadConfig(this);

    /*
    char* prefPath = SDL_GetPrefPath("wufa_app", "ScreenTool_SDL3");
    if (!prefPath) {
        SDL_Log("Failed to get pref path: %s", SDL_GetError());
        LOG_ERROR_STREAM << "Failed to get pref path: %s: " << SDL_GetError();
        return false;
    }
    basePath_ = prefPath;
    SDL_free(prefPath);

    SDL_CreateDirectory((basePath_ + "config").c_str());
    SDL_CreateDirectory((basePath_ + "logs").c_str());
    */

    LOG_INFO_STREAM << "InitSdl...";
    if (!InitSdl())     return false;
    LOG_INFO_STREAM << "InitWindows...";
    if (!InitWindows()) return false;
    LOG_INFO_STREAM << "InitImGui...";
    if (!InitImGui())   return false;

    LOG_INFO_STREAM << "InitMode...";
    tray_ = std::make_unique<Tray>();
    if (!tray_->Init(this)) {
        SDL_Log("Tray init failed");
        LOG_ERROR_STREAM << "Tray init failed";
        return false;
    }
    gui_ = std::make_unique<Gui>();
    if (!gui_->Init(this)) {
        SDL_Log("gui_ init failed");
        LOG_ERROR_STREAM << "gui_ init failed";
        return false;
    }

    toolRenderer_ = std::make_unique<ToolRenderer>();
    toolRenderer_->Init(this);

    StartSingleInstanceListener();

    LOG_INFO_STREAM << "App::Init OK!";
    return true;
}

//命名管道实现自动显示主窗口
void App::StartSingleInstanceListener() {
    std::thread([this]() {
        while (running_) {
            HANDLE hPipe = CreateNamedPipeW(
                L"\\\\.\\pipe\\ScreenTools_SDL3_912ef77d9b981a76_SingleInstance",
                PIPE_ACCESS_INBOUND,
                PIPE_TYPE_MESSAGE | PIPE_READMODE_MESSAGE | PIPE_WAIT,
                1, 0, 0, 0, nullptr);
            if (hPipe == INVALID_HANDLE_VALUE) return;

            BOOL connected = ConnectNamedPipe(hPipe, nullptr) ||
                (GetLastError() == ERROR_PIPE_CONNECTED);

            if (connected && running_) {
                char buf[1];
                DWORD read = 0;
                if (ReadFile(hPipe, buf, 1, &read, nullptr)) {
                    pendingWindowShow_ = true;
                }
            }

            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
        }).detach();
}

bool App::InitPath() {
    const char* exePath = SDL_GetBasePath();
    if (!exePath) {
        SDL_Log("Failed to get exefile path: %s", SDL_GetError());
        LOG_ERROR_STREAM << "Failed to get exefile path: " << SDL_GetError();
        return false;
    }
    exePath_ = std::filesystem::u8path(exePath);
    SDL_free((void*)exePath);

    path_.imagePath_ = exePath_ / "assats" / "image";
    path_.imguiIniPath_ = exePath_ / "config" / "imgui_state.ini";
    path_.jsonConfigPath_ = exePath_ / "config" / "config.json";
    path_.fontPath_ = exePath_ / "font" / "SimHei.ttf";
    path_.fontDictionaryPath_ = exePath_ / "font";
    path_.logPath_ = exePath_ / "log" / "MainLog.log";

    path_.imagePathStr_ = path_.imagePath_.u8string();
    path_.imguiIniPathStr_ = path_.imguiIniPath_.u8string();
    path_.fontPathStr_ = path_.fontPath_.u8string();

    return true;
}

bool App::InitSdl() {
    if (!SDL_Init(SDL_INIT_VIDEO)) {
        SDL_Log("SDL_Init failed: %s", SDL_GetError());
        LOG_ERROR_STREAM << "SDL_Init failed: " << SDL_GetError();
        return false;
    }
    if (!TTF_Init())  {
        printf("Failed to initialize SDL_ttf: %s\n", SDL_GetError());
        return false;
    }
    const char* fontPath = path_.fontPathStr_.c_str();
    sdlFont_ = TTF_OpenFont(fontPath, 18);
    if (!sdlFont_) {
        SDL_Log("TTF_OpenFont failed: %s", SDL_GetError());
        return false;
    }

    return true;
}

bool App::InitWindows() {
    displayID = SDL_GetPrimaryDisplay();
    SDL_GetDisplayBounds(displayID, &displayRect);

    //工具窗口
    toolSdlWindow_ = SDL_CreateWindow(toolWindowTitle_.c_str(), 500, 100,
        SDL_WINDOW_BORDERLESS | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_TRANSPARENT);
    if (!toolSdlWindow_) return false;

    HWND hwnd = (HWND)SDL_GetPointerProperty(SDL_GetWindowProperties(toolSdlWindow_),
        SDL_PROP_WINDOW_WIN32_HWND_POINTER, NULL);

    LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
    SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle | WS_EX_LAYERED | WS_EX_TRANSPARENT | WS_EX_TOOLWINDOW);

    //默认d3d
    toolSdlRenderer_ = SDL_CreateRenderer(toolSdlWindow_, nullptr);
    if (!toolSdlRenderer_) return false;

    SDL_SetRenderVSync(toolSdlRenderer_, 1);

    //////////////////////////////////////////////////////
    
    //主窗口
    mainWindow_ = SDL_CreateWindow(mainWindowTitle_.c_str(), 800, 600, SDL_WINDOW_RESIZABLE);
    if (!mainWindow_) return false;

    //OPENGL
    mainRenderer_ = SDL_CreateRenderer(mainWindow_, "opengl");
    if (!mainRenderer_) {
        // 退路
        mainRenderer_ = SDL_CreateRenderer(mainWindow_, nullptr);
        if (!mainRenderer_) return false;
    }

    renderTarget = SDL_CreateTexture(
        mainRenderer_,
        SDL_PIXELFORMAT_ARGB8888,
        SDL_TEXTUREACCESS_TARGET,
        800, 600);

    SDL_SetRenderVSync(mainRenderer_, 1);
    return true;
}

bool App::InitImGui() {
    IMGUI_CHECKVERSION();
    if (!ImGui::CreateContext()) {
        SDL_Log("ImGui context creation failed");
        return false;
    }
    ImGui::StyleColorsDark();

    ImGuiIO& io = ImGui::GetIO();
    if (!path_.imguiIniPath_.empty()) {
        io.IniFilename = path_.imguiIniPathStr_.c_str();
    }
    if (!path_.fontPath_.empty()) {
        io.Fonts->AddFontFromFileTTF(path_.fontPath_.u8string().c_str(), 18.0f,
            nullptr, io.Fonts->GetGlyphRangesChineseSimplifiedCommon());
    }
    // 触摸屏兼容
    io.ConfigFlags |= ImGuiConfigFlags_IsTouchScreen;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
    io.MouseDragThreshold = 4.0f;

    SetStyle();
    SetColor();

    if (!ImGui_ImplSDL3_InitForSDLRenderer(mainWindow_, mainRenderer_)) {
        SDL_Log("ImGui SDL3 backend init failed");
        return false;
    }
    if (!ImGui_ImplSDLRenderer3_Init(mainRenderer_)) {
        SDL_Log("ImGui SDL_Renderer3 backend init failed");
        return false;
    }
    return true;
}

// 主循环
void App::Run() {
    while (running_) {
        if (pendingWindowShow_.exchange(false)) {
            SetUiVisible(true);
        }

        Uint64 frameStart = SDL_GetTicks();
        ProcessEvents();

        if (uiVisible_) DrawUi();
        DrawTools();

        //60 FPS - 30 FPS
        Uint64 target = uiVisible_ ? 16 : 33;
        Uint64 elapsed = SDL_GetTicks() - frameStart;
        if (elapsed < target) {
            SDL_Delay(target - elapsed);
        }
    }
}

void App::ProcessEvents() {
    SDL_Event event;
    while (SDL_PollEvent(&event)) {
        ImGui_ImplSDL3_ProcessEvent(&event);

        if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED) {
            int w, h;
            SDL_GetWindowSizeInPixels(mainWindow_, &w, &h);
            SDL_DestroyTexture(renderTarget);
            renderTarget = SDL_CreateTexture(mainRenderer_,
                SDL_PIXELFORMAT_ARGB8888, SDL_TEXTUREACCESS_TARGET, w, h);
        }

        if (event.type == SDL_EVENT_WINDOW_CLOSE_REQUESTED) {
            SDL_WindowID wid = event.window.windowID;
            if (wid == SDL_GetWindowID(mainWindow_)) {
                SetUiVisible(false);
            }
            else if (wid == SDL_GetWindowID(toolSdlWindow_)) {
                running_ = false;
            }
        }

        if (event.type == SDL_EVENT_QUIT) {
            running_ = false;
        }
    }
}

void App::DrawUi() {
    //SDL
    SDL_SetRenderTarget(mainRenderer_, renderTarget);
    SDL_SetRenderLogicalPresentation(mainRenderer_,
       renderTarget_W, renderTarget_H,
        SDL_LOGICAL_PRESENTATION_OVERSCAN
    );

    SDL_SetRenderDrawColor(mainRenderer_, 30, 30, 30, 255);
    SDL_RenderClear(mainRenderer_);

    gui_->sdlDraw();

    //change
    SDL_SetRenderTarget(mainRenderer_, NULL);
    SDL_SetRenderLogicalPresentation(mainRenderer_, 0, 0,
        SDL_LOGICAL_PRESENTATION_DISABLED);

    SDL_RenderTexture(mainRenderer_, renderTarget, NULL, NULL);

    //IMGUI
    ImGui_ImplSDLRenderer3_NewFrame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    iam_update_begin_frame();
    iam_clip_update(ImGui::GetIO().DeltaTime);

    gui_->imguiDraw();

    ImGui::Render();
    ImGui_ImplSDLRenderer3_RenderDrawData(ImGui::GetDrawData(), mainRenderer_);
    //all
    SDL_RenderPresent(mainRenderer_);
}

void App::DrawTools() {
    SDL_SetRenderDrawColor(toolSdlRenderer_, 0, 0, 0, 0);
    SDL_RenderClear(toolSdlRenderer_);

    toolRenderer_->Draw();

    SDL_RenderPresent(toolSdlRenderer_);
}

void App::Shutdown() {
    toolRenderer_.reset();
    gui_.reset();
    tray_.reset();

    ImGui_ImplSDLRenderer3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();

    if (toolSdlRenderer_) SDL_DestroyRenderer(toolSdlRenderer_);
    if (toolSdlWindow_)   SDL_DestroyWindow(toolSdlWindow_);
    if (mainRenderer_)  SDL_DestroyRenderer(mainRenderer_);
    if (mainWindow_)    SDL_DestroyWindow(mainWindow_);

    if (sdlFont_) {
        TTF_CloseFont(sdlFont_);
        sdlFont_ = nullptr;
    }
    TTF_Quit();

    SDL_Quit();
}

void App::SetUiVisible(bool v) {
    if(v){
        SDL_ShowWindow(mainWindow_);
        SDL_RestoreWindow(mainWindow_);
        //SDL_RaiseWindow(mainWindow_);//提升Z顺序
        uiVisible_ = true;
    }
    else
    {
        SDL_HideWindow(mainWindow_);
        uiVisible_ = false;
    }
}