#pragma once

#include "../includeIMGUI.hpp"

inline float TweenF(const char* owner, const char* channel, float target,
    float duration = 0.2f,
    iam_ease_type ease = iam_ease_out_cubic)
{
    return iam_tween_float(
        ImHashStr(owner), ImHashStr(channel),
        target, duration,
        iam_ease_preset(ease),
        iam_policy_crossfade,
        ImGui::GetIO().DeltaTime
    );
}

inline ImVec4 TweenColor(const char* owner, const char* channel,
    ImVec4 target, float duration = 0.25f,
    iam_ease_type ease = iam_ease_out_cubic)
{
    float dt = ImGui::GetIO().DeltaTime;
    ImGuiID o = ImHashStr(owner);
    return ImVec4(
        iam_tween_float(o, ImHashStr((std::string(channel) + "_r").c_str()), target.x, duration, iam_ease_preset(ease), iam_policy_crossfade, dt),
        iam_tween_float(o, ImHashStr((std::string(channel) + "_g").c_str()), target.y, duration, iam_ease_preset(ease), iam_policy_crossfade, dt),
        iam_tween_float(o, ImHashStr((std::string(channel) + "_b").c_str()), target.z, duration, iam_ease_preset(ease), iam_policy_crossfade, dt),
        iam_tween_float(o, ImHashStr((std::string(channel) + "_a").c_str()), target.w, duration, iam_ease_preset(ease), iam_policy_crossfade, dt)
    );
}

inline ImVec2 TweenVec2(const char* owner, const char* channel, ImVec2 target,
    float duration = 0.2f,
    iam_ease_type ease = iam_ease_out_cubic)
{
    return iam_tween_vec2(
        ImHashStr(owner), ImHashStr(channel),
        target, duration,
        iam_ease_preset(ease),
        iam_policy_crossfade,
        ImGui::GetIO().DeltaTime
    );
}

inline bool MyToggle(const char* label, bool* v)
{
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.45f, 0.45f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.65f, 0.65f, 0.65f, 1.0f));

    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.66f, 0.45f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.0f, 1.0f, 0.57f, 1.0f));

    bool changed = ImGui::Toggle(label, v, ImGuiToggleFlags_Animated, 0.10f, 1.0f, 1.0f);

    ImGui::PopStyleColor(4);
    return changed;
}

