#include "../headers/sections.h"
#include "../../cheat/Main/Options.hpp"

namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_common_tab();
	void render_manipulate_tab();
	void render_additional_tab();
	void render_bone_selector(const char* prefix, bool is_aimbot);
}

void sections::render_aim_assistance() {
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

	render_header(ICON_ARROW_MERGED, "Aim Assistance", "Aimbot, silent aim and triggerbot settings");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_common_tab();
		// Always show bone selector for aimbot tab
		{ // Always visible
			render_bone_selector("aimbot", true);
		}
		break;
	case 1:
		render_manipulate_tab();
		// Always show bone selector for silent tab
		{ // Always visible
			render_bone_selector("silent", false);
		}
		break;
	case 2:
		render_additional_tab();
		break;
	}
}

namespace {
	void render_header(const char* icon, const char* name, const char* desc) {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const ImVec2 padding = scale(elements->window.content_padding);
		const float header_icon_radius = scale(elements->content.header_icon_radius);
		const float header_desc_height = scale(elements->content.header_desc_height);
		const float header_height = scale(elements->content.header_height_no_subtabs);
		
		const float content_width = size.x - padding.x * 2.f;
		const float circle_text_spacing_x = scale(12);

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

		draw->line(drawlist, pos + ImVec2(0, header_height) - top_animation_pos, pos + ImVec2(size.x, header_height) - top_animation_pos, gui->u32(var->style.colors[style_col_border]), var->style.child_border_size);
	}

	void render_bone_selector(const char* prefix, bool is_aimbot) {
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const ImVec2 preview_size = scale(260, 460);
		const ImVec2 preview_pos = ImVec2(pos.x + size.x + scale(20), pos.y + (size.y - preview_size.y) * 0.5f);
		const ImVec2 window_padding = scale(12, 12);

		// Apply fade animation to preview - sync with page/subtab transitions
		const float preview_alpha = var->style.alpha;

		gui->set_next_window_size(preview_size);
		gui->set_next_window_pos(preview_pos);
		gui->push_var(style_var_window_padding, window_padding);
		gui->push_var(style_var_alpha, preview_alpha);
		
		char window_id[64];
		snprintf(window_id, sizeof(window_id), "bone_selector_%s", prefix);
		
		gui->begin(window_id, nullptr, window_flags_no_resize | window_flags_no_move | window_flags_no_title_bar | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse);
		{
			ImDrawList* drawlist = gui->window_drawlist();
			const ImVec2 self_pos = gui->window_pos();
			const ImVec2 self_size = gui->window_size();

			// Draw 60% opaque background (40% transparent)
			drawlist->AddRectFilled(self_pos, self_pos + self_size, IM_COL32(13, 13, 13, (int)(153 * preview_alpha)), scale(8));

			draw->text(drawlist, font->get("medium12"), self_pos + window_padding, gui->u32(col->text_dimmed, preview_alpha), "BONE SELECTOR");
		
			// Center the bone selector widget in the window
			const ImVec2 widget_size = scale(230, 420);
			const float widget_x = (self_size.x - widget_size.x) * 0.5f;
			const float widget_y = scale(32);
			gui->set_pos(ImVec2(widget_x, widget_y), pos_all);
			
			// Apply alpha to the preview widget
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, preview_alpha);
			widgets->bone_selector_preview(window_id, prefix, preview_alpha);
			ImGui::PopStyleVar();
		}
		gui->end();
		gui->pop_var(); // style_var_alpha
		gui->pop_var(); // style_var_window_padding
	}

	void render_common_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##AimCommon", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("GENERAL");
			widgets->checkbox_setting("Enable aimbot", "Enables aimbot functionality", var->cfg.get<bool>("enable_aimbot"));
			gui->separator();
			widgets->checkbox_key("Aimbot key", "Key to activate aimbot", var->cfg.get<bool>("aimbot_key_enabled"));
			gui->separator();
			widgets->checkbox("Show fov", "Display field of view circle", var->cfg.get<bool>("show_aimbot_fov"));
			gui->separator();
			widgets->checkbox_setting("Visible check", "Only aim when target is visible", var->cfg.get<bool>("aimbot_visible_check"));
			widgets->end_content_group("GENERAL");

			widgets->begin_content_group("AIMBOT SETTINGS");
			widgets->slider_int("Fov##aimbot", var->cfg.get<int>("aimbot_fov"), 0, 300, "%d");
			gui->separator();
			widgets->slider_int("Distance##aimbot", var->cfg.get<int>("aimbot_distance"), 0, 1000, "%d");
			gui->separator();
			widgets->slider_float("X Smooth##aimbot", var->cfg.get<float>("aimbot_x_smooth"), 0.f, 100.f, "%.1f");
			gui->separator();
			widgets->slider_float("Y Smooth##aimbot", var->cfg.get<float>("aimbot_y_smooth"), 0.f, 100.f, "%.1f");
			widgets->end_content_group("AIMBOT SETTINGS");

			widgets->begin_content_group("AIMBOT EXTRA");
			widgets->checkbox("Ignore Dead", "Don't aim at dead players", var->cfg.get<bool>("aimbot_ignore_dead"));
			gui->separator();
			widgets->checkbox("Ignore Invisible", "Don't aim at invisible players", var->cfg.get<bool>("aimbot_ignore_invisible"));
			gui->separator();
			widgets->checkbox("Ignore Friends", "Don't aim at friends", var->cfg.get<bool>("aimbot_ignore_friends"));
			widgets->end_content_group("AIMBOT EXTRA");

			widgets->begin_content_group("FOV COLOR");
			widgets->color_only("Aimbot FOV Color", "Color for aimbot FOV circle", var->cfg.get<ImVec4>("aimbot_fov_color"));
			widgets->end_content_group("FOV COLOR");
		}
		gui->end_content("Content##AimCommon", true);
	}

	void render_manipulate_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##AimManipulate", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("GENERAL");
			widgets->checkbox_setting("Enable Silent", "Enables silent aim functionality", var->cfg.get<bool>("enable_silent"));
			gui->separator();
			widgets->checkbox_key("Silent Key", "Key to activate silent aim", var->cfg.get<bool>("silent_key_enabled"));
			gui->separator();
			widgets->checkbox("Show FOV", "Display field of view circle", var->cfg.get<bool>("show_silent_fov"));
			gui->separator();
			widgets->checkbox_setting("Visible Check", "Only aim when target is visible", var->cfg.get<bool>("silent_visible_check"));
			widgets->end_content_group("GENERAL");

			widgets->begin_content_group("SILENT SETTINGS");
			// WaveShield bypass kapalı: max 50, açık: max 1000
			bool waveshieldActive = var->cfg.get<bool>("silent_waveshield_bypass");
			if (waveshieldActive) {
				widgets->slider_int("Fov##silent", var->cfg.get<int>("silent_fov"), 0, 1000, "%d");
			} else {
				widgets->slider_int("Fov##silent", var->cfg.get<int>("silent_fov"), 0, 50, "%d");
			}
			gui->separator();
			widgets->slider_int("Distance##silent", var->cfg.get<int>("silent_distance"), 0, 1000, "%d");
			gui->separator();
			widgets->slider_int("Miss Chance##silent", var->cfg.get<int>("silent_miss_percentage"), 0, 100, "%d%%");
			gui->separator();
			widgets->checkbox("Show Locked Bone", "Display locked bone indicator", var->cfg.get<bool>("silent_show_locked_bone"));
			gui->separator();
			widgets->checkbox_setting("WaveShield Bypass", "Unlimited FOV bypass for silent aim", var->cfg.get<bool>("silent_waveshield_bypass"));
			widgets->end_content_group("SILENT SETTINGS");

			widgets->begin_content_group("EXTRA");
			widgets->checkbox("Ignore Dead", "Don't aim at dead players", var->cfg.get<bool>("silent_ignore_dead"));
			gui->separator();
			widgets->checkbox("Ignore Invisible", "Don't aim at invisible players", var->cfg.get<bool>("silent_ignore_invisible"));
			gui->separator();
			widgets->checkbox("Ignore Friends", "Don't aim at friends", var->cfg.get<bool>("silent_ignore_friends"));
			widgets->end_content_group("EXTRA");

			widgets->begin_content_group("MAGIC");
			widgets->checkbox_setting("Enable Magic", "Enables magic bullet functionality", var->cfg.get<bool>("enable_magic"));
			gui->separator();
			widgets->checkbox_key("Magic Key", "Key to activate magic bullet", var->cfg.get<bool>("magic_key_enabled"));
			widgets->end_content_group("MAGIC");

			widgets->begin_content_group("FOV COLOR");
			widgets->color_only("Silent FOV Color", "Color for silent FOV circle", var->cfg.get<ImVec4>("silent_fov_color"));
			widgets->end_content_group("FOV COLOR");
		}
		gui->end_content("Content##AimManipulate", true);
	}

	void render_additional_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##AimAdditional", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("GENERAL");
			widgets->checkbox_setting("Enable triggerbot", "Enables triggerbot functionality", var->cfg.get<bool>("enable_triggerbot"));
			gui->separator();
			widgets->checkbox_key("Trigger key", "Key to activate triggerbot", var->cfg.get<bool>("trigger_key_enabled"));
			gui->separator();
			widgets->checkbox_setting("Visible check", "Only trigger when target is visible", var->cfg.get<bool>("trigger_visible_check"));
			gui->separator();
			widgets->checkbox("Auto shoot", "Automatically shoot when target in crosshair", var->cfg.get<bool>("trigger_auto_shoot"));
			widgets->end_content_group("GENERAL");

			widgets->begin_content_group("TRIGGERBOT SETTINGS");
			widgets->slider_int("Distance##trigger", var->cfg.get<int>("trigger_distance"), 0, 500, "%dm");
			gui->separator();
			widgets->slider_int("Delay##trigger", var->cfg.get<int>("trigger_delay"), 0, 200, "%dms");
			widgets->end_content_group("TRIGGERBOT SETTINGS");

			widgets->begin_content_group("TRIGGERBOT EXTRA");
			widgets->checkbox("Ignore Dead", "Don't trigger on dead players", var->cfg.get<bool>("trigger_ignore_dead"));
			gui->separator();
			widgets->checkbox("Ignore Invisible", "Don't trigger on invisible players", var->cfg.get<bool>("trigger_ignore_invisible"));
			gui->separator();
			widgets->checkbox("Ignore Friends", "Don't trigger on friends", var->cfg.get<bool>("trigger_ignore_friends"));
			widgets->end_content_group("TRIGGERBOT EXTRA");
		}
		gui->end_content("Content##AimAdditional", true);
	}
}