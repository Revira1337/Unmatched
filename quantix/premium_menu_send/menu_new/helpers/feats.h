#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <Windows.h>
#include <imgui.h>
#include <imgui_internal.h>
#include <string>
#include <cstdio>
#include <cmath>
#include "render.h"
#include "anim/anim.h"
#include "../defs/colors.h"
#include "../defs/fonts.h"

// ---------------------------------------------------------------------------
// feats.h — compact widget set for the feature pages. Rows are label + desc on
// the left with a control on the right, grouped into rounded cards with an
// uppercase header and thin separators between rows. Visual only.
// ---------------------------------------------------------------------------
namespace ft
{
    inline ImDrawListSplitter g_split;
    inline int   g_rowi = 0;                 // row index inside the current card
    inline const char* g_keynames[] = { "None", "LMB", "RMB", "MB4", "MB5", "Shift", "Alt", "F", "C", "V", "E", "X", "Q" };
    inline ImColor card_bg = ImColor(30, 30, 30, 255);   // neutral grey card interior (no blue tint)

    // Helper function to convert ImGuiKey to VK code for GetAsyncKeyState
    inline int ImGuiKeyToVKCode(ImGuiKey key)
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

    constexpr float ROW_H = 40.f;            // compact row height
    constexpr float PAD_X = 15.f;

    // thin separator above every row except the first one in the card
    inline void sep(ImVec2 p, float w)
    {
        if (g_rowi++ > 0)
            ImGui::GetWindowDrawList()->AddLine(p + ImVec2(PAD_X, 0.f), p + ImVec2(w - PAD_X, 0.f), c::white6, 1.f);
    }

    inline anim_t* an(const char* label, const char* tag)
    {
        // faster, smooth toggle/check animation.
        // Fold the ImGui ID-stack seed (card PushID) into the key so identically
        // labelled widgets in different cards (e.g. two "Box"/"Distance" toggles)
        // never share one animation object.
        char key[160];
        snprintf(key, sizeof key, "%u#%s%s", (unsigned)ImGui::GetID(label), label, tag);
        return anim::animation(key, anim_t(clamp_out, 0.32f));
    }

    // ---- card ----
    inline void card(const char* title)
    {
        ImGui::BeginGroup();
        ImGui::PushID(title ? title : "card");
        ImDrawList* dl = ImGui::GetWindowDrawList();
        g_split.Split(dl, 2);
        g_split.SetCurrentChannel(dl, 1);
        g_rowi = 0;

        ImGui::Dummy(ImVec2(ImGui::GetContentRegionAvail().x, 8.f));
        if (title && title[0])
        {
            ImVec2 p = ImGui::GetCursorScreenPos();
            dl->AddText(f::medium12, 12.f, p + ImVec2(PAD_X, 0.f), c::white36, title);
            ImGui::Dummy(ImVec2(0.f, 15.f));
        }
    }
    inline void endcard()
    {
        ImGui::Dummy(ImVec2(0.f, 8.f));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGui::PopID();
        ImGui::EndGroup();

        ImVec2 mn = ImGui::GetItemRectMin();
        ImVec2 mx = ImGui::GetItemRectMax();
        g_split.SetCurrentChannel(dl, 0);
        dl->AddRectFilled(mn, mx, card_bg, 10.f);
        dl->AddRect(mn, mx, c::white6, 10.f);
        g_split.Merge(dl);

        ImGui::Dummy(ImVec2(0.f, 8.f));
    }

    // reserves a full-width row; fills p/w/hover, returns clicked. draws sep + hover bg.
    inline bool row(const char* id, ImVec2& p, float& w, bool& hovered, float h = ROW_H)
    {
        p = ImGui::GetCursorScreenPos();
        w = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        bool clicked = ImGui::InvisibleButton(id, ImVec2(w, h));
        hovered = ImGui::IsItemHovered();
        if (hovered)
            ImGui::GetWindowDrawList()->AddRectFilled(p, p + ImVec2(w, h), c::white2, 6.f);
        return clicked;
    }

    // ---- a single list row: name on the left, a small right-aligned value (e.g. distance) ----
    inline bool list_entry(const char* id, const char* name, const char* right, ImColor nameCol)
    {
        ImVec2 p; float w; bool hov;
        bool clicked = row(id, p, w, hov);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float cy = ROW_H * 0.5f;
        ImVec2 ns = f::medium14->CalcTextSizeA(14.f, FLT_MAX, -1.f, name, 0, NULL);
        dl->AddText(f::medium14, 14.f, p + ImVec2(PAD_X, cy - ns.y * 0.5f), nameCol, name);
        if (right && right[0])
        {
            ImVec2 rs = f::medium12->CalcTextSizeA(12.f, FLT_MAX, -1.f, right, 0, NULL);
            dl->AddText(f::medium12, 12.f, p + ImVec2(w - PAD_X - rs.x, cy - rs.y * 0.5f), c::white48, right);
        }
        return clicked;
    }

    inline void labels(ImDrawList* dl, ImVec2 p, const char* label, const char* desc, float cy)
    {
        if (desc && desc[0])
        {
            dl->AddText(f::medium14, 15.f, p + ImVec2(PAD_X, cy - 14.f), c::white, label);
            dl->AddText(f::medium12, 11.f, p + ImVec2(PAD_X, cy + 3.f), c::white36, desc);
        }
        else
        {
            ImVec2 ts = f::medium14->CalcTextSizeA(15.f, FLT_MAX, -1.f, label, 0, NULL);
            dl->AddText(f::medium14, 15.f, p + ImVec2(PAD_X, cy - ts.y * 0.5f), c::white, label);
        }
    }

    // ---- switch / toggle ----
    // ---- modern switch pill (shared by toggle + toggle_key) — always neutral greyscale ----
    inline void switch_widget(ImDrawList* dl, ImVec2 s0, float sw, float sh, float t)
    {
        ImVec2 s1 = s0 + ImVec2(sw, sh);
        const float r = sh * 0.5f;

        // ---- soft accent glow when ON ----
        if (t > 0.01f)
        {
            dl->AddRectFilled(s0 - ImVec2(4.f, 4.f), s1 + ImVec2(4.f, 4.f), a(c::primary, 0.10f * t), r + 4.f);
            dl->AddRectFilled(s0 - ImVec2(2.f, 2.f), s1 + ImVec2(2.f, 2.f), a(c::primary, 0.16f * t), r + 2.f);
        }

        // ---- track: dark grey (off) -> accent (on) with a gentle top-down sheen ----
        int v0 = dl->VtxBuffer.Size;
        dl->AddRectFilled(s0, s1, IM_COL32_WHITE, r);
        ShadeVertsLinearGradY(dl, v0, dl->VtxBuffer.Size, s0, s1,
            lerp(ImColor(255, 255, 255, 34), a(c::primary, 1.00f), t),
            lerp(ImColor(255, 255, 255, 16), a(c::primary, 0.78f), t));
        // crisp hairline border (no inner top line)
        dl->AddRect(s0, s1, lerp(a(c::white, 0.14f), a(c::black, 0.24f), t), r, 0, 1.f);

        // ---- knob: always bright white, slides; drop shadow + glossy top highlight ----
        const float kr = r - 2.5f;
        const float kx = s0.x + r + (sw - sh) * t;
        ImVec2 kc(kx, s0.y + r);
        dl->AddCircleFilled(ImVec2(kc.x, kc.y + 1.3f), kr + 0.9f, a(c::black, 0.30f));                 // drop shadow
        dl->AddCircleFilled(kc, kr, c::white);                                                          // knob
        dl->AddCircleFilled(ImVec2(kc.x - kr * 0.30f, kc.y - kr * 0.32f), kr * 0.5f, a(c::white, 0.6f));// gloss
    }

    inline void toggle(const char* label, const char* desc, bool* v)
    {
        ImVec2 p; float w; bool hov;
        if (row((std::string(label) + "##tg").c_str(), p, w, hov)) *v = !*v;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float cy = ROW_H * 0.5f;
        labels(dl, p, label, desc, cy);

        auto a0 = an(label, "##tga"); a0->active = *v; float t = a0->extra();
        const float sw = 38.f, sh = 20.f;
        ImVec2 s0 = p + ImVec2(w - PAD_X - sw, cy - sh * 0.5f);
        switch_widget(dl, s0, sw, sh, t);
    }

    // ---- checkbox / tick (modern, minimalist, smooth) ----
    inline void check(const char* label, const char* desc, bool* v)
    {
        ImVec2 p; float w; bool hov;
        if (row((std::string(label) + "##ck").c_str(), p, w, hov)) *v = !*v;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float cy = ROW_H * 0.5f;
        labels(dl, p, label, desc, cy);

        auto a0 = an(label, "##cka"); a0->active = *v;
        float e = a0->extra();                          // eased 0..1
        float pop = 1.f + 0.10f * sinf(e * 3.14159265f); // subtle grow mid-transition
        const float bs = 18.f * pop;
        ImVec2 bc = p + ImVec2(w - PAD_X - 9.f, cy);
        ImVec2 b0 = bc - ImVec2(bs * 0.5f, bs * 0.5f);
        ImVec2 b1 = bc + ImVec2(bs * 0.5f, bs * 0.5f);

        dl->AddRectFilled(b0, b1, lerp(c::white4, c::primary, e), 5.f);              // bg fades to primary
        dl->AddRect(b0, b1, lerp(c::white24, c::primary, e), 5.f, 0, 1.4f);          // crisp border
        if (e > 0.02f)                                                              // animated centre dot
            dl->AddCircleFilled(bc, (bs * 0.24f) * e, a(c::white, e));
    }

    // ---- slider ----
    inline void slider_base(const char* label, float* v, float mn, float mx, const char* valtext)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        const float h = 46.f;
        ImGui::InvisibleButton((std::string(label) + "##sl").c_str(), ImVec2(w, h));
        bool active = ImGui::IsItemActive();
        bool hovered = ImGui::IsItemHovered();
        ImDrawList* dl = ImGui::GetWindowDrawList();

        dl->AddText(f::medium14, 15.f, p + ImVec2(PAD_X, 8.f), c::white, label);
        ImVec2 vs = f::medium12->CalcTextSizeA(13.f, FLT_MAX, -1.f, valtext, 0, NULL);
        dl->AddText(f::medium12, 13.f, p + ImVec2(w - PAD_X - vs.x, 9.f), c::primary, valtext);

        float bx0 = p.x + PAD_X, bx1 = p.x + w - PAD_X, by = p.y + 33.f;
        float t = (mx > mn) ? (*v - mn) / (mx - mn) : 0.f; t = ImClamp(t, 0.f, 1.f);

        // smooth the visual position toward the real value (slight animation)
        ImGuiID sid = ImGui::GetID((std::string(label) + "##sldisp").c_str());
        ImGuiStorage* st = ImGui::GetStateStorage();
        float disp = st->GetFloat(sid, t);
        disp += (t - disp) * 0.35f;
        if (fabsf(t - disp) < 0.0015f) disp = t;
        st->SetFloat(sid, disp);

        // Animate hover/active state
        ImGuiID hid = ImGui::GetID((std::string(label) + "##slhover").c_str());
        float hover_t = st->GetFloat(hid, 0.f);
        float target_hover = (hovered || active) ? 1.f : 0.f;
        hover_t += (target_hover - hover_t) * 0.25f;
        st->SetFloat(hid, hover_t);

        float kx = bx0 + (bx1 - bx0) * disp;
        ImVec2 kc(kx, by);

        // Track background (rounded, subtle inner shading)
        const float trkR = 2.5f;
        dl->AddRectFilled(ImVec2(bx0, by - trkR), ImVec2(bx1, by + trkR), c::white12, trkR);

        // Filled portion: soft accent glow underneath + crisp fill on top
        if (kx > bx0 + 0.5f)
        {
            dl->AddRectFilled(ImVec2(bx0, by - trkR - 1.f), ImVec2(kx, by + trkR + 1.f),
                              a(c::primary, 0.20f + 0.28f * hover_t), trkR + 1.f);
            dl->AddRectFilled(ImVec2(bx0, by - trkR), ImVec2(kx, by + trkR), c::primary, trkR);
        }

        // Circular knob with grow-on-hover, soft glow ring, and drop shadow
        float kr = 6.f + hover_t * 2.5f;                 // 6px -> 8.5px
        if (active) kr += 0.5f;
        dl->AddShadowCircle(kc, kr, a(c::black, 0.45f), 12.f, ImVec2(0.f, 1.5f));
        if (hover_t > 0.01f)
            dl->AddCircleFilled(kc, kr + 5.f, a(c::primary, 0.16f * hover_t), 28);   // glow halo
        dl->AddCircleFilled(kc, kr, c::white, 28);                                    // white knob
        dl->AddCircle(kc, kr, a(c::primary, 0.35f + 0.45f * hover_t), 28, 1.4f);      // accent ring
        dl->AddCircleFilled(kc, (kr - 2.5f) * (0.6f + 0.4f * hover_t), c::primary, 24);// accent core grows in

        if (active)
        {
            float nt = (ImGui::GetIO().MousePos.x - bx0) / (bx1 - bx0);
            *v = mn + (mx - mn) * ImClamp(nt, 0.f, 1.f);
        }
    }
    inline void slideri(const char* label, int* v, int mn, int mx, const char* suffix = "")
    {
        float f = (float)*v; char buf[48]; snprintf(buf, sizeof buf, "%d%s", *v, suffix);
        slider_base(label, &f, (float)mn, (float)mx, buf);
        *v = (int)(f + 0.5f);
    }
    inline void sliderf(const char* label, float* v, float mn, float mx, const char* suffix = "")
    {
        char buf[48]; snprintf(buf, sizeof buf, "%.1f%s", *v, suffix);
        slider_base(label, v, mn, mx, buf);
    }

    // ---- keybind system: click a bind box -> capture the next key (classic) ----

    inline const char* kb_keyname(int vk)
    {
        if (vk <= 0) return "None";
        switch (vk)   // pretty short names for the mouse buttons
        {
            case VK_LBUTTON:  return "LMB";
            case VK_RBUTTON:  return "RMB";
            case VK_MBUTTON:  return "MMB";
            case VK_XBUTTON1: return "M4";
            case VK_XBUTTON2: return "M5";
        }
        for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
            if (ImGuiKeyToVKCode((ImGuiKey)key) == vk) return ImGui::GetKeyName((ImGuiKey)key);
        return "Key";
    }

    // which bind box is currently listening for a key (0 = none). Tracked by ImGui ID so it is
    // card-scoped and never bleeds between rows that share a label.
    inline ImGuiID kb_capture_id = 0;

    // premium animated keybind box: click to (re)capture the next key. Esc/Backspace/Delete while
    // listening = unbind. RIGHT edge is rightX, vertically centred at cyS. Returns width.
    // Layout-neutral: saves/restores the ImGui cursor so it never shifts the following row.
    inline float keybind_box(const std::string& label, int* k, float rightX, float cyS, float boxH = 22.f)
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const std::string bid = label + "##kbbox";
        const ImGuiID myid = ImGui::GetID(bid.c_str());
        bool capturing = (kb_capture_id == myid);

        // capture the next key while listening. Mouse buttons ARE bindable (right / middle / M4 / M5);
        // only left-click (used to click the box) and the mouse wheel are excluded.
        if (capturing)
        {
            for (int key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key++)
            {
                if (key == ImGuiKey_MouseLeft || key == ImGuiKey_MouseWheelX || key == ImGuiKey_MouseWheelY) continue;
                if (ImGui::IsKeyPressed((ImGuiKey)key, false))
                {
                    if (key == ImGuiKey_Escape || key == ImGuiKey_Backspace || key == ImGuiKey_Delete) *k = 0;
                    else *k = ImGuiKeyToVKCode((ImGuiKey)key);
                    kb_capture_id = 0;
                    capturing = false;
                    break;
                }
            }
        }

        // label text: animated "..." dots while listening, else key name / None
        const float fs = 12.f;
        char dots[4] = "...";
        const char* kn;
        if (capturing) { int nd = 1 + ((int)(ImGui::GetTime() * 2.5f) % 3); dots[nd] = '\0'; kn = dots; }
        else kn = (*k > 0) ? kb_keyname(*k) : "None";

        // measure current text; use the widest form while capturing so the target doesn't wobble
        ImVec2 kmw = f::medium12->CalcTextSizeA(fs, FLT_MAX, -1.f, capturing ? "..." : kn, 0, 0);
        ImVec2 ks  = f::medium12->CalcTextSizeA(fs, FLT_MAX, -1.f, kn, 0, 0);
        float target = ImMax(54.f, kmw.x + 20.f);

        ImGuiStorage* stg = ImGui::GetStateStorage();
        // MYNX-style width tween: the box smoothly morphs to fit the key name when it changes
        ImGuiID wid = ImGui::GetID((bid + "#w").c_str());
        float bw = stg->GetFloat(wid, target);
        bw += (target - bw) * 0.22f;
        if (fabsf(target - bw) < 0.5f) bw = target;
        stg->SetFloat(wid, bw);

        ImVec2 b0(rightX - bw, cyS - boxH * 0.5f), b1(rightX, cyS + boxH * 0.5f);

        // interaction — restore the cursor afterwards so layout is untouched
        ImVec2 saveCur = ImGui::GetCursorPos();
        ImGui::SetCursorScreenPos(b0);
        if (ImGui::InvisibleButton(bid.c_str(), ImVec2(bw, boxH)))
            kb_capture_id = capturing ? 0 : myid;
        bool hov = ImGui::IsItemHovered();
        ImGui::SetCursorPos(saveCur);

        // --- hover / capture ease ---
        ImGuiID aid = ImGui::GetID((bid + "#a").c_str());
        float hv = stg->GetFloat(aid, 0.f);
        hv += (((hov || capturing) ? 1.f : 0.f) - hv) * 0.18f;
        if (hv < 0.001f) hv = 0.f;
        stg->SetFloat(aid, hv);
        float pulse = capturing ? (0.5f + 0.5f * sinf((float)ImGui::GetTime() * 6.5f)) : 0.f;

        const float rnd = 6.f;
        // drop shadow
        dl->AddRectFilled(b0 + ImVec2(0.f, 1.5f), b1 + ImVec2(0.f, 1.5f), a(c::black, 0.18f), rnd);
        // body fill: idle grey -> accent tint on hover; capturing = full accent tint
        dl->AddRectFilled(b0, b1, lerp(c::white4, c::primary12, capturing ? 1.f : hv), rnd);
        // soft outer glow on hover / capture
        if (hv > 0.01f)
            dl->AddRect(b0 - ImVec2(1.f, 1.f), b1 + ImVec2(1.f, 1.f), a(c::primary, 0.16f * hv), rnd + 1.f, 0, 1.5f);
        // border: grey -> accent
        dl->AddRect(b0, b1, lerp(c::white12, c::primary, capturing ? (0.6f + 0.4f * pulse) : hv * 0.9f),
                    rnd, 0, capturing ? 1.5f : 1.2f);
        // capturing pulse ring
        if (capturing)
            dl->AddRect(b0 - ImVec2(1.5f, 1.5f), b1 + ImVec2(1.5f, 1.5f),
                        a(c::primary, 0.25f + 0.35f * pulse), rnd + 1.5f, 0, 1.4f);
        // key text (centred)
        dl->AddText(f::medium12, fs, ImVec2(b0.x + (bw - ks.x) * 0.5f, cyS - ks.y * 0.5f),
                    capturing ? c::primary : (*k > 0 ? c::white : c::white48), kn);
        return bw;
    }

    // ---- keybind row (label + desc, animated bind box on the right) ----
    // Mirrors toggle_key: no full-row InvisibleButton competing with the box (so the box is always
    // clickable) and restores the layout cursor so the next row's separator lands correctly.
    inline void keybind(const char* label, const char* desc, int* k)
    {
        ImVec2 wp = ImGui::GetCursorPos();
        ImVec2 p  = ImGui::GetCursorScreenPos();
        float  w  = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        const float h = ROW_H, cy = h * 0.5f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (ImGui::IsMouseHoveringRect(p, p + ImVec2(w, h)))
            dl->AddRectFilled(p, p + ImVec2(w, h), c::white2, 6.f);
        labels(dl, p, label, desc, cy);
        keybind_box(label, k, p.x + w - PAD_X, p.y + cy);
        ImGui::SetCursorPos(ImVec2(wp.x, wp.y + h));
        ImGui::Dummy(ImVec2(w, 0.f));
    }

    // ---- switch with a bind box immediately to its left ----
    inline void toggle_key(const char* label, const char* desc, bool* v, int* k)
    {
        ImVec2 wp = ImGui::GetCursorPos();
        ImVec2 p  = ImGui::GetCursorScreenPos();
        float  w  = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        const float h = ROW_H, cy = h * 0.5f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (ImGui::IsMouseHoveringRect(p, p + ImVec2(w, h)))
            dl->AddRectFilled(p, p + ImVec2(w, h), c::white2, 6.f);
        labels(dl, p, label, desc, cy);

        // switch (right)
        const float sw = 38.f, sh = 20.f;
        ImVec2 s0 = p + ImVec2(w - PAD_X - sw, cy - sh * 0.5f);
        ImGui::SetCursorScreenPos(s0);
        if (ImGui::InvisibleButton((std::string(label) + "##tkS").c_str(), ImVec2(sw, sh))) *v = !*v;
        auto a0 = an(label, "##tkA"); a0->active = *v; float t = a0->extra();
        switch_widget(dl, s0, sw, sh, t);

        // bind box (left of the switch)
        keybind_box(label, k, s0.x - 10.f, p.y + cy, 20.f);

        ImGui::SetCursorPos(ImVec2(wp.x, wp.y + h));
        ImGui::Dummy(ImVec2(w, 0.f));
    }

    // ---- text input field ----
    inline void input(const char* id, char* buf, int bufsz, const char* hint)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        ImGui::SetCursorScreenPos(p + ImVec2(PAD_X, 5.f));
        ImGui::SetNextItemWidth(w - PAD_X * 2.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, (ImU32)c::white4);
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, (ImU32)c::white6);
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, (ImU32)c::white6);
        ImGui::PushStyleColor(ImGuiCol_Text, (ImU32)c::white);
        ImGui::PushStyleColor(ImGuiCol_TextDisabled, (ImU32)c::white36);
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 8.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(11.f, 8.f));
        ImGui::InputTextWithHint(id, hint, buf, bufsz);
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(5);
        ImGui::Dummy(ImVec2(0.f, 4.f));
    }

    // ---- dropdown / combo ----
    inline void combo(const char* label, const char* desc, int* idx, const char* const* items, int count)
    {
        ImVec2 p; float w; bool hov;
        if (row((std::string(label) + "##cb").c_str(), p, w, hov))
            *idx = (*idx + 1) % count;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float cy = ROW_H * 0.5f;
        labels(dl, p, label, desc, cy);

        const char* cur = items[ImClamp(*idx, 0, count - 1)];
        const float bw = 96.f;
        ImVec2 b0 = p + ImVec2(w - PAD_X - bw, cy - 12.f);
        dl->AddRectFilled(b0, b0 + ImVec2(bw, 24.f), c::white4, 5.f);
        dl->AddRect(b0, b0 + ImVec2(bw, 24.f), c::white12, 5.f);
        dl->AddText(f::medium12, 13.f, b0 + ImVec2(9.f, 5.f), c::white, cur);
        ImVec2 cc = b0 + ImVec2(bw - 13.f, 12.f);
        dl->AddLine(cc + ImVec2(-3.f, -1.5f), cc + ImVec2(0.f, 1.5f), c::white48, 1.4f);
        dl->AddLine(cc + ImVec2(0.f, 1.5f), cc + ImVec2(3.f, -1.5f), c::white48, 1.4f);
    }

    // ---- color swatch + picker popup ----
    // ---- custom color picker body (SV square + hue + alpha + hex) ----
    inline void color_picker(float col[4])
    {
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImGuiIO& io = ImGui::GetIO();

        const float sv_w = 190.f, sv_h = 150.f, bar_w = 14.f, gap = 12.f;
        const float total_w = sv_w + gap + bar_w + gap + bar_w;

        float h, s, v;
        ImGui::ColorConvertRGBtoHSV(col[0], col[1], col[2], h, s, v);
        // keep hue stable when s/v collapse to 0 (hue is undefined there)
        ImGuiStorage* st = ImGui::GetStateStorage();
        ImGuiID hid = ImGui::GetID("##huebak");
        if (s <= 0.f || v <= 0.f) h = st->GetFloat(hid, h);

        ImVec2 org = ImGui::GetCursorScreenPos();

        // --- SV square ---
        ImVec2 sv0 = org, sv1 = sv0 + ImVec2(sv_w, sv_h);
        ImU32 hue_full = ImColor::HSV(h, 1.f, 1.f);
        dl->AddRectFilledMultiColor(sv0, sv1, IM_COL32_WHITE, hue_full, hue_full, IM_COL32_WHITE);
        dl->AddRectFilledMultiColor(sv0, sv1, IM_COL32(0,0,0,0), IM_COL32(0,0,0,0), IM_COL32(0,0,0,255), IM_COL32(0,0,0,255));
        dl->AddRect(sv0, sv1, c::white12, 4.f);
        ImGui::SetCursorScreenPos(sv0);
        ImGui::InvisibleButton("##sv", ImVec2(sv_w, sv_h));
        if (ImGui::IsItemActive())
        {
            s = ImClamp((io.MousePos.x - sv0.x) / sv_w, 0.f, 1.f);
            v = 1.f - ImClamp((io.MousePos.y - sv0.y) / sv_h, 0.f, 1.f);
            ImGui::ColorConvertHSVtoRGB(h, s, v, col[0], col[1], col[2]);
        }
        ImVec2 svc = ImVec2(sv0.x + s * sv_w, sv0.y + (1.f - v) * sv_h);
        dl->AddCircleFilled(svc, 5.f, ImColor(col[0], col[1], col[2], 1.f));
        dl->AddCircle(svc, 5.f, IM_COL32(0,0,0,160), 16, 2.5f);
        dl->AddCircle(svc, 5.f, IM_COL32_WHITE, 16, 1.5f);

        // --- Hue bar ---
        ImVec2 hue0 = ImVec2(sv1.x + gap, sv0.y);
        static const ImU32 hc[7] = {
            IM_COL32(255,0,0,255), IM_COL32(255,255,0,255), IM_COL32(0,255,0,255),
            IM_COL32(0,255,255,255), IM_COL32(0,0,255,255), IM_COL32(255,0,255,255), IM_COL32(255,0,0,255) };
        for (int i = 0; i < 6; i++)
        {
            ImVec2 a = ImVec2(hue0.x, hue0.y + sv_h / 6.f * i);
            ImVec2 b = ImVec2(hue0.x + bar_w, hue0.y + sv_h / 6.f * (i + 1));
            dl->AddRectFilledMultiColor(a, b, hc[i], hc[i], hc[i+1], hc[i+1]);
        }
        dl->AddRect(hue0, hue0 + ImVec2(bar_w, sv_h), c::white12, 3.f);
        ImGui::SetCursorScreenPos(hue0);
        ImGui::InvisibleButton("##hue", ImVec2(bar_w, sv_h));
        if (ImGui::IsItemActive())
        {
            h = ImClamp((io.MousePos.y - hue0.y) / sv_h, 0.f, 1.f);
            ImGui::ColorConvertHSVtoRGB(h, s, v, col[0], col[1], col[2]);
        }
        float hy = hue0.y + h * sv_h;
        dl->AddRectFilled(ImVec2(hue0.x - 3, hy - 3), ImVec2(hue0.x + bar_w + 3, hy + 3), IM_COL32_WHITE, 2.f);
        dl->AddRect(ImVec2(hue0.x - 3, hy - 3), ImVec2(hue0.x + bar_w + 3, hy + 3), IM_COL32(0,0,0,140), 2.f);

        // --- Alpha bar ---
        ImVec2 al0 = ImVec2(hue0.x + bar_w + gap, sv0.y);
        ImU32 atop = ImColor(col[0], col[1], col[2], 1.f), abot = ImColor(col[0], col[1], col[2], 0.f);
        dl->AddRectFilledMultiColor(al0, al0 + ImVec2(bar_w, sv_h), atop, atop, abot, abot);
        dl->AddRect(al0, al0 + ImVec2(bar_w, sv_h), c::white12, 3.f);
        ImGui::SetCursorScreenPos(al0);
        ImGui::InvisibleButton("##al", ImVec2(bar_w, sv_h));
        if (ImGui::IsItemActive())
            col[3] = 1.f - ImClamp((io.MousePos.y - al0.y) / sv_h, 0.f, 1.f);
        float ay = al0.y + (1.f - col[3]) * sv_h;
        dl->AddRectFilled(ImVec2(al0.x - 3, ay - 3), ImVec2(al0.x + bar_w + 3, ay + 3), IM_COL32_WHITE, 2.f);
        dl->AddRect(ImVec2(al0.x - 3, ay - 3), ImVec2(al0.x + bar_w + 3, ay + 3), IM_COL32(0,0,0,140), 2.f);

        st->SetFloat(hid, h);

        // --- preview swatch + hex input ---
        ImVec2 rowp = ImVec2(sv0.x, sv1.y + 12.f);
        dl->AddRectFilled(rowp, rowp + ImVec2(34.f, 26.f), ImColor(col[0], col[1], col[2], col[3]), 5.f);
        dl->AddRect(rowp, rowp + ImVec2(34.f, 26.f), c::white24, 5.f);

        char hex[8];
        int ri = (int)(col[0] * 255.f + 0.5f), gi = (int)(col[1] * 255.f + 0.5f), bi = (int)(col[2] * 255.f + 0.5f);
        snprintf(hex, sizeof hex, "%02X%02X%02X", ri, gi, bi);
        ImGui::SetCursorScreenPos(ImVec2(rowp.x + 44.f, rowp.y));
        ImGui::SetNextItemWidth(total_w - 44.f);
        ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1,1,1,0.04f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(1,1,1,0.06f));
        ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(1,1,1,0.08f));
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1,1,1,0.9f));
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.f);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(8.f, 6.f));
        ImGui::PushFont(f::medium14);
        if (ImGui::InputText("##hex", hex, sizeof hex,
            ImGuiInputTextFlags_CharsHexadecimal | ImGuiInputTextFlags_CharsUppercase | ImGuiInputTextFlags_EnterReturnsTrue))
        {
            unsigned int u = 0;
            if (sscanf(hex, "%x", &u) == 1)
            {
                col[0] = ((u >> 16) & 0xFF) / 255.f;
                col[1] = ((u >> 8)  & 0xFF) / 255.f;
                col[2] = ( u        & 0xFF) / 255.f;
            }
        }
        ImGui::PopFont();
        ImGui::PopStyleVar(2);
        ImGui::PopStyleColor(4);
    }

    inline void color(const char* label, const char* desc, float col[4])
    {
        ImVec2 p; float w; bool hov;
        bool clicked = row((std::string(label) + "##cl").c_str(), p, w, hov);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        const float cy = ROW_H * 0.5f;
        labels(dl, p, label, desc, cy);

        const float cw = 30.f, ch = 18.f;
        ImVec2 c0 = p + ImVec2(w - PAD_X - cw, cy - ch * 0.5f);
        dl->AddRectFilled(c0, c0 + ImVec2(cw, ch), ImColor(col[0], col[1], col[2], col[3]), 4.f);
        dl->AddRect(c0, c0 + ImVec2(cw, ch), c::white24, 4.f);

        std::string pid = std::string(label) + "##clpick";
        if (clicked) ImGui::OpenPopup(pid.c_str());

        ImGui::PushStyleColor(ImGuiCol_PopupBg, ImVec4(0.086f, 0.090f, 0.106f, 1.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(1, 1, 1, 0.08f));
        ImGui::PushStyleVar(ImGuiStyleVar_PopupRounding, 10.f);
        ImGui::PushStyleVar(ImGuiStyleVar_PopupBorderSize, 1.f);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(14.f, 14.f));
        if (ImGui::BeginPopup(pid.c_str()))
        {
            color_picker(col);
            ImGui::EndPopup();
        }
        ImGui::PopStyleVar(3);
        ImGui::PopStyleColor(2);
    }

    // ---- button row ----
    inline bool button(const char* label, const char* desc = "")
    {
        ImVec2 p; float w; bool hov;
        bool clicked = row((std::string(label) + "##bt").c_str(), p, w, hov, 42.f);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        auto a0 = an(label, "##bta"); a0->active = hov; float t = a0->extra();
        ImVec2 b0 = p + ImVec2(10.f, 6.f), b1 = p + ImVec2(w - 10.f, 36.f);
        const float r = 7.f;
        // drop shadow for depth
        dl->AddRectFilled(b0 + ImVec2(0.f, 2.f), b1 + ImVec2(0.f, 2.f), a(c::black, 0.35f), r);
        // body brightens on hover
        dl->AddRectFilled(b0, b1, lerp(c::white6, c::primary24, t), r);
        if (t > 0.001f) dl->AddRectFilled(b0, b1, a(c::primary, 0.10f * t), r);
        // top inner highlight
        dl->AddLine(ImVec2(b0.x + r, b0.y + 1.f), ImVec2(b1.x - r, b0.y + 1.f), a(c::white, 0.06f + 0.06f * t), 1.f);
        // border brightens on hover
        dl->AddRect(b0, b1, a(c::primary, 0.28f + 0.5f * t), r, 0, 1.f);
        // left accent bar grows on hover
        float bh = (b1.y - b0.y) * 0.55f * t;
        if (bh > 0.5f) { float cyb = (b0.y + b1.y) * 0.5f; dl->AddRectFilled(ImVec2(b0.x, cyb - bh * 0.5f), ImVec2(b0.x + 2.5f, cyb + bh * 0.5f), a(c::primary, t), 2.f); }
        // label with subtle shadow, brightening on hover
        ImVec2 ts = f::medium14->CalcTextSizeA(14.f, FLT_MAX, -1.f, label, 0, NULL);
        ImVec2 tp((b0.x + b1.x - ts.x) * 0.5f, (b0.y + b1.y - ts.y) * 0.5f);
        dl->AddText(f::medium14, 14.f, tp + ImVec2(0.f, 1.f), a(c::black, 0.35f), label);
        dl->AddText(f::medium14, 14.f, tp, lerp(c::white72, c::white, t), label);
        return clicked;
    }

    // ---- segmented control: exactly one of N options is always selected ----
    inline void segment(const char* id, const char* const* items, int count, int* sel)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        const float h = 30.f;
        ImVec2 b0 = p + ImVec2(PAD_X, 6.f), b1 = p + ImVec2(w - PAD_X, 6.f + h);
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->AddRectFilled(b0, b1, c::white4, 7.f);
        dl->AddRect(b0, b1, c::white12, 7.f);
        const float segw = (b1.x - b0.x) / (float)count;
        for (int i = 0; i < count; i++)
        {
            ImVec2 s0(b0.x + segw * i, b0.y), s1(b0.x + segw * (i + 1), b1.y);
            ImGui::SetCursorScreenPos(s0);
            char bid[40]; snprintf(bid, sizeof bid, "%s##sg%d", id, i);
            if (ImGui::InvisibleButton(bid, ImVec2(segw, h))) *sel = i;
            bool hov = ImGui::IsItemHovered();
            char tag[10]; snprintf(tag, sizeof tag, "sg%d", i);
            auto a0 = an(id, tag); a0->active = (*sel == i); float t = a0->extra();
            if (t > 0.001f)
            {
                ImVec2 f0 = s0 + ImVec2(3.f, 3.f), f1 = s1 - ImVec2(3.f, 3.f);
                dl->AddRectFilled(f0, f1, lerp(c::primary0, c::primary24, t), 5.f);
                dl->AddRect(f0, f1, a(c::primary, 0.6f * t), 5.f);
            }
            ImVec2 ts = f::medium12->CalcTextSizeA(12.f, FLT_MAX, -1.f, items[i], 0, NULL);
            ImColor tc = (*sel == i) ? c::white : (hov ? c::white72 : c::white48);
            dl->AddText(f::medium12, 12.f, ImVec2((s0.x + s1.x - ts.x) * 0.5f, (s0.y + s1.y - ts.y) * 0.5f), tc, items[i]);
        }
        ImGui::SetCursorScreenPos(ImVec2(p.x, b1.y));
        ImGui::Dummy(ImVec2(w, 6.f));
    }

    // ---- fake search box ----
    inline void search(const char* hint = "Search...")
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        sep(p, w);
        ImGui::InvisibleButton("##search", ImVec2(w, 42.f));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 b0 = p + ImVec2(PAD_X, 4.f), b1 = p + ImVec2(w - PAD_X, 38.f);
        dl->AddRectFilled(b0, b1, c::white4, 8.f);
        dl->AddRect(b0, b1, c::white12, 8.f);
        ImVec2 mc = b0 + ImVec2(16.f, 17.f);
        dl->AddCircle(mc, 5.f, c::white48, 12, 1.5f);
        dl->AddLine(mc + ImVec2(3.5f, 3.5f), mc + ImVec2(7.f, 7.f), c::white48, 1.5f);
        dl->AddText(f::medium14, 14.f, b0 + ImVec2(32.f, 8.f), c::white36, hint);
    }

    // ---- empty placeholder panel ----
    inline void placeholder(const char* text, float height = 120.f)
    {
        ImVec2 p = ImGui::GetCursorScreenPos();
        float w = ImGui::GetContentRegionAvail().x;
        ImGui::Dummy(ImVec2(w, height));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 ts = f::medium14->CalcTextSizeA(14.f, FLT_MAX, -1.f, text, 0, NULL);
        dl->AddText(f::medium14, 14.f, p + ImVec2((w - ts.x) * 0.5f, height * 0.5f - 8.f), c::white24, text);
    }
}
