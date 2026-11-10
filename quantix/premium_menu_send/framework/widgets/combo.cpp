#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"
#include "../settings/elements.h"

bool selectable(const char* label, bool selected, const ImVec2& size_arg, ImGuiSelectableFlags flags = 0)
{
    ImGuiWindow* window = gui->get_window();
    if (window->SkipItems)
        return false;

    const ImVec2 padding = scale(4, 4);
    const float rect_rounding = scale(4);

    ImGuiContext& g = *GImGui;
    ImDrawList* drawlist = window->DrawList;

    ImGuiID id = window->GetID(label);
    ImVec2 label_size = gui->text_size(g.Font, label, NULL, true);
    flags |= ImGuiSelectableFlags_SpanAvailWidth;
    ImVec2 size(gui->content_avail().x, size_arg.y != 0.0f ? size_arg.y : label_size.y);
    ImVec2 pos = window->DC.CursorPos;
    pos.y += window->DC.CurrLineTextBaseOffset;
    gui->item_size(size, 0.0f);
    ImRect bb(pos, pos + size);

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const ImGuiItemFlags extra_item_flags = disabled_item ? (ImGuiItemFlags)ImGuiItemFlags_Disabled : ImGuiItemFlags_None;
    bool is_visible = gui->item_add(bb, id, NULL, extra_item_flags);

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global)
        ImGui::BeginDisabled();

    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }

    bool hovered, held;
    bool pressed = gui->button_behavior(bb, id, &hovered, &held, button_flags);

    if (pressed)
        ImGui::MarkItemEdited(id);

    if (is_visible) {
        const float anim = tween({ id, "selectable_anim" }, 0.f, 1.f, easing::BackEaseInOut, 250ms, selected);
        const ImVec4 text_col = tween({ id, "selectable_text_col" }, col->text_dimmed, col->text, easing::QuadEaseIn, 200ms, hovered || selected);

        draw->rect_filled(drawlist, bb.Min, ImVec2(bb.Min.x + bb.GetWidth() * anim, bb.Max.y), gui->u32(col->gray_4, anim), rect_rounding);

        draw->text_clipped(drawlist, g.Font, bb.Min + padding, bb.Max - padding, gui->u32(text_col), label, nullptr, &label_size, ImVec2(0.0f, 0.5f), &bb);
    }

    if (pressed && !(flags & ImGuiSelectableFlags_NoAutoClosePopups))
        gui->close_current_popup();

    if (disabled_item && !disabled_global)
        ImGui::EndDisabled();

    return pressed;
}


bool begin_combo(const char* label, const char* desc, const char* preview_label, size_t items_count, float max_preview_width, ImGuiComboFlags flags = 0)
{
    const float height = scale(elements->window.widgets_height);
    const float rect_height = scale(elements->window.widgets_rect_height);
    const float preview_icon_spacing_x = scale(6);
    const float rect_rounding = scale(6);
    const ImVec2 rect_padding = ImVec2(var->style.item_inner_spacing.x, var->style.item_inner_spacing.y * 0.5f);

    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = gui->get_window();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.HasFlags;
    g.NextWindowData.ClearFlags();

    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 pos = window->DC.CursorPos;
    ImDrawList* drawlist = window->DrawList;

    const ImVec2 preview_size = gui->text_size(font->get("medium13"), preview_label);
    const ImVec2 icon_size = gui->text_size(font->get("icons14"), ICON_LINE_HEIGHT);

    ImVec2 rect_size = ImVec2(rect_padding.x * 2.f + preview_size.x + preview_icon_spacing_x + icon_size.x, rect_height);

    const float rect_width = tween({ id, "combo_rect_width" }, rect_size.x, rect_size.x, easing::QuadEaseInOut, 200ms);
    rect_size.x = rect_width;

    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect rect_bb(ImVec2(bb.Max.x - rect_size.x, bb.GetCenter().y - rect_size.y * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + rect_size.y * 0.5f));

    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(rect_bb, id))
        return false;

    bool hovered, held;
    bool pressed = gui->button_behavior(rect_bb, id, &hovered, &held);
    if (pressed) {
        gui->open_popup(label);
        gui->popup_skip_click();
    }

    bool popup_open = gui->is_popup_open(label);
    
    static std::unordered_map<const char*, const char*> previews;
    auto [preview, success] = previews.try_emplace(label, preview_label);
    const float preview_switch = tween({ id, "combo_preview" }, 0.f, 1.f, easing::QuadEaseInOut, 150ms, preview->second == preview_label);
    if (preview_switch == 0.f)
    {
        preview->second = preview_label;
    }

    const ImVec4 text_color = tween({ id, "combo_text_color" }, col->text_dimmed, col->text, easing::QuadEaseInOut, 200ms, hovered || held || popup_open || preview_switch != 1.f);

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));
    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->rect_filled(drawlist, rect_bb.Min, rect_bb.Max, gui->u32(col->primary, 0.05f), rect_rounding);

    draw->text_clipped(drawlist, font->get("icons14"), rect_bb.Min + rect_padding, rect_bb.Max - rect_padding, gui->u32(text_color), ICON_LINE_HEIGHT, nullptr, &icon_size, ImVec2(1.f, 0.5f), &rect_bb);

    draw->text_clipped(drawlist, font->get("medium13"), rect_bb.Min + rect_padding, rect_bb.Max - rect_padding, gui->u32(text_color, preview_switch), preview->second, nullptr, nullptr, ImVec2(0.0f, 0.5f), &bb);

    g.NextWindowData.HasFlags = backup_next_window_data_flags;

    const float items_spacing_y = scale(4);
    const float item_height = scale(24);

    const float popup_width = rect_padding.x * 2.f + max_preview_width + preview_icon_spacing_x + icon_size.x + scale(16);
    const ImVec2 popup_pos = rect_bb.GetBL() + ImVec2(-ImMax(0.f, popup_width - rect_size.x), scale(3));
    const ImVec2 popup_size(popup_width, items_count * item_height + (items_count - 1) * items_spacing_y + scale(16));

    bool ret = gui->begin_popup(label, popup_pos, popup_size, popup_open_styles::top);

    return ret;
}

void end_combo(const char* str_id)
{
    gui->end_popup(str_id);
}

bool c_widgets::combo(const char* label, const char* desc, std::span<const char*> items, int& current_item) {
    const ImVec2 popup_padding = scale(8, 8);
    const float items_spacing_y = scale(4);
    const float item_height = scale(24);
    auto* selectable_font = font->get("medium13");

    float max_preview_width = 0.f;
    for (int i = 0; i < items.size(); i++)
    {
        if (float current_width = gui->text_size(selectable_font, items[i]).x; max_preview_width < current_width)
        {
            max_preview_width = current_width;
        }
    }

    bool value_changed = false;
    gui->push_var(style_var_window_padding, popup_padding);
    if (begin_combo(label, desc, items[current_item], items.size(), max_preview_width))
    {
        gui->push_var(style_var_item_spacing, ImVec2(0, items_spacing_y));
        gui->push_font(selectable_font);
        for (int i = 0; i < items.size(); i++)
        {
            gui->push_id(label);
            const bool is_selected = (current_item == i);
            if (selectable(items[i], is_selected, ImVec2(0.f, item_height))) {
                current_item = i;
                value_changed = true;
            }
            if (is_selected)
                ImGui::SetItemDefaultFocus();
            gui->pop_id();
        }
        gui->pop_font();
        gui->pop_var();
        end_combo(label);
    }
    gui->pop_var();

    if (value_changed) {
        ImGui::MarkItemEdited(GImGui->LastItemData.ID);
    }

    return value_changed;
}
