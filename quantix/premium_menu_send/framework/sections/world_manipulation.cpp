#include "../headers/sections.h"

namespace {
	void render_content();
}

void sections::render_world_manipulation() {
	ImDrawList* drawlist = gui->window_drawlist();
	const ImVec2 pos = gui->window_pos();
	const ImVec2 size = gui->window_size();

	const ImVec2 padding = scale(elements->window.content_padding);
	const float header_icon_radius = scale(elements->content.header_icon_radius);
	const float header_desc_height = scale(elements->content.header_desc_height);
	const float header_height = scale(elements->content.header_height_no_subtabs);
	
	const float content_width = size.x - padding.x * 2.f;
	const float circle_text_spacing_x = scale(12);

	const char* icon = ICON_PLANET;
	const char* name = "World Manipulation";
	const char* desc = "Customizing the world and displaying additional information";

	const float old_alpha = var->window.alpha;
	if (var->window.subtab_switched) {
		var->window.alpha = 1.f;
	}
	gui->push_var(style_var_alpha, var->window.alpha);

	const ImVec2 top_animation_pos = ImVec2(0.f, header_height * (1.f - var->style.alpha));

	const ImVec2 content_start_pos = pos + padding;
	const ImRect header_rect(content_start_pos - top_animation_pos, content_start_pos - ImVec2(0.f, top_animation_pos.y) + ImVec2(content_width, header_desc_height));
	const ImRect content_rect(content_start_pos, content_start_pos + ImVec2(content_width, size.y - padding.y * 2.f));

	{
		const ImVec2 circle_center_pos = header_rect.Min + ImVec2(header_icon_radius, header_desc_height * 0.5f);
		draw->circle_filled(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_3));
		draw->circle(drawlist, circle_center_pos, header_icon_radius, gui->u32(col->gray_5), 0, var->style.child_border_size);

		draw->text_clipped(drawlist, font->get("icons16"), circle_center_pos - ImVec2(header_icon_radius, header_icon_radius), circle_center_pos + ImVec2(header_icon_radius, header_icon_radius), gui->u32(col->text), icon, nullptr, nullptr, { 0.5f, 0.5f });

		draw->text(drawlist, font->get("medium13"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), gui->u32(col->text), name);
		draw->text_clipped(drawlist, font->get("medium12"), header_rect.Min + ImVec2(header_icon_radius * 2.f + circle_text_spacing_x, 0), header_rect.Max, gui->u32(col->text_dimmed), desc, nullptr, nullptr, { 0.f, 1.f });
	}

	draw->line(drawlist, pos + ImVec2(0, header_height) - top_animation_pos, pos + ImVec2(size.x, header_height) - top_animation_pos, gui->u32(var->style.colors[style_col_border]), var->style.child_border_size);

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);
	render_content();
}

namespace {
	void render_content() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		

		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));

		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##WorldManipulation", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("GENERAL");
			widgets->checkbox("Enable World ESP", "Activates the display of objects on the map", var->cfg.get<bool>("enable_world_perception"));
			gui->separator();
			widgets->checkbox("Map Boundaries", "Displays the map boundaries", var->cfg.get<bool>("map_boundaries"));
			widgets->end_content_group("GENERAL");

			const char* trajectory_types[] = { "Line", "Circle" };

			widgets->begin_content_group("GRENADES");
			widgets->checkbox("Grenade Trajectory", "Indicates where the grenade will go", var->cfg.get<bool>("grenade_trajectory"));
			gui->separator();
			widgets->combo("Trajectory Type", "Shows the point trajectory of grenades", trajectory_types, var->cfg.get<int>("trajectory_type"));
			gui->separator();
			widgets->checkbox("Show Distance", "Shows how many meters to the grenade", var->cfg.get<bool>("grenade_show_distance"));
			widgets->end_content_group("GRENADES");
		}
		gui->end_content("Content##WorldManipulation", true);
	}
}
