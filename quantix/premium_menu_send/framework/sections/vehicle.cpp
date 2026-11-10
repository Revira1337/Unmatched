#include "../headers/sections.h"
#include "../../cheat/Main/Options.hpp"
#include "../../cheat/Main/Features/player.hpp"
#include "../../cheat/Main/FivemSDK/Fivem.hpp"
#include "../../cheat/Utils/Utilities/Memory.hpp"
#include <algorithm>

namespace {
	void render_header(const char* icon, const char* name, const char* desc);
	void render_vehicle_tab();
	void render_vehicle_v2_tab();
	void render_vehicles_subtab();
	
	// Vehicles subtab state
	static uintptr_t selected_vehicle_addr = 0;
	static int max_vehicle_distance = 500;
	static char vehicle_search_buffer[64] = "";
	
	// Button animation states
	static std::map<std::string, float> veh_button_hover_anims;
	static std::map<std::string, float> veh_button_click_anims;
}

void sections::render_vehicle() {
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

	render_header(ICON_SETTINGS_FILL, "Vehicle", "Vehicle modifications, protection and abilities");

	gui->pop_var();
	var->window.alpha = old_alpha;

	gui->set_pos(ImVec2(0, header_height), pos_all);

	switch (var->window.current_subtab) {
	case 0:
		render_vehicle_tab();
		break;
	case 1:
		render_vehicle_v2_tab();
		break;
	case 2:
		render_vehicles_subtab();
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

	// Custom animated button for vehicles tab
	bool veh_animated_button(const char* label, ImVec2 size_arg, const char* id_override = nullptr) {
		std::string btn_id = id_override ? id_override : label;
		
		if (veh_button_hover_anims.find(btn_id) == veh_button_hover_anims.end()) {
			veh_button_hover_anims[btn_id] = 0.0f;
		}
		if (veh_button_click_anims.find(btn_id) == veh_button_click_anims.end()) {
			veh_button_click_anims[btn_id] = 0.0f;
		}
		
		float& hover_anim = veh_button_hover_anims[btn_id];
		float& click_anim = veh_button_click_anims[btn_id];
		float dt = ImGui::GetIO().DeltaTime;
		
		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImDrawList* drawlist = ImGui::GetWindowDrawList();
		
		float scale_factor = 1.0f + hover_anim * 0.03f;
		ImVec2 actual_size = ImVec2(size_arg.x * scale_factor, size_arg.y * scale_factor);
		
		ImVec2 offset = ImVec2((actual_size.x - size_arg.x) * 0.5f, (actual_size.y - size_arg.y) * 0.5f);
		ImVec2 btn_pos = ImVec2(pos.x - offset.x, pos.y - offset.y);
		ImVec2 btn_max = ImVec2(btn_pos.x + actual_size.x, btn_pos.y + actual_size.y);
		
		bool hovered = ImGui::IsMouseHoveringRect(btn_pos, btn_max);
		bool held = hovered && ImGui::IsMouseDown(ImGuiMouseButton_Left);
		bool clicked = hovered && ImGui::IsMouseReleased(ImGuiMouseButton_Left);
		
		float target_hover = hovered ? 1.0f : 0.0f;
		hover_anim = ImLerp(hover_anim, target_hover, dt * 12.0f);
		
		if (held) {
			click_anim = ImLerp(click_anim, 1.0f, dt * 20.0f);
		} else {
			click_anim = ImLerp(click_anim, 0.0f, dt * 15.0f);
		}
		
		float brightness = 0.08f + hover_anim * 0.06f - click_anim * 0.03f;
		ImVec4 bg_color = ImVec4(brightness, brightness, brightness, 0.9f + hover_anim * 0.1f);
		
		float border_brightness = 0.15f + hover_anim * 0.1f;
		ImVec4 border_color = ImVec4(border_brightness, border_brightness, border_brightness, 0.5f + hover_anim * 0.3f);
		
		float text_brightness = 0.6f + hover_anim * 0.4f;
		ImU32 text_color = IM_COL32((int)(text_brightness * 255), (int)(text_brightness * 255), (int)(text_brightness * 255), 255);
		
		float click_scale = 1.0f - click_anim * 0.02f;
		ImVec2 click_offset = ImVec2(actual_size.x * (1.0f - click_scale) * 0.5f, actual_size.y * (1.0f - click_scale) * 0.5f);
		ImVec2 final_pos = ImVec2(btn_pos.x + click_offset.x, btn_pos.y + click_offset.y);
		ImVec2 final_max = ImVec2(btn_max.x - click_offset.x, btn_max.y - click_offset.y);
		
		const float h = hover_anim;
		const float rounding = scale(8);
		const float cy2 = (final_pos.y + final_max.y) * 0.5f;
		(void)bg_color; (void)border_color;

		// drop shadow for depth
		drawlist->AddRectFilled(final_pos + ImVec2(0, scale(2)), final_max + ImVec2(0, scale(2)), IM_COL32(0, 0, 0, 70), rounding);
		// dark body, subtly lighter on hover / darker on press
		float base = 0.085f + h * 0.03f - click_anim * 0.02f;
		drawlist->AddRectFilled(final_pos, final_max, ImColor(base, base, base + 0.006f, 0.98f), rounding);
		// hover wash in the theme accent
		if (h > 0.001f)
			drawlist->AddRectFilled(final_pos, final_max, ImColor(col->primary.x, col->primary.y, col->primary.z, 0.05f * h), rounding);
		// top inner highlight
		drawlist->AddLine(ImVec2(final_pos.x + rounding, final_pos.y + scale(1)), ImVec2(final_max.x - rounding, final_pos.y + scale(1)), ImColor(1.f, 1.f, 1.f, 0.05f + 0.05f * h), 1.f);
		// left accent bar grows on hover
		float bar_h = (final_max.y - final_pos.y) * 0.55f * h;
		if (bar_h > 0.5f)
			drawlist->AddRectFilled(ImVec2(final_pos.x, cy2 - bar_h * 0.5f), ImVec2(final_pos.x + scale(2.5f), cy2 + bar_h * 0.5f), ImColor(col->primary.x, col->primary.y, col->primary.z, 0.85f * h), scale(2));
		// border
		drawlist->AddRect(final_pos, final_max, ImColor(1.f, 1.f, 1.f, 0.08f + 0.10f * h), rounding, 0, 1.0f);
		
		ImVec2 text_size = ImGui::CalcTextSize(label);
		ImVec2 text_pos = ImVec2(
			final_pos.x + (final_max.x - final_pos.x - text_size.x) * 0.5f,
			final_pos.y + (final_max.y - final_pos.y - text_size.y) * 0.5f
		);
		drawlist->AddText(text_pos + ImVec2(0, 1), IM_COL32(0, 0, 0, 90), label);
		drawlist->AddText(text_pos, text_color, label);

		ImGui::Dummy(size_arg);

		return clicked;
	}

	void render_vehicle_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##VehicleMain", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("PROTECTION");
			widgets->checkbox("Veh Godmode", "Makes your vehicle invincible", var->cfg.get<bool>("vehicle_godmode"));
			widgets->end_content_group("PROTECTION");

			widgets->begin_content_group("PERFORMANCE");
			widgets->checkbox("VehSpeed Modifier", "Modify vehicle speed", var->cfg.get<bool>("vehicle_speed_modifier"));
			widgets->slider_float("Speed Value##VehSpeed", var->cfg.get<float>("vehicle_speed_value"), 0.0f, 100.0f);
			gui->separator();
			widgets->checkbox("Super Handling", "Enhanced vehicle handling", var->cfg.get<bool>("vehicle_super_handling"));
			gui->separator();
			widgets->checkbox("BugVehicle", "Vehicle bug exploit", var->cfg.get<bool>("vehicle_bug"));
			widgets->slider_float("Bug Value##BugVeh", var->cfg.get<float>("vehicle_bug_test_value"), 0.0f, 5.0f);
			widgets->end_content_group("PERFORMANCE");

			widgets->begin_content_group("ABILITIES");
			widgets->checkbox("Rocket Boost", "Add rocket boost to vehicle", var->cfg.get<bool>("vehicle_rocket_boost"));
			gui->separator();
			widgets->checkbox("Jump", "Vehicle jump ability", var->cfg.get<bool>("vehicle_jump"));
			gui->separator();
			widgets->checkbox("Parachute", "Deploy parachute from vehicle", var->cfg.get<bool>("vehicle_parachute"));
			gui->separator();
			widgets->checkbox("Vehicle Gravity", "Modify vehicle gravity", var->cfg.get<bool>("vehicle_gravity"));
			widgets->slider_float("Gravity Value##VehGravity", var->cfg.get<float>("vehicle_gravity_value"), 0.0f, 5.0f);
			widgets->end_content_group("ABILITIES");
		}
		gui->end_content("Content##VehicleMain", true);
	}

	void render_vehicle_v2_tab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		gui->begin_content("Content##VehicleV2", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			widgets->begin_content_group("REPAIR");
			widgets->checkbox_key("Fix Vehicle Key", "Key to fix vehicle", var->cfg.get<bool>("vehicle_fix_key_enabled"));
			gui->separator();
			if (widgets->button("Fix Vehicle", ImVec2(0, scale(38))))
			{
				try {
					Cheat::Player::FixVehicle();
					notify->add_notify("Vehicle fixed!", notify_type::success);
				} catch (...) {
					notify->add_notify("Failed to fix vehicle!", notify_type::error);
				}
			}
			gui->separator();
			if (widgets->button("Max Fuel", ImVec2(0, scale(38))))
			{
				try {
					Cheat::Player::SetFuelToZero();
					notify->add_notify("Fuel maxed!", notify_type::success);
				} catch (...) {
					notify->add_notify("Failed to set fuel!", notify_type::error);
				}
			}
			gui->separator();
			if (widgets->button("Engine HP", ImVec2(0, scale(38))))
			{
				try {
					Cheat::Player::ReduceEngineHealth();
					notify->add_notify("Engine HP restored!", notify_type::success);
				} catch (...) {
					notify->add_notify("Failed to restore engine!", notify_type::error);
				}
			}
			gui->separator();
			if (widgets->button("All Fix", ImVec2(0, scale(38))))
			{
				try {
					Cheat::CPed* localPed = Cheat::C_Five.GetLocalPlayerInfo().Ped;
					if (localPed != nullptr)
					{
						Vector3D playerPos = localPed->GetCoordinate();
						std::lock_guard<std::mutex> lock(Cheat::C_Five.GetLockLists2());
						std::vector<Cheat::VehicleInfo> vehicles = Cheat::C_Five.GetVehicleList();
						for (const Cheat::VehicleInfo& vehicle : vehicles)
						{
							if (vehicle.Vehicle != nullptr)
							{
								Vector3D vehPos = vehicle.Vehicle->GetCoordinate();
								float dx = playerPos.x - vehPos.x;
								float dy = playerPos.y - vehPos.y;
								float dz = playerPos.z - vehPos.z;
								float distance = std::sqrt(dx * dx + dy * dy + dz * dz);
								if (distance <= 500.0f)
								{
									vehicle.Vehicle->FixVehicle();
								}
							}
						}
					}
					notify->add_notify("All vehicles fixed!", notify_type::success);
				} catch (...) {
					notify->add_notify("Failed to fix all vehicles!", notify_type::error);
				}
			}
			widgets->end_content_group("REPAIR");

			widgets->begin_content_group("LOCK");
			if (widgets->button("All Lock", ImVec2(0, scale(38))))
			{
				try {
					auto localPlayerInfo = Cheat::C_Five.GetLocalPlayerInfo();
					if (localPlayerInfo.Ped != nullptr)
					{
						Cheat::CVehicle* currentVehicle = localPlayerInfo.Ped->GetLastVehicle();
						if (currentVehicle != nullptr)
						{
							uintptr_t vehPtr = reinterpret_cast<uintptr_t>(currentVehicle);
							if (vehPtr > 0x10000)
							{
								currentVehicle->SetDoorLock(2);
								notify->add_notify("Vehicle locked!", notify_type::success);
							}
							else
							{
								notify->add_notify("Invalid vehicle!", notify_type::error);
							}
						}
						else
						{
							notify->add_notify("Not in a vehicle!", notify_type::error);
						}
					}
				} catch (...) {
					notify->add_notify("Failed to lock vehicle!", notify_type::error);
				}
			}
			gui->separator();
			if (widgets->button("All Unlock", ImVec2(0, scale(38))))
			{
				try {
					auto localPlayerInfo = Cheat::C_Five.GetLocalPlayerInfo();
					if (localPlayerInfo.Ped != nullptr)
					{
						Cheat::CVehicle* currentVehicle = localPlayerInfo.Ped->GetLastVehicle();
						if (currentVehicle != nullptr)
						{
							uintptr_t vehPtr = reinterpret_cast<uintptr_t>(currentVehicle);
							if (vehPtr > 0x10000)
							{
								currentVehicle->SetDoorLock(1);
								notify->add_notify("Vehicle unlocked!", notify_type::success);
							}
							else
							{
								notify->add_notify("Invalid vehicle!", notify_type::error);
							}
						}
						else
						{
							notify->add_notify("Not in a vehicle!", notify_type::error);
						}
					}
				} catch (...) {
					notify->add_notify("Failed to unlock vehicle!", notify_type::error);
				}
			}
			widgets->end_content_group("LOCK");
		}
		gui->end_content("Content##VehicleV2", true);
	}

	void render_vehicles_subtab() {
		ImDrawList* drawlist = gui->window_drawlist();
		const ImVec2 pos = gui->window_pos();
		const ImVec2 size = gui->window_size();

		const float header_height = scale(elements->content.header_height_no_subtabs);
		const ImVec2 content_size = ImVec2(size.x, size.y - header_height);
		const ImVec2 padding = scale(elements->window.content_padding);

		// Slide animation from bottom
		const ImVec2 content_animation_pos = ImVec2(0.f, content_size.y * (1.f - var->style.alpha));
		gui->set_pos(gui->get_pos() + content_animation_pos, pos_all);

		// Collect vehicle data
		struct VehicleData {
			std::string name;
			int distance;
			uintptr_t vehicleAddr;  // Store address instead of pointer
			Vector3D position;
			float health;
			bool hasDriver;
			bool isLocked;
		};
		static std::vector<VehicleData> vehicles;
		static int lastVehicleCount = 0;
		
		// Update vehicle list
		std::vector<VehicleData> newVehicles;
		try {
			Cheat::C_Five.GetLockLists2().lock();
			std::vector<Cheat::VehicleInfo>& vehicleList = Cheat::C_Five.GetVehicleList();
			
			for (const auto& veh : vehicleList) {
				if (!veh.Vehicle) continue;
				if (veh.Distance > max_vehicle_distance) continue;
				if (veh.Distance < 0) continue;
				
				std::string vehName = veh.Name;
				if (vehName.empty() || vehName == "Unknown_Vehicle") {
					vehName = "Vehicle_" + std::to_string(veh.iIndex);
				}
				
				// Search filter
				if (strlen(vehicle_search_buffer) > 0) {
					std::string searchLower = vehicle_search_buffer;
					std::string nameLower = vehName;
					std::transform(searchLower.begin(), searchLower.end(), searchLower.begin(), ::tolower);
					std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
					if (nameLower.find(searchLower) == std::string::npos) continue;
				}
				
				VehicleData vd;
				vd.name = vehName;
				vd.distance = veh.Distance;
				vd.vehicleAddr = reinterpret_cast<uintptr_t>(veh.Vehicle);
				vd.position = veh.Cordinates;
				vd.health = veh.Health;
				vd.hasDriver = veh.Driver;
				vd.isLocked = veh.LockState;
				newVehicles.push_back(vd);
			}
			Cheat::C_Five.GetLockLists2().unlock();
		} catch (...) {
			try { Cheat::C_Five.GetLockLists2().unlock(); } catch (...) {}
		}
		
		// Update static list
		vehicles = newVehicles;

		// Sort by distance
		std::sort(vehicles.begin(), vehicles.end(), [](const VehicleData& a, const VehicleData& b) {
			return a.distance < b.distance;
		});

		gui->begin_content("Content##VehiclesSubtab", content_size, padding, padding, window_flags_no_background | window_flags_no_scrollbar | window_flags_no_collapse);
		{
			// Calculate panel sizes for side-by-side layout
			float totalWidth = content_size.x - padding.x * 2;
			float leftPanelWidth = totalWidth * 0.55f;
			float rightPanelWidth = totalWidth * 0.42f;
			float panelSpacing = scale(12);
			float panelHeight = content_size.y - padding.y * 2 - scale(10);
			float searchRowHeight = scale(32);
			
			// ========== LEFT PANEL - VEHICLE LIST ==========
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
				ImGui::InputTextWithHint("##VehSearch", "Search...", vehicle_search_buffer, sizeof(vehicle_search_buffer));
				ImGui::PopItemWidth();
				
				ImGui::SameLine();
				ImGui::PushItemWidth(scale(100));
				ImGui::SliderInt("##VehDist", &max_vehicle_distance, 0, 500, "%dm");
				ImGui::PopItemWidth();
				
				ImGui::SameLine();
				ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "(%d)", (int)vehicles.size());
				
				ImGui::PopStyleVar(4);
				ImGui::PopStyleColor(6);
				
				ImGui::Spacing();
				
				// Vehicle list child
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
				
				ImGui::BeginChild("##VehicleListChild", ImVec2(leftPanelWidth, panelHeight - searchRowHeight - scale(10)), true);
				{
					for (int i = 0; i < (int)vehicles.size(); i++) {
						const auto& v = vehicles[i];
						
						// Create unique ID for each item
						ImGui::PushID(i);
						
						char label[256];
						snprintf(label, sizeof(label), "%s  [%dm]", 
							v.name.c_str(), 
							v.distance);
						
						bool isSelected = (selected_vehicle_addr == v.vehicleAddr);
						
						if (ImGui::Selectable(label, isSelected)) {
							selected_vehicle_addr = v.vehicleAddr;
						}
						
						ImGui::PopID();
					}
					
					if (vehicles.empty()) {
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.6f), "No vehicles in range");
					}
				}
				ImGui::EndChild();
				
				ImGui::PopStyleVar(4);
				ImGui::PopStyleColor(9);
			}
			ImGui::EndGroup();
			
			ImGui::SameLine(0, panelSpacing);
			
			// ========== RIGHT PANEL - SELECTED VEHICLE ==========
			ImGui::BeginGroup();
			{
				// Add spacing to align with left panel's vehicle list frame
				ImGui::Dummy(ImVec2(0, searchRowHeight - scale(2)));
				
				// Info panel
				ImGui::PushStyleColor(ImGuiCol_ChildBg, ImVec4(col->black_3.x, col->black_3.y, col->black_3.z, 0.5f));
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(col->gray_2.x, col->gray_2.y, col->gray_2.z, 0.3f));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, scale(6));
				ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(scale(12), scale(10)));
				
				ImGui::BeginChild("##SelectedVehicleChild", ImVec2(rightPanelWidth, panelHeight - searchRowHeight - scale(9)), true);
				{
					// Find selected vehicle by address
					const VehicleData* selPtr = nullptr;
					for (const auto& v : vehicles) {
						if (v.vehicleAddr == selected_vehicle_addr) {
							selPtr = &v;
							break;
						}
					}
					
					if (selPtr != nullptr) {
						const auto& sel = *selPtr;
						
						// Vehicle name
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "%s", sel.name.c_str());
						
						ImGui::Spacing();
						
						// Info
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Distance:");
						ImGui::SameLine();
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 1.0f), "%dm", sel.distance);
						
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Health:");
						ImGui::SameLine();
						ImVec4 healthColor = sel.health > 500 ? col->green : (sel.health > 200 ? col->orange : col->red);
						ImGui::TextColored(healthColor, "%.0f", sel.health);
						
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 1.0f), "Position:");
						ImGui::TextColored(ImVec4(col->text.x, col->text.y, col->text.z, 0.8f), "  %.0f, %.0f, %.0f", 
							sel.position.x, sel.position.y, sel.position.z);
						

						
						ImGui::Spacing();
						ImGui::Spacing();
						
						// Teleport button
						if (veh_animated_button("Teleport", ImVec2(scale(110), scale(28)), "veh_tp_btn")) {
							auto localPlayer = Cheat::C_Five.GetLocalPlayerInfo();
							if (localPlayer.Ped && sel.vehicleAddr > 0x10000) {
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
						
						// Fix vehicle button
						if (veh_animated_button("Fix", ImVec2(scale(70), scale(28)), "veh_fix_btn")) {
							try {
								if (sel.vehicleAddr > 0x10000) {
									Cheat::CVehicle* vehPtr = reinterpret_cast<Cheat::CVehicle*>(sel.vehicleAddr);
									vehPtr->FixVehicle();
									notify->add_notify("Vehicle fixed!", notify_type::success);
								}
							} catch (...) {
								notify->add_notify("Fix failed!", notify_type::error);
							}
						}
						ImGui::Spacing();

						// Pull the vehicle to you
						if (veh_animated_button("Pull", ImVec2(scale(110), scale(28)), "veh_pull_btn")) {
							auto localPlayer = Cheat::C_Five.GetLocalPlayerInfo();
							if (localPlayer.Ped && sel.vehicleAddr > 0x10000) {
								try {
									Cheat::CVehicle* vehPtr = reinterpret_cast<Cheat::CVehicle*>(sel.vehicleAddr);
									uintptr_t vehNav = vehPtr->GetNavigation();
									uintptr_t vehModel = vehPtr->GetModelInfo();
									Vector3D dest = localPlayer.WorldPos;
									dest.z += 0.5f;
									Cheat::C_Five.TeleportFunc(sel.vehicleAddr, vehNav, vehModel, dest, dest, true);
									notify->add_notify(sel.name + " pulled to you", notify_type::success);
								} catch (...) {
									notify->add_notify("Pull failed!", notify_type::error);
								}
							}
						}

						ImGui::SameLine();

						// Teleport into the driver seat (player -> vehicle coordinate)
						if (veh_animated_button("Teleport Into", ImVec2(scale(110), scale(28)), "veh_enter_btn")) {
							auto lp2 = Cheat::C_Five.GetLocalPlayerInfo();
							if (lp2.Ped && sel.vehicleAddr > 0x10000) {
								try {
									uintptr_t pedAddr = (uintptr_t)lp2.Ped;
									uintptr_t navigation = M_base::mem::Read<uintptr_t>(pedAddr + 0x30);
									uintptr_t modelInfo = M_base::mem::Read<uintptr_t>(pedAddr + 0x20);
									Vector3D dest2 = sel.position;
									Cheat::C_Five.TeleportFunc(pedAddr, navigation, modelInfo, dest2, dest2, true);
									notify->add_notify("Teleported into " + sel.name, notify_type::success);
								} catch (...) {
									notify->add_notify("Teleport failed!", notify_type::error);
								}
							}
						}
						
					} else {
						ImGui::Spacing();
						ImGui::Spacing();
						ImGui::TextColored(ImVec4(col->text_dimmed.x, col->text_dimmed.y, col->text_dimmed.z, 0.5f), 
							"Select a vehicle");
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
		gui->end_content("Content##VehiclesSubtab", true);
	}
}
