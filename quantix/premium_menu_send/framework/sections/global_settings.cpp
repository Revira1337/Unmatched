#include "../headers/sections.h"
#include "../../cheat/Main/Options.hpp"
#include <Windows.h>

using namespace std::string_view_literals;

namespace {
	void render_header();
	void render_menu_tab();
	void render_config_tab();
	date get_file_creation_date(const std::filesystem::path& path);
	date get_current_date();
}

void sections::render_global_settings() {
	ImDrawList* drawlist = gui->window_drawlist();
	const ImVec2 pos = gui->window_pos();
	const ImVec2 size = gui->window_size();

	const ImVec2 padding = scale(elements->window.content_padding);
	const float header_height = scale(elements->content.header_height_no_subtabs);

	const float old_alpha = var->window.alpha;
	if (var->window.subtab_switched) {
		var->window.alpha = 1.f;
	}
	gui->push_var(style_var_alpha, var->window.alpha);

	render_header();

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_menu_tab();
		break;
	case 1:
		render_config_tab();
		break;
	}
}

namespace {
	void render_header() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const ImVec2 padding = scale(elements->window.content_padding);
		const float header_icon_radius = scale(elements->content.header_icon_radius);
		const float header_desc_height = scale(elements->content.header_desc_height);
		const float header_height = scale(elements->content.header_height_no_subtabs);
		
		const float content_width = size.x - padding.x * 2.f;
		const float circle_text_spacing_x = scale(12);

		const char* icon = ICON_SETTINGS;
		const char* name = "Global Settings";
		const char* desc = "Menu settings, configurations and preferences";

		const ImVec2 top_animation_pos = ImVec2(0.f, header_height * (1.f - var->style.alpha));

		const ImVec2 content_start_pos = pos + padding;
		const ImRect header_rect(content_start_pos - top_animation_pos, content_start_pos - ImVec2(0.f, top_animation_pos.y) + ImVec2(content_width, header_desc_height));

		{
			const ImVec2 circle_center_pos = header_rect.Min + ImVec2(header_icon_radius, header_desc_height * 0.5f);
			draw->circle_filled(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_3));
			draw->circle(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_5), 0, var->style.child_border_size);

			draw->text_clipped(drawlist, font->get("icons16"), circle_center_pos - ImVec2(header_icon_radius, header_icon_radius), circle_center_pos + ImVec2(header_icon_radius, header_icon_radius), gui->u32(col->text), icon, nullptr, nullptr, { 0.5f, 0.5f });

			draw->text(drawlist, font->get("medium13"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), gui->u32(col->text), name);
			draw->text_clipped(drawlist, font->get("medium12"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), header_rect.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });
		}

		// Unload button on the right side of header
		{
			const char* btn_text = "Unload";
			ImFont* btn_font = font->get("medium12");
			if (!btn_font) btn_font = ImGui::GetFont();
			
			ImVec2 text_size = btn_font->CalcTextSizeA(btn_font->FontSize, FLT_MAX, 0.0f, btn_text);
			const float btn_padding_x = scale(10);
			const float btn_padding_top = scale(4);
			const float btn_padding_bottom = scale(6);
			const float btn_rounding = scale(4);
			
			ImVec2 btn_size = ImVec2(text_size.x + btn_padding_x * 2, text_size.y + btn_padding_top + btn_padding_bottom);
			ImVec2 btn_pos = ImVec2(pos.x + size.x - padding.x - btn_size.x, header_rect.Min.y + (header_desc_height - btn_size.y) * 0.5f);
			
			ImRect btn_rect(btn_pos, btn_pos + btn_size);
			
			// Check hover and click
			ImVec2 mouse_pos = ImGui::GetIO().MousePos;
			bool hovered = (mouse_pos.x >= btn_rect.Min.x && mouse_pos.x <= btn_rect.Max.x && 
			                mouse_pos.y >= btn_rect.Min.y && mouse_pos.y <= btn_rect.Max.y);
			bool clicked = hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
			
			if (clicked) {
				g_Options.General.ShutDown = true;
			}
			
			// Draw button
			ImU32 btn_bg = hovered ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 255);
			ImU32 btn_border = hovered ? IM_COL32(100, 100, 100, 255) : IM_COL32(80, 80, 80, 255);
			
			drawlist->AddRectFilled(btn_rect.Min, btn_rect.Max, btn_bg, btn_rounding);
			drawlist->AddRect(btn_rect.Min, btn_rect.Max, btn_border, btn_rounding, 0, 1.0f);
			
			// Draw text centered
			ImVec2 text_pos = ImVec2(btn_pos.x + btn_padding_x, btn_pos.y + btn_padding_top);
			drawlist->AddText(btn_font, btn_font->FontSize, text_pos, IM_COL32(255, 255, 255, 255), btn_text);
		}

		draw->line(drawlist, pos + ImVec2(0, header_height) - top_animation_pos, pos + ImVec2(size.x, header_height) - top_animation_pos, gui->u32(var->style.colors[style_col_border]), var->style.child_border_size);
	}

	// Helper function to convert VK code to ImGuiKey
	ImGuiKey VKToImGuiKey(int vk) {
		switch (vk) {
			case VK_TAB: return ImGuiKey_Tab;
			case VK_LEFT: return ImGuiKey_LeftArrow;
			case VK_RIGHT: return ImGuiKey_RightArrow;
			case VK_UP: return ImGuiKey_UpArrow;
			case VK_DOWN: return ImGuiKey_DownArrow;
			case VK_PRIOR: return ImGuiKey_PageUp;
			case VK_NEXT: return ImGuiKey_PageDown;
			case VK_HOME: return ImGuiKey_Home;
			case VK_END: return ImGuiKey_End;
			case VK_INSERT: return ImGuiKey_Insert;
			case VK_DELETE: return ImGuiKey_Delete;
			case VK_BACK: return ImGuiKey_Backspace;
			case VK_SPACE: return ImGuiKey_Space;
			case VK_RETURN: return ImGuiKey_Enter;
			case VK_ESCAPE: return ImGuiKey_Escape;
			case VK_LCONTROL: return ImGuiKey_LeftCtrl;
			case VK_LSHIFT: return ImGuiKey_LeftShift;
			case VK_LMENU: return ImGuiKey_LeftAlt;
			case VK_RCONTROL: return ImGuiKey_RightCtrl;
			case VK_RSHIFT: return ImGuiKey_RightShift;
			case VK_RMENU: return ImGuiKey_RightAlt;
			case 0x30: return ImGuiKey_0;
			case 0x31: return ImGuiKey_1;
			case 0x32: return ImGuiKey_2;
			case 0x33: return ImGuiKey_3;
			case 0x34: return ImGuiKey_4;
			case 0x35: return ImGuiKey_5;
			case 0x36: return ImGuiKey_6;
			case 0x37: return ImGuiKey_7;
			case 0x38: return ImGuiKey_8;
			case 0x39: return ImGuiKey_9;
			case 0x41: return ImGuiKey_A;
			case 0x42: return ImGuiKey_B;
			case 0x43: return ImGuiKey_C;
			case 0x44: return ImGuiKey_D;
			case 0x45: return ImGuiKey_E;
			case 0x46: return ImGuiKey_F;
			case 0x47: return ImGuiKey_G;
			case 0x48: return ImGuiKey_H;
			case 0x49: return ImGuiKey_I;
			case 0x4A: return ImGuiKey_J;
			case 0x4B: return ImGuiKey_K;
			case 0x4C: return ImGuiKey_L;
			case 0x4D: return ImGuiKey_M;
			case 0x4E: return ImGuiKey_N;
			case 0x4F: return ImGuiKey_O;
			case 0x50: return ImGuiKey_P;
			case 0x51: return ImGuiKey_Q;
			case 0x52: return ImGuiKey_R;
			case 0x53: return ImGuiKey_S;
			case 0x54: return ImGuiKey_T;
			case 0x55: return ImGuiKey_U;
			case 0x56: return ImGuiKey_V;
			case 0x57: return ImGuiKey_W;
			case 0x58: return ImGuiKey_X;
			case 0x59: return ImGuiKey_Y;
			case 0x5A: return ImGuiKey_Z;
			case VK_F1: return ImGuiKey_F1;
			case VK_F2: return ImGuiKey_F2;
			case VK_F3: return ImGuiKey_F3;
			case VK_F4: return ImGuiKey_F4;
			case VK_F5: return ImGuiKey_F5;
			case VK_F6: return ImGuiKey_F6;
			case VK_F7: return ImGuiKey_F7;
			case VK_F8: return ImGuiKey_F8;
			case VK_F9: return ImGuiKey_F9;
			case VK_F10: return ImGuiKey_F10;
			case VK_F11: return ImGuiKey_F11;
			case VK_F12: return ImGuiKey_F12;
			default: return ImGuiKey_None;
		}
	}

	// Helper function to convert ImGuiKey to VK code
	int ImGuiKeyToVK(ImGuiKey key) {
		switch (key) {
			case ImGuiKey_Tab: return VK_TAB;
			case ImGuiKey_LeftArrow: return VK_LEFT;
			case ImGuiKey_RightArrow: return VK_RIGHT;
			case ImGuiKey_UpArrow: return VK_UP;
			case ImGuiKey_DownArrow: return VK_DOWN;
			case ImGuiKey_PageUp: return VK_PRIOR;
			case ImGuiKey_PageDown: return VK_NEXT;
			case ImGuiKey_Home: return VK_HOME;
			case ImGuiKey_End: return VK_END;
			case ImGuiKey_Insert: return VK_INSERT;
			case ImGuiKey_Delete: return VK_DELETE;
			case ImGuiKey_Backspace: return VK_BACK;
			case ImGuiKey_Space: return VK_SPACE;
			case ImGuiKey_Enter: return VK_RETURN;
			case ImGuiKey_Escape: return VK_ESCAPE;
			case ImGuiKey_LeftCtrl: return VK_LCONTROL;
			case ImGuiKey_LeftShift: return VK_LSHIFT;
			case ImGuiKey_LeftAlt: return VK_LMENU;
			case ImGuiKey_RightCtrl: return VK_RCONTROL;
			case ImGuiKey_RightShift: return VK_RSHIFT;
			case ImGuiKey_RightAlt: return VK_RMENU;
			case ImGuiKey_0: return 0x30;
			case ImGuiKey_1: return 0x31;
			case ImGuiKey_2: return 0x32;
			case ImGuiKey_3: return 0x33;
			case ImGuiKey_4: return 0x34;
			case ImGuiKey_5: return 0x35;
			case ImGuiKey_6: return 0x36;
			case ImGuiKey_7: return 0x37;
			case ImGuiKey_8: return 0x38;
			case ImGuiKey_9: return 0x39;
			case ImGuiKey_A: return 0x41;
			case ImGuiKey_B: return 0x42;
			case ImGuiKey_C: return 0x43;
			case ImGuiKey_D: return 0x44;
			case ImGuiKey_E: return 0x45;
			case ImGuiKey_F: return 0x46;
			case ImGuiKey_G: return 0x47;
			case ImGuiKey_H: return 0x48;
			case ImGuiKey_I: return 0x49;
			case ImGuiKey_J: return 0x4A;
			case ImGuiKey_K: return 0x4B;
			case ImGuiKey_L: return 0x4C;
			case ImGuiKey_M: return 0x4D;
			case ImGuiKey_N: return 0x4E;
			case ImGuiKey_O: return 0x4F;
			case ImGuiKey_P: return 0x50;
			case ImGuiKey_Q: return 0x51;
			case ImGuiKey_R: return 0x52;
			case ImGuiKey_S: return 0x53;
			case ImGuiKey_T: return 0x54;
			case ImGuiKey_U: return 0x55;
			case ImGuiKey_V: return 0x56;
			case ImGuiKey_W: return 0x57;
			case ImGuiKey_X: return 0x58;
			case ImGuiKey_Y: return 0x59;
			case ImGuiKey_Z: return 0x5A;
			case ImGuiKey_F1: return VK_F1;
			case ImGuiKey_F2: return VK_F2;
			case ImGuiKey_F3: return VK_F3;
			case ImGuiKey_F4: return VK_F4;
			case ImGuiKey_F5: return VK_F5;
			case ImGuiKey_F6: return VK_F6;
			case ImGuiKey_F7: return VK_F7;
			case ImGuiKey_F8: return VK_F8;
			case ImGuiKey_F9: return VK_F9;
			case ImGuiKey_F10: return VK_F10;
			case ImGuiKey_F11: return VK_F11;
			case ImGuiKey_F12: return VK_F12;
			default: return 0;
		}
	}

	void render_menu_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##GSettingsMenu", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("SETTINGS");
			widgets->checkbox("StreamProof", "Hide menu from screen capture and streaming", var->cfg.get<bool>("menu_streamproof"));
			gui->separator();
			widgets->checkbox("Hide on Alt-Tab", "Automatically hide menu when switching windows", var->cfg.get<bool>("menu_hide_alt_tab"));
			gui->separator();
			
			// Custom GUI Key widget that syncs with g_Options.General.MenuKey
			{
				ImGuiWindow* window = gui->get_window();
				ImDrawList* dl = window->DrawList;
				
				const char* label = "Gui Key";
				const char* desc = "Key to toggle menu visibility";
				const ImGuiID id = window->GetID(label);
				
				const float height = scale(elements->window.widgets_height);
				const ImVec2 keybox_padding = scale(6, 2);
				const float keybox_rounding = scale(6);
				const float keyname_icon_spacing_x = scale(6);
				
				const ImVec2 wpos = window->DC.CursorPos;
				const ImRect bb(wpos, wpos + ImVec2(gui->content_avail().x, height));
				
				static bool keybox_listen_key = false;
				
				// Convert current MenuKey VK code to ImGuiKey for display
				ImGuiKey current_key = VKToImGuiKey(g_Options.General.MenuKey);
				
				if (keybox_listen_key)
				{
					for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; ++key)
					{
						ImGuiKey k = static_cast<ImGuiKey>(key);
						if (ImGui::IsKeyPressed(k))
						{
							if (k == ImGuiKey_MouseLeft || k == ImGuiKey_Escape)
							{
								// Reset to INSERT on cancel
								g_Options.General.MenuKey = VK_INSERT;
							}
							else
							{
								// Convert ImGuiKey to VK and save
								int vk = ImGuiKeyToVK(k);
								if (vk != 0) {
									g_Options.General.MenuKey = vk;
								}
							}
							keybox_listen_key = false;
							break;
						}
					}
				}
				
				const char* key_name = keybox_listen_key ? "..." : ImGui::GetKeyName(current_key);
				const ImVec2 key_name_size = gui->text_size(font->get("medium13"), key_name);
				const ImVec2 keyboard_icon_size = gui->text_size(font->get("icons15"), ICON_KEYBOARD);
				
				const float keybox_width = key_name_size.x + keyname_icon_spacing_x + keyboard_icon_size.x + keybox_padding.x * 2.f;
				const ImRect keybox_bb(ImVec2(bb.Max.x - keybox_width, bb.GetCenter().y - scale(19) * 0.5f), ImVec2(bb.Max.x, bb.GetCenter().y + scale(19) * 0.5f));
				
				gui->item_size(bb, var->style.frame_padding.y);
				if (gui->item_add(bb, id))
				{
					const ImGuiID keybox_id = ImHashStr("##keybox_gui", 0, id);
					gui->item_add(keybox_bb, keybox_id);
					
					bool keybox_hovered = keybox_bb.Contains(ImGui::GetIO().MousePos);
					bool keybox_pressed = keybox_hovered && ImGui::IsMouseClicked(ImGuiMouseButton_Left);
					if (keybox_pressed)
					{
						keybox_listen_key = true;
					}
					
					const ImVec4 keybox_text_color = keybox_listen_key ? col->text : col->text_dimmed;
					
					draw->text(dl, font->get("semibold13"), bb.Min, gui->u32(col->text), label);
					draw->text_clipped(dl, font->get("medium11"), bb.Min, bb.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });
					
					draw->rect_filled(dl, keybox_bb.Min, keybox_bb.Max, gui->u32(col->primary, 0.05f), keybox_rounding);
					draw->text_clipped(dl, font->get("icons14"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(keybox_text_color), ICON_KEYBOARD, nullptr, &keyboard_icon_size, ImVec2(1.f, 0.5f), &keybox_bb);
					draw->text_clipped(dl, font->get("medium13"), keybox_bb.Min + keybox_padding, keybox_bb.Max - keybox_padding, gui->u32(col->text_dimmed), key_name, nullptr, nullptr, ImVec2(0.0f, 0.5f), &keybox_bb);
				}
			}
			
			gui->separator();
			widgets->checkbox("Watermark", "Show watermark on screen", var->cfg.get<bool>("menu_watermark"));
			gui->separator();
			widgets->slider_int("Overlay Delay##MenuDelay", var->cfg.get<int>("menu_overlay_delay"), 0, 100);
			gui->separator();
			widgets->checkbox("Enable Animations", "Enable menu animations", var->cfg.get<bool>("menu_enable_animations"));
			widgets->end_content_group("SETTINGS");
		}
		gui->end_content("Content##GSettingsMenu", true);
	}

	void render_config_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);
		ImGuiWindow* config_enties_window = nullptr;
		std::string config_to_delete = "";

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##Configs", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// Create Config button
			widgets->begin_content_group("CREATE");
			if (widgets->button("Create Config", ImVec2(0, scale(38))))
			{
				size_t i = 0;
				std::string added_config_name = "Config #"s + std::to_string(++i);

				while (std::filesystem::exists((var->cfg.get_filepath() / added_config_name).replace_extension(".cfg")))
				{
					added_config_name = "Config #"s + std::to_string(++i);
				}

				date added_config_date = get_current_date();

				if (var->cfg.save_to_disk(added_config_name))
				{
					elements->configs.config_list.emplace_back(added_config_name, added_config_date);

					std::ranges::sort(elements->configs.config_list, [](const c_elements::config_item& a, const c_elements::config_item& b) {
						return a < b;
						});

					elements->configs.selected_config = added_config_name;
					notify->add_notify("Config "s + added_config_name + " created.", notify_type::success);
				}
			}
			widgets->end_content_group("CREATE");

			widgets->begin_content_group("LIST");
			config_enties_window = gui->get_window();

			elements->configs.last_fetch_time += std::chrono::duration<float>(ImGui::GetIO().DeltaTime);
			bool should_refetch_list = (elements->configs.last_fetch_time >= elements->configs.fetch_cooldown);

			if (elements->configs.config_list.empty() && !should_refetch_list) {
				should_refetch_list = true;
			}

			if (should_refetch_list)
			{
				elements->configs.config_list.clear();
				for (const auto& config_file : std::filesystem::directory_iterator{ var->cfg.get_filepath() })
				{
					elements->configs.config_list.emplace_back(config_file.path().stem().string(), get_file_creation_date(config_file));
				}
				elements->configs.last_fetch_time = 0s;
				std::ranges::sort(elements->configs.config_list, [](const c_elements::config_item& a, const c_elements::config_item& b) {
					return a < b;
					});
			}

			for (auto& config : elements->configs.config_list)
			{
				auto [deleted, loaded, saved] = widgets->config_item(config.name.c_str(), config.creation_date, elements->configs.selected_config == config.name);

				if (deleted)
				{
					config_to_delete = config.name;
				}
				else if (loaded)
				{
					if (var->cfg.load_from_disk(config.name))
					{
						notify->add_notify("Config " + config.name + " loaded.", notify_type::success);
						elements->configs.selected_config = config.name;
					}
				}
				else if (saved)
				{
					if (var->cfg.save_to_disk(config.name))
					{
						notify->add_notify("Config " + config.name + " saved.", notify_type::success);
						config.creation_date = get_current_date();
						std::ranges::sort(elements->configs.config_list, [](const c_elements::config_item& a, const c_elements::config_item& b) {
							return a < b;
							});
					}
				}
				if (config.name != elements->configs.config_list.back().name)
				{
					gui->separator();
				}
			}
			gui->set_pos(gui->get_pos().y - var->style.item_spacing.y, pos_y);
			widgets->end_content_group("LIST");
		}
		gui->end_content("Content##Configs", true);

		if (!config_to_delete.empty())
		{
			destroy_config_entry(config_enties_window, config_to_delete.c_str());
			var->cfg.delete_from_disk(config_to_delete);
			elements->configs.config_list.erase(std::ranges::find_if(elements->configs.config_list, [&](const c_elements::config_item& item) {
				return config_to_delete == item.name;
				}));
			notify->add_notify("Config " + config_to_delete + " deleted.", notify_type::success);

			if (elements->configs.selected_config == config_to_delete)
			{
				elements->configs.selected_config = "";
			}
			elements->configs.last_fetch_time = elements->configs.fetch_cooldown;
		}
	}

	date get_file_creation_date(const std::filesystem::path& path) {
		namespace fs = std::filesystem;
		using namespace std::chrono;

		date file_date;
		fs::file_time_type ftime = fs::last_write_time(path);

		auto sctp = time_point_cast<system_clock::duration>(ftime - fs::file_time_type::clock::now() + system_clock::now());

		std::time_t tt = system_clock::to_time_t(sctp);
		
		std::tm tm_struct;
		localtime_s(&tm_struct, &tt);

		file_date.day = tm_struct.tm_mday;
		file_date.month = tm_struct.tm_mon + 1;
		file_date.year = tm_struct.tm_year + 1900;

		size_t seconds_since_midnight = tm_struct.tm_hour * 3600 + tm_struct.tm_min * 60 + tm_struct.tm_sec;
		auto truncated_to_seconds = time_point_cast<seconds>(sctp);
		auto remaining_duration = sctp - truncated_to_seconds;
		size_t current_second_milliseconds = duration_cast<milliseconds>(remaining_duration).count();
		file_date.time = static_cast<int>(seconds_since_midnight * 1000 + current_second_milliseconds);

		return file_date;
	}

	date get_current_date() {
		using namespace std::chrono;

		system_clock::time_point now = system_clock::now();

		std::time_t tt = system_clock::to_time_t(now);

		std::tm tm_struct;
		localtime_s(&tm_struct, &tt);

		date current_date_obj;
		current_date_obj.day = tm_struct.tm_mday;
		current_date_obj.month = tm_struct.tm_mon + 1;
		current_date_obj.year = tm_struct.tm_year + 1900;

		size_t seconds_since_midnight = tm_struct.tm_hour * 3600 + tm_struct.tm_min * 60 + tm_struct.tm_sec;
		auto truncated_to_seconds = time_point_cast<seconds>(now);
		auto remaining_duration = now - truncated_to_seconds;
		size_t current_second_milliseconds = duration_cast<milliseconds>(remaining_duration).count();
		current_date_obj.time = static_cast<int>(seconds_since_midnight * 1000 + current_second_milliseconds);

		return current_date_obj;
	}
}