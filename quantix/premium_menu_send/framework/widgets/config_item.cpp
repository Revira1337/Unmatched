#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../settings/elements.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

config_item_return c_widgets::config_item(const char* name, date date, bool is_actived)
{
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return {};

    const ImVec2 delete_button_size = scale(74, 24);
    const ImVec2 save_load_button_size = scale(64, 24);

    const float height = scale(40);
    const float rect_roudning = scale(8);
    const float name_date_spacing_y = scale(5);
    const float icon_text_spacing_x = scale(10);
    const float buttons_spacing_x = var->style.item_inner_spacing.x;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(name);

    const ImVec2 window_start_pos = window->Pos;
    const ImVec2 target_pos = window->DC.CursorPos;

    static std::unordered_map<ImGuiID, ImVec2> config_entries_states;
    auto [it, inserted] = config_entries_states.try_emplace(id, window->Pos);
    ImVec2& last_window_pos = it->second;

    const ImVec2 pos = tween({ id, "config_item_move" }, last_window_pos, target_pos, easing::QuadEaseInOut, 200ms);
    if (var->window.window_moving || var->gui.dpi_changed || var->window.last_window_scrolling || (last_window_pos != window->Pos))
    {
        finish_tween<ImVec2>({ id, "config_item_move" });
    }
    const ImVec2 cursor_pos = pos - window->Pos + window->Scroll;

    last_window_pos = window->Pos;

    const ImVec2 size(gui->content_avail().x, height);
    const ImRect bb(pos, pos + size);
    const ImRect icon_bb(bb.Min, bb.Min + ImVec2(bb.GetHeight(), bb.GetHeight()));

    config_item_return return_value;
    
    gui->item_size(bb);

    const ImVec2 next_cursor_pos = gui->get_pos();

    const float alpha = tween({ id, "config_item_alpha" }, 0.f, 1.f, easing::QuadEaseInOut, 250ms);

    draw->rect(drawlist, icon_bb.Min, icon_bb.Max, gui->u32(col->primary, 0.05f * alpha), rect_roudning, 0, 0.5f);

    draw->text_clipped(drawlist, font->get("icons20"), icon_bb.Min, icon_bb.Max, gui->u32(col->text_dimmed), ICON_FILE, nullptr, nullptr, { 0.5f, 0.5f });

    const std::string date_str = "Created on " + std::to_string(date.day) + '.' + std::to_string(date.month) + '.' + std::to_string(date.year);
    const ImVec2 name_size = gui->text_size(font->get("semibold13"), name);
    const ImVec2 date_size = gui->text_size(font->get("medium11"), date_str.c_str());
    const float text_full_height = name_size.y + name_date_spacing_y + date_size.y;

    const ImVec2 name_pos = ImVec2(icon_bb.Max.x + icon_text_spacing_x, bb.GetCenter().y - text_full_height * 0.5f);
    draw->text(drawlist, font->get("semibold13"), name_pos, gui->u32(col->text, alpha), name);

    const ImVec2 date_pos = name_pos + ImVec2(0, name_size.y + name_date_spacing_y);
    draw->text(drawlist, font->get("medium11"), date_pos, gui->u32(col->text_dimmed, alpha), date_str.c_str());

    const ImVec2 delete_button_pos = ImVec2(bb.Max.x - save_load_button_size.x - buttons_spacing_x - delete_button_size.x, bb.GetCenter().y - delete_button_size.y * 0.5f);
    gui->set_pos(cursor_pos + delete_button_pos - pos, pos_all);
    if (widgets->icon_button(ICON_TRASH, ("Delete##"s + name).c_str(), delete_button_size))
    {
        return_value = { .deleted = true };
    }

    const ImVec2 save_load_button_pos = delete_button_pos + ImVec2(delete_button_size.x + buttons_spacing_x, 0);
    if (is_actived)
    {
        gui->set_pos(cursor_pos + save_load_button_pos - pos, pos_all);
        if (widgets->icon_button(ICON_BACKGROUND, "Save", save_load_button_size, true, ("SaveLoad##"s + name).c_str()))
        {
            return_value = { .saved = true };
        }
    }
    else
    {
        gui->set_pos(cursor_pos + save_load_button_pos - pos, pos_all);
        if (widgets->icon_button(ICON_DOWNLOAD, "Load", save_load_button_size, false, ("SaveLoad##"s + name).c_str()))
        {
            return_value = { .loaded = true };
        }
    }

    gui->set_pos(next_cursor_pos, pos_all);

    return return_value;
}

void destroy_config_entry(ImGuiWindow* window, const char* name)
{
    const ImGuiID id = window->GetID(name);
    reset_tween<ImVec2>({ id, "config_item_move", });
    reset_tween<float>({ id, "config_item_alpha", });
}