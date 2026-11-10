#include "headers/sections.h"
#include "data/fonts.h"
#include "data/images.h"
#include <fstream>

// External debug log function
extern void DebugLog(const char* format, ...);


// External render functions from main.cpp
extern void OnRender();
extern void OnDraw();

void c_gui::render()
{
	try {
		// Safety checks
		if (!var || !elements || !col) {
			return;
		}
		
		try {
			update();
		} catch (...) {
			return;
		}

		try {
			if (notify) {
				notify->process_notifies(ImRect(ImVec2(0, 0), var->window.screen_size), notify_anchor::topright);
			}
		} catch (...) {
		}

		// Calculate sizes - INTEGRATED LAYOUT: Sidebar inside menu window
		const float sidebar_width = scale(60.0f);  // Narrow vertical sidebar
		const ImVec2 content_size = scale(elements->window.size);
		const ImVec2 total_size = ImVec2(sidebar_width + content_size.x, content_size.y);
		
		// Center position for the whole layout (initial position only)
		const ImVec2 layout_start = (var->window.screen_size - total_size) * 0.5f;
		
		// Set window position - sidebar handles dragging automatically
		gui->set_next_window_pos(layout_start, gui_cond_once);
		set_next_window_size(total_size);
		
		begin("menu", nullptr, elements->window.flags);
		{
			const ImVec2 pos = window_pos();
			const ImVec2 size = window_size();
			ImDrawList* drawlist = window_drawlist();
			
			// Store menu position for flyout sync
			var->window.menu_pos = pos;
			
			// Draw sidebar background on left side FIRST (before content)
			const ImVec2 sidebar_min = pos;
			const ImVec2 sidebar_max = ImVec2(pos.x + sidebar_width, pos.y + size.y);
			
			// Content area background (right side)
			const ImVec2 content_min = ImVec2(pos.x + sidebar_width, pos.y);
			const ImVec2 content_max = pos + size;
			drawlist->AddRectFilled(content_min, content_max, gui->u32(col->black), scale(elements->window.rounding), ImDrawFlags_RoundCornersRight);
			
			// Sidebar background (left side) - same color as menu content
			drawlist->AddRectFilled(sidebar_min, sidebar_max, gui->u32(col->black), scale(elements->window.rounding), ImDrawFlags_RoundCornersLeft);
			
			// Draw separator line between sidebar and content
			drawlist->AddLine(
				ImVec2(pos.x + sidebar_width, pos.y + scale(1.0f)),
				ImVec2(pos.x + sidebar_width, pos.y + size.y - scale(1.0f)),
				gui->u32(col->gray_3, 0.5f),
				1.0f
			);
			
			// Create child window for content area (offset to right of sidebar)
			ImGui::SetCursorPos(ImVec2(sidebar_width, 0));
			ImGui::BeginChild("content_area", ImVec2(content_size.x, content_size.y), false, ImGuiWindowFlags_NoBackground);
			
			var->window.alpha = tween("page_alpha", 0.f, 1.f, easing::QuadEaseInOut, elements->window.page_anim_dur, (var->window.current_page == var->window.target_page) && (var->window.current_subtab == var->window.target_subtab));
			if (var->window.alpha == 0.f)
			{
				var->window.current_subtab = var->window.target_subtab;
				var->window.current_page = var->window.target_page;

				if (!var->window.subtab_switched)
				{
					var->window.current_subtab = 0;
					var->window.target_subtab = 0;
				}
			}
			if (var->window.alpha == 1.f && var->window.subtab_switched) {
				var->window.subtab_switched = false;
			}	

			push_var(style_var_alpha, var->window.alpha);
			ImGui::BeginDisabled(var->window.alpha != 1.f);

			DebugLog("About to render page: %d", (int)var->window.current_page);

			try {
				switch (var->window.current_page) {
				case pages::AIM_ASSISTANCE: {
					sections::render_aim_assistance();
					break;
				}
				case pages::PLAYER: {
					sections::render_player();
					break;
				}
				case pages::VEHICLE: {
					sections::render_vehicle();
					break;
				}
				case pages::WEAPON: {
					sections::render_weapon();
					break;
				}
				case pages::ESP: {
					sections::render_esp();
					break;
				}
				case pages::RADAR: {
					sections::render_radar();
					break;
				}
			case pages::GLOBAL_SETTINGS: {
					sections::render_global_settings();
					break;
				}
				}
				DebugLog("Page rendered successfully");
			} catch (const std::exception& e) {
				DebugLog("Page render exception: %s", e.what());
			} catch (...) {
				DebugLog("Page render unknown exception");
			}

			ImGui::EndDisabled();
			pop_var();
			
			ImGui::EndChild();
			
			// Render sidebar content LAST (on top of everything)
			try {
				sections::render_sidebar_content(drawlist, sidebar_min, ImVec2(sidebar_width, size.y));
				DebugLog("Sidebar rendered");
			} catch (const std::exception& e) {
				DebugLog("Sidebar exception: %s", e.what());
			} catch (...) {
				DebugLog("Sidebar unknown exception");
			}
		}
		// Store main menu position AFTER window processing (gets actual dragged position)
		var->window.menu_pos = window_pos();
		end();
		
		DebugLog("Main menu window ended");

		// Draw semi-transparent black background overlay when menu is open
		{
			ImDrawList* bg_drawlist = ImGui::GetBackgroundDrawList();
			ImVec2 screen_size = var->window.screen_size;
			bg_drawlist->AddRectFilled(ImVec2(0, 0), screen_size, IM_COL32(0, 0, 0, 102)); // 40% opacity
		}
		
		// Subtabs are now rendered in the flyout menu (sidebar.cpp)
		// Old bottom subtab bar has been removed
		
		// Call main.cpp render functions for features
		try {
			OnRender();
			OnDraw();
		} catch (...) {
			DebugLog("OnRender/OnDraw exception");
		}
		
		DebugLog("gui->render() completed successfully");
		
	} catch (const std::exception& e) {
		DebugLog("gui->render() FATAL exception: %s", e.what());
	} catch (...) {
		DebugLog("gui->render() FATAL unknown exception");
	}
}

void c_gui::init_fonts()
{
	font->add("semibold14", SFCompactRoundedSemibold_compressed_data, SFCompactRoundedSemibold_compressed_size, 16.f);
	font->add("semibold13", SFCompactRoundedSemibold_compressed_data, SFCompactRoundedSemibold_compressed_size, 15.f);

	font->add("medium14", SFCompactRoundedMedium_compressed_data, SFCompactRoundedMedium_compressed_size, 16.f);
	font->add("medium13", SFCompactRoundedMedium_compressed_data, SFCompactRoundedMedium_compressed_size, 15.f);
	font->add("medium12", SFCompactRoundedMedium_compressed_data, SFCompactRoundedMedium_compressed_size, 14.f);
	font->add("medium11", SFCompactRoundedMedium_compressed_data, SFCompactRoundedMedium_compressed_size, 13.f);

	font->add("regular14", SFCompactRoundedRegular_compressed_data, SFCompactRoundedRegular_compressed_size, 16.f);

	font->add("barcode20", LibreBarcode39_compressed_data, LibreBarcode39_compressed_size, 22.f);

	font->add("cs_icons16", CS2Icons_compressed_data, CS2Icons_compressed_size, 18.f, { 57345, 57860 });

	font->add("icons24", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 24.f, {}, true);
	font->add("icons20", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 20.f, {}, true);
	font->add("icons16", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 16.f, {}, true);
	font->add("icons15", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 15.f, {}, true);
	font->add("icons14", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 14.f, {}, true);
	font->add("icons13", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 13.f, {}, true);
	font->add("icons12", PremiumIcons_compressed_data, PremiumIcons_compressed_size, 12.f, {}, true);
}

void c_image::init(BackendDeviceType device)
{
	image->add(device, "character", esp_char_png, sizeof(esp_char_png), { 371.f, 506.f });
	image->add(device, "skeleton", skeleton_png, sizeof(skeleton_png), { 144.f, 243.f });
	image->add(device, "logo", logo1_png, sizeof(logo1_png), { 750.f, 748.f });

	bool loadedLogo = false;
	loadedLogo = loadedLogo || image->add_file(device, "logo_bg_removed_white", "logo_bg_removed_white.png");
	loadedLogo = loadedLogo || image->add_file(device, "logo_bg_removed_white", "..\\logo_bg_removed_white.png");
	loadedLogo = loadedLogo || image->add_file(device, "logo_bg_removed_white", "..\\..\\logo_bg_removed_white.png");
	loadedLogo = loadedLogo || image->add_file(device, "logo_bg_removed_white", "..\\..\\..\\logo_bg_removed_white.png");
	loadedLogo = loadedLogo || image->add_file(device, "logo_bg_removed_white", "..\\..\\..\\..\\logo_bg_removed_white.png");
	if (!loadedLogo) {
		image->add(device, "logo_bg_removed_white", logo1_png, sizeof(logo1_png), { 750.f, 748.f });
	}
}

void c_gui::set_style()
{
	auto* style = &var->style;

	style->colors[style_col_window_bg] = col->black;
	style->colors[style_col_popup_bg] = col->gray;
	style->colors[style_col_child_bg] = col->black_3;
	style->colors[style_col_text_selected_bg] = ImVec4(col->primary.x, col->primary.y, col->primary.z, 0.5f);
	style->colors[style_col_border] = col->gray_2;
	style->colors[style_col_separator] = col->gray_2;

	style->window_min_size = { 0, 0 };
	style->scrollbar_border_padding = { 0, 0 };
	style->scrollbar_size = 0.f;
	style->grab_min_size = scale(11);
	style->log_slider_deadzone = 0.f;
	style->window_rounding = scale(elements->window.rounding);
	style->window_padding = { 0, 0 };
	style->window_border_size = 0.f;
	style->popup_rounding = scale(6);
	style->popup_border_size = 0.f;
	style->child_border_size = 0.5f;
	style->child_rounding = scale(elements->window.child_rounding);
	style->frame_rounding = scale(8.f);
	style->item_inner_spacing = scale(elements->window.item_inner_spacing);
}

// Dummy implementations for external functions
void OnRender() {
    // Called from old framework - now handled by new menu system
}

void OnDraw() {
    // Called from old framework - now handled by new menu system
}

void DebugLog(const char* format, ...) {
    // Debug logging - can be implemented if needed
}
