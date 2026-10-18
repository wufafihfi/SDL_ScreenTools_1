#pragma once

#include "includeIMGUI.hpp"

inline void SetStyle() {
    ImGuiStyle& style = ImGui::GetStyle();
    // 全局透明
    style.Alpha = 0.8f;
    // 设置各项圆角半径
    style.WindowRounding = 6.0f;      // 主窗口的圆角
    style.ChildRounding = 6.0f;       // 子窗口的圆角
    style.FrameRounding = 5.0f;       // 框架类控件（如按钮）的圆角
    style.PopupRounding = 6.0f;       // 弹出窗口的圆角
    style.ScrollbarRounding = 9.0f;   // 滚动条的圆角
    style.GrabRounding = 4.0f;        // 滑块把手的圆角
    style.TabRounding = 4.0f;         // 标签页的圆角
    // 边框大小
    style.WindowBorderSize = 1.0f;
    style.FrameBorderSize = 1.0f;
}

inline void SetColor() {

}