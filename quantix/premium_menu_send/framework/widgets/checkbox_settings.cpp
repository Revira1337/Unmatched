#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"
#include "../settings/elements.h"

bool c_widgets::checkbox_setting(const char* label, const char* desc, bool& value) {
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

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect checkbox_bb(ImVec2(bb.Max.x - checkbox_size.x, bb.GetCenter().y - checkbox_size.y * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + checkbox_size.y * 0.5f));

    const ImVec2 setting_icon_size = gui->text_size(font->get("icons13"), ICON_SETTINGS_FILL);
    const ImRect setting_bb(ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x - setting_icon_size.x, bb.GetCenter().y - setting_icon_size.y * 0.5f), ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x, bb.GetCenter().y + setting_icon_size.y * 0.5f));

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

    const ImGuiID setting_id = ImHashStr("setting", 0, id);
    bool setting_hovered;
    const bool setting_pressed = gui->button_behavior(setting_bb, setting_id, &setting_hovered, nullptr);
    if (setting_pressed)
    {
        ;//do something
    }

    const ImVec4 bg_color_from(col->primary.x, col->primary.y, col->primary.z, 0.1f);
    const ImVec4 circle_color_from(col->primary.x, col->primary.y, col->primary.z, 0.35f);

    const float anim = tween({ id, "checkbox_anim" }, 0.f, 1.f, easing::BackEaseInOut, 250ms, value);
    const ImVec4 bg_color = tween({ id, "checkbox_bg_color" }, bg_color_from, col->primary, easing::QuadEaseInOut, 200ms, value);
    const ImVec4 circle_color = tween({ id, "checkbox_circle_color" }, circle_color_from, col->black, easing::QuadEaseInOut, 200ms, value);
    const ImVec4 setting_color = tween({ id, "checkbox_setting_color" }, col->text_dimmed, col->text, easing::QuadEaseInOut, 200ms, setting_hovered);

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));

    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->rect_filled(drawlist, checkbox_bb.Min, checkbox_bb.Max, gui->u32(bg_color), checkbox_roudning);

    const float circle_move_pos_x = (checkbox_bb.GetSize().x - circle_padding.x * 2.f - circle_radius * 2.f) * anim;
    draw->circle_filled(drawlist, ImVec2(checkbox_bb.Min.x + circle_padding.x + circle_radius + circle_move_pos_x, checkbox_bb.GetCenter().y), circle_radius, gui->u32(circle_color));

    draw->text(drawlist, font->get("icons13"), setting_bb.Min, gui->u32(setting_color), ICON_SETTINGS_FILL);

    return value_changed;
}