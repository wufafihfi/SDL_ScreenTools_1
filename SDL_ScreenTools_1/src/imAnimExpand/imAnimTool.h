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

