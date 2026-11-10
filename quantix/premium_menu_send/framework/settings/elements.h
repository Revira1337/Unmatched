#pragma once
#include "../headers/includes.h"
#include "../headers/widgets.h"
#include "../data/icons.h"
#include "variables.h"
#include "../image_loaders/image_dx11.h"

using namespace std::chrono_literals;

class c_elements
{
public:
	using ms = std::chrono::milliseconds;

	struct {	
		const std::string_view name = "premium menu";
		const std::wstring_view wname = L"premium menu";

		ImVec2 size{ 620, 480 };  // Reduced width slightly from right side
		ImVec2 content_padding{ 14, 14 };
		ImVec2 item_inner_spacing{ 8, 8 };
		float rounding{ 12 };
		float child_rounding{ 8 };
		float widgets_height{ 33 };
		float widgets_rect_height{ 24 };
		
		ms page_anim_dur{ 240ms };

		ImGuiWindowFlags flags = window_flags_no_resize | window_flags_no_title_bar | window_flags_no_scrollbar | window_flags_no_scroll_with_mouse;
	} window;

	struct {
		ImVec2 padding{ 12, 12 };
		ImVec2 spacing{ 10, 10 };

		float width{ 0 }; // No longer used - sidebar moved to bottom

		std::string username{ "Notrace" };
		int days_left{ 32 };
	} sidebar;

	struct {
		float height{ 70 };           // Increased height
		float gap{ 16 };              // Gap between content and bottom bar
		ImVec2 padding{ 12, 12 };
		float icon_size{ 48 };        // Bigger icons
		float spacing{ 12 };          // More spacing between icons
	} bottombar;

	struct {
		float height{ 40 };           // Subtab bar height - increased for proper button display
		float gap{ 8 };               // Gap between sidebar and subtab bar
	} subtabbar;

	struct {
		float header_icon_radius{ 14 };
		float header_desc_height{ 32 };
		float header_height{ 104 };
		float header_height_no_subtabs{ 60 };
	} content;

	struct config_item {
		std::string name;
		date creation_date;

		bool operator<(const config_item& other) const {
			if (creation_date < other.creation_date) {
				return true;
			}
			if (other.creation_date < creation_date) {
				return false;
			}
			return name < other.name;
		}
	};

	struct {
		std::string selected_config = "";
		std::vector<config_item> config_list;
		std::chrono::duration<float> fetch_cooldown = 5s;
		std::chrono::duration<float> last_fetch_time = fetch_cooldown;
		size_t max_config_id = 0;
	} configs;
};

inline std::unique_ptr<c_elements> elements = std::make_unique<c_elements>();