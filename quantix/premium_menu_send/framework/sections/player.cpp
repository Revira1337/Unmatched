#include "../headers/sections.h"
#include "../../cheat/Main/FivemSDK/Fivem.hpp"
#include "../../cheat/Main/Features/playernames.hpp"
#include "../../cheat/Main/Options.hpp"
#include "../../cheat/Utils/Utilities/Memory.hpp"
#include <algorithm>

namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_player_tab();
	void render_freecam_tab();
	void render_players_subtab();
	
	// Players subtab state
	static int selected_player_netid = -1;
	static int max_distance = 1000;
	static char search_buffer[64] = "";
	
	// Button animation states
	static std::map<std::string, float> button_hover_anims;
	static std::map<std::string, float> button_click_anims;
}

void sections::render_player() {
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

	render_header(ICON_USER, "Player", "Player settings, freecam and player list");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_player_tab();
		break;
	case 1:
		render_freecam_tab();
		break;
	case 2:
		render_players_subtab();
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

	// Custom animated button that matches menu style
	bool animated_button(const char* label, ImVec2 size_arg, const char* id_override = nullptr) {
		std::string btn_id = id_override ? id_override : label;
		
		// Initialize animation states
		if (button_hover_anims.find(btn_id) == button_hover_anims.end()) {
			button_hover_anims[btn_id] = 0.0f;
		}
		if (button_click_anims.find(btn_id) == button_click_anims.end()) {
			button_click_anims[btn_id] = 0.0f;
		}
		
		float& hover_anim = button_hover_anims[btn_id];
		float& click_anim = button_click_anims[btn_id];
		float dt = ImGui::GetIO().DeltaTime;
		
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* drawlist = ImGui::GetWindowDrawList();
		
		// Calculate animated size (scale up on hover)
		float scale_factor = 1.0f + hover_anim * 0.03f;
		ImVec2 actual_size = ImVec2(size_arg.x * scale_factor, size_arg.y * scale_factor);
		
		// Center the scaled button
		ImVec2 offset = ImVec2((actual_size.x - size_arg.x) * 0.5f, (actual_size.y - size_arg.y) * 0.5f);
		ImVec2 btn_pos = ImVec2(pos.x - offset.x, pos.y - offset.y);
		ImVec2 btn_max = ImVec2(btn_pos.x + actual_size.x, btn_pos.y + actual_size.y);
		
		// Check interaction
		bool hovered = ImGui::IsMouseHoveringRect(btn_pos, btn_max);
		bool held = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
		bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		
		// Update animations
		float target_hover = hovered ? 1.0f : 0.0f;
		hover_anim = ImLerp(hover_anim, target_hover, dt * 12.0f);
		
		if (held) {
			click_anim = ImLerp(click_anim, 1.0f, dt * 20.0f);
		} else {
			click_anim = ImLerp(click_anim, 0.0f, dt * 15.0f);
		}
		
		// Calculate colors with animation
		float brightness = 0.08f + hover_anim * 0.06f - click_anim * 0.03f;
		ImVec4 bg_color = ImVec4(brightness, brightness, brightness, 0.9f + hover_anim * 0.1f);
		
		float border_brightness = 0.15f + hover_anim * 0.1f;
		ImVec4 border_color = ImVec4(border_brightness, border_brightness, border_brightness, 0.5f + hover_anim * 0.3f);
		
		float text_brightness = 0.6f + hover_anim * 0.4f;
		ImU32 text_color = IM_COL32((int)(text_brightness * 255), (int)(text_brightness * 255), (int)(text_brightness * 255), 255);
		
		// Apply click scale
		float click_scale = 1.0f - click_anim * 0.02f;
		ImVec2 click_offset = ImVec2(actual_size.x * (1.0f - click_scale) * 0.5f, actual_size.y * (1.0f - click_scale) * 0.5f);
		ImVec2 final_pos = ImVec2(btn_pos.x + click_offset.x, btn_pos.y + click_offset.y);
		ImVec2 final_max = ImVec2(btn_max.x - click_offset.x, btn_max.y - click_offset.y);
		
		// Draw button
		float rounding = scale(6);
		drawlist->AddRectFilled(final_pos, final_max, ImColor(bg_color), rounding);
		drawlist->AddRect(final_pos, final_max, ImColor(border_color), rounding, 0, 1.0f);
		
		// Draw text centered
		ImVec2 text_size = ImGui::CalcTextSize(label);
		ImVec2 text_pos = ImVec2(
			final_pos.x + (final_max.x - final_pos.x - text_size.x) * 0.5f,
			final_pos.y + (final_max.y - final_pos.y - text_size.y) * 0.5f
		);
		drawlist->AddText(text_pos, text_color, label);
		
		// Reserve space
		ImGui::Dummy(size_arg);
		
		return clicked;
	}

	void render_player_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		// Slide animation from bottom
		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##PlayerTab", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// PLAYER content group
			widgets->begin_content_group("PLAYER");
			widgets->checkbox_key("Godmode", "Makes you invincible to all damage", var->cfg.get<bool>("player_godmode"));
			gui->separator();
			widgets->checkbox("Semi-Godmode", "Keeps your health above minimum threshold", var->cfg.get<bool>("player_semi_godmode"));
			widgets->slider_int("Min Health##SemiGod", var->cfg.get<int>("player_semi_godmode_min_health"), 1, 100);
			gui->separator();
			widgets->checkbox("Steal Car", "Instantly steal any vehicle", var->cfg.get<bool>("player_steal_car"));
			gui->separator();
			widgets->checkbox_key("Invisible", "Makes you invisible to other players", var->cfg.get<bool>("player_invisible"));
			gui->separator();
			widgets->checkbox("NoCollision", "Walk through walls and objects", var->cfg.get<bool>("player_no_collision"));
			gui->separator();
			widgets->checkbox("No Fall Damage", "Prevents damage from falling", var->cfg.get<bool>("player_no_fall_damage"));
			gui->separator();
			widgets->checkbox("Shrink", "Makes your character model smaller", var->cfg.get<bool>("player_shrink"));
			widgets->slider_float("Shrink Size##PlayerShrink", var->cfg.get<float>("player_shrink_amount"), 0.1f, 1.0f);
			widgets->end_content_group("PLAYER");

			// STATS content group
			widgets->begin_content_group("STATS");
			widgets->checkbox("Custom FOV", "Adjust your field of view", var->cfg.get<bool>("player_custom_fov"));
			widgets->slider_int("FOV Value##CustomFov", var->cfg.get<int>("player_custom_fov_value"), 60, 120);
			gui->separator();
			widgets->checkbox_key("Health", "Set your health value", var->cfg.get<bool>("player_health"));
			widgets->slider_int("Health Value##PlayerHealth", var->cfg.get<int>("player_health_value"), 1, 100);
			gui->separator();
			widgets->checkbox_key("Armor", "Set your armor value", var->cfg.get<bool>("player_armor"));
			widgets->slider_int("Armor Value##PlayerArmor", var->cfg.get<int>("player_armor_value"), 0, 100);
			gui->separator();
			widgets->checkbox("Absorption", "Damage absorption percentage", var->cfg.get<bool>("player_absorption"));
			widgets->slider_int("Absorption %##PlayerAbsorption", var->cfg.get<int>("player_absorption_percent"), 0, 100);
			widgets->end_content_group("STATS");

			// MOVEMENT content group
			widgets->begin_content_group("MOVEMENT");
			widgets->checkbox("Player Speed", "Modify your movement speed", var->cfg.get<bool>("player_speed"));
			widgets->slider_float("Speed Value##PlayerSpeed", var->cfg.get<float>("player_speed_value"), 0.1f, 10.0f);
			gui->separator();
			widgets->checkbox_key("Strafe Macro", "Automated strafe movement", var->cfg.get<bool>("player_strafe_macro"));
			widgets->slider_int("Strafe Delay (ms)##StrafeMacro", var->cfg.get<int>("player_strafe_delay"), 1, 500);
			widgets->end_content_group("MOVEMENT");
			
			// GHOST PEEK content group
			widgets->begin_content_group("GHOST PEEK");
			widgets->checkbox("Enable Ghost Peek", "Enable ghost peek teleport feature", var->cfg.get<bool>("player_ghost_peek_enabled"));
			gui->separator();
			widgets->keybind_only("Ghost Peek Save", "Key to save current position");
			gui->separator();
			widgets->keybind_only("Ghost Peek TP", "Key to teleport back to saved position");
			widgets->end_content_group("GHOST PEEK");
		}
		gui->end_content("Content##PlayerTab", true);
	}

	void render_freecam_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		// Slide animation from bottom
		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##FreecamTab", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// CAMERA content group
			widgets->begin_content_group("CAMERA");
			widgets->checkbox_key("Freecam", "Enable free camera movement", var->cfg.get<bool>("freecam_enabled"));
			widgets->slider_float("Freecam Speed##FreecamSpeed", var->cfg.get<float>("freecam_speed"), 0.1f, 10.0f);
			widgets->end_content_group("CAMERA");

			// TELEPORT content group
			widgets->begin_content_group("TELEPORT");
			widgets->checkbox_key("Teleport to Waypoint", "Teleport to your map waypoint", var->cfg.get<bool>("freecam_teleport_waypoint"));
			gui->separator();
			widgets->checkbox_key("Teleport to Saved Location", "Teleport to previously saved location", var->cfg.get<bool>("freecam_teleport_saved"));
			gui->separator();
			widgets->checkbox_key("Freecam Teleport", "Teleport to freecam position", var->cfg.get<bool>("freecam_teleport"));
			gui->separator();
			if (widgets->button("Save Current Location", ImVec2(0, scale(38))))
			{
				notify->add_notify("Location saved!", notify_type::success);
			}
			widgets->end_content_group("TELEPORT");
		}
		gui->end_content("Content##FreecamTab", true);
	}

	void render_players_subtab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		// Slide animation from bottom
		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		// Collect player data
		struct PlayerData {
			std::string name;
			int distance;
			bool isFriend;
			Cheat::CPed* ped;
			int netId;
			Vector3D position;
		};
		std::vector<PlayerData> players;
		int totalDetected = 0; // all non-local players in the entity list, before the distance filter

		try {
			std::lock_guard<std::mutex> lock(Cheat::C_Five.GetLockLists());
			std::vector<Cheat::Entity>& entityList = Cheat::C_Five.GetEntitiyList();

			for (const auto& entity : entityList) {
				if (entity.StaticInfo.bIsLocalPlayer) continue;
				totalDetected++;
				if (max_distance > 0 && entity.Distance > max_distance) continue;
				
				std::string playerName = entity.StaticInfo.Name;
				if (playerName.empty()) {
					playerName = Cheat::PlayerNames::GetPedName(entity.StaticInfo.Ped);
				}
				if (playerName.empty() || playerName == "Unknown") {
					playerName = "Player_" + std::to_string(entity.StaticInfo.NetId);
				}
				
				// Search filter
				if (strlen(search_buffer) > 0) {
					std::string searchLower = search_buffer;
					std::string nameLower = playerName;
					std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
					if (nameLower.find(searchLower) == std::string::npos) continue;
				}
				
				PlayerData pd;
				pd.name = playerName;
				pd.distance = entity.Distance;
				pd.isFriend = entity.IsFriend;
				pd.ped = entity.StaticInfo.Ped;
				pd.netId = entity.StaticInfo.NetId;
				pd.position = entity.Cordinates;
				players.push_back(pd);
			}
		} catch (...) {}

		// Sort by distance
		std::sort(players.begin(), players.end(), [](const PlayerData& a, const PlayerData& b) {
			return a.distance < b.distance;
		});

		gui->begin_content("Content##PlayersSubtab", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// Calculate panel sizes for side-by-side layout
			float totalWidth = content_size.x - padding.x * 2;
			float leftPanelWidth = totalWidth * 0.55f;
			float rightPanelWidth = totalWidth * 0.42f;
			float panelSpacing = scale(12);
			float panelHeight = content_size.y - padding.y * 2 - scale(10);
			float searchRowHeight = scale(32);
			
			// ========== LEFT PANEL - PLAYER LIST ==========
			ImGui::BeginGroup();
			{
				// Search and filter row
				ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(col->black_3.x, col->black_3.y, col->black_3.z, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(col->gray.x, col->gray.y, col->gray.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(col->gray.x, col->gray.y, col->gray.z, 0.7f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(col->gray_4.x, col->gray_4.y, col->gray_4.z, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(col->gray_5.x, col->gray_5.y, col->gray_5.z, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, scale(6));
				ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(scale(8), scale(5)));
				ImGui::PushStyleVar(ImGuiStyleVar_FrameBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_GrabRounding, scale(4));
				
				ImGui::PushItemWidth(scale(140));
				ImGui::InputTextWithHint("##Search", "Search...", search_buffer, sizeof(search_buffer));
				ImGui::PopItemWidth();
				
				ImGui::SameLine();
				ImGui::PushItemWidth(scale(100));
				ImGui::SliderInt("##Dist", &max_distance, 0, 2000, "%dm");
				ImGui::PopItemWidth();

				ImGui::SameLine();
				ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "(%d/%d)", (int)players.size(), totalDetected);
				
				ImGui::PopStyleVar(4);
				ImGui::PopStyleColor(6);
				
				ImGui::Spacing();
				
				// Player list child
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(col->black_3.x, col->black_3.y, col->black_3.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.3f));
				ImGui::PushStyleColor(ImGuiCol_ScrollbarBg, ImVec4(col->black_3.x, col->black_3.y, col->black_3.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrab, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.8f));
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabHovered, ImVec4(col->gray_4.x, col->gray_4.y, col->gray_4.z, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_ScrollbarGrabActive, ImVec4(col->gray_5.x, col->gray_5.y, col->gray_5.z, 1.0f));
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.8f));
				ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(col->gray.x, col->gray.y, col->gray.z, 0.6f));
				ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 1.0f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, scale(6));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarSize, scale(8));
				ImGui::PushStyleVar(ImGuiStyleVar_ScrollbarRounding, scale(4));
				
				ImGui::BeginChild("##PlayerListChild", ImVec2(leftPanelWidth, panelHeight - searchRowHeight - scale(10)), true);
				{
					for (int i = 0; i < (int)players.size(); i++) {
							const auto& p = players[i];
						
							// Create unique ID for each item
							ImGui::PushID(i);
						
							char label[256];
							snprintf(label, sizeof(label), "%s%s  [%dm]", 
								p.isFriend ? "[F] " : "", 
								p.name.c_str(), 
								p.distance);
						
							bool isSelected = (selected_player_netid == p.netId);
						
							if (p.isFriend) {
								ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(col->green.x, col->green.y, col->green.z, 1.0f));
							}
						
							if (ImGui::Selectable(label, isSelected)) {
								selected_player_netid = p.netId;
							}
						
							if (p.isFriend) {
								ImGui::PopStyleColor();
							}
						
							ImGui::PopID();
						}
					
					if (players.empty()) {
						if (totalDetected > 0)
							ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.6f), "%d player(s) beyond %dm - raise distance", totalDetected, max_distance);
						else
							ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.6f), "No players detected");
					}
				}
				ImGui::EndChild();
				
				ImGui::PopStyleVar(4);
				ImGui::PopStyleColor(9);
			}
			ImGui::EndGroup();
			
			ImGui::SameLine(0, panelSpacing);
			
			// ========== RIGHT PANEL - SELECTED PLAYER ==========
			ImGui::BeginGroup();
			{
				// Add spacing to align with left panel's player list frame
				ImGui::Dummy(ImVec2(0, searchRowHeight - scale(2)));
				
				// Info panel - aligned with left panel's list
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(col->black_3.x, col->black_3.y, col->black_3.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.3f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, scale(6));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(scale(12), scale(10)));
				
				ImGui::BeginChild("##SelectedPlayerChild", ImVec2(rightPanelWidth, panelHeight - searchRowHeight - scale(9)), true);
				{
					// Find selected player by netId
					const PlayerData* selPtr = nullptr;
					for (const auto& p : players) {
						if (p.netId == selected_player_netid) {
							selPtr = &p;
							break;
						}
					}
					
					if (selPtr != nullptr) {
						const auto& sel = *selPtr;
						
						// Player name
						if (sel.isFriend) {
							ImGui::TextColored(ImVec4(col->green.x, col->green.y, col->green.z, 1.0f), "%s", sel.name.c_str());
						} else {
							ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "%s", sel.name.c_str());
						}
						
						ImGui::Spacing();
						
						// Info
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Distance:");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "%dm", sel.distance);
						
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Net ID:");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "%d", sel.netId);
						
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Position:");
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 0.8f), "  %.0f, %.0f, %.0f", 
							sel.position.x, sel.position.y, sel.position.z);
						
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Status:");
						ImGui::SameLine();
						if (sel.isFriend) {
							ImGui::TextColored(ImVec4(col->green.x, col->green.y, col->green.z, 1.0f), "Friend");
						} else {
							ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "Normal");
						}
						
						ImGui::Spacing();
						ImGui::Spacing();
						
						// Buttons
						if (animated_button("Teleport", ImVec2(scale(110), scale(28)), "tp_btn")) {
							auto localPlayer = Cheat::C_Five.GetLocalPlayerInfo();
							if (localPlayer.Ped) {
								try {
									uintptr_t pedAddr = (uintptr_t)localPlayer.Ped;
									uintptr_t navigation = M_base::mem::Read<uintptr_t>(pedAddr + 0x30);
									uintptr_t modelInfo = M_base::mem::Read<uintptr_t>(pedAddr + 0x20);
									
									Vector3D targetPos = sel.position;
									targetPos.z += 1.0f;
									
									Cheat::C_Five.TeleportFunc(pedAddr, navigation, modelInfo, targetPos, targetPos, true);
									notify->add_notify("Teleported to " + sel.name, notify_type::success);
								} catch (...) {
									notify->add_notify("Teleport failed!", notify_type::error);
								}
							}
						}
						
						ImGui::SameLine();
						
						const char* friendBtnText = sel.isFriend ? "Unfriend" : "Add Friend";
						std::string friendBtnId = sel.isFriend ? "remove_friend_btn" : "add_friend_btn";
						if (animated_button(friendBtnText, ImVec2(scale(110), scale(28)), friendBtnId.c_str())) {
							// Update entity list directly (same as ESP friend add)
							std::lock_guard<std::mutex> lock(Cheat::C_Five.GetLockLists());
							std::vector<Cheat::Entity>& entityList = Cheat::C_Five.GetEntitiyList();
							
							for (auto& entity : entityList) {
								if (entity.StaticInfo.NetId == sel.netId) {
									if (sel.isFriend) {
										// Remove friend
										entity.IsFriend = false;
										entity.StaticInfo.IsFriend = false;
										Cheat::C_Five.FriendList.erase(entity.StaticInfo.Ped);
										Cheat::C_Five.NetIdFriendList.erase(sel.netId);
										notify->add_notify("Removed from friends", notify_type::success);
									} else {
										// Add friend
										entity.IsFriend = true;
										entity.StaticInfo.IsFriend = true;
										Cheat::C_Five.FriendList[entity.StaticInfo.Ped] = true;
										Cheat::C_Five.NetIdFriendList[sel.netId] = true;
										notify->add_notify("Added as friend", notify_type::success);
									}
									break;
								}
							}
						}
						
					} else {
						selected_player_netid = -1;
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.5f), 
							"Select a player");
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.5f), 
							"from the list");
					}
				}
				ImGui::EndChild();
				
				ImGui::PopStyleVar(3);
				ImGui::PopStyleColor(2);
			}
			ImGui::EndGroup();
		}
		gui->end_content("Content##PlayersSubtab", true);
	}
}