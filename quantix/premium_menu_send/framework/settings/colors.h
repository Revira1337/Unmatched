#pragma once
#include "imgui.h"
#include <memory>

class c_colors
{
public:
	ImVec4 black = ImColor(13, 13, 13);
	ImVec4 black_2 = ImColor(16, 16, 16);
	ImVec4 black_3 = ImColor(19, 19, 19);
	ImVec4 gray = ImColor(29, 29, 29);
	ImVec4 gray_2 = ImColor(35, 35, 35);
	ImVec4 gray_3 = ImColor(36, 36, 36);
	ImVec4 gray_4 = ImColor(50, 50, 50);
	ImVec4 gray_5 = ImColor(61, 61, 61);
	ImVec4 primary = ImColor(232, 232, 232);

	ImVec4 text = ImColor(232, 232, 232);
	ImVec4 text_dimmed = ImColor(114, 114, 114);

	ImVec4 bbox = ImColor(50, 242, 104);
	ImVec4 hpbar = ImColor(50, 242, 104);
	ImVec4 armorbar = ImColor(105, 200, 254);
	ImVec4 bar_bg = ImColor(38, 39, 41);

	ImVec4 green = ImColor(119, 215, 125);
	ImVec4 orange = ImColor(215, 167, 119);
	ImVec4 red = ImColor(215, 119, 121);
};

inline std::unique_ptr<c_colors> col = std::make_unique<c_colors>();
