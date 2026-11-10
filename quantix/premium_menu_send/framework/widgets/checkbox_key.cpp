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
    ImGuiKey listen_key()
    {
        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
        {
            ImGuiKey current_key = static_cast<ImGuiKey>(key);
            if (ImGui::IsKeyPressed(current_key))
            {
                return current_key;
            }
        }

        return ImGuiKey_None;
    }
}

bool c_widgets::checkbox_key(const char* label, const char* desc, bool& value) {
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    auto* current_keybind = keybinds->get_bind(label);

    const ImVec2 label_size = gui->text_size(font->get("medium14"), label);

    const float checkbox_roudning = scale(10);
    const float height = scale(elements->window.widgets_height);
    const ImVec2 checkbox_size = scale(32, 19);
    const ImVec2 circle_padding = scale(4, 4);
    const float circle_radius = scale(5.5f);
    const ImVec2 keybox_padding = scale(6, 2);
    const float keybox_rounding = scale(6);
    const float keyname_icon_spacing_x = scale(6);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));
    const ImRect checkbox_bb(ImVec2(bb.Max.x - checkbox_size.x, bb.GetCenter().y - checkbox_size.y * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + checkbox_size.y * 0.5f));

    static std::unordered_map<ImGuiID, bool> keybox_states;
    bool& keybox_listen_key = keybox_states[id];

    if (current_keybind != nullptr && current_keybind->callback == nullptr)
    {
        current_keybind->callback = [&value](bool state, ImGuiKey, c_keybinds::switch_type)
            {
                value = state;
            };
    }

    bool key_changed = false;

    if (keybox_listen_key)
    {
        if (ImGuiKey key = listen_key(); key != ImGuiKey_None)
        {
            if (key == ImGuiKey_MouseLeft || key == ImGuiKey_Escape)
            {
                key = ImGuiKey_None;
                keybinds->try_erase_bind(label);
            }

            if (current_keybind != nullptr)
            {
                current_keybind->key = key;
            }
            else
            {
                current_keybind = &keybinds->try_add_bind(label);
                current_keybind->key = key;
                // Hold mode: Aimbot, Silent, Magic, Trigger, Health, Armor, Strafe Macro, Fix Vehicle Key
                // These should activate while key is held and deactivate when released
                if (strcmp(label, "Silent Key") == 0 || 
                    strcmp(label, "Aimbot key") == 0 || 
                    strcmp(label, "Magic Key") == 0 || 
                    strcmp(label, "Trigger key") == 0 ||
                    strcmp(label, "Health") == 0 ||
                    strcmp(label, "Armor") == 0 ||
                    strcmp(label, "Strafe Macro") == 0 ||
                    strcmp(label, "Fix Vehicle Key") == 0) {
                    current_keybind->type = c_keybinds::switch_type::hold;
                }
                else {
                    // Toggle mode for everything else (Godmode, Invisible, Freecam, Teleports, etc.)
                    current_keybind->type = c_keybinds::switch_type::toggle;
                }

                if (current_keybind->linked_cfg_bind)
                {
                    current_keybind->linked_cfg_bind->key = key;
                    current_keybind->linked_cfg_bind->type = current_keybind->type;
                }
            }
            keybox_listen_key = false;
            key_changed = true;
        }
    }

    const char* key_name = (keybox_listen_key ? "..." : (current_keybind != nullptr ? ImGui::GetKeyName(current_keybind->key) : "None"));
    const ImVec2 key_name_size = gui->text_size(font->get("medium13"), key_name);
    const ImVec2 keyboard_icon_size = gui->text_size(font->get("icons15"), ICON_KEYBOARD);
    const ImGuiID keybox_id = ImHashStr("##keybox", 0, id);

    const float target_keybox_width = key_name_size.x + keyname_icon_spacing_x + keyboard_icon_size.x + keybox_padding.x * 2.f;
    const float keybox_width = tween(keybox_id, keybox_padding.x * 2.f, target_keybox_width, easing::QuadEaseInOut, 200ms);
    if (var->gui.dpi_changed || var->window.window_moving) {
        finish_tween<float>(keybox_id);
    }

    const ImRect keybox_bb(ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x - keybox_width, checkbox_bb.Min.y), ImVec2(checkbox_bb.Min.x - var->style.item_inner_spacing.x, checkbox_bb.Max.y));

    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    gui->item_add(keybox_bb, keybox_id);

    bool value_changed = false;
    bool hovered, held;

    const bool pressed = gui->button_behavior(checkbox_bb, id, &hovered, &held);
    const bool keybox_pressed = (key_changed ? false : gui->button_behavior(keybox_bb, keybox_id, nullptr, nullptr));
    if (pressed && !current_keybind)
    {
        value = !value;
        value_changed = true;
    }
    if (keybox_pressed)
    {
        keybox_listen_key = true;
    }

    static std::unordered_map<ImGuiID, const char*> previews;
    auto [preview, success] = previews.try_emplace(id, key_name);
    const float preview_switch = tween({ id, "checkbox_key_preview" }, 0.f, 1.f, easing::QuadEaseInOut, 100ms, preview->second == key_name);
    if (preview_switch == 0.f)
    {
        preview->second = key_name;
    }

    const ImVec4 bg_color_from(col->primary.x, col->primary.y, col->primary.z, 0.1f);
    const ImVec4 circle_color_from(col->primary.x, col->primary.y, col->primary.z, 0.35f);

    const float anim = tween({ id, "checkbox_anim" }, 0.f, 1.f, easing::BackEaseInOut, 250ms, value);
    const ImVec4 bg_color = tween({ id, "checkbox_bg_color" }, bg_color_from, col->primary, easing::QuadEaseInOut, 200ms, value);
    const ImVec4 circle_color = tween({ id, "checkbox_circle_color" }, circle_color_from, col->black, easing::QuadEaseInOut, 200ms, value);
    const ImVec4 keybox_text_color = tween({ id, "checkbox_keybox_color" }, col->text_dimmed, col->text, easing::QuadEaseInOut, 130ms, keybox_listen_key);

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));

    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->rect_filled(drawlist, checkbox_bb.Min, checkbox_bb.Max, gui->u32(bg_color), checkbox_roudning);

    const float circle_move_pos_x = (checkbox_bb.GetSize().x - circle_padding.x * 2.f - circle_radius * 2.f) * anim;
    draw->circle_filled(drawlist, ImVec2(checkbox_bb.Min.x + circle_padding.x + circle_radius + circle_move_pos_x, checkbox_bb.GetCenter().y), circle_radius, gui->u32(circle_color));

    draw->rect_filled(drawlist, keybox_bb.Min, keybox_bb.Max, gui->u32(col->primary, 0.05f), keybox_rounding);

    draw->text_clipped(drawlist, font->get("icons14"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(keybox_text_color), ICON_KEYBOARD, nullptr, &keyboard_icon_size, ImVec2(1.f, 0.5f), &keybox_bb);

    draw->text_clipped(drawlist, font->get("medium13"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(col->text_dimmed, preview_switch), preview->second, nullptr, nullptr, ImVec2(0.0f, 0.5f), &keybox_bb);

    return value_changed;
}


bool c_widgets::keybind_only(const char* label, const char* desc) {
    ImGuiWindow* window = gui->get_window();
    ImDrawList* drawlist = window->DrawList;
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiID id = window->GetID(label);

    auto* current_keybind = keybinds->get_bind(label);

    const ImVec2 label_size = gui->text_size(font->get("medium14"), label);

    const float height = scale(elements->window.widgets_height);
    const ImVec2 keybox_padding = scale(6, 2);
    const float keybox_rounding = scale(6);
    const float keyname_icon_spacing_x = scale(6);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect bb(pos, pos + ImVec2(gui->content_avail().x, height));

    static std::unordered_map<ImGuiID, bool> keybox_states;
    bool& keybox_listen_key = keybox_states[id];

    bool key_changed = false;

    if (keybox_listen_key)
    {
        if (ImGuiKey key = listen_key(); key != ImGuiKey_None)
        {
            if (key == ImGuiKey_MouseLeft || key == ImGuiKey_Escape)
            {
                key = ImGuiKey_None;
                keybinds->try_erase_bind(label);
            }

            if (current_keybind != nullptr)
            {
                current_keybind->key = key;
            }
            else
            {
                current_keybind = &keybinds->try_add_bind(label);
                current_keybind->key = key;
                current_keybind->type = c_keybinds::switch_type::toggle;

                if (current_keybind->linked_cfg_bind)
                {
                    current_keybind->linked_cfg_bind->key = key;
                    current_keybind->linked_cfg_bind->type = current_keybind->type;
                }
            }
            keybox_listen_key = false;
            key_changed = true;
        }
    }

    const char* key_name = (keybox_listen_key ? "..." : (current_keybind != nullptr ? ImGui::GetKeyName(current_keybind->key) : "None"));
    const ImVec2 key_name_size = gui->text_size(font->get("medium13"), key_name);
    const ImVec2 keyboard_icon_size = gui->text_size(font->get("icons15"), ICON_KEYBOARD);
    const ImGuiID keybox_id = ImHashStr("##keybox", 0, id);

    const float target_keybox_width = key_name_size.x + keyname_icon_spacing_x + keyboard_icon_size.x + keybox_padding.x * 2.f;
    const float keybox_width = tween(keybox_id, keybox_padding.x * 2.f, target_keybox_width, easing::QuadEaseInOut, 200ms);
    if (var->gui.dpi_changed || var->window.window_moving) {
        finish_tween<float>(keybox_id);
    }

    const ImRect keybox_bb(ImVec2(bb.Max.x - keybox_width, bb.GetCenter().y - scale(19) * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + scale(19) * 0.5f));

    gui->item_size(bb, var->style.frame_padding.y);
    if (!gui->item_add(bb, id))
        return false;

    gui->item_add(keybox_bb, keybox_id);

    const bool keybox_pressed = (key_changed ? false : gui->button_behavior(keybox_bb, keybox_id, nullptr, nullptr));
    if (keybox_pressed)
    {
        keybox_listen_key = true;
    }

    static std::unordered_map<ImGuiID, const char*> previews;
    auto [preview, success] = previews.try_emplace(id, key_name);
    const float preview_switch = tween({ id, "keybind_only_preview" }, 0.f, 1.f, easing::QuadEaseInOut, 100ms, preview->second == key_name);
    if (preview_switch == 0.f)
    {
        preview->second = key_name;
    }

    const ImVec4 keybox_text_color = tween({ id, "keybind_only_keybox_color" }, col->text_dimmed, col->text, easing::QuadEaseInOut, 130ms, keybox_listen_key);

    draw->text(drawlist, font->get("semibold13"), bb.Min, gui->u32(col->text), gui->hide_text_hint(label));

    draw->text_clipped(drawlist, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });

    draw->rect_filled(drawlist, keybox_bb.Min, keybox_bb.Max, gui->u32(col->primary, 0.05f), keybox_rounding);

    draw->text_clipped(drawlist, font->get("icons14"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(keybox_text_color), ICON_KEYBOARD, nullptr, &keyboard_icon_size, ImVec2(1.f, 0.5f), &keybox_bb);

    draw->text_clipped(drawlist, font->get("medium13"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(col->text_dimmed, preview_switch), preview->second, nullptr, nullptr, ImVec2(0.0f, 0.5f), &keybox_bb);

    return key_changed;
}
