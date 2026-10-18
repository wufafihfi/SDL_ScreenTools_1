#include "gui.h"

#include "../app.h"
#include "../includeIMGUI.hpp"
#include "../log/LogSystem.h"
#include "../imAnimExpand/imAnimTool.h"
#include "../Save/dataSaver.h"

#include "../globalData.h"

bool Gui::Init(App* app) {
    app_ = app;
    return true;
}

void ImguiMainWindow(App* app);
void rightChildWindow_0(App* app);
void rightChildWindow_1(App* app);
void rightChildWindow_2(App* app);

void Gui::imguiDraw() {
    ImguiMainWindow(app_);
    //ImGui::ShowDemoWindow();
    //static bool showLog = true;
    //SHOW_LOG_WINDOW(&showLog);
}

void ImguiMainWindow(App* app)
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
    switch (current_tab) {
    case 0: rightChildWindow_0(app); break;
    case 1: rightChildWindow_1(app); break;
    case 2: rightChildWindow_2(app); break;
    }
    ImGui::EndChild();

    ImGui::End();
}

inline bool PlaneSelector(const char* id, ImVec2* coord, ImVec2 size);

void rightChildWindow_0(App* app) {
    static int WindowX = 0;
    static int WindowY = 0;
    static int WindowW = 0;
    static int WindowH = 0;
    static SDL_Rect rect = app->GetDisplayRect();
    SDL_Window* toolWindow = app->toolWindow();

    SDL_GetWindowPosition(toolWindow, &WindowX, &WindowY);
    SDL_GetWindowSize(toolWindow, &WindowW, &WindowH);

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

    static int timeTextSize = gData::timeTextSize;
    if (ImGui::SliderInt(u8"字体大小", &timeTextSize, 5, 250)) {
        gData::timeTextSize = timeTextSize;
        SaveConfig(app);
    }

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
}
void rightChildWindow_1(App* app) {

}
void rightChildWindow_2(App* app) {

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

    ImDrawList* draw = ImGui::GetWindowDrawList();

    // 从样式表取色（自动应用 style.Alpha）
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
    SDL_Renderer* renderer_ = app_->mainRenderer();

    SDL_SetRenderDrawBlendMode(renderer_, SDL_BLENDMODE_BLEND);

    // 拿到窗口尺寸，让图形居中
    int w = 0, h = 0;
    SDL_GetCurrentRenderOutputSize(renderer_, &w, &h);
    float cx = w * 0.5f;
    float cy = h * 0.5f;

    // 用系统时间驱动动画
    Uint64 now = SDL_GetTicks();
    float t = now / 1000.0f;              // 秒
    float angle = t * 1.5f;               // 旋转速度
    float pulse = 0.5f + 0.5f * SDL_sinf(t * 2.0f);  // 0~1 呼吸
    float radius = 60.0f + pulse * 40.0f;  // 60~100 像素

    // 旋转的三角形，三个顶点颜色不同
    SDL_Vertex tri[3];
    SDL_Color colors[3] = {
        { 255, 80, 80, 180 },   // 红
        { 80, 255, 80, 180 },   // 绿
        { 80, 80, 255, 180 },   // 蓝
    };

    for (int i = 0; i < 3; ++i) {
        float a = angle + i * (2.0f * SDL_PI_F / 3.0f);
        tri[i].position.x = cx + SDL_cosf(a) * radius;
        tri[i].position.y = cy + SDL_sinf(a) * radius;
        tri[i].color.r = colors[i].r / 255.0f;
        tri[i].color.g = colors[i].g / 255.0f;
        tri[i].color.b = colors[i].b / 255.0f;
        tri[i].color.a = colors[i].a / 255.0f;
        tri[i].tex_coord = { 0.0f, 0.0f };
    }
    SDL_RenderGeometry(renderer_, NULL, tri, 3, NULL, 0);

    // 中心一个半透明圆（用多个小矩形近似，或者用三角形扇）
    // 这里用 8 个三角形拼一个圆
    const int seg = 16;
    SDL_Vertex circle[seg * 3];
    float circleR = 30.0f + pulse * 10.0f;
    for (int i = 0; i < seg; ++i) {
        float a0 = (i / (float)seg) * 2.0f * SDL_PI_F;
        float a1 = ((i + 1) / (float)seg) * 2.0f * SDL_PI_F;

        circle[i * 3 + 0].position = { cx, cy };
        circle[i * 3 + 0].color = { 1.0f, 1.0f, 1.0f, 0.6f };
        circle[i * 3 + 0].tex_coord = { 0.0f, 0.0f };

        circle[i * 3 + 1].position = { cx + SDL_cosf(a0) * circleR, cy + SDL_sinf(a0) * circleR };
        circle[i * 3 + 1].color = { 1.0f, 1.0f, 0.0f, 0.4f };
        circle[i * 3 + 1].tex_coord = { 0.0f, 0.0f };

        circle[i * 3 + 2].position = { cx + SDL_cosf(a1) * circleR, cy + SDL_sinf(a1) * circleR };
        circle[i * 3 + 2].color = { 0.0f, 1.0f, 1.0f, 0.4f };
        circle[i * 3 + 2].tex_coord = { 0.0f, 0.0f };
    }
    SDL_RenderGeometry(renderer_, NULL, circle, seg * 3, NULL, 0);
}