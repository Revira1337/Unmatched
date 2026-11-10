#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"
#include "../settings/elements.h"

namespace {
    bool hex_input(const char* str_id, float color[4], char* buf, int buf_size, float height);

    void color_edit_restore_hs(const float* color, float* H, float* S, float* V)
    {
        ImGuiContext& g{ *GImGui };

        if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1)) *H = g.ColorEditSavedHue;
        if (*V == 0.0f) *S = g.ColorEditSavedSat;
    }

    void render_color_rect_alpha_checkboard(ImDrawList* draw_list, ImVec2 p_min, ImVec2 p_max, ImU32 color, float grid_step, ImVec2 grid_off, float rounding, ImDrawFlags flags)
    {
        if ((flags & ImDrawFlags_RoundCornersMask_) == 0) flags = ImDrawFlags_RoundCornersDefault_;
        if (((color & IM_COL32_A_MASK) >> IM_COL32_A_SHIFT) < 0xFF)
        {
            ImU32 col_bg1 = ImAlphaBlendColors(IM_COL32(204, 204, 204, 255), color);
            ImU32 col_bg2 = ImAlphaBlendColors(IM_COL32(155, 155, 155, 255), color);
            draw_list->AddRectFilled(p_min, p_max, col_bg1, rounding, flags);

            int yi = 0;
            for (float y = p_min.y + grid_off.y; y < p_max.y; y += grid_step, yi++)
            {
                float y1 = ImClamp(y, p_min.y, p_max.y), y2 = ImMin(y + grid_step, p_max.y);
                if (y2 <= y1)
                    continue;
                for (float x = p_min.x + grid_off.x + (yi & 1) * grid_step; x < p_max.x; x += grid_step * 2.0f)
                {
                    float x1 = ImClamp(x, p_min.x, p_max.x), x2 = ImMin(x + grid_step, p_max.x);
                    if (x2 <= x1)
                        continue;
                    ImDrawFlags cell_flags = ImDrawFlags_RoundCornersNone;
                    if (y1 <= p_min.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersTopLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersTopRight; }
                    if (y2 >= p_max.y) { if (x1 <= p_min.x) cell_flags |= ImDrawFlags_RoundCornersBottomLeft; if (x2 >= p_max.x) cell_flags |= ImDrawFlags_RoundCornersBottomRight; }

                    cell_flags = (flags == ImDrawFlags_RoundCornersNone || cell_flags == ImDrawFlags_RoundCornersNone) ? ImDrawFlags_RoundCornersNone : (cell_flags & flags);
                    draw_list->AddRectFilled(ImVec2(x1, y1), ImVec2(x2, y2), col_bg2, rounding, cell_flags);
                }
            }
        }
        else
        {
            draw_list->AddRectFilled(p_min, p_max, color, rounding, flags);
        }
    }

    bool hue_bar(const ImGuiID parent_id, float color[4], float& h, const ImVec2& size, bool force_move) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return false;

        ImGuiIO& io = g.IO;
        ImDrawList* drawlist = window->DrawList;

        const ImGuiID id = ImHashStr("###hue_bar", 0, parent_id);

        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, pos + size);

        const float grab_radius = bb.GetHeight() * 0.5f;
        ImVec2 grab_y_minmax = ImVec2(grab_radius, bb.GetWidth() - grab_radius);

        gui->item_size(bb);
        if (!gui->item_add(bb, id))
            return false;

        bool value_changed = false;
        bool hovered, held;
        const bool pressed = gui->button_behavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);

        struct hue_bar_state {
            float value = 0.f;
            bool initialized = false;
        };

        static std::unordered_map<ImGuiID, hue_bar_state> hue_bar_states;
        auto [it, inserted] = hue_bar_states.try_emplace(id, 0.f, false);
        float& current_state = it->second.value;

        if (inserted || force_move || var->gui.dpi_changed || var->window.window_moving) {
            current_state = grab_y_minmax.x + h * (grab_y_minmax.y - grab_y_minmax.x);

            auto& float_tweens = get_tweens<float>();
            if (auto tween_it = float_tweens.find(id); tween_it != float_tweens.end()) {
                tween_it->second.last_value = current_state;
            }
        }
        if (!force_move)
        {
            if (pressed || held) {
                current_state = ImClamp(io.MousePos.x - bb.Min.x, grab_y_minmax.x, grab_y_minmax.y);
            }

            if (var->gui.dpi_changed || var->window.window_moving) {
                current_state = grab_y_minmax.x + h * (grab_y_minmax.y - grab_y_minmax.x);

                auto& float_tweens = get_tweens<float>();
                if (auto tween_it = float_tweens.find(id); tween_it != float_tweens.end()) {
                    tween_it->second.last_value = current_state;
                }
            }
        }

        float grab_pos = tween(id, grab_y_minmax.x, current_state, easing::QuadEaseOut, 300ms);

        if (!it->second.initialized)
        {
            grab_pos = current_state;
            finish_tween<float>(id);
            it->second.initialized = true;
        }

        const ImU32 col_hues[6 + 1]{ IM_COL32(255, 0, 0, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(255, 255, 0, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(0, 255, 0, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(0, 255, 255, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(0, 0, 255, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(255, 0, 255, IM_F32_TO_INT8_SAT(var->style.alpha)), IM_COL32(255, 0, 0, IM_F32_TO_INT8_SAT(var->style.alpha)) };    const ImU32 col_midgrey = IM_COL32(128, 128, 128, IM_F32_TO_INT8_SAT(var->style.alpha));

        for (int i = 0; i < 6; ++i)
        {
            draw->rect_filled_multi_color(drawlist, ImVec2(pos.x + i * (size.x / 6), bb.Min.y), ImVec2(pos.x + (i + 1) * (size.x / 6), bb.Max.y), col_hues[i], col_hues[i + 1], col_hues[i + 1], col_hues[i], 100.f, i == 0 ? ImDrawFlags_RoundCornersLeft : i == 5 ? ImDrawFlags_RoundCornersRight : ImDrawFlags_RoundCornersNone);
        }

        draw->circle_filled(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4{ color[0], color[1], color[2], 1.f }));
        draw->circle(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4(1.f, 1.f, 1.f, 1.f)), 0, scale(3.f));
        draw->circle(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4(0.f, 0.f, 0.f, 0.1f)), 0, scale(0.7f));

        if (grab_pos != current_state)
        {
            h = ImSaturate((grab_pos - grab_y_minmax.x) / (grab_y_minmax.y - grab_y_minmax.x));
            value_changed = true;
        }

        return value_changed;
    }

    bool alpha_bar(const ImGuiID parent_id, float color[4], float& a, const ImVec2 & size, bool actived, bool force_move) {
        if (!actived)
            return false;

        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return false;

        ImGuiIO& io = g.IO;
        ImDrawList* drawlist = window->DrawList;

        const ImGuiID id = ImHashStr("###alpha_bar", 0, parent_id);

        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, pos + size);

        const float grab_radius = bb.GetHeight() * 0.5f;
        ImVec2 grab_y_minmax = ImVec2(grab_radius, bb.GetWidth() - grab_radius);

        gui->item_size(bb);
        if (!gui->item_add(bb, id))
            return false;

        bool value_changed = false;

        bool hovered, held;
        const bool pressed = gui->button_behavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);

        struct alpha_bar_state {
            float value = 0.f;
            bool initialized = false;
        };

        static std::unordered_map<ImGuiID, alpha_bar_state> alpha_bar_states;
        auto [it, inserted] = alpha_bar_states.try_emplace(id, 0.f, false);
        float& current_state = it->second.value;

        if (inserted || force_move || var->gui.dpi_changed || var->window.window_moving) {
            current_state = grab_y_minmax.x + a * (grab_y_minmax.y - grab_y_minmax.x);

            auto& float_tweens = get_tweens<float>();
            if (auto tween_it = float_tweens.find(id); tween_it != float_tweens.end()) {
                tween_it->second.last_value = current_state;
            }
        }
        if (!force_move)
        {
            if (pressed || held) {
                current_state = ImClamp(io.MousePos.x - bb.Min.x, grab_y_minmax.x, grab_y_minmax.y);
            }

            if (var->gui.dpi_changed || var->window.window_moving) {
                current_state = grab_y_minmax.x + a * (grab_y_minmax.y - grab_y_minmax.x);

                auto& float_tweens = get_tweens<float>();
                if (auto tween_it = float_tweens.find(id); tween_it != float_tweens.end()) {
                    tween_it->second.last_value = current_state;
                }
            }
        }

        float grab_pos = tween(id, grab_y_minmax.x, current_state, easing::QuadEaseOut, 150ms);
        if (!it->second.initialized)
        {
            grab_pos = current_state;
            finish_tween<float>(id);
            it->second.initialized = true;
        }

        float grid_step = ImMin(size.x, size.y) / 2.99f;
        render_color_rect_alpha_checkboard(window->DrawList, bb.Min, bb.Max, 0, grid_step, ImVec2(0.0f, 0.0f), 100.f, 0);

        draw->rect_filled_multi_color(window->DrawList, bb.Min, bb.Max, ImColor{ color[0], color[1], color[2], 0.f }, ImColor{ color[0], color[1], color[2], var->style.alpha }, ImColor{ color[0], color[1], color[2], var->style.alpha }, ImColor{ color[0], color[1], color[2], 0.f }, 100.f);

        draw->circle_filled(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4{ color[0], color[1], color[2], current_state }), 100);
        draw->circle(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4(1.f, 1.f, 1.f, 1.f)), 100, scale(3.f));
        draw->circle(drawlist, ImVec2(bb.Min.x + grab_pos, bb.GetCenter().y), grab_radius, gui->u32(ImVec4(0.f, 0.f, 0.f, 0.1f)), 100, scale(0.7f));

        if (grab_pos != current_state)
        {
            a = ImSaturate((grab_pos - grab_y_minmax.x) / (grab_y_minmax.y - grab_y_minmax.x));
            value_changed = true;
        }

        return value_changed;
    }

    bool color_space(const ImGuiID parent_id, float color[4], float h, float& s, float& v, const ImVec2 & size, bool force_move) {
        ImGuiContext& g = *GImGui;
        ImGuiWindow* window = g.CurrentWindow;
        if (window->SkipItems)
            return false;

        ImGuiIO& io = g.IO;
        ImDrawList* drawlist = window->DrawList;

        const ImGuiID id = ImHashStr("color_space", 0, parent_id);

        ImVec2 pos = window->DC.CursorPos;
        ImRect bb(pos, pos + size);

        gui->item_size(bb);
        if (!gui->item_add(bb, id))
            return false;

        bool value_changed = false;
        bool hovered, held;
        const bool pressed = gui->button_behavior(bb, id, &hovered, &held, ImGuiButtonFlags_None);

        static std::unordered_map<ImGuiID, ImVec2> color_space_states;
        auto [it, inserted] = color_space_states.try_emplace(id, ImVec2((s > 0 ? s : 0.01f) * (bb.GetWidth() - 1), (1.0f - (v > 0 ? v : 0.01f)) * (bb.GetHeight() - 1)));
        ImVec2& current_state = it->second;
        if (!force_move && (pressed || held))
        {
            current_state = ImClamp(io.MousePos - bb.Min, ImVec2(0, 0), bb.GetSize());
        }
        if (var->gui.dpi_changed || var->window.window_moving) {
            current_state = ImVec2((s > 0 ? s : 0.01f) * (bb.GetWidth() - 1), (1.0f - (v > 0 ? v : 0.01f)) * (bb.GetHeight() - 1));

            auto& float_tweens = get_tweens<ImVec2>();
            if (auto it = float_tweens.find(id); it != float_tweens.end()) {
                it->second.last_value = current_state;
            }
        }
        if (force_move) {
            current_state = ImVec2((s > 0 ? s : 0.01f) * (bb.GetWidth() - 1), (1.0f - (v > 0 ? v : 0.01f)) * (bb.GetHeight() - 1));
        }

        const ImVec2 grab_pos = tween(id, ImVec2(0, 0), current_state, easing::QuadEaseOut, 150ms);

        ImVec4 hue_color_f(1, 1, 1, var->style.alpha); ImGui::ColorConvertHSVtoRGB(h, 1, 1, hue_color_f.x, hue_color_f.y, hue_color_f.z);
        ImU32 hue_color32 = ColorConvertFloat4ToU32(hue_color_f);

        draw->rect_filled_multi_color(window->DrawList, bb.Min, bb.Max, gui->u32(col->text), hue_color32, hue_color32, gui->u32(col->text), var->style.popup_rounding, draw_flags_round_corners_top);
        draw->rect_filled_multi_color(window->DrawList, bb.Min - ImVec2(1, 1), bb.Max + ImVec2(1, 1), 0, 0, gui->u32(col->black), gui->u32(col->black), var->style.popup_rounding, draw_flags_round_corners_top);

        auto circle_col{ ImColor(color[0], color[1], color[2], 1.f) };
        const float circle_radius = scale(6.f);

        draw->circle_filled(drawlist, bb.Min + grab_pos, circle_radius, gui->u32(circle_col));
        draw->circle(drawlist, bb.Min + grab_pos, circle_radius, ImColor(255, 255, 255, int(var->style.alpha * 255)), 0, scale(3.f));

        if (grab_pos != current_state)
        {
            s = ImSaturate((grab_pos.x) / (bb.GetWidth() - 1)) <= 0 ? 0.01f : ImSaturate((grab_pos.x) / (bb.GetWidth() - 1));
            v = (1.0f - ImSaturate((grab_pos.y) / (bb.GetHeight() - 1))) <= 0 ? 0.01f : 1.0f - ImSaturate((grab_pos.y) / (bb.GetHeight() - 1));

            value_changed = true;
        }

        return value_changed;
    }

    void color_popup(const ImGuiID id, const char* str_id, float color[4], const ImVec2 & anchored_pos)
    {
        const ImVec2 window_padding = scale(12, 12);
        const ImVec2 color_space_size = scale(168, 168);
        const ImVec2 hue_bar_size = scale(120, 12);
        const ImVec2 alpha_bar_size = scale(120, 12);
        const float hex_input_height = scale(26);
        const float colorspace_hue_spacing_y = scale(12);
        const float hue_alpha_spacing_y = scale(4);
        const float alpha_hex_spacing_y = scale(10);

        bool hex_changed = false;
        bool value_changed = false;
        float h = 1.f, s = 1.f, v = 1.f, r = color[0], g = color[1], b = color[2], a = color[3];

        ImGui::ColorConvertRGBtoHSV(r, g, b, h, s, v);
        color_edit_restore_hs(color, &h, &s, &v);

        ImVec4 col_rgba{ color[0], color[1], color[2], color[3] };

        gui->push_var(style_var_popup_border_size, 0.5f);
        gui->push_var(style_var_window_padding, window_padding);
        gui->push_var(style_var_popup_rounding, scale(10));
        gui->push_var(style_var_item_spacing, ImVec2(0, 0));
        gui->push_color(style_col_popup_bg, gui->u32(col->black_3));

        const float popup_width = color_space_size.x;
        const float popup_height = window_padding.y * 2.f + color_space_size.y + hue_bar_size.y + alpha_bar_size.y + hue_alpha_spacing_y + var->style.item_inner_spacing.y + hex_input_height;

        const ImVec2 pos = ImVec2(anchored_pos.x - popup_width * 0.7f, anchored_pos.y + scale(11));

        if (gui->begin_popup(str_id, pos, { popup_width, popup_height }, popup_open_styles::top))
        {
            static std::unordered_map<ImGuiID, bool> color_picker_states;
            auto [it, inserted] = color_picker_states.try_emplace(id, false);
            bool& force_move_next_frame = it->second;

            gui->set_pos(gui->get_pos() - window_padding, pos_all);
            if (color_space(id, color, h, s, v, color_space_size, force_move_next_frame)) {
                value_changed = true;
            }

            const ImVec2 icon_size = gui->text_size(font->get("icons12"), ICON_EYE_DROPPER);

            const ImVec2 icon_rect_min = ImVec2(window_padding.x, gui->get_pos().y + colorspace_hue_spacing_y);
            const ImVec2 icon_rect_max = icon_rect_min + ImVec2(icon_size.x, hue_bar_size.y + alpha_bar_size.y + hue_alpha_spacing_y);

            draw->text_clipped(gui->window_drawlist(), font->get("icons12"), gui->window_pos() + icon_rect_min, gui->window_pos() + icon_rect_max, gui->u32(col->text), ICON_EYE_DROPPER, nullptr, &icon_size, { 0.5f, 0.5f });

            const ImVec2 hue_bar_pos = ImVec2(icon_rect_max.x + window_padding.x, icon_rect_min.y);
            gui->set_pos(hue_bar_pos, pos_all);
            if (hue_bar(id, color, h, hue_bar_size, force_move_next_frame))
            {
                value_changed = true;
            }

            const ImVec2 alpha_bar_pos = hue_bar_pos + ImVec2(0.f, hue_bar_size.y + hue_alpha_spacing_y);
            gui->set_pos(alpha_bar_pos, pos_all);
            if (alpha_bar(id, color, a, alpha_bar_size, true, force_move_next_frame))
            {
                value_changed = true;
            }

            {
                force_move_next_frame = false;

                int color_rgb[4] = { IM_F32_TO_INT8_UNBOUND(color[0]), IM_F32_TO_INT8_UNBOUND(color[1]), IM_F32_TO_INT8_UNBOUND(color[2]), IM_F32_TO_INT8_UNBOUND(color[3]) };

                char buf[64];
                ImFormatString(buf, IM_ARRAYSIZE(buf), "#%02X%02X%02X%02X", ImClamp(color_rgb[0], 0, 255), ImClamp(color_rgb[1], 0, 255), ImClamp(color_rgb[2], 0, 255), ImClamp(color_rgb[3], 0, 255));
                
                const ImVec2 hex_input_pos = ImVec2(window_padding.x, alpha_bar_pos.y + alpha_bar_size.y + alpha_hex_spacing_y);
                gui->set_pos(hex_input_pos, pos_all);
                if (hex_input("##Text", color, buf, IM_ARRAYSIZE(buf), hex_input_height))
                {
                    value_changed = true;
                    char* p = buf;
                    while (*p == '#' || ImCharIsBlankA(*p))
                        p++;
                    color_rgb[0] = color_rgb[1] = color_rgb[2] = 0;
                    color_rgb[3] = 0xFF;
                    int r = sscanf(p, "%02X%02X%02X%02X", (unsigned int*)&color_rgb[0], (unsigned int*)&color_rgb[1], (unsigned int*)&color_rgb[2], (unsigned int*)&color_rgb[3]);
                    IM_UNUSED(r);

                    for (short i = 0; i < 4; i++)
                    {
                        color[i] = color_rgb[i] / 255.f;
                    }

                    force_move_next_frame = true;
                    hex_changed = true;
                }
            }

            gui->end_popup(str_id);
        }

        gui->pop_color();
        gui->pop_var(4);

        ImGui::ColorConvertHSVtoRGB(h, s, v, r, g, b);

        if (value_changed && !hex_changed) {
            GImGui->ColorEditSavedHue = h;
            GImGui->ColorEditSavedSat = s;
            GImGui->ColorEditSavedColor = ImGui::ColorConvertFloat4ToU32(ImVec4(color[0], color[1], color[2], 0));

            color[0] = r;
            color[1] = g;
            color[2] = b;
            color[3] = a;
        }
    }
}

bool c_widgets::checkbox_color(const char* label, const char* desc, bool& value, ImVec4& color) {
    float color_float[4] = { color.x, color.y, color.z, color.w };
    const bool value_changed = checkbox_color(label, desc, value, color_float);
    color = ImVec4(color_float[0], color_float[1], color_float[2], color_float[3]);
    return value_changed;
}

bool c_widgets::checkbox_color(const char* label, const char* desc, bool& value, float color[4]) {
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const auto toggler_font = font->get("medium14");

    const ImVec2 label_size = gui->text_size(toggler_font, label);

    const float checkbox_roudning = scale(10);
    const float height = scale(elements->window.widgets_height);
    const ImVec2 checkbox_size = scale(32, 19);
    const ImVec2 circle_padding = scale(4, 4);
    const float circle_radius = scale(5.5f);

    const ImVec2 colorbutton_size = scale(15, 15);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect checkbox_bb(ImVec2(bb.Max.x - checkbox_size.x, bb.GetCenter().y - checkbox_size.y * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + checkbox_size.y * 0.5f));

    const ImRect colorbutton_bb(ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x - colorbutton_size.x, bb.GetCenter().y - colorbutton_size.y * 0.5f), ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x, bb.GetCenter().y + colorbutton_size.y * 0.5f));

    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    bool value_changed = false;
    bool hovered, held;
    const bool pressed = gui->button_behavior(checkbox_bb, id, &hovered, &held);
    if (pressed)
    {
        value = !value;
        value_changed = true;
    }

    const ImGuiID colorbutton_id = ImHashStr("colorbutton_bb", 0, id);
    gui->item_add(colorbutton_bb, colorbutton_id);

    const bool colorbutton_pressed = gui->button_behavior(colorbutton_bb, colorbutton_id);
    if (colorbutton_pressed)
    {
        gui->open_popup(label);
    }

    const ImVec4 bg_color_from(col->primary.x, col->primary.y, col->primary.z, 0.1f);
    const ImVec4 circle_color_from(col->primary.x, col->primary.y, col->primary.z, 0.35f);

    const float anim = tween({ id, "checkbox_anim" }, 0.f, 1.f, easing::BackEaseInOut, 250ms, value);
    const ImVec4 bg_color = tween({ id, "checkbox_bg_color" }, bg_color_from, col->primary, easing::QuadEaseInOut, 200ms, value);
    const ImVec4 circle_color = tween({ id, "checkbox_circle_color" }, circle_color_from, col->black, easing::QuadEaseInOut, 200ms, value);

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));

    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->rect_filled(drawlist, checkbox_bb.Min, checkbox_bb.Max, gui->u32(bg_color), checkbox_roudning);

    const float circle_move_pos_x = (checkbox_bb.GetSize().x - circle_padding.x * 2.f - circle_radius * 2.f) * anim;
    draw->circle_filled(drawlist, ImVec2(checkbox_bb.Min.x + circle_padding.x + circle_radius + circle_move_pos_x, checkbox_bb.GetCenter().y), circle_radius, gui->u32(circle_color));

    draw->circle_filled(drawlist, colorbutton_bb.GetCenter(), colorbutton_size.x * 0.5f, gui->u32(ImVec4(color[0], color[1], color[2], 1.f)), 100);

    color_popup(id, label, color, colorbutton_bb.GetBL());

    return value_changed;
}

static ImVec2 input_text_calc_text_size(ImGuiContext* ctx, const char* text_begin, const char* text_end, const char** remaining, ImVec2* out_offset, bool stop_on_new_line)
{
    ImGuiContext& g = *ctx;
    ImFont* font = g.Font;
    const float line_height = g.FontSize;
    const float scale = line_height / font->FontSize;

    ImVec2 text_size = ImVec2(0, 0);
    float line_width = 0.0f;

    const char* s = text_begin;
    while (s < text_end)
    {
        unsigned int c = (unsigned int)*s;
        if (c < 0x80)
            s += 1;
        else
            s += ImTextCharFromUtf8(&c, s, text_end);

        if (c == '\n')
        {
            text_size.x = ImMax(text_size.x, line_width);
            text_size.y += line_height;
            line_width = 0.0f;
            if (stop_on_new_line)
                break;
            continue;
        }
        if (c == '\r')
            continue;

        const float char_width = ((int)c < font->IndexAdvanceX.Size ? font->IndexAdvanceX.Data[c] : font->FallbackAdvanceX) * scale;
        line_width += char_width;
    }

    if (text_size.x < line_width)
        text_size.x = line_width;

    if (out_offset)
        *out_offset = ImVec2(line_width, text_size.y + line_height);

    if (line_width > 0 || text_size.y == 0.0f)
        text_size.y += line_height;

    if (remaining)
        *remaining = s;

    return text_size;
}

namespace ImStb
{
#include "imstb_textedit.h"
}

namespace ImStb
{
    static int     STB_TEXTEDIT_STRINGLEN(const ImGuiInputTextState* obj) { return obj->TextLen; }
    static char    STB_TEXTEDIT_GETCHAR(const ImGuiInputTextState* obj, int idx) { IM_ASSERT(idx <= obj->TextLen); return obj->TextSrc[idx]; }
    static float   STB_TEXTEDIT_GETWIDTH(ImGuiInputTextState* obj, int line_start_idx, int char_idx) { unsigned int c; ImTextCharFromUtf8(&c, obj->TextSrc + line_start_idx + char_idx, obj->TextSrc + obj->TextLen); if ((ImWchar)c == '\n') return IMSTB_TEXTEDIT_GETWIDTH_NEWLINE; ImGuiContext& g = *obj->Ctx; return g.Font->GetCharAdvance((ImWchar)c) * g.FontScale; }
    static char    STB_TEXTEDIT_NEWLINE = '\n';
    static void    STB_TEXTEDIT_LAYOUTROW(StbTexteditRow* r, ImGuiInputTextState* obj, int line_start_idx)
    {
        const char* text = obj->TextSrc;
        const char* text_remaining = NULL;
        const ImVec2 size = input_text_calc_text_size(obj->Ctx, text + line_start_idx, text + obj->TextLen, &text_remaining, NULL, true);
        r->x0 = 0.0f;
        r->x1 = size.x;
        r->baseline_y_delta = size.y;
        r->ymin = 0.0f;
        r->ymax = size.y;
        r->num_chars = (int)(text_remaining - (text + line_start_idx));
    }

#define IMSTB_TEXTEDIT_GETNEXTCHARINDEX  IMSTB_TEXTEDIT_GETNEXTCHARINDEX_IMPL
#define IMSTB_TEXTEDIT_GETPREVCHARINDEX  IMSTB_TEXTEDIT_GETPREVCHARINDEX_IMPL

    static int IMSTB_TEXTEDIT_GETNEXTCHARINDEX_IMPL(ImGuiInputTextState* obj, int idx)
    {
        if (idx >= obj->TextLen)
            return obj->TextLen + 1;
        unsigned int c;
        return idx + ImTextCharFromUtf8(&c, obj->TextSrc + idx, obj->TextSrc + obj->TextLen);
    }

    static int IMSTB_TEXTEDIT_GETPREVCHARINDEX_IMPL(ImGuiInputTextState* obj, int idx)
    {
        if (idx <= 0)
            return -1;
        const char* p = ImTextFindPreviousUtf8Codepoint(obj->TextSrc, obj->TextSrc + idx);
        return (int)(p - obj->TextSrc);
    }

    static bool ImCharIsSeparatorW(unsigned int c)
    {
        static const unsigned int separator_list[] =
        {
            ',', 0x3001, '.', 0x3002, ';', 0xFF1B, '(', 0xFF08, ')', 0xFF09, '{', 0xFF5B, '}', 0xFF5D,
            '[', 0x300C, ']', 0x300D, '|', 0xFF5C, '!', 0xFF01, '\\', 0xFFE5, '/', 0x30FB, 0xFF0F,
            '\n', '\r',
        };
        for (unsigned int separator : separator_list)
            if (c == separator)
                return true;
        return false;
    }

    static int is_word_boundary_from_right(ImGuiInputTextState* obj, int idx)
    {
        // When ImGuiInputTextFlags_Password is set, we don't want actions such as CTRL+Arrow to leak the fact that underlying data are blanks or separators.
        if ((obj->Flags & ImGuiInputTextFlags_Password) || idx <= 0)
            return 0;

        const char* curr_p = obj->TextA.Data + idx;
        const char* prev_p = ImTextFindPreviousUtf8Codepoint(obj->TextA.Data, curr_p);
        unsigned int curr_c; ImTextCharFromUtf8(&curr_c, curr_p, obj->TextA.Data + obj->TextA.Size);
        unsigned int prev_c; ImTextCharFromUtf8(&prev_c, prev_p, obj->TextA.Data + obj->TextA.Size);

        bool prev_white = ImCharIsBlankW(prev_c);
        bool prev_separ = ImCharIsSeparatorW(prev_c);
        bool curr_white = ImCharIsBlankW(curr_c);
        bool curr_separ = ImCharIsSeparatorW(curr_c);
        return ((prev_white || prev_separ) && !(curr_separ || curr_white)) || (curr_separ && !prev_separ);
    }
    static int is_word_boundary_from_left(ImGuiInputTextState* obj, int idx)
    {
        if ((obj->Flags & ImGuiInputTextFlags_Password) || idx <= 0)
            return 0;

        const char* curr_p = obj->TextA.Data + idx;
        const char* prev_p = ImTextFindPreviousUtf8Codepoint(obj->TextA.Data, curr_p);
        unsigned int prev_c; ImTextCharFromUtf8(&prev_c, curr_p, obj->TextA.Data + obj->TextA.Size);
        unsigned int curr_c; ImTextCharFromUtf8(&curr_c, prev_p, obj->TextA.Data + obj->TextA.Size);

        bool prev_white = ImCharIsBlankW(prev_c);
        bool prev_separ = ImCharIsSeparatorW(prev_c);
        bool curr_white = ImCharIsBlankW(curr_c);
        bool curr_separ = ImCharIsSeparatorW(curr_c);
        return ((prev_white) && !(curr_separ || curr_white)) || (curr_separ && !prev_separ);
    }
    static int  STB_TEXTEDIT_MOVEWORDLEFT_IMPL(ImGuiInputTextState* obj, int idx)
    {
        idx = IMSTB_TEXTEDIT_GETPREVCHARINDEX(obj, idx);
        while (idx >= 0 && !is_word_boundary_from_right(obj, idx))
            idx = IMSTB_TEXTEDIT_GETPREVCHARINDEX(obj, idx);
        return idx < 0 ? 0 : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_MAC(ImGuiInputTextState* obj, int idx)
    {
        int len = obj->TextLen;
        idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        while (idx < len && !is_word_boundary_from_left(obj, idx))
            idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        return idx > len ? len : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_WIN(ImGuiInputTextState* obj, int idx)
    {
        idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        int len = obj->TextLen;
        while (idx < len && !is_word_boundary_from_right(obj, idx))
            idx = IMSTB_TEXTEDIT_GETNEXTCHARINDEX(obj, idx);
        return idx > len ? len : idx;
    }
    static int  STB_TEXTEDIT_MOVEWORDRIGHT_IMPL(ImGuiInputTextState* obj, int idx) { ImGuiContext& g = *obj->Ctx; if (g.IO.ConfigMacOSXBehaviors) return STB_TEXTEDIT_MOVEWORDRIGHT_MAC(obj, idx); else return STB_TEXTEDIT_MOVEWORDRIGHT_WIN(obj, idx); }
#define STB_TEXTEDIT_MOVEWORDLEFT       STB_TEXTEDIT_MOVEWORDLEFT_IMPL  // They need to be #define for stb_textedit.h
#define STB_TEXTEDIT_MOVEWORDRIGHT      STB_TEXTEDIT_MOVEWORDRIGHT_IMPL

    static void STB_TEXTEDIT_DELETECHARS(ImGuiInputTextState* obj, int pos, int n)
    {
        IM_ASSERT(obj->TextSrc == obj->TextA.Data);
        char* dst = obj->TextA.Data + pos;
        char* src = obj->TextA.Data + pos + n;
        memmove(dst, src, obj->TextLen - n - pos + 1);
        obj->Edited = true;
        obj->TextLen -= n;
    }

    static bool STB_TEXTEDIT_INSERTCHARS(ImGuiInputTextState* obj, int pos, const char* new_text, int new_text_len)
    {
        const bool is_resizable = (obj->Flags & ImGuiInputTextFlags_CallbackResize) != 0;
        const int text_len = obj->TextLen;
        IM_ASSERT(pos <= text_len);

        if (!is_resizable && (new_text_len + obj->TextLen + 1 > obj->BufCapacity))
            return false;

        IM_ASSERT(obj->TextSrc == obj->TextA.Data);
        if (new_text_len + text_len + 1 > obj->TextA.Size)
        {
            if (!is_resizable)
                return false;
            obj->TextA.resize(text_len + ImClamp(new_text_len, 32, ImMax(256, new_text_len)) + 1);
            obj->TextSrc = obj->TextA.Data;
        }

        char* text = obj->TextA.Data;
        if (pos != text_len)
            memmove(text + pos + new_text_len, text + pos, (size_t)(text_len - pos));
        memcpy(text + pos, new_text, (size_t)new_text_len);

        obj->Edited = true;
        obj->TextLen += new_text_len;
        obj->TextA[obj->TextLen] = '\0';

        return true;
    }

#define STB_TEXTEDIT_K_LEFT         0x200000 // keyboard input to move cursor left
#define STB_TEXTEDIT_K_RIGHT        0x200001 // keyboard input to move cursor right
#define STB_TEXTEDIT_K_UP           0x200002 // keyboard input to move cursor up
#define STB_TEXTEDIT_K_DOWN         0x200003 // keyboard input to move cursor down
#define STB_TEXTEDIT_K_LINESTART    0x200004 // keyboard input to move cursor to start of line
#define STB_TEXTEDIT_K_LINEEND      0x200005 // keyboard input to move cursor to end of line
#define STB_TEXTEDIT_K_TEXTSTART    0x200006 // keyboard input to move cursor to start of text
#define STB_TEXTEDIT_K_TEXTEND      0x200007 // keyboard input to move cursor to end of text
#define STB_TEXTEDIT_K_DELETE       0x200008 // keyboard input to delete selection or character under cursor
#define STB_TEXTEDIT_K_BACKSPACE    0x200009 // keyboard input to delete selection or character left of cursor
#define STB_TEXTEDIT_K_UNDO         0x20000A // keyboard input to perform undo
#define STB_TEXTEDIT_K_REDO         0x20000B // keyboard input to perform redo
#define STB_TEXTEDIT_K_WORDLEFT     0x20000C // keyboard input to move cursor left one word
#define STB_TEXTEDIT_K_WORDRIGHT    0x20000D // keyboard input to move cursor right one word
#define STB_TEXTEDIT_K_PGUP         0x20000E // keyboard input to move cursor up a page
#define STB_TEXTEDIT_K_PGDOWN       0x20000F // keyboard input to move cursor down a page
#define STB_TEXTEDIT_K_SHIFT        0x400000

#define IMSTB_TEXTEDIT_IMPLEMENTATION
#define IMSTB_TEXTEDIT_memmove memmove
#include "imstb_textedit.h"

    static void stb_textedit_replace(ImGuiInputTextState* str, STB_TexteditState* state, const IMSTB_TEXTEDIT_CHARTYPE* text, int text_len)
    {
        stb_text_makeundo_replace(str, state, 0, str->TextLen, text_len);
        ImStb::STB_TEXTEDIT_DELETECHARS(str, 0, str->TextLen);
        state->cursor = state->select_start = state->select_end = 0;
        if (text_len <= 0)
            return;
        if (ImStb::STB_TEXTEDIT_INSERTCHARS(str, 0, text, text_len))
        {
            state->cursor = state->select_start = state->select_end = text_len;
            state->has_preferred_x = 0;
            return;
        }
        IM_ASSERT(0);
    }

}

namespace {
static bool input_text_filter_character(ImGuiContext* ctx, unsigned int* p_char, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback, void* user_data, bool input_source_is_clipboard)
{
    unsigned int c = *p_char;

    bool apply_named_filters = true;
    if (c < 0x20)
    {
        bool pass = false;
        pass |= (c == '\n') && (flags & ImGuiInputTextFlags_Multiline) != 0;
        pass |= (c == '\t') && (flags & ImGuiInputTextFlags_AllowTabInput) != 0;
        if (!pass)
            return false;
        apply_named_filters = false;
    }

    if (input_source_is_clipboard == false)
    {
        if (c == 127)
            return false;

        if (c >= 0xE000 && c <= 0xF8FF)
            return false;
    }

    if (c > IM_UNICODE_CODEPOINT_MAX)
        return false;

    if (apply_named_filters && (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_CharsNoBlank | ImGuiInputTextFlags_CharsScientific | (ImGuiInputTextFlags)ImGuiInputTextFlags_LocalizeDecimalPoint)))
    {
        ImGuiContext& g = *ctx;
        const unsigned c_decimal_point = (unsigned int)g.PlatformIO.Platform_LocaleDecimalPoint;
        if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | (ImGuiInputTextFlags)ImGuiInputTextFlags_LocalizeDecimalPoint))
            if (c == '.' || c == ',')
                c = c_decimal_point;

        if (flags & (ImGuiInputTextFlags_CharsDecimal | ImGuiInputTextFlags_CharsScientific | ImGuiInputTextFlags_CharsHexadecimal))
            if (c >= 0xFF01 && c <= 0xFF5E)
                c = c - 0xFF01 + 0x21;

        if (flags & ImGuiInputTextFlags_CharsDecimal)
            if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsScientific)
            if (!(c >= '0' && c <= '9') && (c != c_decimal_point) && (c != '-') && (c != '+') && (c != '*') && (c != '/') && (c != 'e') && (c != 'E'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsHexadecimal)
            if (!(c >= '0' && c <= '9') && !(c >= 'a' && c <= 'f') && !(c >= 'A' && c <= 'F'))
                return false;

        if (flags & ImGuiInputTextFlags_CharsUppercase)
            if (c >= 'a' && c <= 'z')
                c += (unsigned int)('A' - 'a');

        if (flags & ImGuiInputTextFlags_CharsNoBlank)
            if (ImCharIsBlankW(c))
                return false;

        *p_char = c;
    }

    if (flags & ImGuiInputTextFlags_CallbackCharFilter)
    {
        ImGuiContext& g = *GImGui;
        ImGuiInputTextCallbackData callback_data;
        callback_data.Ctx = &g;
        callback_data.EventFlag = ImGuiInputTextFlags_CallbackCharFilter;
        callback_data.EventChar = (ImWchar)c;
        callback_data.Flags = flags;
        callback_data.UserData = user_data;
        if (callback(&callback_data) != 0)
            return false;
        *p_char = callback_data.EventChar;
        if (!callback_data.EventChar)
            return false;
    }

    return true;
}

static void input_text_reconcile_undo_state_after_user_callback(ImGuiInputTextState* state, const char* new_buf_a, int new_length_a)
{
    const char* old_buf = state->CallbackTextBackup.Data;
    const int old_length = state->CallbackTextBackup.Size - 1;

    const int shorter_length = ImMin(old_length, new_length_a);
    int first_diff;
    for (first_diff = 0; first_diff < shorter_length; first_diff++)
        if (old_buf[first_diff] != new_buf_a[first_diff])
            break;
    if (first_diff == old_length && first_diff == new_length_a)
        return;

    int old_last_diff = old_length - 1;
    int new_last_diff = new_length_a - 1;
    for (; old_last_diff >= first_diff && new_last_diff >= first_diff; old_last_diff--, new_last_diff--)
        if (old_buf[old_last_diff] != new_buf_a[new_last_diff])
            break;

    const int insert_len = new_last_diff - first_diff + 1;
    const int delete_len = old_last_diff - first_diff + 1;
    if (insert_len > 0 || delete_len > 0)
        if (IMSTB_TEXTEDIT_CHARTYPE* p = stb_text_createundo(&state->Stb->undostate, first_diff, delete_len, insert_len))
            for (int i = 0; i < delete_len; i++)
                p[i] = old_buf[first_diff + i];
}

bool input_raw(const char* str_id, char* buf, int buf_size, const ImVec2& size, ImGuiInputTextFlags flags, ImGuiInputTextCallback callback = nullptr, void* callback_user_data = nullptr)
{
    IM_ASSERT(buf != NULL && buf_size >= 0);

    ImGuiWindow* window = gui->get_window();
    if (window->SkipItems)
        return false;

    IM_ASSERT(!((flags & ImGuiInputTextFlags_CallbackCompletion) && (flags & ImGuiInputTextFlags_AllowTabInput)));

    ImGuiContext& g = *GImGui;
    ImGuiIO& io = g.IO;
    const ImGuiStyle& style = g.Style;

    gui->push_font(font->get("medium13"));

    const bool RENDER_SELECTION_WHEN_INACTIVE = false;

    const ImGuiID id = window->GetID(str_id);
    const ImVec2 total_size = size;

    const ImVec2 pos = window->DC.CursorPos;
    const ImVec2 winpos = window->Pos;
    ImDrawList* drawlist = window->DrawList;

    const ImRect total_bb(pos, pos + size);
    const ImRect bb(total_bb.Min, total_bb.Max);

    ImGuiWindow* draw_window = window;
    ImVec2 inner_size = bb.GetSize();
    ImGuiLastItemData item_data_backup;

    gui->item_size(total_bb);
    if (!(flags & ImGuiInputTextFlags_MergedItem))
        if (!gui->item_add(total_bb, id, &bb, ImGuiItemFlags_Inputable)) {
            gui->pop_font();
            return false;
        }

    const bool hovered = ItemHoverable(total_bb, id, g.LastItemData.ItemFlags);
    if (hovered)
        SetMouseCursor(ImGuiMouseCursor_Hand);

    ImGuiInputTextState* state = GetInputTextState(id);

    if (g.LastItemData.ItemFlags & ImGuiItemFlags_ReadOnly)
        flags |= ImGuiInputTextFlags_ReadOnly;
    const bool is_readonly = (flags & ImGuiInputTextFlags_ReadOnly) != 0;
    const bool is_password = (flags & ImGuiInputTextFlags_Password) != 0;
    const bool is_undoable = (flags & ImGuiInputTextFlags_NoUndoRedo) == 0;
    const bool is_resizable = (flags & ImGuiInputTextFlags_CallbackResize) != 0;

    const bool input_requested_by_nav = (g.ActiveId != id) && ((g.NavActivateId == id) && ((g.NavActivateFlags & ImGuiActivateFlags_PreferInput) || (g.NavInputSource == ImGuiInputSource_Keyboard)));

    const bool user_clicked = hovered && io.MouseClicked[0];
    const bool user_scroll_finish = false;
    const bool user_scroll_active = false;
    bool clear_active_id = false;
    bool select_all = false;

    float scroll_y = FLT_MAX;

    const bool init_reload_from_user_buf = (state != NULL && state->WantReloadUserBuf);
    const bool init_changed_specs = (state != NULL && state->Stb->single_line != false);
    const bool init_make_active = (user_clicked || user_scroll_finish || input_requested_by_nav);
    const bool init_state = (init_make_active || user_scroll_active);
    if ((init_state && g.ActiveId != id) || init_changed_specs || init_reload_from_user_buf)
    {
        state = &g.InputTextState;
        state->CursorAnimReset();
        state->WantReloadUserBuf = false;

        InputTextDeactivateHook(state->ID);

        const int buf_len = (int)strlen(buf);
        if (!init_reload_from_user_buf)
        {
            state->TextToRevertTo.resize(buf_len + 1);
            memcpy(state->TextToRevertTo.Data, buf, buf_len + 1);
        }

        bool recycle_state = (state->ID == id && !init_changed_specs && !init_reload_from_user_buf);
        if (recycle_state && (state->TextLen != buf_len || (strncmp(state->TextA.Data, buf, buf_len) != 0)))
            recycle_state = false;

        state->ID = id;
        state->TextA.resize(buf_size + 1);
        state->TextLen = (int)strlen(buf);
        memcpy(state->TextA.Data, buf, state->TextLen + 1);

        if (recycle_state)
        {
            state->CursorClamp();
        }
        else
        {
            state->Scroll = ImVec2(0.0f, 0.0f);
            stb_textedit_initialize_state(state->Stb, false);
        }

        if (init_reload_from_user_buf)
        {
            state->Stb->select_start = state->ReloadSelectionStart;
            state->Stb->cursor = state->Stb->select_end = state->ReloadSelectionEnd;
            state->CursorClamp();
        }

        if (flags & ImGuiInputTextFlags_AutoSelectAll)
            select_all = true;
        if (input_requested_by_nav && (!recycle_state || !(g.NavActivateFlags & ImGuiActivateFlags_TryToPreserveState)))
            select_all = true;
        if (user_clicked && io.KeyCtrl)
            select_all = true;

        if (flags & ImGuiInputTextFlags_AlwaysOverwrite)
            state->Stb->insert_mode = 1;
    }

    const bool is_osx = io.ConfigMacOSXBehaviors;
    if (g.ActiveId != id && init_make_active)
    {
        IM_ASSERT(state && state->ID == id);
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
    }
    if (g.ActiveId == id)
    {
        if (user_clicked)
            SetKeyOwner(ImGuiKey_MouseLeft, id);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
        if (flags & ImGuiInputTextFlags_CallbackHistory)
            g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Up) | (1 << ImGuiDir_Down);
        SetKeyOwner(ImGuiKey_Enter, id);
        SetKeyOwner(ImGuiKey_KeypadEnter, id);
        SetKeyOwner(ImGuiKey_Home, id);
        SetKeyOwner(ImGuiKey_End, id);
        if (is_osx)
            SetKeyOwner(ImGuiMod_Alt, id);
    }

    if (g.ActiveId == id && state == NULL)
        ClearActiveID();

    if (g.ActiveId == id && io.MouseClicked[0] && !init_state && !init_make_active)
        clear_active_id = true;

    bool render_cursor = (g.ActiveId == id) || (state && user_scroll_active);
    bool render_selection = state && (state->HasSelection() || select_all) && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
    bool value_changed = false;
    bool validated = false;

    const bool buf_display_from_state = (render_cursor || render_selection || g.ActiveId == id) && !is_readonly && state;
    const bool is_displaying_hint = false;//(hint != NULL && (buf_display_from_state ? state->TextA.Data : buf)[0] == 0);

    if (is_password && !is_displaying_hint)
    {
        ImFontGlyph* glyph = g.Font->FindGlyph('*');
        ImFont* password_font = &g.InputTextPasswordFont;
        password_font->FontSize = g.Font->FontSize;
        password_font->Scale = g.Font->Scale;
        password_font->Ascent = g.Font->Ascent;
        password_font->Descent = g.Font->Descent;
        password_font->ContainerAtlas = g.Font->ContainerAtlas;
        password_font->FallbackGlyph = glyph;
        password_font->FallbackAdvanceX = glyph->AdvanceX;
        IM_ASSERT(password_font->Glyphs.empty() && password_font->IndexAdvanceX.empty() && password_font->IndexLookup.empty());
        PushFont(password_font);
    }

    bool is_mouse_moved = false;

    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        state->Edited = false;
        state->BufCapacity = buf_size;
        state->Flags = flags;

        g.ActiveIdAllowOverlap = !io.MouseDown[0];

        const float mouse_x = (io.MousePos.x - bb.Min.x) + state->Scroll.x;
        const float mouse_y = g.FontSize * 0.5f;

        if (select_all)
        {
            state->SelectAll();
            state->SelectedAllMouseLock = true;
        }
        else if (hovered && io.MouseClickedCount[0] >= 2 && !io.KeyShift)
        {
            stb_textedit_click(state, state->Stb, mouse_x, mouse_y);
            const int multiclick_count = (io.MouseClickedCount[0] - 2);
            if ((multiclick_count % 2) == 0)
            {
                const bool is_bol = (state->Stb->cursor == 0) || ImStb::STB_TEXTEDIT_GETCHAR(state, state->Stb->cursor - 1) == '\n';
                if (STB_TEXT_HAS_SELECTION(state->Stb) || !is_bol)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT);
                if (!STB_TEXT_HAS_SELECTION(state->Stb))
                    ImStb::stb_textedit_prep_selection_at_cursor(state->Stb);
                state->Stb->cursor = ImStb::STB_TEXTEDIT_MOVEWORDRIGHT_MAC(state, state->Stb->cursor);
                state->Stb->select_end = state->Stb->cursor;
                ImStb::stb_textedit_clamp(state, state->Stb);
            }
            else
            {
                const bool is_eol = ImStb::STB_TEXTEDIT_GETCHAR(state, state->Stb->cursor) == '\n';
                state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART);
                state->OnKeyPressed(STB_TEXTEDIT_K_LINEEND | STB_TEXTEDIT_K_SHIFT);
                state->OnKeyPressed(STB_TEXTEDIT_K_RIGHT | STB_TEXTEDIT_K_SHIFT);
                state->CursorFollow = false;
            }
            state->CursorAnimReset();
        }
        else if (io.MouseClicked[0] && !state->SelectedAllMouseLock)
        {
            if (hovered)
            {
                if (io.KeyShift)
                    stb_textedit_drag(state, state->Stb, mouse_x, mouse_y);
                else
                    stb_textedit_click(state, state->Stb, mouse_x, mouse_y);
                state->CursorAnimReset();
            }
        }
        else if (io.MouseDown[0] && !state->SelectedAllMouseLock && (io.MouseDelta.x != 0.0f || io.MouseDelta.y != 0.0f))
        {
            stb_textedit_drag(state, state->Stb, mouse_x, mouse_y);
            state->CursorAnimReset();
            state->CursorFollow = true;
        }
        if (state->SelectedAllMouseLock && !io.MouseDown[0])
            state->SelectedAllMouseLock = false;

        if ((flags & ImGuiInputTextFlags_AllowTabInput) && !is_readonly)
        {
            if (Shortcut(ImGuiKey_Tab, ImGuiInputFlags_Repeat, id))
            {
                unsigned int c = '\t';
                if (input_text_filter_character(&g, &c, flags, 0, 0, false))
                    state->OnCharPressed(c);
            }
        }

        const bool ignore_char_inputs = (io.KeyCtrl && !io.KeyAlt) || (is_osx && io.KeyCtrl);
        if (io.InputQueueCharacters.Size > 0)
        {
            if (!ignore_char_inputs && !is_readonly && !input_requested_by_nav)
                for (int n = 0; n < io.InputQueueCharacters.Size; n++)
                {
                    unsigned int c = (unsigned int)io.InputQueueCharacters[n];
                    if (c == '\t')
                        continue;
                    if (input_text_filter_character(&g, &c, flags, 0, 0, false))
                        state->OnCharPressed(c);
                }

            io.InputQueueCharacters.resize(0);
        }
    }

    bool revert_edit = false;
    if (g.ActiveId == id && !g.ActiveIdIsJustActivated && !clear_active_id)
    {
        IM_ASSERT(state != NULL);

        const int row_count_per_page = ImMax((int)((inner_size.y - style.FramePadding.y) / g.FontSize), 1);
        state->Stb->row_count_per_page = row_count_per_page;

        const int k_mask = (io.KeyShift ? STB_TEXTEDIT_K_SHIFT : 0);
        const bool is_wordmove_key_down = is_osx ? io.KeyAlt : io.KeyCtrl;
        const bool is_startend_key_down = is_osx && io.KeyCtrl && !io.KeySuper && !io.KeyAlt;

        const ImGuiInputFlags f_repeat = ImGuiInputFlags_Repeat;
        const bool is_cut = (Shortcut(ImGuiMod_Ctrl | ImGuiKey_X, f_repeat, id) || Shortcut(ImGuiMod_Shift | ImGuiKey_Delete, f_repeat, id)) && !is_readonly && !is_password && state->HasSelection();
        const bool is_copy = (Shortcut(ImGuiMod_Ctrl | ImGuiKey_C, 0, id) || Shortcut(ImGuiMod_Ctrl | ImGuiKey_Insert, 0, id)) && !is_password && state->HasSelection();
        const bool is_paste = (Shortcut(ImGuiMod_Ctrl | ImGuiKey_V, f_repeat, id) || Shortcut(ImGuiMod_Shift | ImGuiKey_Insert, f_repeat, id)) && !is_readonly;
        const bool is_undo = (Shortcut(ImGuiMod_Ctrl | ImGuiKey_Z, f_repeat, id)) && !is_readonly && is_undoable;
        const bool is_redo = (Shortcut(ImGuiMod_Ctrl | ImGuiKey_Y, f_repeat, id) || (is_osx && Shortcut(ImGuiMod_Ctrl | ImGuiMod_Shift | ImGuiKey_Z, f_repeat, id))) && !is_readonly && is_undoable;
        const bool is_select_all = Shortcut(ImGuiMod_Ctrl | ImGuiKey_A, 0, id);

        const bool nav_gamepad_active = (io.ConfigFlags & ImGuiConfigFlags_NavEnableGamepad) != 0 && (io.BackendFlags & ImGuiBackendFlags_HasGamepad) != 0;
        const bool is_enter_pressed = IsKeyPressed(ImGuiKey_Enter, true) || IsKeyPressed(ImGuiKey_KeypadEnter, true);
        const bool is_gamepad_validate = nav_gamepad_active && (IsKeyPressed(ImGuiKey_NavGamepadActivate, false) || IsKeyPressed(ImGuiKey_NavGamepadInput, false));
        const bool is_cancel = Shortcut(ImGuiKey_Escape, f_repeat, id) || (nav_gamepad_active && Shortcut(ImGuiKey_NavGamepadCancel, f_repeat, id));

        if (IsKeyPressed(ImGuiKey_LeftArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINESTART : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDLEFT : STB_TEXTEDIT_K_LEFT) | k_mask); }
        else if (IsKeyPressed(ImGuiKey_RightArrow)) { state->OnKeyPressed((is_startend_key_down ? STB_TEXTEDIT_K_LINEEND : is_wordmove_key_down ? STB_TEXTEDIT_K_WORDRIGHT : STB_TEXTEDIT_K_RIGHT) | k_mask); }
        else if (IsKeyPressed(ImGuiKey_Home)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTSTART | k_mask : STB_TEXTEDIT_K_LINESTART | k_mask); }
        else if (IsKeyPressed(ImGuiKey_End)) { state->OnKeyPressed(io.KeyCtrl ? STB_TEXTEDIT_K_TEXTEND | k_mask : STB_TEXTEDIT_K_LINEEND | k_mask); }
        else if (IsKeyPressed(ImGuiKey_Delete) && !is_readonly && !is_cut)
        {
            if (!state->HasSelection())
            {
                if (is_wordmove_key_down)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDRIGHT | STB_TEXTEDIT_K_SHIFT);
            }
            state->OnKeyPressed(STB_TEXTEDIT_K_DELETE | k_mask);
        }
        else if (IsKeyPressed(ImGuiKey_Backspace) && !is_readonly)
        {
            if (!state->HasSelection())
            {
                if (is_wordmove_key_down)
                    state->OnKeyPressed(STB_TEXTEDIT_K_WORDLEFT | STB_TEXTEDIT_K_SHIFT);
                else if (is_osx && io.KeyCtrl && !io.KeyAlt && !io.KeySuper)
                    state->OnKeyPressed(STB_TEXTEDIT_K_LINESTART | STB_TEXTEDIT_K_SHIFT);
            }
            state->OnKeyPressed(STB_TEXTEDIT_K_BACKSPACE | k_mask);
        }
        else if (is_enter_pressed || is_gamepad_validate)
        {
            bool ctrl_enter_for_new_line = (flags & ImGuiInputTextFlags_CtrlEnterForNewLine) != 0;
            if (is_gamepad_validate || (ctrl_enter_for_new_line && !io.KeyCtrl) || (!ctrl_enter_for_new_line && io.KeyCtrl))
            {
                validated = true;
                if (io.ConfigInputTextEnterKeepActive)
                    state->SelectAll();
                else
                    clear_active_id = true;
            }
            else if (!is_readonly)
            {
                unsigned int c = '\n';
                if (input_text_filter_character(&g, &c, flags, 0, 0, false))
                    state->OnCharPressed(c);
            }
        }
        else if (is_cancel)
        {
            if (flags & ImGuiInputTextFlags_EscapeClearsAll)
            {
                if (buf[0] != 0)
                {
                    revert_edit = true;
                }
                else
                {
                    render_cursor = render_selection = false;
                    clear_active_id = true;
                }
            }
            else
            {
                clear_active_id = revert_edit = true;
                render_cursor = render_selection = false;
            }
        }
        else if (is_undo || is_redo)
        {
            state->OnKeyPressed(is_undo ? STB_TEXTEDIT_K_UNDO : STB_TEXTEDIT_K_REDO);
            state->ClearSelection();
        }
        else if (is_select_all)
        {
            state->SelectAll();
            state->CursorFollow = true;
        }
        else if (is_cut || is_copy)
        {
            if (g.PlatformIO.Platform_SetClipboardTextFn != NULL)
            {
                const int ib = state->HasSelection() ? ImMin(state->Stb->select_start, state->Stb->select_end) : 0;
                const int ie = state->HasSelection() ? ImMax(state->Stb->select_start, state->Stb->select_end) : state->TextLen;

                char backup = state->TextA.Data[ie];
                state->TextA.Data[ie] = 0;
                SetClipboardText(state->TextA.Data + ib);
                state->TextA.Data[ie] = backup;
            }
            if (is_cut)
            {
                if (!state->HasSelection())
                    state->SelectAll();
                state->CursorFollow = true;
                stb_textedit_cut(state, state->Stb);
            }
        }
        else if (is_paste)
        {
            if (const char* clipboard = GetClipboardText())
            {
                const int clipboard_len = (int)strlen(clipboard);
                char* clipboard_filtered = (char*)IM_ALLOC(clipboard_len + 1);
                int clipboard_filtered_len = 0;
                for (const char* s = clipboard; *s != 0; )
                {
                    unsigned int c;
                    int len = ImTextCharFromUtf8(&c, s, NULL);
                    s += len;
                    if (!input_text_filter_character(&g, &c, flags, 0, 0, true))
                        continue;
                    memcpy(clipboard_filtered + clipboard_filtered_len, s - len, len);
                    clipboard_filtered_len += len;
                }
                clipboard_filtered[clipboard_filtered_len] = 0;
                if (clipboard_filtered_len > 0)
                {
                    stb_textedit_paste(state, state->Stb, clipboard_filtered, clipboard_filtered_len);
                    state->CursorFollow = true;
                }
                MemFree(clipboard_filtered);
            }
        }

        render_selection |= state->HasSelection() && (RENDER_SELECTION_WHEN_INACTIVE || render_cursor);
    }

    const char* apply_new_text = NULL;
    int apply_new_text_length = 0;
    if (g.ActiveId == id)
    {
        IM_ASSERT(state != NULL);
        if (revert_edit && !is_readonly)
        {
            if (flags & ImGuiInputTextFlags_EscapeClearsAll)
            {
                IM_ASSERT(buf[0] != 0);
                apply_new_text = "";
                apply_new_text_length = 0;
                value_changed = true;
                IMSTB_TEXTEDIT_CHARTYPE empty_string;
                stb_textedit_replace(state, state->Stb, &empty_string, 0);
            }
            else if (strcmp(buf, state->TextToRevertTo.Data) != 0)
            {
                apply_new_text = state->TextToRevertTo.Data;
                apply_new_text_length = state->TextToRevertTo.Size - 1;

                value_changed = true;
                stb_textedit_replace(state, state->Stb, state->TextToRevertTo.Data, state->TextToRevertTo.Size - 1);
            }
        }

        const bool apply_edit_back_to_user_buffer = !revert_edit || (validated && (flags & ImGuiInputTextFlags_EnterReturnsTrue) != 0);
        if (apply_edit_back_to_user_buffer)
        {
            if ((flags & (ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory | ImGuiInputTextFlags_CallbackEdit | ImGuiInputTextFlags_CallbackAlways)) != 0)
            {
                IM_ASSERT(callback != NULL);

                ImGuiInputTextFlags event_flag = 0;
                ImGuiKey event_key = ImGuiKey_None;
                if ((flags & ImGuiInputTextFlags_CallbackCompletion) != 0 && Shortcut(ImGuiKey_Tab, 0, id))
                {
                    event_flag = ImGuiInputTextFlags_CallbackCompletion;
                    event_key = ImGuiKey_Tab;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressed(ImGuiKey_UpArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_UpArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackHistory) != 0 && IsKeyPressed(ImGuiKey_DownArrow))
                {
                    event_flag = ImGuiInputTextFlags_CallbackHistory;
                    event_key = ImGuiKey_DownArrow;
                }
                else if ((flags & ImGuiInputTextFlags_CallbackEdit) && state->Edited)
                {
                    event_flag = ImGuiInputTextFlags_CallbackEdit;
                }
                else if (flags & ImGuiInputTextFlags_CallbackAlways)
                {
                    event_flag = ImGuiInputTextFlags_CallbackAlways;
                }

                if (event_flag)
                {
                    ImGuiInputTextCallbackData callback_data;
                    callback_data.Ctx = &g;
                    callback_data.EventFlag = event_flag;
                    callback_data.Flags = flags;
                    callback_data.UserData = callback_user_data;

                    state->CallbackTextBackup.resize(state->TextLen + 1);
                    memcpy(state->CallbackTextBackup.Data, state->TextA.Data, state->TextLen + 1);

                    char* callback_buf = is_readonly ? buf : state->TextA.Data;
                    callback_data.EventKey = event_key;
                    callback_data.Buf = callback_buf;
                    callback_data.BufTextLen = state->TextLen;
                    callback_data.BufSize = state->BufCapacity;
                    callback_data.BufDirty = false;

                    const int utf8_cursor_pos = callback_data.CursorPos = state->Stb->cursor;
                    const int utf8_selection_start = callback_data.SelectionStart = state->Stb->select_start;
                    const int utf8_selection_end = callback_data.SelectionEnd = state->Stb->select_end;

                    callback(&callback_data);

                    callback_buf = is_readonly ? buf : state->TextA.Data;
                    IM_ASSERT(callback_data.Buf == callback_buf);
                    IM_ASSERT(callback_data.BufSize == state->BufCapacity);
                    IM_ASSERT(callback_data.Flags == flags);
                    const bool buf_dirty = callback_data.BufDirty;
                    if (callback_data.CursorPos != utf8_cursor_pos || buf_dirty) { state->Stb->cursor = callback_data.CursorPos; state->CursorFollow = true; }
                    if (callback_data.SelectionStart != utf8_selection_start || buf_dirty) { state->Stb->select_start = (callback_data.SelectionStart == callback_data.CursorPos) ? state->Stb->cursor : callback_data.SelectionStart; }
                    if (callback_data.SelectionEnd != utf8_selection_end || buf_dirty) { state->Stb->select_end = (callback_data.SelectionEnd == callback_data.SelectionStart) ? state->Stb->select_start : callback_data.SelectionEnd; }
                    if (buf_dirty)
                    {
                        IM_ASSERT(callback_data.BufTextLen == (int)strlen(callback_data.Buf));
                        input_text_reconcile_undo_state_after_user_callback(state, callback_data.Buf, callback_data.BufTextLen);
                        state->TextLen = callback_data.BufTextLen;
                        state->TextA.Size = state->TextLen + 1;
                        state->CursorAnimReset();
                    }
                }
            }

            if (!is_readonly && strcmp(state->TextA.Data, buf) != 0)
            {
                apply_new_text = state->TextA.Data;
                apply_new_text_length = state->TextLen;
                value_changed = true;
            }
        }
    }

    if (g.InputTextDeactivatedState.ID == id)
    {
        if (g.ActiveId != id && IsItemDeactivatedAfterEdit() && !is_readonly && strcmp(g.InputTextDeactivatedState.TextA.Data, buf) != 0)
        {
            apply_new_text = g.InputTextDeactivatedState.TextA.Data;
            apply_new_text_length = g.InputTextDeactivatedState.TextA.Size - 1;
            value_changed = true;
        }
        g.InputTextDeactivatedState.ID = 0;
    }

    if (apply_new_text != NULL)
    {
        IM_ASSERT(apply_new_text_length >= 0);
        if (is_resizable)
        {
            ImGuiInputTextCallbackData callback_data;
            callback_data.Ctx = &g;
            callback_data.EventFlag = ImGuiInputTextFlags_CallbackResize;
            callback_data.Flags = flags;
            callback_data.Buf = buf;
            callback_data.BufTextLen = apply_new_text_length;
            callback_data.BufSize = ImMax(buf_size, apply_new_text_length + 1);
            callback_data.UserData = callback_user_data;
            callback(&callback_data);
            buf = callback_data.Buf;
            buf_size = callback_data.BufSize;
            apply_new_text_length = ImMin(callback_data.BufTextLen, buf_size - 1);
            IM_ASSERT(apply_new_text_length <= buf_size);
        }
        ImStrncpy(buf, apply_new_text, ImMin(apply_new_text_length + 1, buf_size));
    }

    if (g.ActiveId == id && clear_active_id)
        ClearActiveID();
    else if (g.ActiveId == id)
        g.WantTextInputNextFrame = 1;

    if (render_cursor)
        SetMouseCursor(ImGuiMouseCursor_TextInput);

    RenderNavHighlight(total_bb, id);

    const ImVec4 clip_rect(bb.Min.x, bb.Min.y, bb.Max.x, bb.Max.y);
    ImVec2 draw_pos = ImVec2(bb.Min.x, bb.GetCenter().y - gui->text_size(font->get("medium13"), buf).y * 0.5f);
    ImVec2 text_size(0.0f, 0.0f);

    const int buf_display_max_length = 2 * 1024 * 1024;
    const char* buf_display = buf_display_from_state ? state->TextA.Data : buf;
    const char* buf_display_end = NULL;

    const tween_id cursor_tween_id = tween_id(id, "input_cursor");

    auto& active_tweens = get_tweens<float>();
    static std::unordered_map<ImGuiID, bool> cursors_moved_data;
    static std::unordered_map<ImGuiID, ImVec4> cursors_y_data;
    static std::unordered_map<ImGuiID, bool> cursors_focus_data;
    bool skip_animation = (!cursors_focus_data[id] && (render_cursor || render_selection));
    cursors_focus_data[id] = (render_cursor || render_selection);

    if (render_cursor || render_selection)
    {

        IM_ASSERT(state != NULL);
        if (!is_displaying_hint)
            buf_display_end = buf_display + state->TextLen;

        const char* text_begin = state->TextA.Data;
        const char* text_end = text_begin + state->TextLen;
        ImVec2 cursor_offset, select_start_offset;
        {
            int cursor_line_no = render_cursor ? -1 : -1000;
            int selmin_line_no = render_selection ? -1 : -1000;
            const char* cursor_ptr = render_cursor ? text_begin + state->Stb->cursor : NULL;
            const char* selmin_ptr = render_selection ? text_begin + ImMin(state->Stb->select_start, state->Stb->select_end) : NULL;

            int line_count = 1;
            if (cursor_line_no == -1)
                cursor_line_no = line_count;
            if (selmin_line_no == -1)
                selmin_line_no = line_count;

            cursor_offset.x = input_text_calc_text_size(&g, ImStrbol(cursor_ptr, text_begin), cursor_ptr, nullptr, nullptr, false).x;
            if (cursor_offset.x != input_text_calc_text_size(&g, ImStrbol(text_end, text_begin), text_end, nullptr, nullptr, false).x)
                is_mouse_moved = true;

            cursor_offset.y = cursor_line_no * g.FontSize;
            if (selmin_line_no >= 0)
            {
                select_start_offset.x = input_text_calc_text_size(&g, ImStrbol(selmin_ptr, text_begin), selmin_ptr, nullptr, nullptr, false).x;
                select_start_offset.y = selmin_line_no * g.FontSize;
            }
        }

        if (render_cursor && state->CursorFollow)
        {
            if (!(flags & ImGuiInputTextFlags_NoHorizontalScroll))
            {
                const float scroll_increment_x = inner_size.x * 0.25f;
                const float visible_width = bb.GetWidth();
                if (cursor_offset.x < state->Scroll.x)
                    state->Scroll.x = IM_TRUNC(ImMax(0.0f, cursor_offset.x - scroll_increment_x));
                else if (cursor_offset.x - visible_width >= state->Scroll.x)
                    state->Scroll.x = IM_TRUNC(cursor_offset.x - visible_width + scroll_increment_x);
            }
            else
            {
                state->Scroll.y = 0.0f;
            }

            state->CursorFollow = false;
        }

        // Draw selection

        const ImVec2 draw_scroll = ImVec2(state->Scroll.x, 0.0f);
        if (render_selection)
        {
            const char* text_selected_begin = text_begin + ImMin(state->Stb->select_start, state->Stb->select_end);
            const char* text_selected_end = text_begin + ImMax(state->Stb->select_start, state->Stb->select_end);

            ImU32 bg_color = gui->u32(var->style.colors[style_col_text_selected_bg], render_cursor ? 0.8f : 0.3f);
            float bg_offy_up = -1.0f;
            float bg_offy_dn = 2.0f;
            ImVec2 rect_pos = draw_pos + select_start_offset - draw_scroll;
            for (const char* p = text_selected_begin; p < text_selected_end; )
            {
                if (rect_pos.y > clip_rect.w + g.FontSize)
                    break;
                if (rect_pos.y < clip_rect.y)
                {
                    p = (const char*)memchr((void*)p, '\n', text_selected_end - p);
                    p = p ? p + 1 : text_selected_end;
                }
                else
                {
                    ImVec2 rect_size = input_text_calc_text_size(&g, p, text_selected_end, &p, NULL, true);
                    if (rect_size.x <= 0.0f) rect_size.x = IM_TRUNC(g.Font->GetCharAdvance((ImWchar)' ') * 0.50f);
                    ImRect rect(rect_pos + ImVec2(0.0f, bg_offy_up - g.FontSize), rect_pos + ImVec2(rect_size.x, bg_offy_dn));
                    rect.ClipWith(clip_rect);
                    if (rect.Overlaps(clip_rect))
                        draw_window->DrawList->AddRectFilled(rect.Min, rect.Max, bg_color);
                    rect_pos.x = draw_pos.x - draw_scroll.x;
                }
                rect_pos.y += g.FontSize;
            }
        }

        const ImVec2 cursor_screen_pos = ImTrunc(draw_pos + cursor_offset - draw_scroll);

        const float target_val = clip_rect.x + CalcTextSize(buf_display, buf_display_end).x;
        float clip_x = target_val;
        if (auto it = active_tweens.find(cursor_tween_id); !render_selection && !is_mouse_moved && it != active_tweens.end())
        {
            if (!cursors_moved_data[id])
                clip_x = it->second.last_value;
            if (skip_animation)
            {
                it->second.elapsed_time = 200ms;
                it->second.last_value = cursor_screen_pos.x;
                clip_x = cursor_screen_pos.x;
            }
        }

        // Draw text

        if ((buf_display_end - buf_display) < buf_display_max_length)
        {
            draw->text(drawlist, g.Font, g.FontSize, draw_pos - draw_scroll, gui->u32(col->text), buf_display, buf_display_end, 0.0f, &clip_rect);
        }

        // Draw blinking cursor
        if (render_cursor)
        {
            state->CursorAnim += io.DeltaTime;
            bool cursor_is_visible = (!g.IO.ConfigInputTextCursorBlink) || (state->CursorAnim <= 0.0f) || ImFmod(state->CursorAnim, 1.20f) <= 0.80f;

            if (auto it = active_tweens.find(cursor_tween_id); it != active_tweens.end()) {
                auto& active_tween = it->second;

                if (is_mouse_moved)
                    cursors_moved_data[id] = true;
                else {
                    if (cursors_moved_data[id]) {
                        if (active_tween.last_value >= cursor_screen_pos.x - (cursor_screen_pos.x * 0.025f))
                            cursors_moved_data[id] = false;
                    }
                }
            }

            float move = tween(cursor_tween_id, draw_pos.x, cursor_screen_pos.x, easing::QuadEaseInOut, 200ms);

            if (skip_animation)
            {
                auto it = active_tweens.find(cursor_tween_id);
                it->second.elapsed_time = 200ms;
                it->second.last_value = cursor_screen_pos.x;
                move = cursor_screen_pos.x;
            }

            ImRect cursor_screen_rect(move, cursor_screen_pos.y - g.FontSize + 0.5f, move + 1.0f, cursor_screen_pos.y - 1.5f);

            if (var->window.target_page == var->window.current_page && cursor_is_visible && cursor_screen_rect.Overlaps(clip_rect))
                draw_window->DrawList->AddLine(cursor_screen_rect.Min, cursor_screen_rect.GetBL(), gui->u32(col->text));
            cursors_y_data[id] = ImVec4(cursor_screen_rect.Min.y, cursor_screen_rect.GetBL().y, cursor_screen_pos.x, 0.f);


            if (!is_readonly)
            {
                g.PlatformImeData.WantVisible = true;
                g.PlatformImeData.InputPos = ImVec2(cursor_screen_pos.x - 1.0f, cursor_screen_pos.y - g.FontSize);
                g.PlatformImeData.InputLineHeight = g.FontSize;
            }
        }
    }
    else
    {
        if (!is_displaying_hint && g.ActiveId == id)
            buf_display_end = buf_display + state->TextLen;
        else if (!is_displaying_hint)
            buf_display_end = buf_display + strlen(buf_display);

        // Draw text

        if (buf[0] != '\0' && (buf_display_end - buf_display) < buf_display_max_length)
        {
            draw->text(drawlist, g.Font, g.FontSize, draw_pos, gui->u32(col->text), buf_display, buf_display_end, 0.0f, &clip_rect);
        }

        if (auto it = active_tweens.find(cursor_tween_id); it != active_tweens.end()) {
            auto& cursors_data = cursors_y_data[id];

            if (IsMouseDragging(ImGuiMouseButton_Left))
                cursors_data.z = 1.f;

            if (cursors_data.z != 1.f) {
                float move = tween(cursor_tween_id, draw_pos.x, draw_pos.x, easing::QuadEaseInOut, 200ms);

                if (var->gui.dpi_changed) {
                    auto it = active_tweens.find(cursor_tween_id);
                    it->second.elapsed_time = 200ms;
                    it->second.last_value = it->second.to;
                    move = draw_pos.x;
                }

                if (!var->gui.dpi_changed && var->window.target_page == var->window.current_page && !IsMouseDragging(ImGuiMouseButton_Left) && move != draw_pos.x) {
                    //draw_window->DrawList->AddLine(ImVec2(move, cursors_data.x), ImVec2(move + 1.f, cursors_data.y), gui->u32(text_col));
                }
            }
        }
    }

    if (is_password && !is_displaying_hint)
        PopFont();

    if (g.LogEnabled && (!is_password || is_displaying_hint))
    {
        LogSetNextTextDecoration("{", "}");
        LogRenderedText(&draw_pos, buf_display, buf_display_end);
    }

    if (value_changed && !(flags & ImGuiItemFlags_NoMarkEdited))
        MarkItemEdited(id);

    gui->pop_font();

    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags | ImGuiItemStatusFlags_Inputable);
    return value_changed;
}

bool hex_input(const char* str_id, float color[4], char* buf, int buf_size, float height)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(str_id);

    std::string_view buf_str(buf, buf_size);
    const ImVec2 text_size = gui->text_size(font->get("medium13"), buf_str.data());

    const float rect_roudning = scale(4);
    const ImVec2 padding = scale(5, 5);
    const ImVec2 color_rect_size = ImVec2(height - padding.y * 2.f, height - padding.y * 2.f);
    const ImVec2 input_size = ImVec2(padding.x * 4.f + text_size.x + color_rect_size.x, height);
    const ImVec2 input_area_size = ImVec2(input_size.x - padding.x * 2.f - color_rect_size.x, height - padding.y * 2.f);

    const ImVec2 cursor_pos = gui->get_pos();
    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect input_bb(ImVec2(bb.Max.x - input_size.x, bb.Min.y), bb.Max);
    gui->item_size(bb, var->style.frame_padding.y);
    
    draw->text_clipped(drawlist, font->get("medium13"), bb.Min, bb.Max, gui->u32(col->text), "Hex", nullptr, nullptr, {0.f, 0.5f});

    draw->rect_filled(drawlist, input_bb.Min, input_bb.Max, gui->u32(col->black), rect_roudning);

    float grid_step = ImMin(bb.GetWidth(), bb.GetHeight()) / 2.99f;
    render_color_rect_alpha_checkboard(window->DrawList, input_bb.Min + padding, input_bb.Min + padding + color_rect_size, 0, grid_step, ImVec2(0.0f, 0.0f), rect_roudning, 0);

    draw->rect_filled(drawlist, input_bb.Min + padding, input_bb.Min + padding + color_rect_size, gui->u32(ImVec4(color[0], color[1], color[2], color[3])), rect_roudning);

    if (ImGui::IsKeyPressed(ImGuiKey_MouseLeft) && input_bb.Contains(g.IO.MousePos))
    {
        gui->popup_skip_click();
    }

    gui->set_pos(cursor_pos + input_bb.Min + padding + ImVec2(color_rect_size.x + padding.x, 0) - pos, pos_all);
    return input_raw(str_id, buf, buf_size, input_area_size, ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_CharsUppercase);
}
}

bool c_widgets::color_only(const char* label, const char* desc, ImVec4& color) {
    float color_float[4] = { color.x, color.y, color.z, color.w };
    const bool value_changed = color_only(label, desc, color_float);
    color = ImVec4(color_float[0], color_float[1], color_float[2], color_float[3]);
    return value_changed;
}

bool c_widgets::color_only(const char* label, const char* desc, float color[4]) {
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    const float height = scale(elements->window.widgets_height);
    const ImVec2 colorbutton_size = scale(15, 15);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect colorbutton_bb(ImVec2(bb.Max.x - colorbutton_size.x, bb.GetCenter().y - colorbutton_size.y * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + colorbutton_size.y * 0.5f));

    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    bool value_changed = false;

    const ImGuiID colorbutton_id = ImHashStr("colorbutton_bb", 0, id);
    gui->item_add(colorbutton_bb, colorbutton_id);

    const bool colorbutton_pressed = gui->button_behavior(colorbutton_bb, colorbutton_id);
    if (colorbutton_pressed)
    {
        gui->open_popup(label);
    }

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));
    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->circle_filled(drawlist, colorbutton_bb.GetCenter(), colorbutton_size.x * 0.5f, gui->u32(ImVec4(color[0], color[1], color[2], 1.f)), 100);

    color_popup(id, label, color, colorbutton_bb.GetBL());

    return value_changed;
}
