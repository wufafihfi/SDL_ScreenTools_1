#include "gui.h"

#include "../app.h"
#include "../includeIMGUI.hpp"
#include "../log/LogSystem.h"
#include "../imguiExpand/imguiTool.h"
#include "../file/dataSaver.h"
#include "../file/FileFinder.h"

#include "../globalData.h"

#include <algorithm>

bool Gui::Init(App* app) {
    app_ = app;

    SDL_Window* window = app_->mainWindow();
    SDL_Renderer* renderer = app_->mainRenderer();

    SDL_GetWindowPosition(window, &gData::mainWindowX, &gData::mainWindowY);
    SDL_GetWindowSize(window, &gData::mainWindowW, &gData::mainWindowH);

    gData::toolFontFinder.SetRootPath(app_->GetPathStruct()->fontDictionaryPath_.u8string());
    gData::toolFontFinder.SetFontMode(gData::isFontMode);
    gData::toolFontFinder.SetRecursive(false);
    gData::toolFontFinder.SetCaseSensitive(true);
    gData::toolFontFinder.Rescan();

    gData::textureFinder.SetRootPath(app_->GetPathStruct()->imagePathStr_);
    gData::textureFinder.SetExtensions({ ".png", ".jpg" });
    gData::textureFinder.SetRecursive(false);
    gData::textureFinder.SetCaseSensitive(true);
    gData::textureFinder.Rescan();

    texture = IMG_LoadTexture(app->mainRenderer(),
        gData::textureFinder.Get(gData::bkImgSelected).fullPath.c_str()
    );
    if (!texture) {
        LOG_ERROR_STREAM << u8"图片加载失败: " << SDL_GetError();
        SDL_Log(u8"图片加载失败: %s", SDL_GetError());
    }

    return true;
}

void Gui::Shutdown() {
    SDL_DestroyTexture(texture);
}

void ImguiMainWindow(App* app, Gui* self);
void rightChildWindow_0(App* app, Gui* self);
void rightChildWindow_1(App* app, Gui* self);
void rightChildWindow_2(App* app, Gui* self);

void Gui::imguiDraw() {
    ImguiMainWindow(app_,this);
    //ImGui::ShowDemoWindow();
    //static bool showLog = true;
    SHOW_LOG_WINDOW(&gData::debugMode);
}

bool pageChangeAnim(int direction);

void ImguiMainWindow(App* app, Gui* self)
{
    ImGuiViewport* vp = ImGui::GetMainViewport();
    ImGui::SetNextWindowPos(vp->Pos);
    ImGui::SetNextWindowSize(vp->Size);

    ImGuiWindowFlags flags =
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoResize |
        ImGuiWindowFlags_NoMove |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoSavedSettings;

    ImGui::Begin(u8"主窗口", nullptr, flags);

    static int current_tab = 0;
    const char* tab_names[] = { u8"功能", u8"设置", u8"关于" };

    ImGui::BeginChild("LeftTabs", ImVec2(120, 0), true);

    static bool first_frame = true;
    static const ImVec4 btnColor = ImGui::GetStyleColorVec4(ImGuiCol_Button);
    static ImVec2 highlight_pos = ImVec2(0,0);
    static ImVec2 target_pos = ImVec2(ImGui::GetCursorPosX(), 20);
    static ImVec2 size = ImVec2(120, 40);

    highlight_pos = TweenVec2("LeftTabs", "highlight", target_pos, 0.2f);

    ImDrawList* draw = ImGui::GetWindowDrawList();
    draw->AddRectFilled(
        highlight_pos,
        ImVec2(highlight_pos.x + size.x, highlight_pos.y + size.y),
        ImGui::GetColorU32(btnColor),
        4.0f
    );

    ImGui::Dummy(ImVec2(0, 2));
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 20));
    ImGui::PushStyleVar(ImGuiStyleVar_SelectableRounding, 4.0f);
    for (int i = 0; i < 3; i++) {
        ImGui::PushID(i);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0, 0, 0, 0));
        ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(1, 1, 1, 0.1f));
        ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(1, 1, 1, 0.15f));

        if (ImGui::Selectable(tab_names[i], current_tab == i)) {
            current_tab = i;
            target_pos = ImGui::GetItemRectMin();
        }
        if (first_frame) {
            first_frame = false;
            target_pos = ImGui::GetItemRectMin();
            ImVec2 itemMax = ImGui::GetItemRectMax();
            size = ImVec2(itemMax.x - target_pos.x, itemMax.y - target_pos.y);
        }

        ImGui::PopStyleColor(3);
        ImGui::PopID();
        ImGui::Dummy(ImVec2(0, 10));
    }
    ImGui::PopStyleVar(2);
    ImGui::EndChild();

    ImGui::SameLine();

    ImGui::BeginChild("Content", ImVec2(0, 0), true);
    static int last_tab = current_tab;
    switch (current_tab) {
    case 0: rightChildWindow_0(app, self); break;
    case 1: rightChildWindow_1(app, self); break;
    case 2: rightChildWindow_2(app, self); break;
    }
    ImGui::EndChild();

    ImGui::End();
}

inline bool PlaneSelector(const char* id, ImVec2* coord, ImVec2 size);

void rightChildWindow_0(App* app, Gui* self) {
    static int WindowX = gData::toolWindowX;
    static int WindowY = gData::toolWindowY;
    static int WindowW = gData::toolWindowW;
    static int WindowH = gData::toolWindowH;
    static SDL_Rect rect = app->GetDisplayRect();

    //位置
    static ImVec2 planeSize = ImVec2(400, 250);
    static ImVec2 pos = ImVec2(
        gData::windowPos.x / rect.w,
        gData::windowPos.y / rect.h
    );
    bool posChanged = false;
    ImGui::Text(u8"时钟显示位置");
    if (PlaneSelector("##plane1", &pos, planeSize)) {
        posChanged = true;
    }
    gData::windowPos = ImVec2(
        pos.x * rect.w,
        pos.y * rect.h
    );
    if (posChanged) {
        SaveConfig(app);
    }
    ImGui::Text(u8"X:%0.1f Y:%0.1f", pos.x, pos.y);

    //大小
    static int timeTextSize = gData::timeTextSize;
    if (ImGui::SliderInt(u8"字体大小", &timeTextSize, 5, 250)) {
        gData::timeTextSizeChanged = true;
        gData::timeTextSize = timeTextSize;
        SaveConfig(app);
    }

    //颜色
    static float color[4] = { 
        gData::timeColor.r / 255.0f, 
        gData::timeColor.g / 255.0f,
        gData::timeColor.b / 255.0f,
        gData::timeColor.a / 255.0f
    };
    bool colorChanged = false;
    if (ImGui::ColorEdit3(u8"时钟颜色", color, ImGuiColorEditFlags_Float)) {
        colorChanged = true;
    }
    ImGui::TextDisabled(u8"(点击色块进入拾色器)");
    float alpha_pct = color[3] * 100.0f;
    if (ImGui::SliderFloat(u8"透明度", &alpha_pct, 0.0f, 100.0f, "%.0f%%")) {
        color[3] = alpha_pct / 100.0f;
        colorChanged = true;
    }
    gData::timeColor.r = (Uint8)(color[0] * 255.0f + 0.5f);
    gData::timeColor.g = (Uint8)(color[1] * 255.0f + 0.5f);
    gData::timeColor.b = (Uint8)(color[2] * 255.0f + 0.5f);
    gData::timeColor.a = (Uint8)(color[3] * 255.0f + 0.5f);
    if (colorChanged) {
        SaveConfig(app);
    }

    //选择文件
    //字体
    const char* toolFontPreview = (gData::toolFontSelected >= 0 && gData::toolFontSelected < gData::toolFontFinder.GetNamesCount())
        ? gData::toolFontFinder.Get(gData::toolFontSelected).fileName.c_str()
        : u8"<选择文件>";

    if (ImGui::BeginCombo(u8"时间字体选择", toolFontPreview)) {
        for (int i = 0; i < gData::toolFontFinder.GetNamesCount(); ++i) {
            const bool isSelected = (gData::toolFontSelected == i);
            if (ImGui::Selectable(gData::toolFontFinder.Get(i).fileName.c_str(), isSelected)) {
                gData::toolFontSelected = i;
                gData::toolFontChangeFlag = true;
                SaveConfig(app);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", gData::toolFontFinder.Get(i).fullPath.c_str());
            }
        }
        ImGui::EndCombo();
    }

    //背景
    const char* bkImgPreview = (gData::bkImgSelected >= 0 && gData::bkImgSelected < gData::textureFinder.GetNamesCount())
        ? gData::textureFinder.Get(gData::bkImgSelected).fileName.c_str()
        : u8"<选择文件>";

    if (ImGui::BeginCombo(u8"背景图选择", bkImgPreview)) {
        for (int i = 0; i < gData::textureFinder.GetNamesCount(); ++i) {
            const bool isSelected = (gData::bkImgSelected == i);
            if (ImGui::Selectable(gData::textureFinder.Get(i).fileName.c_str(), isSelected)) {
                gData::bkImgSelected = i;
                self->texture = IMG_LoadTexture(app->mainRenderer(), 
                    gData::textureFinder.Get(i).fullPath.c_str()
                );
                if (!self->texture) {
                    LOG_ERROR_STREAM << u8"图片加载失败: " << SDL_GetError();
                    SDL_Log(u8"图片加载失败: %s", SDL_GetError());
                }
                SaveConfig(app);
            }
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", gData::textureFinder.Get(i).fullPath.c_str());
            }
        }
        ImGui::EndCombo();
    }

    //调试
    MyToggle(u8"调试模式", &gData::debugMode);
}
void rightChildWindow_1(App* app, Gui* self) {
    if (MyToggle(u8"启动时扫描系统字体(关闭后所选的系统字体将无法加载)", &gData::isFontMode)) {
        SaveConfig(app);
    }
}
void rightChildWindow_2(App* app, Gui* self) {
    ImGui::Text(u8"版本:v0.2-alpha");
}

//切换动画
bool pageChangeAnim(int direction) {
    ImVec2 window_pos = ImGui::GetWindowPos();
    ImVec2 window_size = ImGui::GetWindowSize();

    // 2. 获取样式中的 Padding
    float padding_x = ImGui::GetStyle().WindowPadding.x;
    float padding_y = ImGui::GetStyle().WindowPadding.y;

    ImVec2 child_min_pos = ImVec2(0, window_pos.y + padding_y);
    ImVec2 contentSize = ImVec2(0, window_pos.y + window_size.y - padding_y);

    if (contentSize.x <= 0 || contentSize.y <= 0) return false;

    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    const char* owner = "PageTransition";
    const char* channel = "FillHeight";

    float targetHeight = 0.0f;
    if (direction > 0) {
        targetHeight = contentSize.y;
    }
    else if (direction < 0) {
        targetHeight = 0.0f;
    }
    else {
        return true;
    }

    float currentHeight = TweenF(owner, channel, targetHeight, 0.2f, iam_ease_out_cubic);
    
    ImVec2 rect_min = ImVec2(child_min_pos.x, child_min_pos.y);
    ImVec2 rect_max = ImVec2(child_min_pos.x + contentSize.x, child_min_pos.y + currentHeight);

    draw_list->AddRectFilled(
        rect_min,
        rect_max,
        ImGui::GetColorU32(ImGuiCol_Button), 
        4.0f
    );

    bool isComplete = false;
    static int last_dir = direction;
    if (direction > 0) {
        isComplete = (currentHeight >= contentSize.y - 0.1f);
    }
    else {
        isComplete = (currentHeight <= 0.1f);
    }
    if (!isComplete  && direction != last_dir) {
        isComplete = true;
    }
    if (isComplete) {
        direction = last_dir;
    }

    return isComplete;
}

inline bool PlaneSelector(const char* id, ImVec2* coord, ImVec2 size = ImVec2(120, 120))
{
    ImGui::PushID(id);

    ImVec2 p = ImGui::GetCursorScreenPos();
    ImGui::InvisibleButton("##plane", size);
    bool hovered = ImGui::IsItemHovered();
    bool active = ImGui::IsItemActive();

    bool changed = false;

    const float snap_threshold = 0.06f;
    const float snap_target = 0.5f;

    if (active && ImGui::IsMouseDown(ImGuiMouseButton_Left))
    {
        ImVec2 mouse = ImGui::GetIO().MousePos;
        float nx = (mouse.x - p.x) / size.x;
        float ny = (mouse.y - p.y) / size.y;
        if (ImFabs(nx - snap_target) < snap_threshold) nx = snap_target;
        if (ImFabs(ny - snap_target) < snap_threshold) ny = snap_target;
        coord->x = ImClamp(nx, 0.0f, 1.0f);
        coord->y = ImClamp(ny, 0.0f, 1.0f);
        changed = true;
    }

    coord->x = std::clamp(coord->x, 0.0f, 1.0f);
    coord->y = std::clamp(coord->y, 0.0f, 1.0f);

    ImDrawList* draw = ImGui::GetWindowDrawList();

    ImU32 col_bg = ImGui::GetColorU32(ImGuiCol_FrameBg);
    ImU32 col_border = ImGui::GetColorU32(ImGuiCol_Border);
    ImU32 col_hover = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
    ImU32 col_active = ImGui::GetColorU32(ImGuiCol_ButtonActive);
    ImU32 col_accent = ImGui::GetColorU32(ImGuiCol_CheckMark);
    ImU32 col_text = ImGui::GetColorU32(ImGuiCol_Text);

    // 网格线：用边框色，降低 alpha
    ImU32 col_grid = ImGui::GetColorU32(ImGuiCol_Border, 0.35f);

    // 背景
    draw->AddRectFilled(p, ImVec2(p.x + size.x, p.y + size.y), col_bg, 4.0f);

    // 网格线（4x4）
    for (int i = 1; i < 4; i++)
    {
        float t = i / 4.0f;
        draw->AddLine(ImVec2(p.x + size.x * t, p.y),
            ImVec2(p.x + size.x * t, p.y + size.y), col_grid);
        draw->AddLine(ImVec2(p.x, p.y + size.y * t),
            ImVec2(p.x + size.x, p.y + size.y * t), col_grid);
    }

    // 中段吸附高亮
    bool snap_x = ImFabs(coord->x - snap_target) < 0.001f;
    bool snap_y = ImFabs(coord->y - snap_target) < 0.001f;
    ImU32 snap_col = ImGui::GetColorU32(ImGuiCol_CheckMark, 0.6f);
    if (snap_x)
        draw->AddLine(ImVec2(p.x + size.x * 0.5f, p.y),
            ImVec2(p.x + size.x * 0.5f, p.y + size.y), snap_col, 2.0f);
    if (snap_y)
        draw->AddLine(ImVec2(p.x, p.y + size.y * 0.5f),
            ImVec2(p.x + size.x, p.y + size.y * 0.5f), snap_col, 2.0f);

    // 边框
    ImU32 border_col = active ? col_active : hovered ? col_hover : col_border;
    draw->AddRect(p, ImVec2(p.x + size.x, p.y + size.y), border_col, 4.0f, 0, active ? 2.0f : 1.0f);

    // 十字准线
    ImVec2 cursor_pos = ImVec2(p.x + coord->x * size.x, p.y + coord->y * size.y);
    ImU32 cross_col = ImGui::GetColorU32(ImGuiCol_CheckMark, 0.8f);
    draw->AddLine(ImVec2(cursor_pos.x, p.y), ImVec2(cursor_pos.x, p.y + size.y), cross_col);
    draw->AddLine(ImVec2(p.x, cursor_pos.y), ImVec2(p.x + size.x, cursor_pos.y), cross_col);

    // 当前点
    draw->AddCircleFilled(cursor_pos, 5.0f, col_accent);
    draw->AddCircle(cursor_pos, 5.0f, col_text, 0, 1.5f);

    // 数值显示
    //char buf[64];
    //snprintf(buf, sizeof(buf), "X: %.2f  Y: %.2f", coord->x, coord->y);
    //ImGui::SetCursorScreenPos(ImVec2(p.x, p.y + size.y + 4));
    //ImGui::TextDisabled("%s", buf);

    ImGui::PopID();
    return changed;
}

void Gui::sdlDraw() {
    SDL_Window* window = app_->mainWindow();
    SDL_Renderer* renderer = app_->mainRenderer();

    SDL_GetWindowPosition(window, &gData::mainWindowX, &gData::mainWindowY);
    SDL_GetWindowSize(window, &gData::mainWindowW, &gData::mainWindowH);

    SDL_FRect dst = { 0, 0, gData::renderTarget_W, gData::renderTarget_H };
    SDL_RenderTexture(renderer, texture, NULL, &dst);  // NULL 表示整张纹理
}