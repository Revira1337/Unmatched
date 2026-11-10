#pragma once
#include "includes.h"
#include "../settings/variables.h"

using namespace ImGui;

inline ImVec2 scale(const ImVec2& vec) {
    return ImVec2(roundf(vec.x * var->gui.stored_dpi), roundf(vec.y * var->gui.stored_dpi));
}

inline ImVec2 scale(float x, float y) {
    return ImVec2(roundf(x * var->gui.stored_dpi), roundf(y * var->gui.stored_dpi));
}

inline float scale(float value) {
    return roundf(value * var->gui.stored_dpi);
}

enum positions
{
    pos_all,
    pos_x,
    pos_y
};

enum class popup_open_styles {
    topleft,
    topright,
    top,
    bottom,
    center
};

class c_gui
{
public:
    const char* hide_text_hint(const char* text);

    float fixed_speed(float speed) { return speed / ImGui::GetIO().Framerate; }

    //ImVec2 calc_anchor_pos(const ImRect& rect, anchor anch, const ImVec2& offset);

    bool begin(std::string_view name, bool* p_open = nullptr, window_flags flags = window_flags_none);

    void end();

    void push_color(style_col idx, ImU32 col);

    void pop_color(int count = 1);

    void push_var(style_var idx, float val);

    void push_var(style_var idx, const ImVec2& val);

    void pop_var(int count = 1);

    void push_font(ImFont* font);

    void pop_font();

    void set_pos(const ImVec2& pos, int type);

    void set_pos(float pos, int type);

    ImVec2 get_pos();

    void set_screen_pos(const ImVec2& pos, int type);

    void set_screen_pos(float pos, int type);

    ImVec2 get_screen_pos();

    void begin_group();

    void end_group();

    void begin_content(std::string_view id, const ImVec2& size, const ImVec2& padding = ImVec2(0, 0), const ImVec2& spacing = ImVec2(0, 0), window_flags win_flags = 0, child_flags ch_flags = 0, bool border = false);

    void end_content(std::string_view id, bool bot_gradient = false);

    bool begin_popup(const char* id, const ImVec2& pos, const ImVec2& size = ImVec2(0, 0), popup_open_styles style = popup_open_styles::top, bool is_modal = false, std::function<void(ImDrawList*, const ImVec2&, const ImVec2&)> custom_bg_draw_callback = nullptr);

    bool begin_popup_modal(const char* id, const ImVec2& size = ImVec2(0, 0));

    bool begin_tooltip(const char* id, const ImVec2& pos, const ImVec2& size = ImVec2(0, 0), popup_open_styles style = popup_open_styles::top);

    void set_item_tooltip(const char* text);

    void end_popup(const char* id, bool manual_skip_control = false);
    
    void end_tooltip(const char* id);

    bool is_popup_open(const char* str_id);

    void close_popup(const char* str_id);

    void open_popup(const char* str_id);

    void popup_skip_click();

    void close_current_popup();

    void sameline(float offset_from_start_x = 0.f, float spacing_w = -1.f);

    void dummy(const ImVec2& size);

    bool begin_def_child(std::string_view name, const ImVec2& size_arg = ImVec2(0, 0), child_flags child_flags = 0, window_flags window_flags = 0);

    void end_def_child();

    void set_next_window_pos(const ImVec2& pos, gui_cond cond = 0, const ImVec2& pivot = ImVec2(0, 0));

    void set_next_window_size(const ImVec2& size, gui_cond cond = 0);

    ImVec2 text_size(ImFont* font, const char* text, const char* text_end = nullptr, bool hide_text_after_double_hash = false, float wrap_width = -1.f);

    ImVec2 window_size();

    float window_width();

    float window_height();

    ImDrawList* window_drawlist();

    ImDrawList* foreground_drawlist();

    ImDrawList* background_drawlist();

    ImVec2 window_pos();

    ImGuiWindow* get_window();

    void push_id(const char* str_id);

    void push_id(const char* str_id_begin, const char* str_id_end);

    void push_id(const void* ptr_id);

    void push_id(int int_id);

    void pop_id();

    ImGuiID get_id(const char* str, const char* str_end);

    ImGuiID get_id(const void* ptr);

    ImGuiID get_id(int n);

    ImVec2 content_avail();

    ImVec2 content_max();

    void item_size(const ImVec2& size, float text_baseline_y = -1.f);

    void item_size(const ImRect& bb, float text_baseline_y = -1.f);

    bool item_add(const ImRect& bb, ImGuiID id, const ImRect* nav_bb_arg = NULL, ImGuiItemFlags extra_flags = 0);

    bool is_window_hovered(ImGuiHoveredFlags flags = 0);

    bool is_window_focused(ImGuiFocusedFlags flags = 0);

    void set_window_focus();

    void set_window_focus(const char* name);

    bool is_rect_visible(const ImVec2& size);

    ImVec2 adjust_window_pos(const ImVec2& rect, const ImVec2& window_size);

    bool button_behavior(const ImRect& bb, ImGuiID id, bool* out_hovered = nullptr, bool* out_held = nullptr, ImGuiButtonFlags flags = 0);

    bool invisible_button(const char* str_id, const ImVec2& size_arg, ImGuiButtonFlags flags = 0);

    void separator();

    ImVec4 u32_to_float4(ImU32 in);

    ImU32 float4_to_u32(const ImVec4& in);

    void rgb_to_hsv(float r, float g, float b, float& out_h, float& out_s, float& out_v);

    void hsv_to_rgb(float h, float s, float v, float& out_r, float& out_g, float& out_b);

    bool item_hoverable(const ImRect& bb, ImGuiID id, ImGuiItemFlags item_flags = 0);

    bool is_item_hovered(ImGuiHoveredFlags flags = 0);

    bool is_item_active();

    bool is_item_clicked(mouse_button mouse_button = 0);

    bool mouse_down(mouse_button button);

    bool mouse_clicked(mouse_button button, bool repeat = false);

    bool mouse_released(mouse_button button);

    bool mouse_double_clicked(mouse_button button);

    const char* text_end(const char* text);

    template<typename T>
    const char* get_fmt(char* text, T* value, std::string_view fmt)
    {
        ImFormatString(text, IM_ARRAYSIZE(text), fmt.data(), *value);
        return text + strlen(text);
    }

    bool is_window_cond(ImGuiWindow* window, const std::vector<std::string>& names);

    void set_style();

    void draw_decorations();

    void init_fonts();

    void update();

    void render();

    ImVec4 col_alpha(const ImVec4& col, float alpha);

    ImU32 u32(const ImVec4& col, float alpha = 0.99991f, bool fixed_alpha = false);

    const std::array<ImVec2, 6> calc_octagon_points(const ImRect& bb);

    void dpi_mouse_control();

    void cursor_message(std::string_view text);

    void handle_cursor_messages(ImDrawList* drawlist, ImFont* font);

    struct scrollbar_context {
        bool is_mouse_scroll = false;
        float clicked_pos = 0.f;
    };

    auto& get_scrollbars_context() {
        static std::map<ImGuiID, scrollbar_context> s_ctx;
        return s_ctx;
    }
};

inline std::unique_ptr<c_gui> gui = std::make_unique<c_gui>();