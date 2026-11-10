#include <Windows.h>
#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/font.h"
#include "../headers/draw.h"
#include "../headers/config.h"
#include "../settings/elements.h"

void c_keybinds::load_binds_from_config(config& cfg)
{
	for (auto& option : cfg.get_options())
	{
		if (std::holds_alternative<config_keybind>(option.second))
		{
			config_keybind& cfg_keybind = std::get<config_keybind>(option.second);
			binds.emplace(option.first, keybind{ .key = cfg_keybind.key, .type = cfg_keybind.type, .linked_cfg_bind = &std::get<config_keybind>(option.second) });
		}
	}
}

// Helper function to convert ImGuiKey to VK code for GetAsyncKeyState
static int ImGuiKeyToVKCode(ImGuiKey key)
{
	switch (key)
	{
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
		case ImGuiKey_LeftSuper: return VK_LWIN;
		case ImGuiKey_RightCtrl: return VK_RCONTROL;
		case ImGuiKey_RightShift: return VK_RSHIFT;
		case ImGuiKey_RightAlt: return VK_RMENU;
		case ImGuiKey_RightSuper: return VK_RWIN;
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
		case ImGuiKey_MouseLeft: return VK_LBUTTON;
		case ImGuiKey_MouseRight: return VK_RBUTTON;
		case ImGuiKey_MouseMiddle: return VK_MBUTTON;
		case ImGuiKey_MouseX1: return VK_XBUTTON1;
		case ImGuiKey_MouseX2: return VK_XBUTTON2;
		default: return 0;
	}
}

void c_keybinds::process_binds()
{
	active_binds_coint_ = 0;
	for (auto& [bind_name, bind_data] : binds)
	{
		if (bind_data.disabled)
		{
			continue;
		}

		if (bind_data.key == ImGuiKey_None)
		{
			continue;
		}

		// Convert ImGuiKey to VK code for GetAsyncKeyState
		int vkCode = ImGuiKeyToVKCode(bind_data.key);
		if (vkCode == 0)
		{
			continue;
		}

		// Use GetAsyncKeyState for reliable key detection even when menu is hidden
		bool keyPressed = (GetAsyncKeyState(vkCode) & 0x8000) != 0;

		if (bind_data.type == switch_type::hold)
		{
			bind_data.value = keyPressed;
		}
		else // toggle mode
		{
			static std::unordered_map<std::string, bool> prevKeyStates;
			bool& prevState = prevKeyStates[bind_name];
			
			// Toggle on key press (not hold)
			if (keyPressed && !prevState)
			{
				bind_data.value = !bind_data.value;
			}
			prevState = keyPressed;
		}

		if (bind_data.callback && !bind_data.callback_called)
		{
			bind_data.callback(bind_data.value, bind_data.key, bind_data.type);
			bind_data.callback_called = true;
		}

		if (!bind_data.value)
		{
			bind_data.callback_called = false;
		}
		else
		{
			active_binds_coint_++;
		}
	}
}