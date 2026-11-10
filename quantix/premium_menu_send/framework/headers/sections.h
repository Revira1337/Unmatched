#pragma once

#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/widgets.h"
#include "../headers/font.h"
#include "../headers/draw.h"
#include "../headers/animations.h"
#include "../settings/variables.h"
#include "../settings/elements.h"
#include "../settings/colors.h"
#include "../image_loaders/image_dx11.h"

namespace sections {
	void render_sidebar();
	void render_sidebar_content(ImDrawList* drawlist, const ImVec2& pos, const ImVec2& size);
	
	void render_aim_assistance();

	void render_player();
	void render_vehicle();
	void render_weapon();

	void render_esp();
	void render_radar();
	void render_world_manipulation();

	void render_global_settings();
}