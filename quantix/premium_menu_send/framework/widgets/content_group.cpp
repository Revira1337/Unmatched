#include "../headers/widgets.h"
#include "../headers/includes.h"
#include "../headers/functions.h"
#include "../headers/animations.h"
#include "../settings/colors.h"
#include "../headers/draw.h"
#include "../headers/font.h"
#include "../data/icons.h"

void c_widgets::begin_content_group(const char* label)
{
	const ImVec2 padding = scale(14, 14);
	const ImVec2 spacing = scale(10, 10);

	const float title_spacing_x = scale(10);

	ImGuiWindow* window = gui->get_window();
	ImDrawList* drawlist = window->DrawList;
	
	const ImVec2 pos = window->DC.CursorPos;
	const ImVec2 cursor_pos = gui->get_pos();
	const float title_height = gui->text_size(font->get("medium12"), label, nullptr, true).y;

	draw->text(drawlist, font->get("medium12"), pos + ImVec2(title_spacing_x, 0), gui->u32(col->text_dimmed), gui->hide_text_hint(label));

	gui->set_pos(cursor_pos + ImVec2(0, title_height + var->style.item_inner_spacing.x), pos_all);
	gui->begin_content(label, ImVec2(0,0), padding, spacing, window_flags_no_scrollbar | window_flags_no_scroll_with_mouse | window_flags_no_move, (child_flags_always_auto_resize | child_flags_auto_resize_y), true);
}

void c_widgets::end_content_group(const char* label)
{
	gui->end_content(label);
}