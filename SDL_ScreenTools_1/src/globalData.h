#pragma once

#include "includeIMGUI.hpp"

#include <SDL3/SDL.h>

#include "file/FileFinder.h"

#include <string>
#include <cstdint>

namespace gData {
	//启动
	const int renderTarget_W = 1920;
	const int renderTarget_H = 1080;

	//运行时
	inline bool debugMode = false;

	inline int mainWindowX = 0;
	inline int mainWindowY = 0;
	inline int mainWindowW = 0;
	inline int mainWindowH = 0;

	inline int toolWindowX = 0;
	inline int toolWindowY = 0;
	inline int toolWindowW = 0;
	inline int toolWindowH = 0;

	inline FileFinder toolFontFinder;
	inline FileFinder textureFinder;

	//need save{
	inline bool mainWindowIsShow = true;
	inline bool toolWindowIsShow = true;
	inline bool isFontMode = true;
	inline bool toolFontChangeFlag = true;// 启动后加载1次
	inline bool bkImgChangeFlag = false;
	inline bool timeTextSizeChanged = false;

	inline int toolFontSelected = -1;
	inline int toolFontID = -1;
	inline int bkImgSelected = -1;
	inline int timeTextSize = 40;

	inline SDL_Color timeColor = { 255,255,255,255 };

	inline ImVec2 windowPos = ImVec2(0, 0);
	//}
}