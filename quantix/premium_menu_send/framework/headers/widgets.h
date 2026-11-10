#pragma once
#include "includes.h"
#include "animations.h"
#include "binds.h"
#include "config.h"
#include "../settings/colors.h"

struct esp_preview_state;
struct date;
struct config_item_return;

class c_widgets
{
public:
    void begin_content_group(const char* label);
    void end_content_group(const char* label);

    bool button(const char* label, const ImVec2& size_arg);
    bool icon_button(const char* icon, const char* label, const ImVec2& size_arg = { 0, 0 }, bool style_inversed = false, const char* str_id = nullptr);
    config_item_return config_item(const char* name, date date, bool is_actived);
    bool tab(const char* label, const char* icon, const ImVec2& size, bool is_active);
    bool subtab(const char* label, bool is_active);
    bool checkbox(const char* label, const char* desc, bool& value);
    bool checkbox_setting(const char* label, const char* desc, bool& value);
    bool checkbox_key(const char* label, const char* desc, bool& value);
    bool keybind_only(const char* label, const char* desc);
    bool checkbox_color(const char* label, const char* desc, bool& value, ImVec4& color);
    bool checkbox_color(const char* label, const char* desc, bool& value, float color[4]);
    bool color_only(const char* label, const char* desc, ImVec4& color);
    bool color_only(const char* label, const char* desc, float color[4]);
    bool slider_float(const char* label, float& v, float v_min, float v_max, const char* format = "%.1f", ImGuiSliderFlags flags = 0);
    bool slider_int(const char* label, int& v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0);
    bool combo(const char* label, const char* desc, std::span<const char*> items, int& current_item);
    bool input(const char* label, const char* desc, const char* hint, char* buf, int buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* callback_user_data = nullptr);
    bool input(const char* label, const char* desc, const char* hint, std::string& str, int buf_size, ImGuiInputTextFlags flags = 0, ImGuiInputTextCallback callback = nullptr, void* callback_user_data = nullptr);
    bool esp_preview(const char* str_id, esp_preview_state& state);
    bool bone_selector_preview(const char* str_id, const char* prefix, float preview_alpha);
};

void destroy_config_entry(ImGuiWindow* window, const char* name);

struct date {
    int day;
    int month;
    int year;
    int time;

    bool operator<(const date& other) const {
        if (year != other.year) {
            return year > other.year;
        }
        if (month != other.month) {
            return month > other.month;
        }
        if (day != other.day) {
            return day > other.day;
        }
        return time > other.time;
    }
};

struct config_item_return {
    bool deleted = false;
    bool loaded = false;
    bool saved = false;
};

enum displacement
{
    TOP,
    LEFT,
    RIGHT,
    BOTTOM
};

struct esp_preview_state
{
    struct text_place
    {
        ImRect rect;
        bool occupied = false;
    };
    struct bar_place
    {
        ImRect rect;
        bool occupied = false;
    };
    struct text_item
    {
        std::string data;
        std::string_view font_id;
        ImRect rect;
        ImVec4 color;
        bool enabled = true;
        displacement displacement_type;
    };
    struct bar_item
    {
        ImVec2 size;
        ImVec2 pos;
        ImVec4 color;
        bool enabled = true;
        displacement displacement_type;
    };

    bool bounding_box = true;
    int box_type = 0;  // 0 = Normal, 1 = Dashed (corners only)
    ImVec4 bounding_box_color = col->bbox;

    bool health_bar = true;
    ImVec4 health_bar_color = col->hpbar;

    bool armor_bar = true;
    ImVec4 armor_bar_color = col->armorbar;
    
    bool skeleton = false;

    static constexpr auto MAX_AVAIL_PLACES = 4;

    std::array<std::vector<bar_place>, MAX_AVAIL_PLACES> bar_places; 
    std::array<std::vector<text_place>, MAX_AVAIL_PLACES> text_places;
    std::vector<text_item> text_items;
    std::vector<bar_item> bar_items;

    struct dragged_item_info {
        enum dragged_type { none, bar, text };
        dragged_type type = dragged_type::none;
        size_t index = 0;
    };

    dragged_item_info dragged_item;
    dragged_item_info swapped_item;
    ImRect start_dragging_place;
    bool swapped = false;
};

void init_esp_preview(esp_preview_state& state);

inline std::unique_ptr<c_widgets> widgets = std::make_unique<c_widgets>();

enum class notify_anchor : uint8_t {
    topright
};

enum class notify_type
{
    success,
    warning,
    error
};

class c_notify
{
public:
    void process_notifies(const ImRect& work_rect, notify_anchor anchor);

    void add_notify(const std::string& text, notify_type type);

private:
    static constexpr uint16_t MAX_VISIBLE_NOTIFIES{ 30 };

    static constexpr ImVec2 WORK_RECT_PADDING{ 24, 24 };

    static constexpr std::chrono::duration<float> NOTIFY_DURATION{ 5s };
    static constexpr ImVec2 NOTIFY_PADDING{ 12, 12 };
    static constexpr float NOTIFY_SPACING{ 12 };
    static constexpr float NOTIFY_HEIGHT{ 72 };

private:
    struct notification
    {
    public:
        notification(uint16_t id, const std::string& text, notify_type type);

        void render(const ImVec2& pos, const ImVec2& size, bool focused_notify);

        [[nodiscard]] tween_id get_alpha_anim_id() const { return alpha_anim_id; }
        [[nodiscard]] tween_id get_move_anim_id() const { return move_anim_id; }

    public:
        using duration_t = std::chrono::duration<float>;

        uint16_t id;
        std::string text;

        notify_type type;

        duration_t duration{ 0 };
        ImVec2 pos{ 0, 0 };
        bool expired = false;
        bool destroy_next_frame = false;

    private:
        tween_id alpha_anim_id;
        tween_id move_anim_id;
    };

    std::vector<notification> notifications;

    uint16_t current_max_id{ 0 };
};

inline std::unique_ptr<c_notify> notify = std::make_unique<c_notify>();

enum class anchor {
    topleft,
    top,
    topright,
    left,
    middle,
    right,
    bottomleft,
    bottom,
    bottomright
};