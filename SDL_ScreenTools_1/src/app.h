#pragma once

#include <windows.h>

#include <SDL3/SDL.h>
#include <SDL3_ttf/SDL_ttf.h>
#include <memory>
#include <string>
#include <filesystem>

class Tray;
class Gui;
class ToolRenderer;

class App {
public:
	App();
	~App();

	bool Mutex();
	bool Init();
	void Run();
	void Shutdown();

	SDL_DisplayID GetDisplayID()const { return displayID; }
	SDL_Rect GetDisplayRect()const { return displayRect; }

	SDL_Window* mainWindow()const { return mainWindow_; }
	SDL_Renderer* mainRenderer()const { return mainRenderer_; }
	SDL_Window* toolWindow()const { return toolSdlWindow_; }
	SDL_Renderer* toolRenderer()const { return toolSdlRenderer_; }

	TTF_Font* sdlFont()const { return sdlFont_; }

	//Title
	std::string* GetMainWindowTitle() { return &mainWindowTitle_; }
	std::string* GetToolWindowTitle() { return &toolWindowTitle_; }
	std::string* GetTrayTitle() { return &trayTitle_; }

	//Path
	std::filesystem::path* GetJsonConfigPath() { return &jsonConfigPath_; }
	std::string* GetFontPath() { return &fontPathStr_; }

	bool  UiVisible() const { return uiVisible_; }
	void  SetUiVisible(bool v);
	void  RequestQuit() { running_ = false; }

private:
	void StartSingleInstanceListener();

	bool InitPath();
	bool InitSdl();
	bool InitWindows();
	bool InitImGui();
	void ProcessEvents();
	void DrawUi();
	void DrawTools();

	HANDLE hMutex;
	std::atomic<bool> pendingWindowShow_{ false };

	SDL_DisplayID displayID;
	SDL_Rect displayRect = {0,0,0,0};

	SDL_Window* mainWindow_ = nullptr;
	SDL_Renderer* mainRenderer_ = nullptr;
	SDL_Window* toolSdlWindow_ = nullptr;
	SDL_Renderer* toolSdlRenderer_ = nullptr;

	TTF_Font* sdlFont_ = nullptr;

	std::unique_ptr<Tray>			tray_;
	std::unique_ptr<Gui>			gui_;
	std::unique_ptr<ToolRenderer>	toolRenderer_;

	bool running_ = true;
	bool uiVisible_ = true;

	std::filesystem::path exePath_;

	//Titles
	std::string mainWindowTitle_ = u8"主窗口";
	std::string toolWindowTitle_ = "toolWindow";
	std::string trayTitle_ = u8"屏幕工具-SDL3";

	//Path
	//std::string basePath_;
	std::filesystem::path imguiIniPath_;
	std::filesystem::path jsonConfigPath_;
	std::filesystem::path logPath_;
	std::filesystem::path fontPath_;

	// utf-8 str
	std::string imguiIniPathStr_;
	std::string fontPathStr_;
};