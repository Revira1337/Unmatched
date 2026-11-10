#pragma once
#include <chrono>
#include <functional>
#include <map>
#include <array>
#include <string>
#include <algorithm>

#define _USE_MATH_DEFINES
#include <math.h>

#include "imgui.h"

using namespace std::chrono_literals;

enum class easing {
    LinearInterpolation,
    QuadEaseIn,
    QuadEaseOut,
    QuadEaseInOut,
    QuadEaseOutIn,
    CubicEaseIn,
    CubicEaseOut,
    CubicEaseInOut,
    CubicEaseOutIn,
    QuartEaseIn,
    QuartEaseOut,
    QuartEaseInOut,
    QuartEaseOutIn,
    QuintEaseIn,
    QuintEaseOut,
    QuintEaseInOut,
    QuintEaseOutIn,
    SineEaseIn,
    SineEaseOut,
    SineEaseInOut,
    SineEaseOutIn,
    CircEaseIn,
    CircEaseOut,
    CircEaseInOut,
    CircEaseOutIn,
    ExponentialEaseIn,
    ExponentialEaseOut,
    ExponentialEaseInOut,
    ExponentialEaseOutIn,
    ElasticEaseIn,
    ElasticEaseOut,
    ElasticEaseInOut,
    ElasticEaseOutIn,
    BackEaseIn,
    BackEaseOut,
    BackEaseInOut,
    BackEaseOutIn,
    BounceEaseIn,
    BounceEaseOut,
    BounceEaseInOut,
    SpringEaseIn,
    SpringEaseOut,
    SpringEaseInOut,
    WiggleEaseOut,
    COUNT
};

// Easing functions taken from 
// https://github.com/warrenm/AHEasing/blob/master/AHEasing/easing.c
// Within unlicense

namespace easing_funcs {
    // Modeled after the line y = x
    inline float LinearInterpolation(float p)
    {
        return p;
    }

    // Modeled after the parabola y = x^2
    inline float QuadEaseIn(float p)
    {
        return p * p;
    }

    // Modeled after the parabola y = -x^2 + 2x
    inline float QuadEaseOut(float p)
    {
        return -(p * (p - 2));
    }

    // Modeled after the piecewise quadratic
    // y = (1/2)((2x)^2)             ; [0, 0.5)
    // y = -(1/2)((2x-1)*(2x-3) - 1) ; [0.5, 1]
    inline float QuadEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 2 * p * p;
        }
        else
        {
            return (-2 * p * p) + (4 * p) - 1;
        }
    }

    inline float QuadEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * QuadEaseOut(p * 2);
        return 0.5 * QuadEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after the cubic y = x^3
    inline float CubicEaseIn(float p)
    {
        return p * p * p;
    }

    // Modeled after the cubic y = (x - 1)^3 + 1
    inline float CubicEaseOut(float p)
    {
        float f = (p - 1);
        return f * f * f + 1;
    }

    // Modeled after the piecewise cubic
    // y = (1/2)((2x)^3)       ; [0, 0.5)
    // y = (1/2)((2x-2)^3 + 2) ; [0.5, 1]
    inline float CubicEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 4 * p * p * p;
        }
        else
        {
            float f = ((2 * p) - 2);
            return 0.5 * f * f * f + 1;
        }
    }

    inline float CubicEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * CubicEaseOut(p * 2);
        return 0.5 * CubicEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after the quartic x^4
    inline float QuartEaseIn(float p)
    {
        return p * p * p * p;
    }

    // Modeled after the quartic y = 1 - (x - 1)^4
    inline float QuartEaseOut(float p)
    {
        float f = (p - 1);
        return f * f * f * (1 - p) + 1;
    }

    // Modeled after the piecewise quartic
    // y = (1/2)((2x)^4)        ; [0, 0.5)
    // y = -(1/2)((2x-2)^4 - 2) ; [0.5, 1]
    inline float QuartEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 8 * p * p * p * p;
        }
        else
        {
            float f = (p - 1);
            return -8 * f * f * f * f + 1;
        }
    }

    inline float QuartEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * QuartEaseOut(p * 2);
        return 0.5 * QuartEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after the quintic y = x^5
    inline float QuintEaseIn(float p)
    {
        return p * p * p * p * p;
    }

    // Modeled after the quintic y = (x - 1)^5 + 1
    inline float QuintEaseOut(float p)
    {
        float f = (p - 1);
        return f * f * f * f * f + 1;
    }

    // Modeled after the piecewise quintic
    // y = (1/2)((2x)^5)       ; [0, 0.5)
    // y = (1/2)((2x-2)^5 + 2) ; [0.5, 1]
    inline float QuintEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 16 * p * p * p * p * p;
        }
        else
        {
            float f = ((2 * p) - 2);
            return  0.5 * f * f * f * f * f + 1;
        }
    }

    inline float QuintEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * QuintEaseOut(p * 2);
        return 0.5 * QuintEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after quarter-cycle of sine wave
    inline float SineEaseIn(float p)
    {
        return sin((p - 1) * M_PI_2) + 1;
    }

    // Modeled after quarter-cycle of sine wave (different phase)
    inline float SineEaseOut(float p)
    {
        return sin(p * M_PI_2);
    }

    // Modeled after half sine wave
    inline float SineEaseInOut(float p)
    {
        return 0.5 * (1 - cos(p * M_PI));
    }

    inline float SineEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * SineEaseOut(p * 2);
        return 0.5 * SineEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after shifted quadrant IV of unit circle
    inline float CircEaseIn(float p)
    {
        return 1 - sqrt(1 - (p * p));
    }

    // Modeled after shifted quadrant II of unit circle
    inline float CircEaseOut(float p)
    {
        return sqrt((2 - p) * p);
    }

    // Modeled after the piecewise circular function
    // y = (1/2)(1 - sqrt(1 - 4x^2))           ; [0, 0.5)
    // y = (1/2)(sqrt(-(2x - 3)*(2x - 1)) + 1) ; [0.5, 1]
    inline float CircEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * (1 - sqrt(1 - 4 * (p * p)));
        }
        else
        {
            return 0.5 * (sqrt(-((2 * p) - 3) * ((2 * p) - 1)) + 1);
        }
    }

    inline float CircEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * CircEaseOut(p * 2);
        return 0.5 * CircEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after the exponential function y = 2^(10(x - 1))
    inline float ExponentialEaseIn(float p)
    {
        return (p == 0.0) ? p : pow(2, 10 * (p - 1));
    }

    // Modeled after the exponential function y = -2^(-10x) + 1
    inline float ExponentialEaseOut(float p)
    {
        return (p == 1.0) ? p : 1 - pow(2, -10 * p);
    }

    // Modeled after the piecewise exponential
    // y = (1/2)2^(10(2x - 1))         ; [0,0.5)
    // y = -(1/2)*2^(-10(2x - 1))) + 1 ; [0.5,1]
    inline float ExponentialEaseInOut(float p)
    {
        if (p == 0.0 || p == 1.0) return p;

        if (p < 0.5)
        {
            return 0.5 * pow(2, (20 * p) - 10);
        }
        else
        {
            return -0.5 * pow(2, (-20 * p) + 10) + 1;
        }
    }

    inline float ExponentialEaseOutIn(float p) {
        if (p == 0.0 || p == 1.0) return p;
        if (p < 0.5) return 0.5 * ExponentialEaseOut(p * 2);
        return 0.5 * ExponentialEaseIn((p * 2) - 1) + 0.5;
    }

    // Modeled after the damped sine wave y = sin(13pi/2*x)*pow(2, 10 * (x - 1))
    inline float ElasticEaseIn(float p)
    {
        return sin(13 * M_PI_2 * p) * pow(2, 10 * (p - 1));
    }

    // Modeled after the damped sine wave y = sin(-13pi/2*(x + 1))*pow(2, -10x) + 1
    inline float ElasticEaseOut(float p)
    {
        return sin(-13 * M_PI_2 * (p + 1)) * pow(2, -10 * p) + 1;
    }

    // Modeled after the piecewise exponentially-damped sine wave:
    // y = (1/2)*sin(13pi/2*(2*x))*pow(2, 10 * ((2*x) - 1))      ; [0,0.5)
    // y = (1/2)*(sin(-13pi/2*((2x-1)+1))*pow(2,-10(2*x-1)) + 2) ; [0.5, 1]
    inline float ElasticEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * sin(13 * M_PI_2 * (2 * p)) * pow(2, 10 * ((2 * p) - 1));
        }
        else
        {
            return 0.5 * (sin(-13 * M_PI_2 * ((2 * p - 1) + 1)) * pow(2, -10 * (2 * p - 1)) + 2);
        }
    }

    inline float ElasticEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * ElasticEaseOut(p * 2);
        return 0.5 * ElasticEaseIn((p * 2) - 1) + 0.5;
    }


    // Modeled after the overshooting cubic y = x^3-x*sin(x*pi)
    inline float BackEaseIn(float p)
    {
        return p * p * p - p * sin(p * M_PI);
    }

    // Modeled after overshooting cubic y = 1-((1-x)^3-(1-x)*sin((1-x)*pi))
    inline float BackEaseOut(float p)
    {
        float f = (1 - p);
        return 1 - (f * f * f - f * sin(f * M_PI));
    }

    // Modeled after the piecewise overshooting cubic function:
    // y = (1/2)*((2x)^3-(2x)*sin(2*x*pi))           ; [0, 0.5)
    // y = (1/2)*(1-((1-x)^3-(1-x)*sin((1-x)*pi))+1) ; [0.5, 1]
    inline float BackEaseInOut(float p)
    {
        if (p < 0.5)
        {
            float f = 2 * p;
            return 0.5 * (f * f * f - f * sin(f * M_PI));
        }
        else
        {
            float f = (1 - (2 * p - 1));
            return 0.5 * (1 - (f * f * f - f * sin(f * M_PI))) + 0.5;
        }
    }

    inline float BackEaseOutIn(float p) {
        if (p < 0.5) return 0.5 * BackEaseOut(p * 2);
        return 0.5 * BackEaseIn((p * 2) - 1) + 0.5;
    }

    inline float BounceEaseOut(float p)
    {
        if (p < 4 / 11.0)
        {
            return (121 * p * p) / 16.0;
        }
        else if (p < 8 / 11.0)
        {
            return (363 / 40.0 * p * p) - (99 / 10.0 * p) + 17 / 5.0;
        }
        else if (p < 9 / 10.0)
        {
            return (4356 / 361.0 * p * p) - (35442 / 1805.0 * p) + 16061 / 1805.0;
        }
        else
        {
            return (54 / 5.0 * p * p) - (513 / 25.0 * p) + 268 / 25.0;
        }
    }

    inline float BounceEaseIn(float p)
    {
        return 1 - BounceEaseOut(1 - p);
    }

    inline float BounceEaseInOut(float p)
    {
        if (p < 0.5)
        {
            return 0.5 * BounceEaseIn(p * 2);
        }
        else
        {
            return 0.5 * BounceEaseOut(p * 2 - 1) + 0.5;
        }
    }

    inline float SpringEaseOut(float p) {
        if (p == 0) return 0;
        if (p == 1) return 1;
        return 1 - (pow(2, -10 * p) * cos(p * M_PI * 4.5f));
    }

    inline float SpringEaseIn(float p) {
        return 1.0f - SpringEaseOut(1.0f - p);
    }

    inline float SpringEaseInOut(float p) {
        if (p < 0.5f) {
            return SpringEaseIn(p * 2.0f) * 0.5f;
        }
        return 0.5f + SpringEaseOut((p - 0.5f) * 2.0f) * 0.5f;
    }

    inline float WiggleEaseOut(float p) {
        constexpr float amplitude = 0.1f;
        constexpr float frequency = 15.0f;

        if (p == 0 || p == 1) return p;
        return p + sin(p * M_PI * frequency) * amplitude * (1 - p);
    }
}

using easing_func_ptr = float(*)(float);

inline std::array<easing_func_ptr, static_cast<size_t>(easing::COUNT)> easing_functions = {
    easing_funcs::LinearInterpolation,
    easing_funcs::QuadEaseIn,
    easing_funcs::QuadEaseOut,
    easing_funcs::QuadEaseInOut,
    easing_funcs::QuadEaseOutIn,
    easing_funcs::CubicEaseIn,
    easing_funcs::CubicEaseOut,
    easing_funcs::CubicEaseInOut,
    easing_funcs::CubicEaseOutIn,
    easing_funcs::QuartEaseIn,
    easing_funcs::QuartEaseOut,
    easing_funcs::QuartEaseInOut,
    easing_funcs::QuartEaseOutIn,
    easing_funcs::QuintEaseIn,
    easing_funcs::QuintEaseOut,
    easing_funcs::QuintEaseInOut,
    easing_funcs::QuintEaseOutIn,
    easing_funcs::SineEaseIn,
    easing_funcs::SineEaseOut,
    easing_funcs::SineEaseInOut,
    easing_funcs::SineEaseOutIn,
    easing_funcs::CircEaseIn,
    easing_funcs::CircEaseOut,
    easing_funcs::CircEaseInOut,
    easing_funcs::CircEaseOutIn,
    easing_funcs::ExponentialEaseIn,
    easing_funcs::ExponentialEaseOut,
    easing_funcs::ExponentialEaseInOut,
    easing_funcs::ExponentialEaseOutIn,
    easing_funcs::ElasticEaseIn,
    easing_funcs::ElasticEaseOut,
    easing_funcs::ElasticEaseInOut,
    easing_funcs::ElasticEaseOutIn,
    easing_funcs::BackEaseIn,
    easing_funcs::BackEaseOut,
    easing_funcs::BackEaseInOut,
    easing_funcs::BackEaseOutIn,
    easing_funcs::BounceEaseIn,
    easing_funcs::BounceEaseOut,
    easing_funcs::BounceEaseInOut,
    easing_funcs::SpringEaseIn,
    easing_funcs::SpringEaseOut,
    easing_funcs::SpringEaseInOut,
    easing_funcs::WiggleEaseOut
};

inline easing_func_ptr get_easing_func(easing e) {
    return easing_functions[static_cast<size_t>(e)];
}

// For ImVec4 animations
inline ImVec4 operator*(ImVec4 const& lhs, float rhs) {
    return ImVec4{ lhs.x * rhs, lhs.y * rhs, lhs.z * rhs, lhs.w * rhs };
}
inline bool operator!=(ImColor const& lhs, ImColor const& rhs) {
    return ImVec4(lhs) != ImVec4(rhs);
}

template<typename T, std::enable_if_t<std::is_arithmetic_v<T>, int> = 0>
float calculate_distance(const T& from, const T& to) {
    return std::abs(static_cast<float>(to) - static_cast<float>(from));
}

inline float calculate_distance(const ImVec2& from, const ImVec2& to) {
    ImVec2 diff = { to.x - from.x, to.y - from.y };
    return sqrtf(diff.x * diff.x + diff.y * diff.y);
}

inline float calculate_distance(const ImVec4& from, const ImVec4& to) {
    ImVec4 diff = { to.x - from.x, to.y - from.y, to.z - from.z, to.w - from.w };
    return sqrtf(diff.x * diff.x + diff.y * diff.y + diff.z * diff.z + diff.w * diff.w);
}


struct tween_id {
    tween_id(const std::string& name) : id{ ImGui::GetID(name.data()) } {};

    tween_id(std::string_view name) : id{ ImGui::GetID(name.data()) } {};

    tween_id(const char* name) : id{ ImGui::GetID(name) } {};
    
    tween_id(const ImGuiID id) : id{ id } {};

    tween_id(const char* parent_name, const char* name) : id{ ImHashStr(name, 0, ImGui::GetID(parent_name)) } {};

    tween_id(const ImGuiID parent_id, const char* name) : id{ ImHashStr(name, 0, parent_id) } {};

    tween_id(const char* name, int integer_uuid) : id{ ImHashStr(name, 0, ImGui::GetID(integer_uuid)) } {};

    ImGuiID id = 0;

    bool operator<(const tween_id& other) const {
        return id < other.id;
    }
};

using tween_duration = std::chrono::duration<float>;

template <typename T>
struct tween_data {
    tween_duration elapsed_time;
    tween_duration target_duration;
    T from;
    T to;
    T last_value;
};

template <typename T>
inline void finish_tween(tween_id id) {
    auto& tweens = get_tweens<T>();
    if (auto it = tweens.find(id); it != tweens.end())
    {
        it->second.last_value = it->second.to;
        it->second.elapsed_time = it->second.target_duration;
    }
}

template <typename T>
inline void reset_tween(tween_id id) {
    auto& tweens = get_tweens<T>();
    if (auto it = tweens.find(id); it != tweens.end())
    {
        tweens.erase(it);
    }
}

template <typename T>
inline auto& get_tweens() {
    static std::map<tween_id, tween_data<T>> tweens;
    return tweens;
}

// T requires both overloaded operator*(T, float) and operator!=(T, T) on custom types!
template <typename T>
T tween(tween_id id, const T& from, const T& to, easing easing = easing::LinearInterpolation, tween_duration duration = 300ms, std::optional<bool> condition = std::nullopt, bool relative_duration = false)
{
    const float deltatime = ImGui::GetIO().DeltaTime;

    auto& tweens = get_tweens<T>();
    auto [emplaced_tween, first_try] = tweens.try_emplace(id, tween_data<T>{tween_duration(0), duration, from, to, from});
    auto& current_tween = emplaced_tween->second;
    if (current_tween.to != to) {
        current_tween.elapsed_time = tween_duration(0);
        current_tween.from = current_tween.last_value;
        current_tween.to = to;

        if (relative_duration) {
            float dist = calculate_distance(current_tween.from, current_tween.to);
            if (dist > 1e-6f) {
                current_tween.target_duration = duration * dist;
            }
            else {
                current_tween.target_duration = tween_duration(0);
            }
        }
        else {
            current_tween.target_duration = duration;
        }
    }

    if (condition.has_value() && !condition.value()) {
        current_tween.elapsed_time -= static_cast<tween_duration>(deltatime);
        if (current_tween.elapsed_time.count() < 0) {
            current_tween.elapsed_time = tween_duration(0);
        }
    }
    else {
        current_tween.elapsed_time += static_cast<tween_duration>(deltatime);
        if (current_tween.elapsed_time > current_tween.target_duration) {
            current_tween.elapsed_time = current_tween.target_duration;
        }
    }

    if (current_tween.target_duration.count() <= 0.0f) {
        current_tween.last_value = current_tween.to;
        return current_tween.to;
    }

    const float progress = current_tween.elapsed_time.count() / current_tween.target_duration.count();

    if (progress >= 1.0f) {
        current_tween.last_value = current_tween.to;
        return current_tween.to;
    }

    if (progress <= 0.0f) {
        current_tween.last_value = current_tween.from;
        return current_tween.from;
    }

    T value = current_tween.from + (current_tween.to - current_tween.from) * (get_easing_func(easing))(progress);
    current_tween.last_value = value;
    return value;
}

inline void tween_showcase_window() {
    constexpr std::array<const char*, static_cast<size_t>(easing::COUNT)> easing_names = {
        "LinearInterpolation",
        "QuadEaseIn", "QuadEaseOut", "QuadEaseInOut", "QuadEaseOutIn",
        "CubicEaseIn", "CubicEaseOut", "CubicEaseInOut", "CubicEaseOutIn",
        "QuartEaseIn", "QuartEaseOut", "QuartEaseInOut", "QuartEaseOutIn",
        "QuintEaseIn", "QuintEaseOut", "QuintEaseInOut", "QuintEaseOutIn",
        "SineEaseIn", "SineEaseOut", "SineEaseInOut", "SineEaseOutIn",
        "CircEaseIn", "CircEaseOut", "CircEaseInOut", "CircEaseOutIn",
        "ExponentialEaseIn", "ExponentialEaseOut", "ExponentialEaseInOut", "ExponentialEaseOutIn",
        "ElasticEaseIn", "ElasticEaseOut", "ElasticEaseInOut", "ElasticEaseOutIn",
        "BackEaseIn", "BackEaseOut", "BackEaseInOut", "BackEaseOutIn",
        "BounceEaseIn", "BounceEaseOut", "BounceEaseInOut",
        "SpringEaseIn", "SpringEaseOut", "SpringEaseInOut", "WiggleOut"
    };

    static std::array<float, static_cast<size_t>(easing::COUNT)> animation_progresses;

    constexpr int num_columns = 3;
    constexpr float animation_duration_sec = 2.0f;
    constexpr ImVec2 child_size(180, 70);
    constexpr float circle_radius = 8.0f;
    constexpr float padding = 15.0f;

    const float delta_time = ImGui::GetIO().DeltaTime;

    ImGui::Begin("Tweens Showcase", nullptr, ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoResize);
    if (ImGui::BeginTable("Tweens Grid", num_columns))
    {
        for (int i = 0; i < static_cast<int>(easing::COUNT); ++i)
        {
            ImGui::TableNextColumn();

            const char* name = easing_names[i];
            auto easing_func = get_easing_func(static_cast<easing>(i));

            float& progress = animation_progresses[i];

            ImGui::BeginChild(name, child_size, ImGuiChildFlags_Border);

            ImDrawList* draw_list = ImGui::GetWindowDrawList();
            const ImVec2 p0 = ImGui::GetCursorScreenPos();

            ImGui::SetCursorPos(ImVec2(5, 5));
            ImGui::TextUnformatted(name);

            const float eased_progress = easing_func(progress);

            const float track_y = p0.y + child_size.y / 1.7f;
            const float track_start_x = p0.x + padding;
            const float track_end_x = p0.x + child_size.x - padding;
            const float track_width = track_end_x - track_start_x;

            draw_list->AddLine(
                ImVec2(track_start_x, track_y),
                ImVec2(track_end_x, track_y),
                IM_COL32(255, 255, 255, 50)
            );

            draw_list->AddCircleFilled(
                ImVec2(track_start_x + eased_progress * track_width, track_y),
                circle_radius,
                IM_COL32(255, 255, 0, 255)
            );

            ImGui::EndChild();

            if (ImGui::IsItemHovered())
            {
                ImGui::SetMouseCursor(ImGuiMouseCursor_Hand);
                progress += delta_time / animation_duration_sec;
                if (progress > 1.0f) {
                    progress = fmod(progress, 1.0f);
                }
            }
        }
        ImGui::EndTable();
    }
    ImGui::End();
}

// zeronex