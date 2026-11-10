#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <D3DX11tex.h>
#include <vector>

#include "render.h"
#include "../helpers/anim/anim.h"
#include "../helpers/restore.h"
#include "../defs/fonts.h"
#include "../defs/textures.h"
#include "../defs/colors.h"

using namespace ImGui;

inline float global_a = 0.f;

inline bool checkbox(const char* label, bool* v, bool header = false)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    auto curr_b = anim::animation((std::string(label) + "##checkbox").c_str(), anim_t(clamp_out, 0.1f));
    curr_b->active = (*v);

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 total_size = ImVec2(305.f, 21.f);

    const ImVec2 pos = window->DC.CursorPos + ImVec2(14.f, 12.f);
    //const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(total_size.x, 14.f));
    const ImRect nav_bb(pos, pos + ImVec2(total_size.x, total_size.y));
    ItemSize(nav_bb, style.FramePadding.y - 2.f);
    const bool is_visible = ItemAdd(nav_bb, id);

    bool checked = *v;

    bool hovered, held;
    bool pressed = ButtonBehavior(nav_bb, id, &hovered, &held);

    if (pressed)
        checked = !checked;

    if (*v != checked)
    {
        *v = checked;
        pressed = true; // return value
        MarkItemEdited(id);
    }
    
    const ImRect check_bb(nav_bb.Max - ImVec2(24.f, 17.f), nav_bb.Max - ImVec2(0.f, 3.f));

    float extra_a = curr_b->val;
    if (is_visible)
    {
        if (header)
        {
            window->DrawList->AddRect(check_bb.Min - ImVec2(1.f, 1.f), check_bb.Max + ImVec2(1.f, 1.f), lerp(c::bg0, c::bg, extra_a), 512.f, 0, 3.f);
        }

        ImColor col2 = lerp(lerp(c::white6, c::white24, global_a), lerp(c::white24, c::primary, global_a), extra_a);
        ImColor col3 = lerp(lerp(c::white4, c::white6, global_a), lerp(c::white48, c::secondary, global_a), extra_a);

        int vert_start_idx = window->DrawList->VtxBuffer.Size;
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, IM_COL32_WHITE, 512.f);
        int vert_end_idx = window->DrawList->VtxBuffer.Size;
        ShadeVertsLinearGradX(window->DrawList, vert_start_idx, vert_end_idx,
            check_bb.Min, check_bb.Max, col2, col3);

        float extra_b = 0.f;
        float extra_c = 0.f;

        if (extra_a <= 0.3f)
        {
            extra_c = extra_a * 3.333f;
        }
        else if (extra_a > 0.3f && extra_a <= 0.6f)
        {
            extra_b = (extra_a - 0.3f) * 3.333f;
            extra_c = 1.f;
        }
        else
        {
            extra_b = 1.f;
            extra_c = 1.f - ((extra_a - 0.6f) * 2.5f);
        }

        ImVec2 circle = ImVec2(check_bb.Min.x + 7.f + 10.f * extra_b, check_bb.Min.y + 7.f);
        ImColor col4 = lerp(lerp(c::white24, c::white48, global_a), lerp(c::white48, c::white, global_a), extra_a);
        window->DrawList->AddCircleFilled(circle, 5.f - 2.f * extra_c, col4);

        const ImVec2 label_pos = ImVec2(nav_bb.Min.x, nav_bb.Min.y);
        ImColor col = lerp(c::white72, c::white, global_a);
        if (header)
        {
            col = lerp(c::white72, c::bg, global_a);
        }
        window->DrawList->AddText(f::medium14, 20.f, label_pos, col, label);
    }

    return pressed;
}

inline bool skeleton_checkbox(const char* label, bool* v)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    auto curr_b = anim::animation((std::string(label) + "##skeleton").c_str(), anim_t(clamp_out, 0.1f));
    curr_b->active = (*v);

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID((std::string(label) + "##skeleton").c_str());
    const ImVec2 total_size = ImVec2(19.f, 19.f);

    const ImVec2 pos = window->DC.CursorPos;
    const ImRect nav_bb(pos, pos + ImVec2(total_size.x, total_size.y));
    ItemSize(nav_bb, style.FramePadding.y - 2.f);
    const bool is_visible = ItemAdd(nav_bb, id);

    bool checked = *v;

    bool hovered, held;
    bool pressed = ButtonBehavior(nav_bb, id, &hovered, &held);

    if (pressed)
        checked = !checked;

    if (*v != checked)
    {
        *v = checked;
        pressed = true; // return value
        MarkItemEdited(id);
    }

    float extra_a = curr_b->val;
    if (is_visible)
    {
        
        ImColor col2 = lerp(lerp(c::white0, c::gray, global_a), lerp(c::gray, c::secondary, global_a), extra_a);
        ImColor col3 = lerp(lerp(c::white0, c::gray2, global_a), lerp(c::gray2, c::primary, global_a), extra_a);
        ImColor col4 = lerp(lerp(c::white0, c::white36, global_a), lerp(c::white36, c::white, global_a), extra_a);

        int vert_start_idx = window->DrawList->VtxBuffer.Size;
        window->DrawList->AddCircleFilled(nav_bb.Min + ImVec2(total_size.x / 2.f, total_size.y / 2.f), total_size.x / 2.f, IM_COL32_WHITE);
        int vert_end_idx = window->DrawList->VtxBuffer.Size;
        ShadeVertsLinearGradY(window->DrawList, vert_start_idx, vert_end_idx,
            nav_bb.Min, nav_bb.Max, col2, col3);

        window->DrawList->AddCircle(nav_bb.Min + ImVec2(total_size.x / 2.f, total_size.y / 2.f),
            (total_size.x / 2.f - 4.f * extra_a), col4, 360, 1.f + 3.f * extra_a);
        /*float extra_b = 0.f;
        float extra_c = 0.f;

        if (extra_a <= 0.3f)
        {
            extra_c = extra_a * 3.333f;
        }
        else if (extra_a > 0.3f && extra_a <= 0.6f)
        {
            extra_b = (extra_a - 0.3f) * 3.333f;
            extra_c = 1.f;
        }
        else
        {
            extra_b = 1.f;
            extra_c = 1.f - ((extra_a - 0.6f) * 2.5f);
        }

        ImVec2 circle = ImVec2(check_bb.Min.x + 7.f + 10.f * extra_b, check_bb.Min.y + 7.f);
        ImColor col4 = lerp(lerp(c::white24, c::white48, global_a), lerp(c::white48, c::white, global_a), extra_a);
        window->DrawList->AddCircleFilled(circle, 5.f - 2.f * extra_c, col4);

        const ImVec2 label_pos = ImVec2(nav_bb.Min.x, nav_bb.Min.y);
        ImColor col = lerp(c::white72, c::white, global_a);
        
        window->DrawList->AddText(f::medium14, 20.f, label_pos, col, label);*/
    }

    return pressed;
}

inline static const char* KeyNames[] = {
    "n/a",
    "LBUTTON",
    "RBUTTON",
    "CANCEL",
    "MBUTTON",
    "XBUTTON1",
    "XBUTTON2",
    "Unknown",
    "BACK",
    "TAB",
    "Unknown",
    "Unknown",
    "CLEAR",
    "RETURN",
    "Unknown",
    "Unknown",
    "SHIFT",
    "CONTROL",
    "MENU",
    "PAUSE",
    "CAPITAL",
    "KANA",
    "Unknown",
    "JUNJA",
    "FINAL",
    "KANJI",
    "Unknown",
    "ESCAPE",
    "CONVERT",
    "NONCONVERT",
    "ACCEPT",
    "MODECHANGE",
    "SPACE",
    "PRIOR",
    "NEXT",
    "END",
    "HOME",
    "LEFT",
    "UP",
    "RIGHT",
    "DOWN",
    "SELECT",
    "PRINT",
    "EXECUTE",
    "SNAPSHOT",
    "INSERT",
    "DELETE",
    "HELP",
    "0",
    "1",
    "2",
    "3",
    "4",
    "5",
    "6",
    "7",
    "8",
    "9",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "A",
    "B",
    "C",
    "D",
    "E",
    "F",
    "G",
    "H",
    "I",
    "J",
    "K",
    "L",
    "M",
    "N",
    "O",
    "P",
    "Q",
    "R",
    "S",
    "T",
    "U",
    "V",
    "W",
    "X",
    "Y",
    "Z",
    "LWIN",
    "RWIN",
    "APPS",
    "Unknown",
    "SLEEP",
    "NUMPAD0",
    "NUMPAD1",
    "NUMPAD2",
    "NUMPAD3",
    "NUMPAD4",
    "NUMPAD5",
    "NUMPAD6",
    "NUMPAD7",
    "NUMPAD8",
    "NUMPAD9",
    "MULTIPLY",
    "ADD",
    "SEPARATOR",
    "SUBTRACT",
    "DECIMAL",
    "DIVIDE",
    "F1",
    "F2",
    "F3",
    "F4",
    "F5",
    "F6",
    "F7",
    "F8",
    "F9",
    "F10",
    "F11",
    "F12",
    "F13",
    "F14",
    "F15",
    "F16",
    "F17",
    "F18",
    "F19",
    "F20",
    "F21",
    "F22",
    "F23",
    "F24",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "NUMLOCK",
    "SCROLL",
    "OEM_NEC_EQUAL",
    "OEM_FJ_MASSHOU",
    "OEM_FJ_TOUROKU",
    "OEM_FJ_LOYA",
    "OEM_FJ_ROYA",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "Unknown",
    "LSHIFT",
    "RSHIFT",
    "LCONTROL",
    "RCONTROL",
    "LMENU",
    "RMENU"
};
inline static const int KeyCodes[] = {
    0x0,  //Undefined
    0x01,
    0x02,
    0x03,
    0x04,
    0x05,
    0x06,
    0x07, //Undefined
    0x08,
    0x09,
    0x0A, //Reserved
    0x0B, //Reserved
    0x0C,
    0x0D,
    0x0E, //Undefined
    0x0F, //Undefined
    0x10,
    0x11,
    0x12,
    0x13,
    0x14,
    0x15,
    0x16, //IME On
    0x17,
    0x18,
    0x19,
    0x1A, //IME Off
    0x1B,
    0x1C,
    0x1D,
    0x1E,
    0x1F,
    0x20,
    0x21,
    0x22,
    0x23,
    0x24,
    0x25,
    0x26,
    0x27,
    0x28,
    0x29,
    0x2A,
    0x2B,
    0x2C,
    0x2D,
    0x2E,
    0x2F,
    0x30,
    0x31,
    0x32,
    0x33,
    0x34,
    0x35,
    0x36,
    0x37,
    0x38,
    0x39,
    0x3A, //Undefined
    0x3B, //Undefined
    0x3C, //Undefined
    0x3D, //Undefined
    0x3E, //Undefined
    0x3F, //Undefined
    0x40, //Undefined
    0x41,
    0x42,
    0x43,
    0x44,
    0x45,
    0x46,
    0x47,
    0x48,
    0x49,
    0x4A,
    0x4B,
    0x4C,
    0x4B,
    0x4E,
    0x4F,
    0x50,
    0x51,
    0x52,
    0x53,
    0x54,
    0x55,
    0x56,
    0x57,
    0x58,
    0x59,
    0x5A,
    0x5B,
    0x5C,
    0x5D,
    0x5E, //Rservered
    0x5F,
    0x60, //Numpad1
    0x61, //Numpad2
    0x62, //Numpad3
    0x63, //Numpad4
    0x64, //Numpad5
    0x65, //Numpad6
    0x66, //Numpad7
    0x67, //Numpad8
    0x68, //Numpad8
    0x69, //Numpad9
    0x6A,
    0x6B,
    0x6C,
    0x6D,
    0x6E,
    0x6F,
    0x70, //F1
    0x71, //F2
    0x72, //F3
    0x73, //F4
    0x74, //F5
    0x75, //F6
    0x76, //F7
    0x77, //F8
    0x78, //F9
    0x79, //F10
    0x7A, //F11
    0x7B, //F12
    0x7C, //F13
    0x7D, //F14
    0x7E, //F15
    0x7F, //F16
    0x80, //F17
    0x81, //F18
    0x82, //F19
    0x83, //F20
    0x84, //F21
    0x85, //F22
    0x86, //F23
    0x87, //F24
    0x88, //Unkown
    0x89, //Unkown
    0x8A, //Unkown
    0x8B, //Unkown
    0x8C, //Unkown
    0x8D, //Unkown
    0x8E, //Unkown
    0x8F, //Unkown
    0x90,
    0x91,
    0x92, //OEM Specific
    0x93, //OEM Specific
    0x94, //OEM Specific
    0x95, //OEM Specific
    0x96, //OEM Specific
    0x97, //Unkown
    0x98, //Unkown
    0x99, //Unkown
    0x9A, //Unkown
    0x9B, //Unkown
    0x9C, //Unkown
    0x9D, //Unkown
    0x9E, //Unkown 
    0x9F, //Unkown
    0xA0,
    0xA1,
    0xA2,
    0xA3,
    0xA4,
    0xA5
};

inline bool keybind(const char* label, int* k)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;
    auto curr_act = anim::animation((std::string(label) + "##actkeybind").c_str(), anim_t(clamp_out, 0.1f));
    auto curr_b = anim::animation((std::string(label) + "##keybind").c_str(), anim_t(clamp_out, 0.1f));

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 total_size = ImVec2(305.f, 25.f);

    const ImVec2 pos = window->DC.CursorPos + ImVec2(14.f, 12.f);
    //const ImRect total_bb(window->DC.CursorPos, window->DC.CursorPos + ImVec2(total_size.x, 14.f));
    const ImRect nav_bb(pos, pos + ImVec2(total_size.x, total_size.y));
    ItemSize(nav_bb, style.FramePadding.y - 2.f);
    const bool is_visible = ItemAdd(nav_bb, id);

    bool hovered, held;
    bool pressed = ButtonBehavior(nav_bb, id, &hovered, &held);

    std::string key_sz = KeyNames[*(int*)k];

    curr_b->active = (*(int*)k) != 0;

    if (curr_act->active)
    {
        key_sz = "...";
        curr_b->active = false;
        if (curr_act->val > 0.8f)
        {
            for (auto& Key : KeyCodes)
            {
                if (GetAsyncKeyState(Key) & 0x8000) {
                    if (Key == VK_ESCAPE)
                        *(int*)k = 0;
                    else
                        *(int*)k = Key;
                    curr_act->active = false;
                }
            }
        }
    }
    
    ImVec2 text_size = f::medium14->CalcTextSizeA(20.f, FLT_MAX, -1.f, key_sz.c_str(), 0, NULL);

    const ImRect check_bb(nav_bb.Max - ImVec2(12.f + text_size.x, 25.f), nav_bb.Max);

    float extra_a = curr_b->val;
    if (is_visible)
    {
       
        ImColor col2 = lerp(lerp(c::white2, c::white4, global_a), lerp(c::white48, c::secondary, global_a), extra_a);
        ImColor col3 = lerp(lerp(c::white3, c::white2, global_a), lerp(c::white24, c::primary, global_a), extra_a);

        int vert_start_idx = window->DrawList->VtxBuffer.Size;
        window->DrawList->AddRectFilled(check_bb.Min, check_bb.Max, IM_COL32_WHITE, 4.f);
        int vert_end_idx = window->DrawList->VtxBuffer.Size;
        ShadeVertsLinearGradY(window->DrawList, vert_start_idx, vert_end_idx,
            check_bb.Min, check_bb.Max, col2, col3);

        window->DrawList->AddRect(check_bb.Min, check_bb.Max, c::white4, 4.f);

        ImColor col4 = lerp(lerp(c::white48, c::white72, global_a), lerp(ImColor(20, 20, 20, 255), c::bg, global_a), extra_a);
        ImVec2 t_pos = ImVec2(check_bb.Min.x + 7.f, check_bb.Min.y + 2.f);
        window->DrawList->AddText(f::medium14, 20.f, t_pos, col4, key_sz.c_str());

        /*float extra_b = 0.f;
        float extra_c = 0.f;

        if (extra_a <= 0.3f)
        {
            extra_c = extra_a * 3.333f;
        }
        else if (extra_a > 0.3f && extra_a <= 0.6f)
        {
            extra_b = (extra_a - 0.3f) * 3.333f;
            extra_c = 1.f;
        }
        else
        {
            extra_b = 1.f;
            extra_c = 1.f - ((extra_a - 0.6f) * 2.5f);
        }

        ImVec2 circle = ImVec2(check_bb.Min.x + 7.f + 10.f * extra_b, check_bb.Min.y + 7.f);
        ImColor col4 = lerp(lerp(c::white24, c::white48, global_a), lerp(c::white48, c::white, global_a), extra_a);
        window->DrawList->AddCircleFilled(circle, 5.f - 2.f * extra_c, col4);*/

        const ImVec2 label_pos = ImVec2(nav_bb.Min.x, nav_bb.Min.y + 1.f);
        ImColor col = lerp(c::white72, c::white, global_a);
        
        window->DrawList->AddText(f::medium14, 20.f, label_pos, col, label);
    }

    if (pressed)
        curr_act->active = !curr_act->active;

    return pressed;
}

inline bool slider_scalar(const char* label, ImGuiDataType data_type, void* p_data, const void* p_min, const void* p_max, const char* format, ImGuiSliderFlags flags)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    const ImVec2 total_size = ImVec2(305.f, 29.f);;

    const ImVec2 pos = window->DC.CursorPos + ImVec2(14.f, 12.f);
    const ImRect total_bb(pos, pos + total_size);
    //const ImRect nav_bb(pos, pos + ImVec2(total_size.x - 3.f, 24.f));
    const ImRect extra_nav_bb(pos + ImVec2(-4.f, 0.f), pos + ImVec2(total_size.x + 4.f, total_size.y));

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id))
        return false;

    // Default format string when passing NULL
    if (format == NULL)
        format = DataTypeGetInfo(data_type)->PrintFmt;

    const bool hovered = ItemHoverable(total_bb, id, g.LastItemData.ItemFlags);

    // Tabbing or CTRL-clicking on Slider turns it into an input box
    const bool clicked = hovered && IsMouseClicked(0, ImGuiInputFlags_None, id);
    const bool make_active = (clicked || g.NavActivateId == id);
    if (make_active && clicked)
        SetKeyOwner(ImGuiKey_MouseLeft, id);

    // Store initial value (not used by main lib but available as a convenience but some mods e.g. to revert)
    if (make_active)
        memcpy(&g.ActiveIdValueOnActivation, p_data, DataTypeGetInfo(data_type)->Size);

    if (make_active)
    {
        SetActiveID(id, window);
        SetFocusID(id, window);
        FocusWindow(window);
        g.ActiveIdUsingNavDirMask |= (1 << ImGuiDir_Left) | (1 << ImGuiDir_Right);
    }

    //bg
    window->DrawList->AddRectFilled(total_bb.Min + ImVec2(0.f, 25.f), total_bb.Max, c::white4, 512.f);

    // Slider behavior
    ImRect grab_bb;
    const bool value_changed = SliderBehavior(extra_nav_bb, id, data_type, p_data, p_min, p_max, format, flags, &grab_bb);
    if (value_changed)
        MarkItemEdited(id);
    float pp = grab_bb.GetCenter().x - window->DC.CursorPos.x;

    static std::map<ImGuiID, elastic_point> anim;
    auto it_anim = anim.find(id);

    if (it_anim == anim.end())
    {
        anim.insert({ id, elastic_point(pp, 0.3f, 0.8f, 1.2f) });
        it_anim = anim.find(id);
    }

    //grab
    if (grab_bb.Max.x > grab_bb.Min.x)
    {
        it_anim->second.set_target(pp);
        float cur_pos = window->DC.CursorPos.x + it_anim->second.get_pos();
        if (total_bb.Min.x < cur_pos)
            window->DrawList->AddRectFilled(total_bb.Min + ImVec2(0.f, 25.f), ImVec2(cur_pos, total_bb.Max.y), lerp(c::white36, c::primary, global_a), 512.f);
        else
            it_anim->second.avoid_v();
        if (total_bb.Max.x < cur_pos)
            it_anim->second.avoid_v();

        window->DrawList->AddCircleFilled(ImVec2(cur_pos, total_bb.Max.y - 2.f), 4.f, lerp(ImColor(138, 138, 140, 255), c::white, global_a));
    }
    it_anim->second.update(0.16f);
    // Display value using user-provided display format so user can add prefix/suffix/decorations to the value.
    char value_buf[64];
    const char* value_buf_end = value_buf + DataTypeFormatString(value_buf, IM_ARRAYSIZE(value_buf), data_type, p_data, format);
    const char* text_display_end = FindRenderedTextEnd(value_buf, value_buf_end);
    ImVec2 text_size = f::medium14->CalcTextSizeA(20.f, FLT_MAX, -1.f, value_buf, 0, NULL);
    ImVec2 t_pos = ImVec2(total_bb.Max.x - text_size.x, total_bb.Min.y);
    window->DrawList->AddText(f::medium14, 20.f, t_pos,
        lerp(c::white48, c::white72, global_a), value_buf, text_display_end);

    const ImVec2 label_pos = ImVec2(total_bb.Min.x, total_bb.Min.y);
    window->DrawList->AddText(f::medium14, 20.f, label_pos,
        lerp(c::white72, c::white, global_a), label);

    return value_changed;
}

inline bool slider(const char* label, int* v, int v_min, int v_max, const char* format = "%d", ImGuiSliderFlags flags = 0)
{
    return slider_scalar(label, ImGuiDataType_S32, v, &v_min, &v_max, format, flags);
}

inline static void fColorEditRestoreHS(const float* col, float* H, float* S, float* V)
{
    ImGuiContext& g = *GImGui;
    IM_ASSERT(g.ColorEditCurrentID != 0);
    if (g.ColorEditSavedID != g.ColorEditCurrentID || g.ColorEditSavedColor != ImGui::ColorConvertFloat4ToU32(ImVec4(col[0], col[1], col[2], 0)))
        return;

    // When S == 0, H is undefined.
    // When H == 1 it wraps around to 0.
    if (*S == 0.0f || (*H == 0.0f && g.ColorEditSavedHue == 1))
        *H = g.ColorEditSavedHue;

    // When V == 0, S is undefined.
    if (*V == 0.0f)
        *S = g.ColorEditSavedSat;
}

inline bool color_button(const char* desc_id, const ImVec4& col)
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    const ImVec2 pos = window->DC.CursorPos;
    window->DrawList->AddCircleFilled(pos + ImVec2(10.f, 10.f), 7.f, a(col, 0.12f + 0.12f * global_a));
    window->DrawList->AddCircleFilled(pos + ImVec2(10.f, 10.f), 5.f, a(col, 0.48f + 0.52f * global_a));
    bool ret = ::InvisibleButton((std::string(desc_id)+"##cb").c_str(), ImVec2(21.f, 21.f));
    return ret;
}

inline bool color_edit(const char* label, float col[4])
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;
    const float square_sz = GetFrameHeight();
    const char* label_display_end = FindRenderedTextEnd(label);
    float w_full = CalcItemWidth();
    g.NextItemData.ClearFlags();

    BeginGroup();
    PushID(label);
    const bool set_current_color_edit_id = (g.ColorEditCurrentID == 0);
    if (set_current_color_edit_id)
        g.ColorEditCurrentID = window->IDStack.back();

    // If we're not showing any slider there's no point in doing any HSV conversions
    const ImGuiColorEditFlags flags_untouched = 0;
    ImGuiColorEditFlags flags = 0;
    // Read stored options
    if (!(flags & ImGuiColorEditFlags_DisplayMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DisplayMask_);
    if (!(flags & ImGuiColorEditFlags_DataTypeMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_DataTypeMask_);
    if (!(flags & ImGuiColorEditFlags_PickerMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_PickerMask_);
    if (!(flags & ImGuiColorEditFlags_InputMask_))
        flags |= (g.ColorEditOptions & ImGuiColorEditFlags_InputMask_);
    flags |= (g.ColorEditOptions & ~(ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_));
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_DisplayMask_)); // Check that only 1 is selected
    IM_ASSERT(ImIsPowerOfTwo(flags & ImGuiColorEditFlags_InputMask_));   // Check that only 1 is selected

    const bool alpha = false;
    const bool hdr = (flags & ImGuiColorEditFlags_HDR) != 0;
    const int components = alpha ? 4 : 3;
    const float w_button = (flags & ImGuiColorEditFlags_NoSmallPreview) ? 0.0f : (square_sz + style.ItemInnerSpacing.x);
    const float w_inputs = ImMax(w_full - w_button, 1.0f);
    w_full = w_inputs + w_button;

    // Convert to the formats we need
    float f[4] = { col[0], col[1], col[2], alpha ? col[3] : 1.0f };
    if ((flags & ImGuiColorEditFlags_InputHSV) && (flags & ImGuiColorEditFlags_DisplayRGB))
        ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
    else if ((flags & ImGuiColorEditFlags_InputRGB) && (flags & ImGuiColorEditFlags_DisplayHSV))
    {
        // Hue is lost when converting from grayscale rgb (saturation=0). Restore it.
        ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);
        fColorEditRestoreHS(col, &f[0], &f[1], &f[2]);
    }
    int i[4] = { IM_F32_TO_INT8_UNBOUND(f[0]), IM_F32_TO_INT8_UNBOUND(f[1]), IM_F32_TO_INT8_UNBOUND(f[2]), IM_F32_TO_INT8_UNBOUND(f[3]) };

    bool value_changed = false;
    bool value_changed_as_float = false;

    const ImVec2 pos = window->DC.CursorPos + ImVec2(14.f, 16.f);
    const float inputs_offset_x = (style.ColorButtonPosition == ImGuiDir_Left) ? w_button : 0.0f;
    //window->DC.CursorPos.x = pos.x + inputs_offset_x;

    ImGuiWindow* picker_active_window = NULL;
    if (!(flags & ImGuiColorEditFlags_NoSmallPreview))
    {
        const float button_offset_x = ((flags & ImGuiColorEditFlags_NoInputs) || (style.ColorButtonPosition == ImGuiDir_Left)) ? 0.0f : w_inputs + style.ItemInnerSpacing.x;
        window->DC.CursorPos = ImVec2(pos.x + 291, pos.y);

        const ImVec4 col_v4(col[0], col[1], col[2], alpha ? col[3] : 1.0f);
        if (color_button("##ColorButton", col_v4))
        {
            if (!(flags & ImGuiColorEditFlags_NoPicker))
            {
                // Store current color and open a picker
                g.ColorPickerRef = col_v4;
                OpenPopup("picker");
                SetNextWindowPos(g.LastItemData.Rect.GetBL() + ImVec2(0.0f, style.ItemSpacing.y));
            }
        }

        if (BeginPopup("picker", ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground))
        {
            if (g.CurrentWindow->BeginCount == 1)
            {
                //bg
                auto draw = ::GetWindowDrawList();
                const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);
                const ImVec2& region = ::GetContentRegionMax() - ImVec2(2.f, 2.f);
                draw->AddRectFilled(p, p + region, c::white2, 8.f);

                picker_active_window = g.CurrentWindow;
                ImGuiColorEditFlags picker_flags_to_forward = ImGuiColorEditFlags_DataTypeMask_ | ImGuiColorEditFlags_PickerMask_ | ImGuiColorEditFlags_InputMask_ | ImGuiColorEditFlags_HDR | ImGuiColorEditFlags_NoAlpha | ImGuiColorEditFlags_AlphaBar;
                ImGuiColorEditFlags picker_flags = (flags_untouched & picker_flags_to_forward) | ImGuiColorEditFlags_DisplayMask_ | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_AlphaPreviewHalf;
                SetNextItemWidth(square_sz * 10.0f); // Use 256 + bar sizes?
                value_changed |= ColorPicker4((std::string(label) + "##picker").c_str(), col, picker_flags | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoSidePreview, &g.ColorPickerRef.x);

                //border
                draw->AddRect(p, p + region, c::white6, 8.f);
            }
            EndPopup();
        }
    }

    if (label != label_display_end && !(flags & ImGuiColorEditFlags_NoLabel))
    {
        // Position not necessarily next to last submitted button (e.g. if style.ColorButtonPosition == ImGuiDir_Left),
        // but we need to use SameLine() to setup baseline correctly. Might want to refactor SameLine() to simplify this.
        //SameLine(0.0f, style.ItemInnerSpacing.x);
        //window->DC.CursorPos.x = pos.x + ((flags & ImGuiColorEditFlags_NoInputs) ? w_button : w_full + style.ItemInnerSpacing.x);
        //TextEx(label, label_display_end);
        const ImVec2 label_pos = pos;
        ImColor col = lerp(c::white72, c::white, global_a); 
        window->DrawList->AddText(f::medium14, 20.f, label_pos, col, label, label_display_end);
    }

    // Convert back
    if (value_changed && picker_active_window == NULL)
    {
        if (!value_changed_as_float)
            for (int n = 0; n < 4; n++)
                f[n] = i[n] / 255.0f;
        if ((flags & ImGuiColorEditFlags_DisplayHSV) && (flags & ImGuiColorEditFlags_InputRGB))
        {
            g.ColorEditSavedHue = f[0];
            g.ColorEditSavedSat = f[1];
            ColorConvertHSVtoRGB(f[0], f[1], f[2], f[0], f[1], f[2]);
            g.ColorEditSavedID = g.ColorEditCurrentID;
            g.ColorEditSavedColor = ColorConvertFloat4ToU32(ImVec4(f[0], f[1], f[2], 0));
        }
        if ((flags & ImGuiColorEditFlags_DisplayRGB) && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(f[0], f[1], f[2], f[0], f[1], f[2]);

        col[0] = f[0];
        col[1] = f[1];
        col[2] = f[2];
        if (alpha)
            col[3] = f[3];
    }

    if (set_current_color_edit_id)
        g.ColorEditCurrentID = 0;
    PopID();
    EndGroup();

    // Drag and Drop Target
    // NB: The flag test is merely an optional micro-optimization, BeginDragDropTarget() does the same test.
    if ((g.LastItemData.StatusFlags & ImGuiItemStatusFlags_HoveredRect) && !(g.LastItemData.ItemFlags & ImGuiItemFlags_ReadOnly) && !(flags & ImGuiColorEditFlags_NoDragDrop) && BeginDragDropTarget())
    {
        bool accepted_drag_drop = false;
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_3F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * 3); // Preserve alpha if any //-V512 //-V1086
            value_changed = accepted_drag_drop = true;
        }
        if (const ImGuiPayload* payload = AcceptDragDropPayload(IMGUI_PAYLOAD_TYPE_COLOR_4F))
        {
            memcpy((float*)col, payload->Data, sizeof(float) * components);
            value_changed = accepted_drag_drop = true;
        }

        // Drag-drop payloads are always RGB
        if (accepted_drag_drop && (flags & ImGuiColorEditFlags_InputHSV))
            ColorConvertRGBtoHSV(col[0], col[1], col[2], col[0], col[1], col[2]);
        EndDragDropTarget();
    }

    // When picker is being actively used, use its active id so IsItemActive() will function on ColorEdit4().
    if (picker_active_window && g.ActiveId != 0 && g.ActiveIdWindow == picker_active_window)
        g.LastItemData.ID = g.ActiveId;

    if (value_changed && g.LastItemData.ID != 0) // In case of ID collision, the second EndGroup() won't catch g.ActiveId
        MarkItemEdited(g.LastItemData.ID);

    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.f, -14.f));

    return value_changed;
}

struct data_s
{
    float sz_y{};
    float y2{};
};

inline static std::map<ImGuiID, data_s> data;
inline std::map<ImGuiID, data_s>::iterator it_data;

inline void child(const char* str_id, bool *v, bool empty)
{
    ImGuiID id = GetCurrentWindow()->GetID(str_id);
    
    it_data = data.find(id);

    if (it_data == data.end())
    {
        data.insert({ id, data_s() });
        it_data = data.find(id);
    }
    std::string g = str_id;
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(16.f, 0.f));
    ::BeginChild(("##child" + g).c_str(), ImVec2(333.f, 45.f + it_data->second.sz_y), 0, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground);
    {
        auto draw = ::GetWindowDrawList();
        const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);
        const ImVec2& region = ::GetContentRegionMax() - ImVec2(2.f, 2.f);
        auto curr_b = anim::animation((std::string(str_id) + "##child").c_str(), anim_t(clamp_out, 0.1f));
        float extra_a = curr_b->val;
        float extra_b = std::clamp(extra_a * 2.f, 0.f, 1.f);
        float extra_c = std::clamp((extra_b - 0.5f) * 2.f, 0.f, 1.f);

        draw->AddRectFilled(p, p + region, lerp(c::white2, c::white4, extra_a), 8.f);

        int vert_start_idx = draw->VtxBuffer.Size;
        draw->AddRectFilled(p, p + ImVec2(region.x, 43.f), IM_COL32_WHITE, 8.f, !empty ? ImDrawFlags_RoundCornersTop : ImDrawFlags_RoundCornersAll);
        int vert_end_idx = draw->VtxBuffer.Size;
        ShadeVertsLinearGradX(draw, vert_start_idx, vert_end_idx,
            p, p + ImVec2(region.x, 43.f), lerp(c::white6, c::primary, extra_b), lerp(c::white2, c::secondary, extra_c));

        global_a = extra_a;
        checkbox(g.c_str(), v, true);
        curr_b->active = (*v);

        it_data->second.y2 = ImGui::GetCursorPos().y;
        if (!empty)
            ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(0.f, 14.f));
    }
}
inline void end_child(const char* str_id)
{
    auto draw = ::GetWindowDrawList();
    const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);
    const ImVec2& region = ::GetContentRegionMax() - ImVec2(2.f, 2.f);
    ImGuiID id = GetCurrentWindow()->GetID(str_id);
    
    if (it_data != data.end())
        it_data->second.sz_y = ImGui::GetCursorPos().y - it_data->second.y2;

    draw->AddRect(p, p + region, c::white6, 8.f);
    ::EndChild();
}

inline bool begincombo(const char* label, const char* preview_value, int all_count, int enabled_count, ImGuiComboFlags flags = 0)
{
    ImGuiContext& g = *GImGui;
    ImGuiWindow* window = GetCurrentWindow();

    ImGuiNextWindowDataFlags backup_next_window_data_flags = g.NextWindowData.HasFlags;
    g.NextWindowData.ClearFlags(); // We behave like Begin() and need to consume those values
    if (window->SkipItems)
        return false;

    const ImGuiStyle& style = g.Style;
    const ImGuiID id = window->GetID(label);
    IM_ASSERT((flags & (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)) != (ImGuiComboFlags_NoArrowButton | ImGuiComboFlags_NoPreview)); // Can't use both flags together
    if (flags & ImGuiComboFlags_WidthFitPreview)
        IM_ASSERT((flags & (ImGuiComboFlags_NoPreview | (ImGuiComboFlags)ImGuiComboFlags_CustomPreview)) == 0);

    auto curr = anim::animation(label, anim_t(clamp_out, 0.15f));
    const ImVec2 total_size = ImVec2(305.f, 33.f + 21.f + 4.f);
    const ImVec2 pos = window->DC.CursorPos + ImVec2(14.f, 12.f);
    const ImRect total_bb(pos, pos + total_size);
    const ImRect bb(pos + ImVec2(0.f, 21.f + 4.f), pos + total_size);

    ItemSize(total_bb, style.FramePadding.y);
    if (!ItemAdd(total_bb, id, &bb))
        return false;

    // Open on click
    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held);
    const ImGuiID popup_id = ImHashStr("##ComboPopup", 0, id);
    bool popup_open = IsPopupOpen(popup_id, ImGuiPopupFlags_None);
    if (pressed && !popup_open)
    {
        OpenPopupEx(popup_id, ImGuiPopupFlags_None);
        popup_open = true;
    }

    window->DrawList->AddRectFilled(bb.Min, bb.Max, lerp(c::white2, c::white4, global_a), 4.f);
    window->DrawList->AddRect(bb.Min, bb.Max, c::white6, 4.f);

    ImVec2 arr_pos = ImVec2(bb.Max.x - 11.f - 9.f, bb.Min.y + 11.f);
    ImColor col2 = lerp(lerp(c::white24, c::white48, global_a), lerp(c::white24, c::white72, global_a), curr->extra());
    rot_start();
    window->DrawList->AddText(f::icons10, 10.f, arr_pos, col2, "e");
    rot_end(180.f * curr->extra());
    curr->active = popup_open;

    ImColor col = lerp(c::white72, c::white, global_a);

    // Render preview and label
    if (preview_value != NULL && !(flags & ImGuiComboFlags_NoPreview))
    {
        const ImVec2 preview_pos = ImVec2(bb.Min.x + 8.f, bb.Min.y + 6.f);
        window->DrawList->AddText(f::medium14, 20.f, preview_pos, col2, preview_value);
    }
    //popup bg 36, 37, 39
    const ImVec2 label_pos = ImVec2(total_bb.Min.x, total_bb.Min.y);
    window->DrawList->AddText(f::medium14, 20.f, label_pos, col, label);

    std::string counter = std::to_string(enabled_count) + "/" + std::to_string(all_count);
    ImVec2 counter_size = f::medium14->CalcTextSizeA(20.f, FLT_MAX, -1.f, counter.c_str(), 0, NULL);
    window->DrawList->AddText(f::medium14, 20.f, label_pos + ImVec2(total_size.x - counter_size.x, 0.f), col, counter.c_str());

    if (!popup_open)
        return false;

    g.NextWindowData.HasFlags = backup_next_window_data_flags;
    return BeginComboPopup(popup_id, bb, flags);
}

inline bool selectable_(const char* owner, const char* label, bool last, bool selected = false, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0))
{
    ImGuiWindow* window = GetCurrentWindow();
    if (window->SkipItems)
        return false;

    auto curr = anim::animation((std::string(label) + owner).c_str(), anim_t(clamp_out, 0.15f));
    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    // Submit label or explicit size to ItemSize(), whereas ItemAdd() will submit a larger/spanning rectangle.
    ImGuiID id = window->GetID(label);
    ImVec2 label_size = CalcTextSize(label, NULL, true);
    ImVec2 size(297.f, 25.f);
    ImVec2 pos = window->DC.CursorPos;
    //pos.y += window->DC.CurrLineTextBaseOffset;
    ItemSize(size, 0.0f);

    // Fill horizontal space
    // We don't support (size < 0.0f) in Selectable() because the ItemSpacing extension would make explicitly right-aligned sizes not visibly match other widgets.
    const bool span_all_columns = (flags & ImGuiSelectableFlags_SpanAllColumns) != 0;
    const float min_x = span_all_columns ? window->ParentWorkRect.Min.x : pos.x;
    const float max_x = span_all_columns ? window->ParentWorkRect.Max.x : window->WorkRect.Max.x;


    // Selectables are meant to be tightly packed together with no click-gap, so we extend their box to cover spacing between selectable.
    // FIXME: Not part of layout so not included in clipper calculation, but ItemSize currently doesn't allow offsetting CursorPos.
    ImRect bb(min_x, pos.y, min_x + size.x, pos.y + size.y);
    if ((flags & ImGuiSelectableFlags_NoPadWithHalfSpacing) == 0)
    {
        const float spacing_x = span_all_columns ? 0.0f : style.ItemSpacing.x;
        const float spacing_y = style.ItemSpacing.y;
        const float spacing_L = IM_TRUNC(spacing_x * 0.50f);
        const float spacing_U = IM_TRUNC(spacing_y * 0.50f);
        bb.Min.x -= spacing_L;
        bb.Min.y -= spacing_U;
        bb.Max.x += (spacing_x - spacing_L);
        bb.Max.y += (spacing_y - spacing_U);
    }
    //if (g.IO.KeyCtrl) { GetForegroundDrawList()->AddRect(bb.Min, bb.Max, IM_COL32(0, 255, 0, 255)); }

    const bool disabled_item = (flags & ImGuiSelectableFlags_Disabled) != 0;
    const ImGuiItemFlags extra_item_flags = disabled_item ? (ImGuiItemFlags)ImGuiItemFlags_Disabled : ImGuiItemFlags_None;
    bool is_visible;
    if (span_all_columns)
    {
        // Modify ClipRect for the ItemAdd(), faster than doing a PushColumnsBackground/PushTableBackgroundChannel for every Selectable..
        const float backup_clip_rect_min_x = window->ClipRect.Min.x;
        const float backup_clip_rect_max_x = window->ClipRect.Max.x;
        window->ClipRect.Min.x = window->ParentWorkRect.Min.x;
        window->ClipRect.Max.x = window->ParentWorkRect.Max.x;
        is_visible = ItemAdd(bb, id, NULL, extra_item_flags);
        window->ClipRect.Min.x = backup_clip_rect_min_x;
        window->ClipRect.Max.x = backup_clip_rect_max_x;
    }
    else
    {
        is_visible = ItemAdd(bb, id, NULL, extra_item_flags);
    }

    const bool is_multi_select = (g.LastItemData.ItemFlags & ImGuiItemFlags_IsMultiSelect) != 0;
    if (!is_visible)
        if (!is_multi_select || !g.BoxSelectState.UnclipMode || !g.BoxSelectState.UnclipRect.Overlaps(bb)) // Extra layer of "no logic clip" for box-select support (would be more overhead to add to ItemAdd)
            return false;

    const bool disabled_global = (g.CurrentItemFlags & ImGuiItemFlags_Disabled) != 0;
    if (disabled_item && !disabled_global) // Only testing this as an optimization
        BeginDisabled();

    // FIXME: We can standardize the behavior of those two, we could also keep the fast path of override ClipRect + full push on render only,
    // which would be advantageous since most selectable are not selected.
    if (span_all_columns)
    {
        if (g.CurrentTable)
            TablePushBackgroundChannel();
        else if (window->DC.CurrentColumns)
            PushColumnsBackground();
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_HasClipRect;
        g.LastItemData.ClipRect = window->ClipRect;
    }

    // We use NoHoldingActiveID on menus so user can click and _hold_ on a menu then drag to browse child entries
    ImGuiButtonFlags button_flags = 0;
    if (flags & ImGuiSelectableFlags_NoHoldingActiveID) { button_flags |= ImGuiButtonFlags_NoHoldingActiveId; }
    if (flags & ImGuiSelectableFlags_NoSetKeyOwner) { button_flags |= ImGuiButtonFlags_NoSetKeyOwner; }
    if (flags & ImGuiSelectableFlags_SelectOnClick) { button_flags |= ImGuiButtonFlags_PressedOnClick; }
    if (flags & ImGuiSelectableFlags_SelectOnRelease) { button_flags |= ImGuiButtonFlags_PressedOnRelease; }
    if (flags & ImGuiSelectableFlags_AllowDoubleClick) { button_flags |= ImGuiButtonFlags_PressedOnClickRelease | ImGuiButtonFlags_PressedOnDoubleClick; }
    if ((flags & ImGuiSelectableFlags_AllowOverlap) || (g.LastItemData.ItemFlags & ImGuiItemFlags_AllowOverlap)) { button_flags |= ImGuiButtonFlags_AllowOverlap; }

    // Multi-selection support (header)
    const bool was_selected = selected;
    if (is_multi_select)
    {
        // Handle multi-select + alter button flags for it
        MultiSelectItemHeader(id, &selected, &button_flags);
    }

    bool hovered, held;
    bool pressed = ButtonBehavior(bb, id, &hovered, &held, button_flags);

    // Multi-selection support (footer)
    if (is_multi_select)
    {
        MultiSelectItemFooter(id, &selected, &pressed);
    }
    else
    {
        // Auto-select when moved into
        // - This will be more fully fleshed in the range-select branch
        // - This is not exposed as it won't nicely work with some user side handling of shift/control
        // - We cannot do 'if (g.NavJustMovedToId != id) { selected = false; pressed = was_selected; }' for two reasons
        //   - (1) it would require focus scope to be set, need exposing PushFocusScope() or equivalent (e.g. BeginSelection() calling PushFocusScope())
        //   - (2) usage will fail with clipped items
        //   The multi-select API aim to fix those issues, e.g. may be replaced with a BeginSelection() API.
        if ((flags & ImGuiSelectableFlags_SelectOnNav) && g.NavJustMovedToId != 0 && g.NavJustMovedToFocusScopeId == g.CurrentFocusScopeId)
            if (g.NavJustMovedToId == id)
                selected = pressed = true;
    }

    // Update NavId when clicking or when Hovering (this doesn't happen on most widgets), so navigation can be resumed with keyboard/gamepad
    if (pressed || (hovered && (flags & ImGuiSelectableFlags_SetNavIdOnHover)))
    {
        if (!g.NavHighlightItemUnderNav && g.NavWindow == window && g.NavLayer == window->DC.NavLayerCurrent)
        {
            SetNavID(id, window->DC.NavLayerCurrent, g.CurrentFocusScopeId, WindowRectAbsToRel(window, bb)); // (bb == NavRect)
            if (g.IO.ConfigNavCursorVisibleAuto)
                g.NavCursorVisible = false;
        }
    }
    if (pressed)
        MarkItemEdited(id);

    if (selected != was_selected)
        g.LastItemData.StatusFlags |= ImGuiItemStatusFlags_ToggledSelection;
    curr->active = selected;
    float extra_a = curr->val;
    float extra_b = std::clamp(extra_a * 2.f, 0.f, 1.f);
    float extra_c = std::clamp((extra_b - 0.5f) * 2.f, 0.f, 1.f);

    ImColor col2 = lerp(c::white0, lerp(c::white48, c::secondary, global_a), extra_c);
    ImColor col3 = lerp(c::white0, lerp(c::white24, c::primary, global_a), extra_b);
    ImColor col4 = lerp(lerp(c::white24, c::white48, global_a), lerp(ImColor(20, 20, 20, 255), c::bg, global_a), extra_a);


    // Render
    if (is_visible)
    {
        int vert_start_idx = window->DrawList->VtxBuffer.Size;
        window->DrawList->AddRectFilled(bb.Min, bb.Max, IM_COL32_WHITE, last ? 4.f : 0, last ? ImDrawFlags_RoundCornersBottom : ImDrawFlags_None);
        int vert_end_idx = window->DrawList->VtxBuffer.Size;
        ShadeVertsLinearGradX(window->DrawList, vert_start_idx, vert_end_idx,
            bb.Min, bb.Max, col3, col2);

        if (extra_c > 0.f)
        {
            ImVec2 circle_pos = ImVec2(bb.Min.x + 1.f + 7.f * extra_c, bb.Min.y + 16.5f);
            window->DrawList->AddCircleFilled(circle_pos, 0.5f + 1.5f * extra_c, col4);
        }
    }

    if (span_all_columns)
    {
        if (g.CurrentTable)
            TablePopBackgroundChannel();
        else if (window->DC.CurrentColumns)
            PopColumnsBackground();
    }

    // Text stays at the submission position. Alignment/clipping extents ignore SpanAllColumns.
    if (is_visible)
    {
        const ImVec2 label_pos = ImVec2(bb.Min.x + 8.f + 10.f * extra_c, bb.Min.y + 6.f);
        window->DrawList->AddText(f::medium14, 20.f, label_pos, col4, label);
    }
    // Automatically close popups
    if (pressed && (window->Flags & ImGuiWindowFlags_Popup) && !(flags & ImGuiSelectableFlags_NoAutoClosePopups) && (g.LastItemData.ItemFlags & ImGuiItemFlags_AutoClosePopups))
        CloseCurrentPopup();

    if (disabled_item && !disabled_global)
        EndDisabled();

    // Selectable() always returns a pressed state!
    // Users of BeginMultiSelect()/EndMultiSelect() scope: you may call ImGui::IsItemToggledSelection() to retrieve
    // selection toggle, only useful if you need that state updated (e.g. for rendering purpose) before reaching EndMultiSelect().
    IMGUI_TEST_ENGINE_ITEM_INFO(id, label, g.LastItemData.StatusFlags);
    return pressed; //-V1020
}

inline bool selectable(const char* owner, const char* label, bool last, bool* p_selected, ImGuiSelectableFlags flags = 0, const ImVec2& size_arg = ImVec2(0, 0))
{
    if (selectable_(owner, label, last, *p_selected, flags, size_arg))
    {
        *p_selected = !*p_selected;
        return true;
    }
    return false;
}

inline void MultiCombo(const char* label, bool combos[], const char* items[], int items_count)
{
    std::vector<std::string> vec;
    static std::string preview;
    int j = 0;
    for (int i = 0; i < items_count; i++)
    {
        if (combos[i])
        {
            vec.push_back(items[i]);
            if (j > 2)
                preview = vec.at(0) + ", " + vec.at(1) + ", " + vec.at(2) + ", ...";
            else if (j)
                preview += ", " + (std::string)items[i];
            else
                preview = items[i];

            j++;
        }
    }
    if (begincombo(label, preview.c_str(), items_count, j))
    {
        ImGuiWindow* window = GetCurrentWindow();
        const auto& p = ::GetWindowPos();
        auto curr = anim::get_anim_obj(CONST_HASH(label));
        ImVec2 region = ::GetContentRegionMax();
        region.y *= curr->val;

        //bg popup
        window->DrawList->PushClipRectFullScreen();
        window->DrawList->AddRectFilled(p - ImVec2(0.f, 3.f), p + region + ImVec2(4.f, -4.f), lerp(ImColor(30, 30, 34, 255), ImColor(36, 37, 39, 255), global_a), 4.f, ImDrawFlags_RoundCornersBottom);
        window->DrawList->PopClipRect();

        //selectable popup
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 5.f);
        window->DrawList->PushClipRect(p - ImVec2(0.f, 3.f), p + region + ImVec2(4.f, 0.f));
        for (int i = 0; i < items_count; i++) {
            selectable(label, items[i], items_count == (i + 1), &combos[i], ImGuiSelectableFlags_DontClosePopups);
        }
        window->DrawList->PopClipRect();

        //border popup
        window->DrawList->PushClipRect(p - ImVec2(0.f, 3.f), p + region + ImVec2(4.f, 0.f));
        window->DrawList->AddRect(p - ImVec2(0.f, 4.f), p + region + ImVec2(4.f, -4.f), ImColor(44, 45, 48, 255), 4.f, ImDrawFlags_RoundCornersBottom);
        window->DrawList->PopClipRect();

        //fix size
        ImGui::SetCursorPosY(ImGui::GetCursorPos().y + 1.f);
        EndCombo();
    }

    preview = "None";
}
