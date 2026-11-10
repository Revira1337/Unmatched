#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"
#include "../settings/elements.h"

template<typename TYPE, typename SIGNEDTYPE, typename FLOATTYPE>
bool slider_behavior_t(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, TYPE* v, TYPE* animated_v, const TYPE v_min, const TYPE v_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    const ImGuiAxis axis = (flags & ImGuiSliderFlags_Vertical) ? ImGuiAxis_Y : ImGuiAxis_X;
    const bool is_logarithmic = (flags & ImGuiSliderFlags_Logarithmic) != 0;
    const bool is_floating_point = (data_type == ImGuiDataType_Float) || (data_type == ImGuiDataType_Double);
    const float v_range_f = (float)(v_min < v_max ? v_max - v_min : v_min - v_max);

    const float grab_padding = 0.f;
    const float slider_sz = (bb.Max[axis] - bb.Min[axis]) - grab_padding * 2.0f;
    float grab_sz = var->style.grab_min_size;
    if (!is_floating_point && v_range_f >= 0.0f)
        grab_sz = ImMax(slider_sz / (v_range_f + 1), var->style.grab_min_size);
    grab_sz = ImMin(grab_sz, slider_sz);
    const float slider_usable_sz = slider_sz - grab_sz;
    const float slider_usable_pos_min = bb.Min[axis] + grab_padding + grab_sz * 0.5f;
    const float slider_usable_pos_max = bb.Max[axis] - grab_padding - grab_sz * 0.5f;

    float logarithmic_zero_epsilon = 0.0f;
    float zero_deadzone_halfsize = 0.0f;
    if (is_logarithmic)
    {
        const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 1;
        logarithmic_zero_epsilon = ImPow(0.1f, (float)decimal_precision);
        zero_deadzone_halfsize = (var->style.log_slider_deadzone * 0.5f) / ImMax(slider_usable_sz, 1.0f);
    }

    bool value_changed = false;
    TYPE v_new = *v;
    if (g.ActiveId == id)
    {
        bool set_new_value = false;
        float clicked_t = 0.0f;
        if (g.ActiveIdSource == ImGuiInputSource_Mouse)
        {
            if (!g.IO.MouseDown[0])
            {
                ImGui::ClearActiveID();
            }
            else
            {
                const float mouse_abs_pos = g.IO.MousePos[axis];
                if (g.ActiveIdIsJustActivated)
                {
                    float grab_t = ImGui::ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
                    if (axis == ImGuiAxis_Y)
                        grab_t = 1.0f - grab_t;
                    const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
                    const bool clicked_around_grab = (mouse_abs_pos >= grab_pos - grab_sz * 0.5f - 1.0f) && (mouse_abs_pos <= grab_pos + grab_sz * 0.5f + 1.0f); // No harm being extra generous here.
                    g.SliderGrabClickOffset = (clicked_around_grab && is_floating_point) ? mouse_abs_pos - grab_pos : 0.0f;
                }
                if (slider_usable_sz > 0.0f)
                    clicked_t = ImSaturate((mouse_abs_pos - g.SliderGrabClickOffset - slider_usable_pos_min) / slider_usable_sz);
                if (axis == ImGuiAxis_Y)
                    clicked_t = 1.0f - clicked_t;
                set_new_value = true;
            }
        }
        else if (g.ActiveIdSource == ImGuiInputSource_Keyboard || g.ActiveIdSource == ImGuiInputSource_Gamepad)
        {
            if (g.ActiveIdIsJustActivated)
            {
                g.SliderCurrentAccum = 0.0f;
                g.SliderCurrentAccumDirty = false;
            }

            float input_delta = (axis == ImGuiAxis_X) ? ImGui::GetNavTweakPressedAmount(axis) : -ImGui::GetNavTweakPressedAmount(axis);
            if (input_delta != 0.0f)
            {
                const bool tweak_slow = ImGui::IsKeyDown((g.NavInputSource == ImGuiInputSource_Gamepad) ? ImGuiKey_NavGamepadTweakSlow : ImGuiKey_NavKeyboardTweakSlow);
                const bool tweak_fast = ImGui::IsKeyDown((g.NavInputSource == ImGuiInputSource_Gamepad) ? ImGuiKey_NavGamepadTweakFast : ImGuiKey_NavKeyboardTweakFast);
                const int decimal_precision = is_floating_point ? ImParseFormatPrecision(format, 3) : 0;
                if (decimal_precision > 0)
                {
                    input_delta /= 100.0f;
                    if (tweak_slow)
                        input_delta /= 10.0f;
                }
                else
                {
                    if ((v_range_f >= -100.0f && v_range_f <= 100.0f && v_range_f != 0.0f) || tweak_slow)
                        input_delta = ((input_delta < 0.0f) ? -1.0f : +1.0f) / v_range_f;
                    else
                        input_delta /= 100.0f;
                }
                if (tweak_fast)
                    input_delta *= 10.0f;

                g.SliderCurrentAccum += input_delta;
                g.SliderCurrentAccumDirty = true;
            }

            float delta = g.SliderCurrentAccum;
            if (g.NavActivatePressedId == id && !g.ActiveIdIsJustActivated)
            {
                ImGui::ClearActiveID();
            }
            else if (g.SliderCurrentAccumDirty)
            {
                clicked_t = ImGui::ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

                if ((clicked_t >= 1.0f && delta > 0.0f) || (clicked_t <= 0.0f && delta < 0.0f))
                {
                    set_new_value = false;
                    g.SliderCurrentAccum = 0.0f;
                }
                else
                {
                    set_new_value = true;
                    float old_clicked_t = clicked_t;
                    clicked_t = ImSaturate(clicked_t + delta);

                    v_new = ImGui::ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
                    if (is_floating_point && !(flags & ImGuiSliderFlags_NoRoundToFormat))
                        v_new = ImGui::RoundScalarWithFormatT<TYPE>(format, data_type, v_new);
                    float new_clicked_t = ImGui::ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, v_new, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

                    if (delta > 0)
                        g.SliderCurrentAccum -= ImMin(new_clicked_t - old_clicked_t, delta);
                    else
                        g.SliderCurrentAccum -= ImMax(new_clicked_t - old_clicked_t, delta);
                }

                g.SliderCurrentAccumDirty = false;
            }
        }

        if (set_new_value)
            if (flags & ImGuiSliderFlags_ReadOnly)
                set_new_value = false;

        if (set_new_value)
        {
            v_new = ImGui::ScaleValueFromRatioT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, clicked_t, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);

            if (is_floating_point && !(flags & ImGuiSliderFlags_NoRoundToFormat))
                v_new = ImGui::RoundScalarWithFormatT<TYPE>(format, data_type, v_new);

            if (*v != v_new)
            {
                *v = v_new;
                value_changed = true;
            }
        }
    }

    *animated_v = tween<TYPE>({ id, "slider_begavior_t" }, 0, *v, easing::QuadEaseOut, 300ms);

    if (slider_sz < 1.0f)
    {
        *out_grab_bb = ImRect(bb.Min, bb.Min);
    }
    else
    {
        float grab_t = ScaleRatioFromValueT<TYPE, SIGNEDTYPE, FLOATTYPE>(data_type, *animated_v, v_min, v_max, is_logarithmic, logarithmic_zero_epsilon, zero_deadzone_halfsize);
        if (axis == ImGuiAxis_Y)
            grab_t = 1.0f - grab_t;

        const float grab_pos = ImLerp(slider_usable_pos_min, slider_usable_pos_max, grab_t);
        if (axis == ImGuiAxis_X)
            *out_grab_bb = ImRect(grab_pos - grab_sz * 0.5f, bb.Min.y + grab_padding, grab_pos + grab_sz * 0.5f, bb.Max.y - grab_padding);
        else
            *out_grab_bb = ImRect(bb.Min.x + grab_padding, grab_pos - grab_sz * 0.5f, bb.Max.x - grab_padding, grab_pos + grab_sz * 0.5f);
    }

    return value_changed;
}

static const signed char    IM_S8_MIN = -128;
static const signed char    IM_S8_MAX = 127;
static const unsigned char  IM_U8_MIN = 0;
static const unsigned char  IM_U8_MAX = 0xFF;
static const signed short   IM_S16_MIN = -32768;
static const signed short   IM_S16_MAX = 32767;
static const unsigned short IM_U16_MIN = 0;
static const unsigned short IM_U16_MAX = 0xFFFF;
static const ImS32          IM_S32_MIN = INT_MIN;    // (-2147483647 - 1), (0x80000000);
static const ImS32          IM_S32_MAX = INT_MAX;    // (2147483647), (0x7FFFFFFF)
static const ImU32          IM_U32_MIN = 0;
static const ImU32          IM_U32_MAX = UINT_MAX;   // (0xFFFFFFFF)
#ifdef LLONG_MIN
static const ImS64          IM_S64_MIN = LLONG_MIN;  // (-9223372036854775807ll - 1ll);
static const ImS64          IM_S64_MAX = LLONG_MAX;  // (9223372036854775807ll);
#else
static const ImS64          IM_S64_MIN = -9223372036854775807LL - 1;
static const ImS64          IM_S64_MAX = 9223372036854775807LL;
#endif
static const ImU64          IM_U64_MIN = 0;
#ifdef ULLONG_MAX
static const ImU64          IM_U64_MAX = ULLONG_MAX; // (0xFFFFFFFFFFFFFFFFull);
#else
static const ImU64          IM_U64_MAX = (2ULL * 9223372036854775807LL + 1);
#endif

bool slider_behavior(const ImRect& bb, ImGuiID id, ImGuiDataType data_type, void* p_v, void* animated_v, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags, ImRect* out_grab_bb)
{
    IM_ASSERT((flags == 1 || (flags & ImGuiSliderFlags_InvalidMask_) == 0) && "Invalid ImGuiSliderFlags flags! Has the legacy 'float power' argument been mistakenly cast to flags? Call function with ImGuiSliderFlags_Logarithmic flags instead.");
    IM_ASSERT((flags & ImGuiSliderFlags_WrapAround) == 0);

    switch (data_type)
    {
    case ImGuiDataType_S8: {
        ImS32 v32 = (ImS32) * (ImS8*)p_v;
        ImS32 animated_v32 = (ImS32) * (ImS8*)animated_v;
        bool r = slider_behavior_t<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, &animated_v32, *(const ImS8*)p_min, *(const ImS8*)p_max, format, flags, out_grab_bb);
        if (r) {
            *(ImS8*)p_v = (ImS8)v32;
            *(ImS8*)animated_v = (ImS8)v32;
        }
        return r;
    }
    case ImGuiDataType_U8: {
        ImU32 v32 = (ImU32) * (ImU8*)p_v;
        ImU32 animated_v32 = (ImU32) * (ImU8*)animated_v;
        bool r = slider_behavior_t<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, &animated_v32, *(const ImU8*)p_min, *(const ImU8*)p_max, format, flags, out_grab_bb);
        if (r) {
            *(ImU8*)p_v = (ImU8)v32;
            *(ImU8*)animated_v = (ImU8)v32;
        }
        return r;
    }
    case ImGuiDataType_S16: {
        ImS32 v32 = (ImS32) * (ImS16*)p_v;
        ImS32 animated_v32 = (ImS32) * (ImS16*)animated_v;
        bool r = slider_behavior_t<ImS32, ImS32, float>(bb, id, ImGuiDataType_S32, &v32, &animated_v32, *(const ImS16*)p_min, *(const ImS16*)p_max, format, flags, out_grab_bb);
        if (r) {
            *(ImS16*)p_v = (ImS16)v32;
            *(ImS16*)animated_v = (ImS16)v32;
        }
        return r;
    }
    case ImGuiDataType_U16: {
        ImU32 v32 = (ImU32) * (ImU16*)p_v;
        ImU32 animated_v32 = (ImU32) * (ImU16*)animated_v;
        bool r = slider_behavior_t<ImU32, ImS32, float>(bb, id, ImGuiDataType_U32, &v32, &animated_v32, *(const ImU16*)p_min, *(const ImU16*)p_max, format, flags, out_grab_bb);
        if (r) {
            *(ImU16*)p_v = (ImU16)v32;
            *(ImU16*)animated_v = (ImU16)v32;
        }
        return r;
    }
    case ImGuiDataType_S32: {
        IM_ASSERT(*(const ImS32*)p_min >= IM_S32_MIN / 2 && *(const ImS32*)p_max <= IM_S32_MAX / 2);

        auto r = slider_behavior_t<ImS32, ImS32, float >(bb, id, data_type, (ImS32*)p_v, (ImS32*)animated_v, *(const ImS32*)p_min, *(const ImS32*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_U32: {
        IM_ASSERT(*(const ImU32*)p_max <= IM_U32_MAX / 2);

        auto r = slider_behavior_t<ImU32, ImS32, float >(bb, id, data_type, (ImU32*)p_v, (ImU32*)animated_v, *(const ImU32*)p_min, *(const ImU32*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_S64: {
        IM_ASSERT(*(const ImS64*)p_min >= IM_S64_MIN / 2 && *(const ImS64*)p_max <= IM_S64_MAX / 2);

        auto r = slider_behavior_t<ImS64, ImS64, double>(bb, id, data_type, (ImS64*)p_v, (ImS64*)animated_v, *(const ImS64*)p_min, *(const ImS64*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_U64: {
        IM_ASSERT(*(const ImU64*)p_max <= IM_U64_MAX / 2);

        auto r = slider_behavior_t<ImU64, ImS64, double>(bb, id, data_type, (ImU64*)p_v, (ImU64*)animated_v, *(const ImU64*)p_min, *(const ImU64*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_Float: {
        IM_ASSERT(*(const float*)p_min >= -FLT_MAX / 2.0f && *(const float*)p_max <= FLT_MAX / 2.0f);

        auto r = slider_behavior_t<float, float, float >(bb, id, data_type, (float*)p_v, (float*)animated_v, *(const float*)p_min, *(const float*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_Double: {
        IM_ASSERT(*(const double*)p_min >= -DBL_MAX / 2.0f && *(const double*)p_max <= DBL_MAX / 2.0f);

        auto r = slider_behavior_t<double, double, double>(bb, id, data_type, (double*)p_v, (double*)animated_v, *(const double*)p_min, *(const double*)p_max, format, flags, out_grab_bb);
        return r;
    }
    case ImGuiDataType_COUNT: break;
    }
    IM_ASSERT(0);
    return false;
}

bool slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, void* animated_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
{
    ImGuiWindow* window = gui->get_window();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    ImDrawList* drawlist = window->DrawList;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);

    const float height = scale(elements->window.widgets_height);
    const float slider_height = scale(5);
    const float grab_slider_diff = var->style.grab_min_size * 0.5f - slider_height * 0.5f;

    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 label_size = gui->text_size(font->get("semibold13"), label, NULL, true);
    const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(gui->content_avail().x, height));
    const ImRect frame_bb(total_bb.Min + ImVec2(0.f, height - grab_slider_diff - slider_height), total_bb.Max - ImVec2(0.f, grab_slider_diff));

    const bool temp_input_allowed = (flags & ImGuiSliderFlags_NoInput) == 0;
    gui->item_size(total_bb, style.FramePadding.y);
    if (!gui->item_add(total_bb, id, &total_bb, temp_input_allowed ? ImGuiItemFlags_Inputable : 0))
        return false;

    if (format == NULL)
        format = ImGui::DataTypeGetInfo(data_type)->PrintFmt;

    bool hovered = gui->item_hoverable(total_bb, id);

    const bool clicked = hovered && ImGui::IsMouseClicked(0, ImGuiInputFlags_None, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (make_active && clicked)
        ImGui::SetKeyOwner(ImGuiKey_MouseLeft, id);

    if (make_active)
    {
        ImGui::SetActiveID(id, window);
        ImGui::SetFocusID(id, window);
        ImGui::FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    ImRect grab_bb;
    const bool value_changed = slider_behavior(frame_bb, id, data_type, p_data, animated_data, p_min, p_max, format, flags, &grab_bb);
    if (value_changed)
        ImGui::MarkItemEdited(id);

    static std::unordered_map<ImGuiID, ImVec2> item_card_states;
    auto [it, inserted] = item_card_states.try_emplace(id, window->Pos);
    ImVec2& last_window_pos = it->second;

    const float grab_pos_x = tween<float>({ id, "slider_scalar" }, frame_bb.Min.x, grab_bb.GetCenter().x, easing::QuadEaseOut, 85ms);
    if (var->window.window_moving || var->gui.dpi_changed || var->window.last_window_scrolling || (last_window_pos != window->Pos))
    {
        finish_tween<float>({ id, "slider_scalar" });
    }
    last_window_pos = window->Pos;

    draw->rect_filled(drawlist, frame_bb.Min, frame_bb.Max, gui->u32(col->gray_2), 200.f);
    draw->rect_filled(drawlist, frame_bb.Min, ImVec2(grab_pos_x, frame_bb.Max.y), gui->u32(col->primary), 200.f);

    if (grab_bb.Max.x > grab_bb.Min.x) {
        draw->circle_filled(drawlist, ImVec2(grab_pos_x, grab_bb.GetCenter().y), var->style.grab_min_size * 0.5f, gui->u32(col->primary));
    }

    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, animated_data, format);
    draw->text_clipped(drawlist, font->get("semibold13"), total_bb.Min, total_bb.Max, gui->u32(col->text_dimmed), value_buf, value_buf_end, NULL, ImVec2(1.f, 0.f));

    if (label_size.x > 0.0f) {
        draw->text(drawlist, font->get("semibold13"), total_bb.Min, gui->u32(col->text), gui->hide_text_hint(label));
    }

    if (hovered || (g.ActiveId == id && g.IO.MouseDownDuration[0] > 0.15f)) {
        g.MouseCursor = ImGuiMouseCursor_Hand;
    }

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | (temp_input_allowed ? ImGuiItemStatusFlags_Inputable : 0));
    return value_changed;
}

bool c_widgets::slider_float(const char* label, float& v, float v_min, float v_max, const char* format, ImGuiSliderFlags flags)
{
    float tmp;
    return slider_scalar(label, ImGuiDataType_Float, &v, &tmp, &v_min, &v_max, format, flags);
}

bool c_widgets::slider_int(const char* label, int& v, int v_min, int v_max, const char* format, ImGuiSliderFlags flags)
{
    int tmp;
    return slider_scalar(label, ImGuiDataType_S32, &v, &tmp, &v_min, &v_max, format, flags);
}