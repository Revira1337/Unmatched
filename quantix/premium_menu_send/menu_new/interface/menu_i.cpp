#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <D3DX11tex.h>
#include <string>
#include <ctime>

#include "menu_i.h"
#include "../helpers/anim/anim.h"
#include "../helpers/restore.h"
#include "../helpers/render.h"
#include "../helpers/widgets.h"
#include "../defs/fonts.h"
#include "../defs/textures.h"
#include "../defs/colors.h"
#include "../bytes/bg.h"
#include "../bytes/loadbg.h"
#include "../bytes/medium.h"
#include "../bytes/semibold.h"
#include "../bytes/logo.h"
#include "../../framework/data/brand_logo_data.h"
#include "../bytes/icon.h"
#include "../bytes/esp.h"
#include "../bytes/fa_icons.h"
#include "elements_manager.h"
#include "esp_shared.h"
#include "settings_shared.h"
#include <ctime>
#include <cstdio>
#include <cmath>
#include <vector>
#include <windows.h>
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")

using namespace ImGui;

//need for anim imgui AddText
float alpha_text = 1.f;

// ---------------------------------------------------------------------------
// Lightweight system stats for the watermark (real CPU% + real GPU%).
// Sampled at most once per second on the render thread — no extra thread.
//   CPU : GetSystemTimes (idle vs. total)
//   GPU : PDH  "\GPU Engine(*engtype_3D)\Utilization Percentage" summed
// Everything is guarded; if a source is unavailable the value simply stays 0.
// ---------------------------------------------------------------------------
namespace sysstat
{
    struct Stats
    {
        // CPU
        ULARGE_INTEGER pIdle{}, pKernel{}, pUser{};
        bool cpuInit = false;
        // GPU (PDH)
        PDH_HQUERY   gpuQuery   = nullptr;
        PDH_HCOUNTER gpuCounter = nullptr;
        bool gpuTried = false, gpuOk = false;
        std::vector<unsigned char> gpuBuf;

        float cpu = 0.f, gpu = 0.f;
        double lastSample = -1.0;

        void sampleCpu()
        {
            FILETIME idle, kernel, user;
            if (!GetSystemTimes(&idle, &kernel, &user)) return;
            ULARGE_INTEGER i, k, u;
            i.LowPart = idle.dwLowDateTime;   i.HighPart = idle.dwHighDateTime;
            k.LowPart = kernel.dwLowDateTime; k.HighPart = kernel.dwHighDateTime;
            u.LowPart = user.dwLowDateTime;   u.HighPart = user.dwHighDateTime;
            if (cpuInit)
            {
                unsigned long long idleD = i.QuadPart - pIdle.QuadPart;
                unsigned long long sysD  = (k.QuadPart - pKernel.QuadPart) + (u.QuadPart - pUser.QuadPart);
                if (sysD > 0)
                {
                    float busy = (float)(sysD - idleD) / (float)sysD * 100.f;
                    cpu = busy < 0.f ? 0.f : (busy > 100.f ? 100.f : busy);
                }
            }
            pIdle = i; pKernel = k; pUser = u; cpuInit = true;
        }

        void ensureGpu()
        {
            if (gpuTried) return;
            gpuTried = true;
            if (PdhOpenQueryW(nullptr, 0, &gpuQuery) != ERROR_SUCCESS) { gpuQuery = nullptr; return; }
            if (PdhAddEnglishCounterW(gpuQuery, L"\\GPU Engine(*engtype_3D)\\Utilization Percentage", 0, &gpuCounter) != ERROR_SUCCESS)
            {
                PdhCloseQuery(gpuQuery); gpuQuery = nullptr; return;
            }
            PdhCollectQueryData(gpuQuery);   // prime the counter
            gpuOk = true;
        }

        void sampleGpu()
        {
            ensureGpu();
            if (!gpuOk) return;
            if (PdhCollectQueryData(gpuQuery) != ERROR_SUCCESS) return;
            DWORD bufSize = 0, itemCount = 0;
            PDH_STATUS st = PdhGetFormattedCounterArrayW(gpuCounter, PDH_FMT_DOUBLE, &bufSize, &itemCount, nullptr);
            if (st != PDH_MORE_DATA || bufSize == 0) return;
            if (gpuBuf.size() < bufSize) gpuBuf.resize(bufSize);
            auto* items = reinterpret_cast<PDH_FMT_COUNTERVALUE_ITEM_W*>(gpuBuf.data());
            if (PdhGetFormattedCounterArrayW(gpuCounter, PDH_FMT_DOUBLE, &bufSize, &itemCount, items) != ERROR_SUCCESS) return;
            double sum = 0.0;
            for (DWORD n = 0; n < itemCount; n++)
                if (items[n].FmtValue.CStatus == ERROR_SUCCESS || items[n].FmtValue.CStatus == 0)
                    sum += items[n].FmtValue.doubleValue;
            if (sum < 0.0) sum = 0.0; if (sum > 100.0) sum = 100.0;
            gpu = (float)sum;
        }

        void update()
        {
            double now = ImGui::GetTime();
            if (lastSample < 0.0 || now - lastSample >= 1.0)
            {
                sampleCpu();
                sampleGpu();
                lastSample = now;
            }
        }
    };
    inline Stats g_stats;

    // Cached Windows user name (queried once).
    inline const char* username()
    {
        static char name[64] = { 0 };
        static bool done = false;
        if (!done)
        {
            done = true;
            DWORD n = sizeof(name);
            if (!GetUserNameA(name, &n) || name[0] == '\0')
            { name[0] = 'U'; name[1] = 's'; name[2] = 'e'; name[3] = 'r'; name[4] = '\0'; }
        }
        return name;
    }
}

// ---------------------------------------------------------------------------
// Vertical layout of the main panel:
//   [ SUBBAR ]  floating subtab strip (above the active tab)
//   [ TOPBAR ]  main tab icons
//   (PANEL_GAP)
//   [ CONTENT ] page title
// The esp-preview is a separate panel on the right.
// ---------------------------------------------------------------------------
constexpr float MENU_W    = 720.f;
constexpr float MENU_H    = 578.f;
constexpr float SUBBAR_H  = 40.f;   // floating subtab strip at the very top
constexpr float TOPBAR_H  = 52.f;   // main tab icon bar
constexpr float PANEL_GAP = 10.f;   // gap between the top bar and the content
constexpr float ESP_W     = 340.f;  // width of the esp-preview panel (wide for side text/bars)
constexpr float ESP_GAP   = 12.f;   // gap between the main panel and esp panel

constexpr float TOPBAR_Y  = SUBBAR_H;                        // 40
constexpr float CONTENT_Y = SUBBAR_H + TOPBAR_H + PANEL_GAP; // 102

// ---- sidebar layout (settings::layout == 0) ----------------------------------
// A left navigation rail replaces the top bar; the content area sits to its right.
constexpr float SIDEBAR_W   = 224.f;                 // width of the left rail
constexpr float SIDE_CW     = 700.f;                 // content width in sidebar mode
constexpr float SIDE_MENU_W = SIDEBAR_W + SIDE_CW;   // total window width (924)
constexpr float SIDE_CY     = 14.f;                  // content top padding in sidebar mode

inline bool  side_mode() { return settings::layout == 0; }              // sidebar vs topbar
inline float win_w()     { return side_mode() ? SIDE_MENU_W : MENU_W; } // active window width

class _c_menu : public c_menu
{
public:
    void setup_data(ID3D11Device* device) override
    {
        MGR->g_pd3dDevice = g_pd3dDevice = device;
        if (tex::bg == nullptr)
            D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, bg, sizeof(bg), 0, 0, &tex::bg, 0);
        if (tex::sidebar_bg == nullptr)
            D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, sidebar, sizeof(sidebar), 0, 0, &tex::sidebar_bg, 0);
        if (tex::main_logo == nullptr)
            D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, logo, sizeof(logo), 0, 0, &tex::main_logo, 0);
        if (tex::brand_logo == nullptr)
            D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, brand_logo_png, sizeof(brand_logo_png), 0, 0, &tex::brand_logo, 0);
        if (tex::esp == nullptr)
            D3DX11CreateShaderResourceViewFromMemory(g_pd3dDevice, esp, sizeof(esp), 0, 0, &tex::esp, 0);

        ImGuiIO& io = ImGui::GetIO();

        // The ttf data are static global arrays, NOT heap allocations. Tell the
        // atlas not to take ownership so it doesn't free() them on shutdown
        // (that caused a heap-corruption assert when closing the menu).
        ImFontConfig font_cfg;
        font_cfg.FontDataOwnedByAtlas = false;

        f::sbold16 = io.Fonts->AddFontFromMemoryTTF(&semibold_ttf, sizeof semibold_ttf, 18.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::logo72 = io.Fonts->AddFontFromMemoryTTF(&semibold_ttf, sizeof semibold_ttf, 74.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::medium14 = io.Fonts->AddFontFromMemoryTTF(&medium_ttf, sizeof medium_ttf, 20.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::medium12 = io.Fonts->AddFontFromMemoryTTF(&medium_ttf, sizeof medium_ttf, 18.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons60 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 60.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons30 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 30.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons20 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 20.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons13 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 13.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons12 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 12.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());
        f::icons10 = io.Fonts->AddFontFromMemoryTTF(&icons_ttf, sizeof icons_ttf, 10.f, &font_cfg, io.Fonts->GetGlyphRangesCyrillic());

        // Font Awesome 6 (solid) — only the glyphs we use for the top-bar tabs.
        static const ImWchar icon_ranges[] = {
            0xe19b, 0xe19b, // gun
            0xf007, 0xf007, // user
            0xf013, 0xf013, // gear
            0xf05b, 0xf05b, // crosshairs
            0xf06e, 0xf06e, // eye
            0xf1b9, 0xf1b9, // car
            0xf7c0, 0xf7c0, // satellite-dish
            // current sidebar tab icons
            0xf140, 0xf140, // bullseye    (aim)
            0xf14e, 0xf14e, // compass     (radar)
            0xf1de, 0xf1de, // sliders     (settings)
            0xf1e2, 0xf1e2, // bomb        (weapon)
            0xf1e5, 0xf1e5, // binoculars  (visuals)
            0xf21b, 0xf21b, // user-secret (player)
            0xf5e4, 0xf5e4, // car-side    (vehicle)
            0,
        };
        f::tab_icons = io.Fonts->AddFontFromMemoryCompressedTTF(fa_icons_compressed_data, fa_icons_compressed_size, 22.f, NULL, icon_ranges);
    }
    void setup_imgui() override
    {
        ImGuiStyle* style = &::GetStyle();
        ImVec4* colors = style->Colors;

        colors[ImGuiCol_Text] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_TextDisabled] = ImVec4(0.50f, 0.50f, 0.50f, 1.00f);
        colors[ImGuiCol_WindowBg] = ImColor(0, 0, 0, 255);
        colors[ImGuiCol_ChildBg] = ImColor(18, 18, 18, 255);
        colors[ImGuiCol_PopupBg] = ImColor(12, 12, 12, 240);
        colors[ImGuiCol_Border] = ImColor(0, 0, 0, 0);
        colors[ImGuiCol_BorderShadow] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_FrameBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.055f);
        colors[ImGuiCol_FrameBgHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.09f);
        colors[ImGuiCol_FrameBgActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.13f);
        colors[ImGuiCol_TitleBg] = ImVec4(0.04f, 0.04f, 0.04f, 1.00f);
        colors[ImGuiCol_TitleBgActive] = ImVec4(0.16f, 0.16f, 0.16f, 1.00f);
        colors[ImGuiCol_TitleBgCollapsed] = ImVec4(0.00f, 0.00f, 0.00f, 0.51f);
        colors[ImGuiCol_MenuBarBg] = ImVec4(0.10f, 0.10f, 0.10f, 1.00f);
        colors[ImGuiCol_ScrollbarBg] = c::white4;
        colors[ImGuiCol_ScrollbarGrab] = c::white12;
        colors[ImGuiCol_ScrollbarGrabHovered] = c::white12;
        colors[ImGuiCol_ScrollbarGrabActive] = c::white12;
        colors[ImGuiCol_CheckMark] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_SliderGrab] = ImVec4(0.78f, 0.78f, 0.78f, 1.00f);
        colors[ImGuiCol_SliderGrabActive] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_Button] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        colors[ImGuiCol_ButtonHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
        colors[ImGuiCol_ButtonActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.28f);
        colors[ImGuiCol_Header] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        colors[ImGuiCol_HeaderHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.16f);
        colors[ImGuiCol_HeaderActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.22f);
        colors[ImGuiCol_Separator] = colors[ImGuiCol_Border];
        colors[ImGuiCol_SeparatorHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
        colors[ImGuiCol_SeparatorActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.45f);
        colors[ImGuiCol_ResizeGrip] = ImVec4(1.00f, 1.00f, 1.00f, 0.10f);
        colors[ImGuiCol_ResizeGripHovered] = ImVec4(1.00f, 1.00f, 1.00f, 0.20f);
        colors[ImGuiCol_ResizeGripActive] = ImVec4(1.00f, 1.00f, 1.00f, 0.30f);
        colors[ImGuiCol_Tab] = ImLerp(colors[ImGuiCol_Header], colors[ImGuiCol_TitleBgActive], 0.80f);
        colors[ImGuiCol_TabHovered] = colors[ImGuiCol_HeaderHovered];
        colors[ImGuiCol_TabActive] = ImLerp(colors[ImGuiCol_HeaderActive], colors[ImGuiCol_TitleBgActive], 0.60f);
        colors[ImGuiCol_TabUnfocused] = ImLerp(colors[ImGuiCol_Tab], colors[ImGuiCol_TitleBg], 0.80f);
        colors[ImGuiCol_TabUnfocusedActive] = ImLerp(colors[ImGuiCol_TabActive], colors[ImGuiCol_TitleBg], 0.40f);
        colors[ImGuiCol_PlotLines] = ImVec4(0.61f, 0.61f, 0.61f, 1.00f);
        colors[ImGuiCol_PlotLinesHovered] = ImVec4(1.00f, 0.43f, 0.35f, 1.00f);
        colors[ImGuiCol_PlotHistogram] = ImVec4(0.90f, 0.70f, 0.00f, 1.00f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(1.00f, 0.60f, 0.00f, 1.00f);
        colors[ImGuiCol_TableHeaderBg] = ImVec4(0.19f, 0.19f, 0.20f, 1.00f);
        colors[ImGuiCol_TableBorderStrong] = ImVec4(0.31f, 0.31f, 0.35f, 1.00f);
        colors[ImGuiCol_TableBorderLight] = ImVec4(0.23f, 0.23f, 0.25f, 1.00f);
        colors[ImGuiCol_TableRowBg] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
        colors[ImGuiCol_TableRowBgAlt] = ImVec4(1.00f, 1.00f, 1.00f, 0.06f);
        colors[ImGuiCol_TextSelectedBg] = ImVec4(1.00f, 1.00f, 1.00f, 0.22f);
        colors[ImGuiCol_DragDropTarget] = ImVec4(1.00f, 1.00f, 0.00f, 0.90f);
        colors[ImGuiCol_NavHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 1.00f);
        colors[ImGuiCol_NavWindowingHighlight] = ImVec4(1.00f, 1.00f, 1.00f, 0.70f);
        colors[ImGuiCol_NavWindowingDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.20f);
        colors[ImGuiCol_ModalWindowDimBg] = ImVec4(0.80f, 0.80f, 0.80f, 0.35f);
        colors[ImGuiCol_WindowShadow] = ImVec4(0.08f, 0.08f, 0.08f, 0.35f);

        style->ChildRounding = 0.f;
        style->WindowRounding = 16.f;
        style->WindowPadding = ImVec2(0, 0);
        style->ItemSpacing.y = 8.f;
        style->ScrollbarSize = 5.f;
        style->ScrollbarRounding = 512.f;
        style->WindowShadowSize = 0.f; // no hazy shadow behind the menu
    }

    // Font Awesome icon for a tab, centered at c.
    void tab_icon(const std::string& key, ImVec2 c, ImColor col)
    {
        const char* g =
            key == "aim"      ? "\xef\x85\x80" : // bullseye     (U+F140)
            key == "eye"      ? "\xef\x87\xa5" : // binoculars   (U+F1E5)
            key == "player"   ? "\xef\x88\x9b" : // user-secret  (U+F21B)
            key == "car"      ? "\xef\x97\xa4" : // car-side     (U+F5E4)
            key == "weapon"   ? "\xef\x87\xa2" : // bomb         (U+F1E2)
            key == "radar"    ? "\xef\x85\x8e" : // compass      (U+F14E)
            key == "settings" ? "\xef\x87\x9e" : // sliders      (U+F1DE)
                                "";

        const float sz = 20.f;
        ImVec2 ts = f::tab_icons->CalcTextSizeA(sz, FLT_MAX, -1.f, g, NULL, NULL);
        ::GetWindowDrawList()->AddText(f::tab_icons, sz, c - ts * 0.5f, col, g);
    }

    // Main tab icon bar. Icons are centered; the active icon has a sliding
    // highlight. Remembers the active icon center so the subtab strip can
    // anchor above it.
    void topbar()
    {
        auto draw = ::GetWindowDrawList();
        const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);

        const int   count   = int(MGR->tabs.size());
        const float slot    = 46.f;
        const float start_x = p.x + (MENU_W - slot * count) * 0.5f;
        const float icon_y  = p.y + TOPBAR_Y + (TOPBAR_H - slot) * 0.5f;
        const ImVec2 tab_size(slot, slot);

        // sliding highlight behind the active icon
        static elastic_point x_ind = elastic_point(0.f, 0.3f, 0.8f, 1.2f);
        x_ind.set_target(slot * screen);
        x_ind.update(0.16f);
        ImVec2 hl = ImVec2(start_x + x_ind.get_pos(), icon_y);
        draw->AddRectFilled(hl, hl + tab_size, c::primary12, 12.f);

        for (int i = 0; i < count; i++)
        {
            auto& element = MGR->tabs.at(i);
            ImVec2 pos = ImVec2(start_x + slot * i, icon_y);
            auto curr = anim::animation((element.label + "##tabtop").c_str(), anim_t(clamp_out, 0.1f));

            float active = ImMax(curr->val, screen == i ? 1.f : 0.f);
            tab_icon(element.icon, pos + tab_size * 0.5f, lerp(c::white48, c::primary, active));

            ::GetCurrentWindow()->DC.CursorPos = pos;
            if (::InvisibleButton((element.label + "##tabtop").c_str(), tab_size))
                screen = i;
            curr->active = ::IsItemHovered();
        }

        // decorative accent lines flanking the icon group, fading toward the edges
        const float cy    = p.y + TOPBAR_Y + TOPBAR_H * 0.5f;
        const float e_pad = 26.f;   // distance from the panel edge
        const float e_gap = 24.f;   // gap between line and the icon group
        // left line
        {
            float x0 = p.x + e_pad, x1 = start_x - e_gap;
            if (x1 > x0)
            {
                int vs = draw->VtxBuffer.Size;
                draw->AddRectFilled(ImVec2(x0, cy - 0.5f), ImVec2(x1, cy + 0.5f), IM_COL32_WHITE);
                ShadeVertsLinearGradX(draw, vs, draw->VtxBuffer.Size, ImVec2(x0, cy), ImVec2(x1, cy), c::white0, c::white24);
                draw->AddCircleFilled(ImVec2(x1, cy), 2.f, c::primary);
            }
        }
        // right line
        {
            float x0 = start_x + slot * count + e_gap, x1 = p.x + MENU_W - e_pad;
            if (x1 > x0)
            {
                int vs = draw->VtxBuffer.Size;
                draw->AddRectFilled(ImVec2(x0, cy - 0.5f), ImVec2(x1, cy + 0.5f), IM_COL32_WHITE);
                ShadeVertsLinearGradX(draw, vs, draw->VtxBuffer.Size, ImVec2(x0, cy), ImVec2(x1, cy), c::white24, c::white0);
                draw->AddCircleFilled(ImVec2(x0, cy), 2.f, c::primary);
            }
        }

        // center of the active icon (absolute) for the subtab strip
        active_tab_cx = start_x + slot * screen + slot * 0.5f;
    }

    // Floating subtab strip that hovers above the active tab. Only the active
    // tab's subtabs are shown; it elastically slides when the tab changes.
    void subtab_bar()
    {
        auto draw = ::GetWindowDrawList();
        const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);

        auto& tab = MGR->tabs.at(screen);
        const int n = int(tab.subtabs.size());
        if (n <= 0)
            return;

        const float h       = 30.f;
        const float pad_in  = 15.f;  // padding inside each subtab button
        const float pad_bar = 5.f;   // padding at the strip ends

        // measure buttons
        float widths[8]; float total = 0.f;
        for (int i = 0; i < n; i++)
        {
            widths[i] = f::medium12->CalcTextSizeA(18.f, FLT_MAX, -1.f, tab.subtabs[i].name.c_str(), 0, NULL).x + pad_in * 2.f;
            total += widths[i];
        }
        const float bar_w = total + pad_bar * 2.f;

        // horizontal center: above the active tab, clamped inside the panel
        float target_cx = ImClamp(active_tab_cx, p.x + 8.f + bar_w * 0.5f, p.x + MENU_W - 8.f - bar_w * 0.5f);

        static elastic_point bar_x = elastic_point(0.f, 0.35f, 0.85f, 1.2f);
        if (!bar_init) { bar_x.set_pos(target_cx); bar_init = true; }
        bar_x.set_target(target_cx);
        bar_x.update(0.16f);

        const float cx  = bar_x.get_pos();
        const float bx0 = cx - bar_w * 0.5f;
        const float by0 = p.y + (SUBBAR_H - h) * 0.5f;

        // connector line down to the top bar
        draw->AddRectFilled(ImVec2(cx - 1.f, by0 + h), ImVec2(cx + 1.f, p.y + TOPBAR_Y), c::white12);

        // strip background
        ImVec2 b0(bx0, by0), b1(bx0 + bar_w, by0 + h);
        draw->AddRectFilled(b0, b1, c::bg, h * 0.5f);
        draw->AddRect(b0, b1, c::white6, h * 0.5f);

        // sliding active highlight (local offset animated)
        float active_local = pad_bar, active_w = widths[0];
        {
            float lx = pad_bar;
            for (int i = 0; i < n; i++) { if (i == tab.active_sub) { active_local = lx; active_w = widths[i]; } lx += widths[i]; }
        }
        static elastic_point sub_hl = elastic_point(pad_bar, 0.4f, 0.85f, 1.2f);
        sub_hl.set_target(active_local);
        sub_hl.update(0.16f);
        float hlx = bx0 + sub_hl.get_pos();
        draw->AddRectFilled(ImVec2(hlx, by0 + 3.f), ImVec2(hlx + active_w, by0 + h - 3.f), c::primary12, (h - 6.f) * 0.5f);

        // buttons
        float lx = bx0 + pad_bar;
        for (int i = 0; i < n; i++)
        {
            auto& st = tab.subtabs[i];
            ImVec2 bpos(lx, by0);
            ImVec2 bsz(widths[i], h);
            std::string id = std::to_string(screen) + "_" + st.name;
            auto curr = anim::animation((id + "##sub").c_str(), anim_t(clamp_out, 0.1f));

            float active = ImMax(curr->val, tab.active_sub == i ? 1.f : 0.f);
            ImVec2 tsz = f::medium12->CalcTextSizeA(18.f, FLT_MAX, -1.f, st.name.c_str(), 0, NULL);
            draw->AddText(f::medium12, 18.f, bpos + (bsz - tsz) * 0.5f, lerp(c::white48, c::white, active), st.name.c_str());

            ::GetCurrentWindow()->DC.CursorPos = bpos;
            if (::InvisibleButton((id + "##subbtn").c_str(), bsz))
                tab.active_sub = i;
            curr->active = ::IsItemHovered();
            lx += widths[i];
        }
    }

    // Professional vector brand mark — drawn entirely from primitives (NO image
    // asset), so the build stays a single exe. A rounded accent tile with a soft
    // top-down sheen and a minimalist ring-and-tail "Q" monogram; scales at any
    // size and follows the accent colour. (param is `ctr`, not `c`, so it doesn't
    // shadow the colour namespace `c`.)
    void draw_mark(ImVec2 ctr, float S, float alpha = 1.f)
    {
        auto draw = ::GetWindowDrawList();
        const float h = S * 0.5f;
        const ImVec2 a0 = ctr - ImVec2(h, h), a1 = ctr + ImVec2(h, h);

        // The real Quantix logo (logo.png embedded as bytes -> tex::main_logo), tinted so
        // it follows the menu accent. Replaces the old vector "Q" monogram. UVs crop the
        // transparent padding so the mark fills the tile. Single-exe safe (no external file).
        if (tex::main_logo)
            draw->AddImage((ImTextureID)tex::main_logo, a0, a1,
                           ImVec2(0.06f, 0.06f), ImVec2(0.94f, 0.94f), a(c::primary, alpha));
    }

    // Left navigation rail (sidebar layout). A brand header sits on top, then a
    // vertical list of tabs; the selected tab expands (accordion) to reveal its
    // subtabs beneath it — joined by a soft tree rail — while a sliding accent bar
    // tracks the active row. A user chip anchors the bottom.
    // (named side_rail, not sidebar, to avoid clashing with the global `sidebar`
    //  texture byte-array used in setup_data.)
    void side_rail()
    {
        auto draw = ::GetWindowDrawList();
        const ImVec2 p = ::GetWindowPos() + ImVec2(1.f, 1.f);   // window top-left

        const float PADX    = 16.f;
        const float rowH    = 40.f;
        const float subH    = 30.f;
        const float iconCol = p.x + 30.f;                 // tab icon center x / tree rail x
        const float textX   = p.x + 50.f;                 // tab label x
        const float rowX0   = p.x + 8.f;
        const float rowX1   = p.x + SIDEBAR_W - 8.f;

        // ---- brand header ----
        {
            const float LS = 37.f;
            float hx = p.x + PADX, hy = p.y + 12.f;
            draw_mark(ImVec2(hx + LS * 0.5f, hy + LS * 0.5f), LS);
            // two-line brand lockup: "Quantix" over a muted "Software"
            draw->AddText(f::sbold16, 18.f, ImVec2(hx + LS + 12.f, hy + LS * 0.5f - 15.f), c::white, "Quantix");
            draw->AddText(f::medium12, 11.f, ImVec2(hx + LS + 12.f, hy + LS * 0.5f + 3.f), a(c::white, 0.42f), "Software");
            draw->AddLine(ImVec2(p.x + PADX, p.y + 60.f), ImVec2(p.x + SIDEBAR_W - PADX, p.y + 60.f), c::white6, 1.f);
        }

        // ---- tab list (accordion) ----
        float y = p.y + 74.f;
        float activeRowCy = y + rowH * 0.5f;
        const int count = int(MGR->tabs.size());

        for (int i = 0; i < count; i++)
        {
            auto& tab = MGR->tabs.at(i);
            const bool selected = (i == screen);

            auto expA = anim::animation((tab.label + "##side_exp").c_str(), anim_t(clamp_out, 0.16f));
            expA->active = selected;
            float e = expA->extra();                       // eased 0..1 expansion

            auto hvA = anim::animation((tab.label + "##side_hv").c_str(), anim_t(clamp_out, 0.16f));
            auto acA = anim::animation((tab.label + "##side_ac").c_str(), anim_t(clamp_out, 0.16f));
            acA->active = selected;
            float hov = hvA->val;
            float act = ImMax(acA->val, selected ? 1.f : 0.f);

            // row background: soft accent when active, faint white on hover
            ImVec2 r0(rowX0, y + 2.f), r1(rowX1, y + rowH - 2.f);
            if (act > 0.01f)       draw->AddRectFilled(r0, r1, a(c::primary, 0.10f * act), 9.f);
            else if (hov > 0.01f)  draw->AddRectFilled(r0, r1, a(c::white, 0.045f * hov), 9.f);

            // icon + label
            tab_icon(tab.icon, ImVec2(iconCol, y + rowH * 0.5f), lerp(c::white48, c::primary, act));
            draw->AddText(f::medium12, 18.f, ImVec2(textX, y + rowH * 0.5f - 9.f),
                          lerp(c::white48, c::white, ImMax(act, hov * 0.6f)), tab.label.c_str());

            // click target
            ::GetCurrentWindow()->DC.CursorPos = ImVec2(rowX0, y);
            if (::InvisibleButton((tab.label + "##side_tab").c_str(), ImVec2(rowX1 - rowX0, rowH)))
                screen = i;
            hvA->active = ::IsItemHovered();

            if (selected) activeRowCy = y + rowH * 0.5f;
            y += rowH;

            // expanded subtabs (only rendered while the tab has any open-height)
            const int n = int(tab.subtabs.size());
            const float fullSub = n * subH + 6.f;
            const float groupH  = e * fullSub;
            if (n > 0 && groupH > 0.5f)
            {
                draw->PushClipRect(ImVec2(rowX0, y), ImVec2(rowX1, y + groupH + 1.f), true);
                const float sy = y + 4.f;

                // tree rail down the icon column
                float railTop = sy + subH * 0.5f;
                float railBot = sy + (n - 0.5f) * subH;
                draw->AddLine(ImVec2(iconCol, railTop), ImVec2(iconCol, railBot), a(c::white12, e), 1.5f);

                // sliding active highlight — only the open tab drives it, so switching
                // subtabs glides the bright rail segment + pill from the old row to the
                // new one instead of snapping. Snaps (no glide) when the tab changes.
                if (selected)
                {
                    static elastic_point sub_slide(0.f, 0.34f, 0.86f, 1.1f);
                    static bool ss_init = false; static int ss_last = -1;
                    float tgt = (sy + tab.active_sub * subH + subH * 0.5f) - p.y;
                    if (!ss_init || ss_last != screen) { sub_slide.set_pos(tgt); ss_init = true; ss_last = screen; }
                    sub_slide.set_target(tgt);
                    sub_slide.update(0.16f);
                    float hcy = p.y + sub_slide.get_pos();
                    draw->AddRectFilled(ImVec2(iconCol + 8.f, hcy - subH * 0.5f + 3.f), ImVec2(rowX1, hcy + subH * 0.5f - 3.f), a(c::primary, 0.09f * e), 8.f);
                    draw->AddRectFilled(ImVec2(iconCol - 1.5f, hcy - subH * 0.30f), ImVec2(iconCol + 1.5f, hcy + subH * 0.30f), a(c::primary, e), 2.f);
                }

                for (int s = 0; s < n; s++)
                {
                    auto& st = tab.subtabs[s];
                    float scy = sy + s * subH + subH * 0.5f;

                    auto sacA = anim::animation((tab.label + "|" + st.name + "##side_sac").c_str(), anim_t(clamp_out, 0.16f));
                    sacA->active = (tab.active_sub == s);
                    float sact = sacA->val;                    // eased active state -> text crossfade
                    auto shvA = anim::animation((tab.label + "|" + st.name + "##side_shv").c_str(), anim_t(clamp_out, 0.16f));
                    float shov = shvA->val;

                    // connector tick + faint node, both dimming as the row becomes active
                    // so they melt into the sliding highlight instead of doubling up
                    draw->AddLine(ImVec2(iconCol, scy), ImVec2(iconCol + 13.f, scy), a(c::white12, e * (1.f - sact)), 1.f);
                    if (sact < 0.98f)
                        draw->AddCircleFilled(ImVec2(iconCol, scy), 2.f, a(c::white24, e * (1.f - sact)));

                    draw->AddText(f::medium12, 17.f, ImVec2(textX + 6.f, scy - 8.f),
                                  lerp(a(c::white36, e), a(c::white, e), ImMax(sact, shov * 0.6f)), st.name.c_str());

                    if (selected)   // only the open tab's subtabs are interactive
                    {
                        ::GetCurrentWindow()->DC.CursorPos = ImVec2(rowX0, sy + s * subH);
                        if (::InvisibleButton((tab.label + "|" + st.name + "##side_sub").c_str(), ImVec2(rowX1 - rowX0, subH)))
                        {
                            screen = i;
                            tab.active_sub = s;
                        }
                        shvA->active = ::IsItemHovered();
                    }
                }
                draw->PopClipRect();
                y += groupH;
            }
        }

        // ---- sliding accent bar on the far-left edge, tracking the active row ----
        {
            static elastic_point acc(0.f, 0.32f, 0.85f, 1.1f);
            static bool acc_init = false;
            float target = activeRowCy - p.y;                 // window-relative so it survives dragging
            if (!acc_init) { acc.set_pos(target); acc_init = true; }
            acc.set_target(target);
            acc.update(0.16f);
            float cyb = p.y + acc.get_pos();
            draw->AddRectFilled(ImVec2(p.x + 2.f, cyb - 11.f), ImVec2(p.x + 5.f, cyb + 11.f), c::primary, 2.f);
        }

        // ---- user chip (bottom) ----
        {
            const float chipH = 54.f;
            float cyc = p.y + MENU_H - chipH - 12.f;
            draw->AddLine(ImVec2(p.x + PADX, cyc - 8.f), ImVec2(p.x + SIDEBAR_W - PADX, cyc - 8.f), c::white6, 1.f);

            const float AV = 34.f;
            ImVec2 avc(p.x + PADX + AV * 0.5f, cyc + chipH * 0.5f);
            draw_mark(avc, AV);

            const char* user = sysstat::username();
            float tx = p.x + PADX + AV + 10.f;
            draw->AddText(f::sbold16, 15.f, ImVec2(tx, cyc + chipH * 0.5f - 15.f), c::white, user);
            draw->AddText(f::medium12, 12.f, ImVec2(tx, cyc + chipH * 0.5f + 2.f), a(c::white, 0.40f), "Quantix \xC2\xB7 3 Months");
        }
    }

    // Page content: the active subtab's features, laid out in two scrolling columns.
    void content()
    {
        auto& tab = MGR->tabs.at(screen);
        auto& sub = tab.subtabs[tab.active_sub];

        // page transition: fade + gentle vertical rise whenever the page changes
        int page_id = screen * 100 + tab.active_sub;
        if (page_id != cur_page_id) { cur_page_id = page_id; page_anim = 0.f; }
        float pdt = ImGui::GetIO().DeltaTime; if (pdt > 0.05f) pdt = 0.05f;
        page_anim = ImMin(1.f, page_anim + pdt / 0.50f);   // slower & heavier (~0.5s)
        float pu = 1.f - page_anim;
        float pe = 1.f - pu * pu * pu * pu * pu;            // easeOutQuint — long, smooth, weighty settle
        float rise = (1.f - pe) * 28.f;                     // larger travel for a heavier feel

        const float pad = 16.f;
        const bool  sm  = side_mode();
        const float cx0 = sm ? SIDEBAR_W : 0.f;                    // content-region left edge
        const float cw  = sm ? SIDE_CW   : MENU_W;                 // content-region width
        const float childTop = sm ? (SIDE_CY + 50.f) : CONTENT_Y; // where the scrolling list begins

        // sidebar mode: a slim page header (tab breadcrumb + subtab title) above the cards
        if (sm)
        {
            auto wdl = ::GetWindowDrawList();
            const ImVec2 p = ::GetWindowPos() + ImVec2(1.f, 1.f);
            float hx = p.x + cx0 + pad;
            wdl->AddText(f::medium12, 12.f, ImVec2(hx, p.y + SIDE_CY + 5.f), c::white36, tab.label.c_str());
            wdl->AddText(f::sbold16, 22.f, ImVec2(hx, p.y + SIDE_CY + 19.f), c::white, sub.name.c_str());
        }

        ImGui::SetCursorPos(ImVec2(cx0 + pad, childTop + pad + rise));
        ImGui::BeginChild("##page", ImVec2(cw - pad * 2.f, MENU_H - childTop - pad - 8.f - rise), false,
            ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NavFlattened);
        {
            if (sub.callback)
            {
                // no automatic item spacing: rows sit flush, card gaps are explicit
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.f, 0.f));
                if (ImGui::BeginTable("##cols", 2, ImGuiTableFlags_PadOuterX))
                {
                    ImGui::TableNextRow();
                    ImGui::TableSetColumnIndex(0);
                    sub.callback(0);
                    ImGui::TableSetColumnIndex(1);
                    sub.callback(1);
                    ImGui::EndTable();
                }
                ImGui::PopStyleVar();
            }
            else
            {
                ImVec2 av = ImGui::GetContentRegionAvail();
                ImVec2 pp = ImGui::GetCursorScreenPos();
                const char* title = sub.name.c_str();
                ImVec2 tsz = f::sbold16->CalcTextSizeA(28.f, FLT_MAX, -1.f, title, 0, NULL);
                ImGui::GetWindowDrawList()->AddText(f::sbold16, 28.f, pp + ImVec2((av.x - tsz.x) * 0.5f, av.y * 0.5f - 20.f), c::white, title);
            }
        }
        ImGui::EndChild();

        // fade-in mask: panel-coloured cover over the content area, fading out as the page settles
        if (pe < 0.999f)
        {
            auto pdl = ::GetWindowDrawList();
            const ImVec2 pp = ::GetWindowPos() + ImVec2(1.f, 1.f);
            if (sm)
                pdl->AddRectFilled(pp + ImVec2(SIDEBAR_W, 0.f), pp + ImVec2(SIDE_MENU_W, MENU_H), a(c::bg, 1.f - pe), 16.f, ImDrawFlags_RoundCornersRight);
            else
                pdl->AddRectFilled(pp + ImVec2(0.f, CONTENT_Y), pp + ImVec2(MENU_W, MENU_H), a(c::bg, 1.f - pe), 16.f);
        }
    }

    // A single premium hitbox dot: hollow ring when off, filled + glowing when
    // on, grows on hover. Toggles *v on click. Returns whether it is hovered.
    bool esp_dot(const char* name, ImVec2 c, bool* v)
    {
        ImGuiWindow* window = ::GetCurrentWindow();
        auto draw = window->DrawList;
        const float R = 7.5f;
        ImRect bb(c - ImVec2(R, R), c + ImVec2(R, R));
        ImGuiID id = window->GetID((std::string(name) + "##espdot").c_str());
        ItemAdd(bb, id);
        bool hovered, held;
        bool pressed = ButtonBehavior(bb, id, &hovered, &held);
        (void)held;
        if (pressed)
            *v = !*v;

        auto on_a = anim::animation((std::string(name) + "##espdoton").c_str(), anim_t(clamp_out, 0.1f));
        on_a->active = *v;
        auto hv_a = anim::animation((std::string(name) + "##espdoth").c_str(), anim_t(clamp_out, 0.1f));
        hv_a->active = hovered;

        float f = on_a->val, h = hv_a->val, ga = global_a;
        float rad = R + 1.6f * h;
        float glow = ImMax(f, h);

        if (glow > 0.01f)
            draw->AddCircleFilled(c, rad + 4.f, a(c::primary, 0.16f * glow * ga));
        draw->AddCircleFilled(c, rad, a(c::black, 0.30f * ga));                       // dark backing
        draw->AddCircle(c, rad, a(lerp(c::white48, c::primary, f), ga), 32, 1.6f);    // ring
        if (f > 0.01f)
            draw->AddCircleFilled(c, (rad - 3.f) * f, a(c::primary, ga));             // fill grows with on-state
        draw->AddCircleFilled(c, 2.0f, a(lerp(c::white72, c::white, f), ga));         // bright core

        return hovered;
    }

    // Preview panel. Its own draggable window. Aim Assist tab -> clickable bone
    // hitboxes; Visuals tab -> ESP element preview driven by esps:: state; other
    // tabs -> hidden. The character/skeleton/dots are shrunk and centered.
    void esp_preview()
    {
        const int mode = (screen == 0) ? 1 : (screen == 1) ? 2 : 0;   // 1=aim, 2=esp, 0=hidden
        bool show = (mode == 1 && settings::aim_preview) || (mode == 2 && settings::esp_preview_on);
        if (show) disp_mode = mode;                                    // remember what we display

        // open/close animation (fade + slide)
        prev_a += ((show ? 1.f : 0.f) - prev_a) * 0.20f;
        if (prev_a < 0.02f)
            return;
        const int dmode = disp_mode;

        const float PANEL_H  = side_mode() ? MENU_H : (MENU_H - TOPBAR_Y);
        const float PANEL_TY = side_mode() ? 0.f    : TOPBAR_Y;
        ImVec2 mainpos = ::GetWindowPos();
        ::SetNextWindowSize(ImVec2(ESP_W, PANEL_H));
        // pinned to the right of the main menu (slides in/out); panel itself doesn't move.
        ::SetNextWindowPos(mainpos + ImVec2(win_w() + ESP_GAP + 2.f + (1.f - prev_a) * 22.f, PANEL_TY), ImGuiCond_Always);
        ::Begin("##esp_preview", nullptr,
            ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoBackground |
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings);

        global_a = 1.f;
        ImDrawList* draw = ::GetWindowDrawList();
        ImVec2 panel = ::GetWindowPos();

        // background + soft bottom glow + border
        draw->AddRectFilled(panel, panel + ImVec2(ESP_W, PANEL_H), c::bg, 16.f);

        // drifting starfield behind the preview (bottom -> top), matching the menu backdrop
        {
            float st = (float)ImGui::GetTime();
            auto sfrac = [](float x) { return x - floorf(x); };
            auto shash = [&](float nn) { return sfrac(sinf(nn) * 43758.5453f); };
            draw->PushClipRect(panel, panel + ImVec2(ESP_W, PANEL_H), true);
            const int PSTARS = 28;
            for (int i = 0; i < PSTARS; i++)
            {
                float fi = (float)i;
                float speed = 5.f + shash(fi * 2.3f) * 10.f;
                float sx = panel.x + shash(fi) * ESP_W;
                float sway = sinf(st * (0.2f + shash(fi * 1.7f) * 0.2f) + fi) * 6.f;
                float sy = panel.y + PANEL_H - fmodf(shash(fi * 3.1f) * PANEL_H + st * speed, PANEL_H);
                float tw = 0.5f + 0.5f * sinf(st * (0.8f + shash(fi * 4.9f) * 1.2f) + fi * 1.3f);
                float rad = 0.7f + shash(fi * 5.7f) * 1.1f;
                draw->AddCircleFilled(ImVec2(sx + sway, sy), rad, a(c::white, 0.08f + 0.20f * tw), 8);
            }
            draw->PopClipRect();
        }

        draw->PushClipRect(panel + ImVec2(1.f, 1.f), panel + ImVec2(ESP_W - 1.f, PANEL_H - 1.f), true);
        draw->AddShadowCircle(panel + ImVec2(ESP_W * 0.5f, PANEL_H * 0.55f), 26.f, a(c::primary, 0.30f), 230.f, ImVec2(0.f, 0.f));
        int gv = draw->VtxBuffer.Size;
        draw->AddRectFilled(panel + ImVec2(0.f, PANEL_H - 175.f), panel + ImVec2(ESP_W, PANEL_H), IM_COL32_WHITE, 16.f, ImDrawFlags_RoundCornersBottom);
        ShadeVertsLinearGradY(draw, gv, draw->VtxBuffer.Size, panel + ImVec2(0.f, PANEL_H - 175.f), panel + ImVec2(0.f, PANEL_H), a(c::primary, 0.f), a(c::primary, 0.13f));
        draw->PopClipRect();

        // shrunk (0.8), centered character
        const float S = 0.8f;
        const float iw = 249.f * S, ih = 492.f * S;
        ImVec2 img_tl = panel + ImVec2((ESP_W - iw) * 0.5f, (PANEL_H - ih) * 0.5f);
        draw->AddImage(ImTextureID(tex::esp), img_tl, img_tl + ImVec2(iw, ih));

        static const struct { const char* name; float x, y; } HB[] = {
            {"Head",       122,  75}, {"Neck",       122, 108},
            {"Chest",      122, 159}, {"L Shoulder",  75, 122},
            {"L Elbow",     44, 205}, {"L Hand",      32, 262},
            {"R Shoulder", 173, 122}, {"R Elbow",    205, 205},
            {"R Hand",     218, 262}, {"Pelvis",     122, 248},
            {"L Knee",      96, 370}, {"L Foot",      93, 474},
            {"R Knee",     155, 370}, {"R Foot",     161, 474},
        };
        static const int BONES[][2] = {
            {0,1},{1,2},{2,9},{1,3},{1,6},{3,4},{4,5},{6,7},{7,8},{9,10},{10,11},{9,12},{12,13},
        };
        const int N = 14;
        ImVec2 centers[N];
        for (int i = 0; i < N; i++)
            centers[i] = img_tl + ImVec2(HB[i].x * S, HB[i].y * S);

        if (dmode == 1)   // ---------- AIM: clickable bone hitboxes ----------
        {
            // Determine which bone array to use based on active subtab
            // Tab 0 = Aim Assist: subtab 0 = Aimbot, subtab 1 = Silent, subtab 2 = Trigger (use Aimbot bones)
            auto& tab = MGR->tabs.at(0); // Aim Assist tab
            bool* bone_array = (tab.active_sub == 1) ? g_Options.Silent.bones : g_Options.Aimbot.bones;
            
            static int bone_pref = 0;
            static bool bone_init = false;
            const char* pnames[4] = { "Closest Bone", "Head Only", "Upper Body", "Lower Body" };
            auto set_preset = [&](int pr) {
                for (int i = 0; i < N; i++) bone_array[i] = false;
                if (pr == 0)      { for (int i = 0; i < N; i++) bone_array[i] = true; }
                else if (pr == 1) { bone_array[0] = true; }
                else if (pr == 2) { const int u[] = { 0,1,2,3,4,6,7 };    for (int x : u) bone_array[x] = true; }
                else              { const int l[] = { 5,8,9,10,11,12,13 }; for (int x : l) bone_array[x] = true; }
            };
            if (!bone_init) { set_preset(bone_pref); bone_init = true; }
            auto match_pr = [&]() -> int {
                for (int pr = 0; pr < 4; pr++) {
                    bool pv[14] = {};
                    if (pr == 0)      { for (int i = 0; i < N; i++) pv[i] = true; }
                    else if (pr == 1) { pv[0] = true; }
                    else if (pr == 2) { const int u[] = { 0,1,2,3,4,6,7 };    for (int x : u) pv[x] = true; }
                    else              { const int l[] = { 5,8,9,10,11,12,13 }; for (int x : l) pv[x] = true; }
                    bool m = true;
                    for (int i = 0; i < N; i++) if (pv[i] != bone_array[i]) { m = false; break; }
                    if (m) return pr;
                }
                return -1;
            };
            // bone-priority selector at the top (click to cycle)
            {
                ImVec2 b0 = panel + ImVec2(16.f, 12.f), b1 = panel + ImVec2(ESP_W - 16.f, 42.f);
                ImGui::SetCursorScreenPos(b0);
                if (ImGui::InvisibleButton("##bonepref", b1 - b0)) { bone_pref = (bone_pref + 1) % 4; set_preset(bone_pref); }
                bool bhov = ImGui::IsItemHovered();
                draw->AddRectFilled(b0, b1, bhov ? c::white6 : c::white4, 8.f);
                draw->AddRect(b0, b1, c::white12, 8.f);
                float cyb = (b0.y + b1.y) * 0.5f;
                draw->AddText(f::medium12, 11.f, ImVec2(b0.x + 12.f, cyb - 6.f), c::white36, "BONE PRIORITY");
                int mt = match_pr();
                const char* val = (mt < 0) ? "Custom" : pnames[mt];
                ImVec2 vs = f::medium14->CalcTextSizeA(13.f, FLT_MAX, -1.f, val, 0, NULL);
                draw->AddText(f::medium14, 13.f, ImVec2(b1.x - 20.f - vs.x, cyb - 7.f), (mt < 0) ? c::white : c::primary, val);
                ImVec2 chv(b1.x - 13.f, cyb);
                draw->AddLine(chv + ImVec2(-3.f, -1.5f), chv + ImVec2(0.f, 1.5f), c::white48, 1.4f);
                draw->AddLine(chv + ImVec2(0.f, 1.5f), chv + ImVec2(3.f, -1.5f), c::white48, 1.4f);
            }
            for (auto& bn : BONES)
            {
                ImVec2 ca = centers[bn[0]], cb = centers[bn[1]];
                float oa = anim::get_anim_obj(CONST_HASH((std::string(HB[bn[0]].name) + "##espdoton").c_str()))->val;
                float ob = anim::get_anim_obj(CONST_HASH((std::string(HB[bn[1]].name) + "##espdoton").c_str()))->val;
                float on = ImMin(oa, ob);
                if (on > 0.04f)
                    draw->AddLine(ca, cb, a(c::primary, 0.22f * on), 5.f);
                draw->AddLine(ca, cb, lerp(c::white24, c::primary, on), 1.4f + 1.f * on);
            }
            int hov = -1;
            for (int i = 0; i < N; i++)
                if (esp_dot(HB[i].name, centers[i], &bone_array[i]))
                    hov = i;
            if (hov >= 0)
            {
                const char* nm = HB[hov].name;
                ImVec2 cc = centers[hov];
                ImVec2 ts = f::medium12->CalcTextSizeA(14.f, FLT_MAX, -1.f, nm, 0, NULL);
                ImVec2 pad(9.f, 5.f), sz = ts + pad * 2.f;
                float ay = cc.y - 14.f - sz.y;
                ImVec2 tl = (ay < panel.y + 4.f) ? ImVec2(cc.x - sz.x * 0.5f, cc.y + 14.f) : ImVec2(cc.x - sz.x * 0.5f, ay);
                tl.x = ImClamp(tl.x, panel.x + 4.f, panel.x + ESP_W - sz.x - 4.f);
                draw->AddRectFilled(tl, tl + sz, a(c::black, 0.85f), 5.f);
                draw->AddRect(tl, tl + sz, a(c::primary, 0.5f), 5.f);
                draw->AddText(f::medium12, 14.f, tl + pad, c::white, nm);
            }
        }
        else             // ---------- ESP: element preview (draggable elements) ----------
        {
            // character bounding box (image space, scaled) — slightly enlarged all round
            ImVec2 cb0 = img_tl + ImVec2(6.f * S, 0.f * S);
            ImVec2 cb1 = img_tl + ImVec2(244.f * S, 500.f * S);
            ImVec2 cc = (cb0 + cb1) * 0.5f;
            float cx = cc.x;

            // ---- esp preset selector at the top (click to cycle) ----
            {
                struct EP { bool box, dashed, skel, name, dist, wep, hp, ar, id, filled; int sn, sw, sd, sh, sa, si; };
                static const EP PR[3] = {
                    { true,  true,  true,  true,  true,  true,  true,  true,  true,  false, 0, 5, 2, 0, 0, 1 },  // Advanced (ID upper-left, filled box off)
                    { false, false, true,  true,  false, false, true,  true,  false, false, 0, 1, 2, 0, 0, 1 },  // Basic
                    { false, false, false, true,  false, false, false, false, false, false, 0, 1, 2, 0, 0, 1 },  // Name only
                };
                const char* enames[3] = { "Advanced", "Basic", "Name only" };
                static int esp_pref = 0; static bool esp_pinit = false;
                auto apply = [&](int p) {
                    const EP& e = PR[p];
                    esps::box = e.box; esps::dashed = e.dashed; esps::skeleton = e.skel; esps::name = e.name;
                    esps::distance = e.dist; esps::weapon = e.wep; esps::health = e.hp; esps::armor = e.ar;
                    esps::show_id = e.id; esps::filled = e.filled;
                    esps::slot_name = e.sn; esps::slot_weapon = e.sw; esps::slot_distance = e.sd;
                    esps::slot_health = e.sh; esps::slot_armor = e.sa; esps::slot_id = e.si;
                };
                if (!esp_pinit) { apply(0); esp_pinit = true; }
                auto match_e = [&]() -> int {
                    for (int p = 0; p < 3; p++) { const EP& e = PR[p];
                        if (esps::box == e.box && esps::dashed == e.dashed && esps::skeleton == e.skel && esps::name == e.name &&
                            esps::distance == e.dist && esps::weapon == e.wep && esps::health == e.hp && esps::armor == e.ar &&
                            esps::show_id == e.id && esps::filled == e.filled &&
                            esps::slot_name == e.sn && esps::slot_weapon == e.sw && esps::slot_distance == e.sd &&
                            esps::slot_health == e.sh && esps::slot_armor == e.sa && esps::slot_id == e.si) return p;
                    }
                    return -1;
                };
                ImVec2 b0 = panel + ImVec2(16.f, 12.f), b1 = panel + ImVec2(ESP_W - 16.f, 42.f);
                ImGui::SetCursorScreenPos(b0);
                if (ImGui::InvisibleButton("##esppreset", b1 - b0)) { esp_pref = (esp_pref + 1) % 3; apply(esp_pref); }
                bool ehov = ImGui::IsItemHovered();
                draw->AddRectFilled(b0, b1, ehov ? c::white6 : c::white4, 8.f);
                draw->AddRect(b0, b1, c::white12, 8.f);
                float cyb = (b0.y + b1.y) * 0.5f;
                draw->AddText(f::medium12, 11.f, ImVec2(b0.x + 12.f, cyb - 6.f), c::white36, "ESP PRESET");
                int mt = match_e();
                const char* val = (mt < 0) ? "Custom" : enames[mt];
                ImVec2 vs = f::medium14->CalcTextSizeA(13.f, FLT_MAX, -1.f, val, 0, NULL);
                draw->AddText(f::medium14, 13.f, ImVec2(b1.x - 20.f - vs.x, cyb - 7.f), (mt < 0) ? c::white : c::primary, val);
                ImVec2 chv(b1.x - 13.f, cyb);
                draw->AddLine(chv + ImVec2(-3.f, -1.5f), chv + ImVec2(0.f, 1.5f), c::white48, 1.4f);
                draw->AddLine(chv + ImVec2(0.f, 1.5f), chv + ImVec2(3.f, -1.5f), c::white48, 1.4f);
            }

            // skeleton — lines only, no joint dots
            if (esps::skeleton)
            {
                ImColor sc = ImColor(esps::c_skel[0], esps::c_skel[1], esps::c_skel[2], esps::c_skel[3]);
                for (auto& bn : BONES)
                    draw->AddLine(centers[bn[0]], centers[bn[1]], sc, 1.2f);
            }

            // head dot — big hollow circle on the head bone (no dark outline)
            if (esps::head_dot)
            {
                ImColor hc = ImColor(esps::c_head[0], esps::c_head[1], esps::c_head[2], esps::c_head[3]);
                draw->AddCircle(centers[0], 11.f, hc, 32, 1.8f);
            }

            // filled box (chams-like translucent fill) — drawn under the box outline, bound to
            // the Filled Box colour so changing it updates live here.
            if (esps::filled)
            {
                ImColor fc = ImColor(esps::c_filled[0], esps::c_filled[1], esps::c_filled[2], esps::c_filled[3]);
                draw->AddRectFilled(cb0, cb1, fc, 2.f);
            }

            // box
            if (esps::box)
            {
                ImColor bc = ImColor(esps::c_box[0], esps::c_box[1], esps::c_box[2], esps::c_box[3]);
                if (!esps::dashed)
                    draw->AddRect(cb0, cb1, bc, 2.f, 0, 1.6f);
                else
                {
                    float L = 14.f;
                    ImVec2 tr(cb1.x, cb0.y), bl(cb0.x, cb1.y);
                    draw->AddLine(cb0, cb0 + ImVec2(L, 0), bc, 1.6f);  draw->AddLine(cb0, cb0 + ImVec2(0, L), bc, 1.6f);
                    draw->AddLine(tr, tr + ImVec2(-L, 0), bc, 1.6f);   draw->AddLine(tr, tr + ImVec2(0, L), bc, 1.6f);
                    draw->AddLine(bl, bl + ImVec2(L, 0), bc, 1.6f);    draw->AddLine(bl, bl + ImVec2(0, -L), bc, 1.6f);
                    draw->AddLine(cb1, cb1 + ImVec2(-L, 0), bc, 1.6f); draw->AddLine(cb1, cb1 + ImVec2(0, -L), bc, 1.6f);
                }
            }

            // ---------- element layout constants ----------
            const float boxw = cb1.x - cb0.x, boxh = cb1.y - cb0.y;
            const float GX = 9.f, BAR_TH = 5.f, BAR_STEP = BAR_TH + 4.f;

            auto nearest_of = [](ImVec2* pts, int cnt, ImVec2 mp) {
                int best = 0; float bd = FLT_MAX;
                for (int k = 0; k < cnt; k++) { ImVec2 d = pts[k] - mp; float dd = d.x * d.x + d.y * d.y; if (dd < bd) { bd = dd; best = k; } }
                return best;
            };

            // ---------- bars first (they push the texts): 0 bottom, 1 left, 2 right ----------
            ImVec2 BS[3] = { ImVec2(cx, cb1.y + 8.f + BAR_TH * 0.5f), ImVec2(cb0.x - GX, cc.y), ImVec2(cb1.x + GX, cc.y) };
            float bottom_bars = 0.f, left_w = 0.f, right_w = 0.f;   // accumulated bar extents per side
            float bstack[3] = { 0,0,0 };

            auto do_bar = [&](const char* id, int& slot, float col[4], float fill)
            {
                ImColor cf = ImColor(col[0], col[1], col[2], col[3]);
                bool vert = (slot != 0);
                ImVec2 b0, sz;
                if (!vert) { b0 = ImVec2(cx - boxw * 0.5f, cb1.y + 8.f + bstack[0]); sz = ImVec2(boxw, BAR_TH); }
                else { float sx = (slot == 1 ? cb0.x - GX : cb1.x + GX); float sgn = (slot == 1 ? -1.f : 1.f); b0 = ImVec2(sx - BAR_TH * 0.5f + sgn * bstack[slot], cc.y - boxh * 0.5f); sz = ImVec2(BAR_TH, boxh); }

                ImGui::SetCursorScreenPos(b0 - ImVec2(2.f, 2.f));
                ImGui::InvisibleButton(id, sz + ImVec2(4.f, 4.f));
                bool active = ImGui::IsItemActive();
                int eff = slot;
                if (active || ImGui::IsItemDeactivated())
                {
                    ImVec2 mp = ImGui::GetIO().MousePos;
                    int nearest = nearest_of(BS, 3, mp);
                    eff = nearest;
                    if (ImGui::IsItemDeactivated()) slot = nearest;
                    if (active)
                    {
                        for (int k = 0; k < 3; k++)   // bar-shaped slot glows
                        {
                            bool n = (k == nearest); ImU32 gl = a(c::primary, n ? 0.30f : 0.10f), go = a(c::primary, n ? 1.f : 0.5f);
                            if (k == 0) { ImVec2 g0(cx - boxw * 0.5f, cb1.y + 8.f); draw->AddRectFilled(g0, g0 + ImVec2(boxw, BAR_TH), gl, 2.f); draw->AddRect(g0, g0 + ImVec2(boxw, BAR_TH), go, 2.f); }
                            else { float sx = (k == 1 ? cb0.x - GX : cb1.x + GX); ImVec2 g0(sx - BAR_TH * 0.5f, cc.y - boxh * 0.5f); draw->AddRectFilled(g0, g0 + ImVec2(BAR_TH, boxh), gl, 2.f); draw->AddRect(g0, g0 + ImVec2(BAR_TH, boxh), go, 2.f); }
                        }
                        if (nearest == 0) { ImVec2 m0 = mp - ImVec2(boxw * 0.5f, BAR_TH * 0.5f); draw->AddRectFilled(m0, m0 + ImVec2(boxw, BAR_TH), ImColor(0, 0, 0, 160), 2.f); draw->AddRectFilled(m0, m0 + ImVec2(boxw * fill, BAR_TH), cf, 2.f); }
                        else { ImVec2 m0 = mp - ImVec2(BAR_TH * 0.5f, boxh * 0.5f); draw->AddRectFilled(m0, m0 + ImVec2(BAR_TH, boxh), ImColor(0, 0, 0, 160), 2.f); draw->AddRectFilled(m0 + ImVec2(0.f, boxh * (1.f - fill)), m0 + ImVec2(BAR_TH, boxh), cf, 2.f); }
                    }
                }
                if (eff == 0) bottom_bars += BAR_STEP; else if (eff == 1) left_w += BAR_STEP; else right_w += BAR_STEP;
                bstack[eff] += BAR_STEP;

                if (!active)   // parked render
                {
                    if (ImGui::IsItemHovered()) draw->AddRect(b0 - ImVec2(2.f, 2.f), b0 + sz + ImVec2(2.f, 2.f), a(c::primary, 0.4f), 3.f);
                    draw->AddRectFilled(b0, b0 + sz, ImColor(0, 0, 0, 160), 2.f);
                    if (!vert) draw->AddRectFilled(b0, b0 + ImVec2(sz.x * fill, sz.y), cf, 2.f);
                    else       draw->AddRectFilled(b0 + ImVec2(0.f, sz.y * (1.f - fill)), b0 + sz, cf, 2.f);
                }
            };
            if (esps::health) do_bar("##e_hp", esps::slot_health, esps::c_health, 0.72f);
            if (esps::armor)  do_bar("##e_ar", esps::slot_armor, esps::c_armor, 0.5f);

            // ---------- text slots (6): anchor + alignment; pushed out by side bars ----------
            // ha: -1 right-edge at p.x / +1 left-edge at p.x / 0 centred.  va: -1 above / +1 below / 0 centred.
            const float up_y = cb0.y + 18.f, lo_y = cb1.y - 18.f;   // symmetric upper/lower side rows
            struct { ImVec2 p; int ha, va; } TSA[6] = {
                { ImVec2(cx, cb0.y - 8.f),                       0, -1 },  // 0 head-top
                { ImVec2(cb0.x - 12.f - left_w, up_y),          -1,  0 },  // 1 upper-left
                { ImVec2(cb1.x + 12.f + right_w, up_y),         +1,  0 },  // 2 upper-right
                { ImVec2(cb0.x - 12.f - left_w, lo_y),          -1,  0 },  // 3 lower-left
                { ImVec2(cb1.x + 12.f + right_w, lo_y),         +1,  0 },  // 4 lower-right
                { ImVec2(cx, cb1.y + 8.f + bottom_bars + 8.f),  0, +1 },  // 5 bottom
            };
            float tstack[6] = { 0,0,0,0,0,0 };
            // top (0) and bottom (5) hold only ONE text each; sides may stack
            int text_occ[6] = { 0,0,0,0,0,0 };
            if (esps::name)     text_occ[esps::slot_name]++;
            if (esps::weapon)   text_occ[esps::slot_weapon]++;
            if (esps::distance) text_occ[esps::slot_distance]++;
            if (esps::show_id)  text_occ[esps::slot_id]++;
            auto text_tl = [&](int slot, ImVec2 ts, float stack) -> ImVec2 {
                auto& q = TSA[slot];
                float x = q.ha < 0 ? q.p.x - ts.x : (q.ha > 0 ? q.p.x : q.p.x - ts.x * 0.5f);
                float y = q.va < 0 ? q.p.y - ts.y : (q.va > 0 ? q.p.y : q.p.y - ts.y * 0.5f);
                return ImVec2(x, y + (q.va < 0 ? -stack : stack));
            };
            auto do_text = [&](const char* id, int& slot, const char* s, float col[4])
            {
                ImVec2 ts = f::medium14->CalcTextSizeA(14.f, FLT_MAX, -1.f, s, 0, NULL);
                ImVec2 tl = text_tl(slot, ts, tstack[slot]);
                tstack[slot] += ts.y + 3.f;
                ImGui::SetCursorScreenPos(tl - ImVec2(3.f, 2.f));
                ImGui::InvisibleButton(id, ts + ImVec2(6.f, 4.f));
                bool active = ImGui::IsItemActive();
                ImVec2 dp = tl;
                if (active || ImGui::IsItemDeactivated())
                {
                    ImVec2 mp = ImGui::GetIO().MousePos;
                    // top/bottom are single-slot: forbid if already taken by another text
                    auto ok = [&](int k) { return (k != 0 && k != 5) || (text_occ[k] - (slot == k ? 1 : 0)) == 0; };
                    int nearest = slot; float best = FLT_MAX;
                    for (int k = 0; k < 6; k++) { if (!ok(k)) continue; ImVec2 g = text_tl(k, ts, 0.f) + ts * 0.5f; ImVec2 d = g - mp; float dd = d.x * d.x + d.y * d.y; if (dd < best) { best = dd; nearest = k; } }
                    if (active)
                    {
                        dp = mp - ts * 0.5f;
                        for (int k = 0; k < 6; k++) { if (!ok(k)) continue; ImVec2 g = text_tl(k, ts, 0.f); bool n = (k == nearest); draw->AddRectFilled(g - ImVec2(4.f, 3.f), g + ts + ImVec2(4.f, 3.f), a(c::primary, n ? 0.30f : 0.10f), 3.f); draw->AddRect(g - ImVec2(4.f, 3.f), g + ts + ImVec2(4.f, 3.f), a(c::primary, n ? 1.f : 0.45f), 3.f, 0, n ? 1.7f : 1.f); }
                    }
                    else slot = nearest;
                }
                else if (ImGui::IsItemHovered())
                    draw->AddRectFilled(tl - ImVec2(3.f, 2.f), tl + ts + ImVec2(3.f, 2.f), a(c::primary, 0.18f), 3.f);
                draw->AddText(f::medium14, 14.f, dp + ImVec2(1.f, 1.f), ImColor(0, 0, 0, 200), s);
                draw->AddText(f::medium14, 14.f, dp, ImColor(col[0], col[1], col[2], col[3]), s);
            };
            if (esps::name)     do_text("##e_name", esps::slot_name, "Ryzex", esps::c_name);
            if (esps::weapon)   do_text("##e_wep",  esps::slot_weapon, "Pistol", esps::c_wep);
            if (esps::distance) do_text("##e_dist", esps::slot_distance, "42m", esps::c_dist);
            if (esps::show_id)  do_text("##e_id",   esps::slot_id, "#123", esps::c_id);
        }

        // border
        draw->AddRect(panel, panel + ImVec2(ESP_W, PANEL_H), c::white6, 16.f);

        // open/close fade — the game background is black, so fading to black hides it
        if (prev_a < 0.999f)
            draw->AddRectFilled(panel, panel + ImVec2(ESP_W, PANEL_H), ImColor(0.f, 0.f, 0.f, 1.f - prev_a), 16.f);

        ::End();
    }

    // true only once the intro splash has finished (so INS can't skip it)
    bool splash_finished() override { return splash_done; }

    // full-screen translucent black behind the menu; on the actual menu it also carries
    // a gentle, clearly-visible drifting glow animation (the splash stays clean).
    void draw_backdrop()
    {
        auto dl = ::GetBackgroundDrawList();
        const float W = 1920.f, H = 1080.f;
        const float t = (float)ImGui::GetTime();
        const bool  menu = splash_done;                 // true once the real menu is up

        // transparent black wash — a touch darker once the menu is open
        // (the drifting backdrop starfield was removed; the preview keeps its own dots)
        dl->AddRectFilled(ImVec2(0.f, 0.f), ImVec2(W, H), ImColor(0.f, 0.f, 0.f, menu ? 0.82f : 0.70f));
        (void)t;
    }

    void draw() override
    {
        anim::update_animation_speed();
        settings::apply_accent();   // recolor the menu from the accent picker

        if (!splash_done)           // classy intro before the menu appears
        {
            // Transparent intro: NO backdrop dim, so FiveM stays visible behind the
            // centered logo (only the mark + ring + wordmark are drawn on top).
            begin("General");       // keep Begin/End balanced (end() runs in the loop)
            draw_splash();
            return;
        }

        // The Unload button raises unload_prompt -> open the confirm / exiting splash.
        if (MGR->unload_prompt) { MGR->unload_prompt = false; if (unload_state == 0) { unload_state = 1; unload_t = 0.f; } }

        draw_backdrop();            // dim behind the menu (and the unload splash)

        if (unload_state != 0)      // the unload splash fully replaces the menu
        {
            begin("General");       // keep Begin/End balanced (end() runs in the loop)
            draw_unload();
            return;
        }

        begin("General");
        {
            background();
            if (side_mode())
            {
                side_rail();   // left rail: tabs + accordion subtabs
            }
            else
            {
                topbar();      // sets active_tab_cx
                subtab_bar();  // floating strip above the active tab
            }
            content();     // feature pages
            esp_preview(); // separate draggable window (own Begin/End)
        }
        // watermark() is drawn from the render loop every frame (see Cheat.cpp),
        // so it stays visible even when the menu is closed.
        //call end() in render loop
    }

    // classy startup / loading splash — plays once, then reveals the menu.
    // Centred Quantix eye-logo with a circular loading ring sweeping around it,
    // brand + "PRIVATE" tagline underneath. Clean, modern, single splash.
    void draw_splash()
    {
        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime; if (dt > 0.05f) dt = 0.05f;   // clamp startup hitches
        splash_t += dt;

        const float CX = 1920.f * 0.5f, CY = 1080.f * 0.44f;   // logo centre (room for text below)
        auto dl = ::GetForegroundDrawList();

        auto cl   = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };
        auto eout = [](float t) { float u = 1.f - t; return 1.f - u * u * u; };   // easeOutCubic
        const float PI2 = 6.2831853f;

        const float FADE_T0 = 3.1f, FADE_DUR = 0.5f, END_T = FADE_T0 + FADE_DUR;
        float fade = 1.f - cl((splash_t - FADE_T0) / FADE_DUR);

        // NOTE: no full-screen cover here — the intro is transparent so the game shows
        // through; `fade` only drives the logo / ring / wordmark alpha below.

        ImVec2 lc(CX, CY);

        // ---- logo: fade + gentle scale-in, with a soft glow ----
        float lp = eout(cl(splash_t / 0.6f));
        float logoA = lp * fade;
        float scl = 0.92f + 0.08f * lp;
        const float BASE = 168.f;                             // base logo size
        float LH = BASE * scl, LW = LH * 0.956f;              // near-square content aspect
        float pulse = 0.5f + 0.5f * sinf(splash_t * 1.7f);
        dl->AddCircleFilled(lc, BASE * 0.52f * scl, a(c::white, (0.04f + 0.04f * pulse) * logoA), 48);
        if (tex::brand_logo)
            dl->AddImage((ImTextureID)tex::brand_logo,
                         ImVec2(CX - LW * 0.5f, CY - LH * 0.5f), ImVec2(CX + LW * 0.5f, CY + LH * 0.5f),
                         ImVec2(0.240f, 0.232f), ImVec2(0.760f, 0.776f), a(c::white, logoA));

        // ---- circular loading ring around the logo ----
        float ringA = fade * eout(cl((splash_t - 0.3f) / 0.5f));
        const float R = BASE * 0.5f + 34.f;
        dl->AddCircle(lc, R, a(c::white, 0.09f * ringA), 96, 2.f);        // faint track
        float head = fmodf(splash_t * 2.5f, PI2);
        const float sweep = 1.95f;
        dl->PathArcTo(lc, R, head, head + sweep, 48);
        dl->PathStroke(a(c::white, 0.92f * ringA), 0, 3.f);
        ImVec2 hp(lc.x + cosf(head + sweep) * R, lc.y + sinf(head + sweep) * R);
        dl->AddCircleFilled(hp, 3.4f, a(c::white, ringA));               // leading dot
        float h2 = fmodf(-splash_t * 1.5f, PI2);                          // subtle inner counter-tick
        dl->PathArcTo(lc, R - 9.f, h2, h2 + 0.6f, 20);
        dl->PathStroke(a(c::white, 0.24f * ringA), 0, 1.6f);

        // ---- centred, letter-spaced text helper ----
        auto drawSpaced = [&](ImFont* fnt, float fs, const char* s, float trk, float cy, ImU32 col)
        {
            float total = 0.f; char cc[2] = { 0, 0 };
            for (const char* p = s; *p; ++p) { cc[0] = *p; total += fnt->CalcTextSizeA(fs, FLT_MAX, -1.f, cc, 0, 0).x + trk; }
            if (total > 0.f) total -= trk;
            float x = CX - total * 0.5f;
            for (const char* p = s; *p; ++p) { cc[0] = *p; float w = fnt->CalcTextSizeA(fs, FLT_MAX, -1.f, cc, 0, 0).x; dl->AddText(fnt, fs, ImVec2(x, cy - fs * 0.5f), col, cc); x += w + trk; }
        };

        // ---- brand + tagline below the ring ----
        float brandY = CY + R + 42.f;
        float bp = eout(cl((splash_t - 0.35f) / 0.5f));
        drawSpaced(f::logo72,  30.f, "QUANTIX", 6.f, brandY + (1.f - bp) * 8.f, a(c::white,   bp * fade));
        float tp = eout(cl((splash_t - 0.55f) / 0.5f));
        drawSpaced(f::medium14, 12.f, "PRIVATE", 6.f, brandY + 28.f,            a(c::white48, tp * fade));

        if (splash_t >= END_T) splash_done = true;
    }

    // Unload confirmation / exiting splash. Opened by the Settings "Unload" button
    // (MGR->unload_prompt). Drawn on the foreground list over the dimmed backdrop; the
    // menu itself is hidden while it's up. State 1 = confirm (Cancel / Unload buttons),
    // state 2 = "Exiting..." spinner, then close_requested quits the app.
    void draw_unload()
    {
        ImGuiIO& io = ImGui::GetIO();
        float dt = io.DeltaTime; if (dt > 0.05f) dt = 0.05f;
        unload_t += dt;

        auto dl = ::GetForegroundDrawList();
        auto cl   = [](float x) { return x < 0.f ? 0.f : (x > 1.f ? 1.f : x); };
        auto eout = [](float t) { float u = 1.f - t; return 1.f - u * u * u; };
        const float CX = 1920.f * 0.5f, CY = 1080.f * 0.5f;
        const float PI2 = 6.2831853f;

        float ent = eout(cl(unload_t / 0.28f));   // entrance ease
        const float A = ent;                       // master alpha

        // ---- card body ----
        const float CW = 344.f, CH = 214.f;
        float rise = (1.f - ent) * 14.f;
        ImVec2 c0(CX - CW * 0.5f, CY - CH * 0.5f + rise);
        ImVec2 c1(CX + CW * 0.5f, CY + CH * 0.5f + rise);
        float cx = (c0.x + c1.x) * 0.5f;

        dl->AddShadowRect(c0, c1, a(c::black, 0.45f * A), 60.f, ImVec2(0.f, 10.f), 0, 18.f);
        int v0 = dl->VtxBuffer.Size;
        dl->AddRectFilled(c0, c1, IM_COL32_WHITE, 18.f);
        ShadeVertsLinearGradY(dl, v0, dl->VtxBuffer.Size, c0, c1, a(ImColor(24, 24, 28, 255), A), a(ImColor(15, 15, 18, 255), A));
        dl->AddRect(c0, c1, a(c::white, 0.08f * A), 18.f, 0, 1.f);

        if (unload_state == 1)   // ---------- confirm ----------
        {
            // brand mark near the top
            ImVec2 logoC(cx, c0.y + 46.f);
            if (tex::main_logo)
                dl->AddImage((ImTextureID)tex::main_logo, logoC - ImVec2(24.f, 24.f), logoC + ImVec2(24.f, 24.f),
                             ImVec2(0.10f, 0.10f), ImVec2(0.90f, 0.90f), a(c::primary, A));

            const char* title = "Unload Quantix?";
            ImVec2 ts = f::sbold16->CalcTextSizeA(20.f, FLT_MAX, -1.f, title, 0, NULL);
            dl->AddText(f::sbold16, 20.f, ImVec2(cx - ts.x * 0.5f, c0.y + 80.f), a(c::white, A), title);
            const char* sub = "Are you sure you want to unload the menu?";
            ImVec2 ss = f::medium12->CalcTextSizeA(13.f, FLT_MAX, -1.f, sub, 0, NULL);
            dl->AddText(f::medium12, 13.f, ImVec2(cx - ss.x * 0.5f, c0.y + 106.f), a(c::white, 0.5f * A), sub);

            const float BW = 142.f, BH = 40.f, GAP = 14.f;
            float by = c1.y - 24.f - BH;
            ImVec2 cn0(cx - BW - GAP * 0.5f, by), cn1(cn0.x + BW, by + BH);
            ImVec2 un0(cx + GAP * 0.5f, by),       un1(un0.x + BW, by + BH);

            ImVec2 mp = io.MousePos;
            bool click = ImGui::IsMouseClicked(0);
            auto in = [&](ImVec2 b0, ImVec2 b1) { return mp.x >= b0.x && mp.x <= b1.x && mp.y >= b0.y && mp.y <= b1.y; };

            bool hC = in(cn0, cn1);
            dl->AddRectFilled(cn0, cn1, a(c::white, (hC ? 0.12f : 0.06f) * A), 9.f);
            dl->AddRect(cn0, cn1, a(c::white, 0.14f * A), 9.f, 0, 1.f);
            { const char* t = "Cancel"; ImVec2 s = f::medium14->CalcTextSizeA(15.f, FLT_MAX, -1.f, t, 0, NULL);
              dl->AddText(f::medium14, 15.f, ImVec2((cn0.x + cn1.x - s.x) * 0.5f, (cn0.y + cn1.y - s.y) * 0.5f), a(c::white, (hC ? 1.f : 0.8f) * A), t); }

            bool hU = in(un0, un1);
            const ImColor rd(232, 58, 58, 255);   // danger red so "Destruct" clearly reads as destructive
            dl->AddRectFilled(un0, un1, a(rd, (hU ? 0.45f : 0.28f) * A), 9.f);
            dl->AddRect(un0, un1, a(rd, (hU ? 1.f : 0.70f) * A), 9.f, 0, 1.3f);
            { const char* t = "Destruct"; ImVec2 s = f::medium14->CalcTextSizeA(15.f, FLT_MAX, -1.f, t, 0, NULL);
              dl->AddText(f::medium14, 15.f, ImVec2((un0.x + un1.x - s.x) * 0.5f, (un0.y + un1.y - s.y) * 0.5f), a(c::white, A), t); }

            if (click && hC) { unload_state = 0; unload_t = 0.f; }
            if (click && hU) { unload_state = 2; unload_t = 0.f; }
        }
        else                     // ---------- exiting ----------
        {
            // vertically-centred group: spinner ring + logo, then title, then subtitle
            ImVec2 logoC(cx, c0.y + 78.f);
            const float R = 34.f;
            dl->AddCircle(logoC, R, a(c::white, 0.10f * A), 64, 2.f);
            float head = fmodf(unload_t * 4.5f, PI2);
            dl->PathArcTo(logoC, R, head, head + 1.9f, 40);
            dl->PathStroke(a(c::primary, 0.95f * A), 0, 3.f);
            if (tex::main_logo)
                dl->AddImage((ImTextureID)tex::main_logo, logoC - ImVec2(21.f, 21.f), logoC + ImVec2(21.f, 21.f),
                             ImVec2(0.10f, 0.10f), ImVec2(0.90f, 0.90f), a(c::primary, A));

            const char* title = "Exiting...";
            ImVec2 ts = f::sbold16->CalcTextSizeA(20.f, FLT_MAX, -1.f, title, 0, NULL);
            dl->AddText(f::sbold16, 20.f, ImVec2(cx - ts.x * 0.5f, c0.y + 130.f), a(c::white, A), title);
            const char* sub = "Unloading Quantix, goodbye.";
            ImVec2 ss = f::medium12->CalcTextSizeA(13.f, FLT_MAX, -1.f, sub, 0, NULL);
            dl->AddText(f::medium12, 13.f, ImVec2(cx - ss.x * 0.5f, c0.y + 156.f), a(c::white, 0.5f * A), sub);

            if (unload_t >= 1.2f)
                MGR->close_requested = true;   // render loop reads this and quits
        }
    }

    // small, clean, modern watermark — a compact two-row card (nothing like the old wide capsule):
    //   (logo)  Quantix              │  CPU 12%
    //          <user> · 3 Months     │  GPU 23%
    void watermark() override
    {
        if (!settings::watermark || !splash_done)   // hidden during the intro splash
            return;
        auto dl = ::GetForegroundDrawList();

        sysstat::g_stats.update();
        char cpuStr[8], gpuStr[8];
        snprintf(cpuStr, sizeof cpuStr, "%d%%", (int)(sysstat::g_stats.cpu + 0.5f));
        snprintf(gpuStr, sizeof gpuStr, "%02d%%", (int)(sysstat::g_stats.gpu + 0.5f));
        const char* brand = "Quantix";
        const char* user  = sysstat::username();
        const char* sub   = "3 Months";

        auto mw = [](ImFont* fnt, float fs, const char* s)
        { return fnt->CalcTextSizeA(fs, FLT_MAX, -1.f, s, 0, NULL).x; };

        const float fsBrand = 14.f, fsSub = 11.f, fsLbl = 10.5f, fsVal = 12.f;

        // sub row = "<user> · 3 Months" (dot drawn, not a font glyph)
        const float dotGap = 6.f, dotR = 1.5f;
        float wUser = mw(f::medium12, fsSub, user);
        float wSub  = mw(f::medium12, fsSub, sub);
        float subRowW = wUser + dotGap + dotR * 2.f + dotGap + wSub;

        float wBrand   = mw(f::sbold16, fsBrand, brand);
        float leftColW = ImMax(wBrand, subRowW);

        float wLbl = ImMax(mw(f::medium12, fsLbl, "CPU"), mw(f::medium12, fsLbl, "GPU"));
        float wVal = ImMax(mw(f::sbold16, fsVal, cpuStr), mw(f::sbold16, fsVal, gpuStr));
        const float lblValGap = 8.f;
        float rightColW = wLbl + lblValGap + wVal;

        // small near-square logo (cropped to its content bbox)
        const float LS = 26.f, LW = LS * 0.956f;
        const ImVec2 uv0(0.240f, 0.232f), uv1(0.760f, 0.776f);

        const float PADX = 13.f, gapLogo = 11.f, gapBD = 15.f, divW = 1.f, gapDB = 15.f;
        const float H = 46.f;
        float bw = PADX + LW + gapLogo + leftColW + gapBD + divW + gapDB + rightColW + PADX;

        ImVec2 b0(18.f, 18.f), b1(b0.x + bw, b0.y + H);
        float cy = (b0.y + b1.y) * 0.5f;
        const float rnd = 10.f;

        // soft shadow + clean glassy body (subtle gradient, hairline border, thin top highlight)
        dl->AddShadowRect(b0, b1, a(c::black, 0.45f), 26.f, ImVec2(0.f, 4.f), 0, rnd);
        int v0 = dl->VtxBuffer.Size;
        dl->AddRectFilled(b0, b1, IM_COL32_WHITE, rnd);
        ShadeVertsLinearGradY(dl, v0, dl->VtxBuffer.Size, b0, b1,
                              ImColor(26, 27, 34, 240), ImColor(17, 18, 23, 240));
        dl->AddLine(ImVec2(b0.x + rnd, b0.y + 1.f), ImVec2(b1.x - rnd, b0.y + 1.f), a(c::white, 0.06f), 1.f);

        const float PADY = 8.f;
        float rowH = (H - PADY * 2.f) * 0.5f;
        float r1 = b0.y + PADY + rowH * 0.5f;   // top row centre
        float r2 = b0.y + PADY + rowH + rowH * 0.5f;   // bottom row centre

        float x = b0.x + PADX;

        // logo with a faint accent glow
        if (tex::brand_logo)
        {
            dl->AddCircleFilled(ImVec2(x + LW * 0.5f, cy), LS * 0.62f, a(c::primary, 0.08f), 32);
            dl->AddImage((ImTextureID)tex::brand_logo, ImVec2(x, cy - LS * 0.5f), ImVec2(x + LW, cy + LS * 0.5f),
                         uv0, uv1, IM_COL32(255, 255, 255, 255));
        }
        x += LW + gapLogo;

        // left column: brand (top) + "<user> · 3 Months" (bottom)
        float hBrand = f::sbold16->CalcTextSizeA(fsBrand, FLT_MAX, -1.f, brand, 0, NULL).y;
        float hSub   = f::medium12->CalcTextSizeA(fsSub, FLT_MAX, -1.f, user, 0, NULL).y;
        dl->AddText(f::sbold16, fsBrand, ImVec2(x, r1 - hBrand * 0.5f), c::white, brand);
        {
            float sx = x;
            dl->AddText(f::medium12, fsSub, ImVec2(sx, r2 - hSub * 0.5f), a(c::white, 0.42f), user);
            sx += wUser + dotGap;
            dl->AddCircleFilled(ImVec2(sx + dotR, r2), dotR, a(c::white, 0.32f), 8);
            sx += dotR * 2.f + dotGap;
            dl->AddText(f::medium12, fsSub, ImVec2(sx, r2 - hSub * 0.5f), a(c::white, 0.42f), sub);
        }
        x += leftColW + gapBD;

        // thin vertical divider
        float divH = H * 0.5f;
        dl->AddRectFilled(ImVec2(x, cy - divH * 0.5f), ImVec2(x + divW, cy + divH * 0.5f), a(c::white, 0.10f));
        x += divW + gapDB;

        // right column: CPU (top) + GPU (bottom); labels left, accent values right-aligned
        float rightEdge = b1.x - PADX;
        float hLbl = f::medium12->CalcTextSizeA(fsLbl, FLT_MAX, -1.f, "CPU", 0, NULL).y;
        float hVal = f::sbold16->CalcTextSizeA(fsVal, FLT_MAX, -1.f, cpuStr, 0, NULL).y;
        auto statRow = [&](const char* lbl, const char* val, float ry)
        {
            dl->AddText(f::medium12, fsLbl, ImVec2(x, ry - hLbl * 0.5f), c::white36, lbl);
            float vw = mw(f::sbold16, fsVal, val);
            dl->AddText(f::sbold16, fsVal, ImVec2(rightEdge - vw, ry - hVal * 0.5f), c::primary, val);
        };
        statRow("CPU", cpuStr, r1);
        statRow("GPU", gpuStr, r2);
    }

    bool begin(const char* name) override
    {
        // Don't draw black background - let FiveM show through
        // ::GetBackgroundDrawList()->AddRectFilled(ImVec2(0, 0), ImVec2(1920, 1080), ImColor(0, 0, 0, 255));

        // +2 px so the panels (drawn at a +1,+1 offset) are not clipped at the
        // bottom/right, which would notch the rounded corners.
        ::SetNextWindowSize(ImVec2(win_w() + 2.f, MENU_H + 2.f));
        // Centre menu first time, then allow dragging
        ::SetNextWindowPos(ImVec2(1920.f * 0.5f, 1080.f * 0.5f), ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        // NoMove: we drive the position ourselves so we can add a smooth, weighted follow.
        bool ret = ::Begin(name, nullptr, ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoBackground | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoMove);

        // ---- weighted / eased window dragging ----------------------------------------
        // Grab from the top bar strip; the window then trails the cursor with a soft lag
        // instead of snapping to it, giving a smooth "heavy" motion. Settles on release.
        {
            ImGuiIO& io = ImGui::GetIO();
            ImVec2 cur  = ImGui::GetWindowPos();
            ImVec2 wsz  = ImGui::GetWindowSize();
            if (!drag_init) { drag_smooth = cur; drag_init = true; }

            // drag handle: the top strip (empty parts; buttons/tab rows won't grab).
            // sidebar mode -> the brand-header band; topbar mode -> the tab bar.
            float handleH = side_mode() ? 60.f : (TOPBAR_Y + TOPBAR_H);
            ImRect handle(cur, ImVec2(cur.x + wsz.x, cur.y + handleH));
            bool overHandle = handle.Contains(io.MousePos);

            if (splash_done && unload_state == 0 && ImGui::IsMouseClicked(0) && overHandle &&
                !ImGui::IsAnyItemHovered() && !ImGui::IsAnyItemActive())
            {
                drag_active = true;
                drag_grab = ImVec2(io.MousePos.x - cur.x, io.MousePos.y - cur.y);
            }
            if (!ImGui::IsMouseDown(0)) drag_active = false;

            // target = where the cursor wants the window; when idle, target = current (no drift)
            ImVec2 target = cur;
            if (drag_active) target = ImVec2(io.MousePos.x - drag_grab.x, io.MousePos.y - drag_grab.y);

            // frame-rate-independent easing; ~0.20/frame @60fps -> visibly weighted but responsive
            float k = 1.f - powf(1.f - 0.20f, io.DeltaTime * 60.f);
            if (k < 0.f) k = 0.f; if (k > 1.f) k = 1.f;
            drag_smooth.x += (target.x - drag_smooth.x) * k;
            drag_smooth.y += (target.y - drag_smooth.y) * k;

            ImGui::SetWindowPos(drag_smooth);
        }
        return ret;
    }

    void background() override
    {
        auto draw = ::GetWindowDrawList();
        const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);

        if (side_mode())
        {
            // one rounded card for the whole window (content surface)
            draw->AddRectFilled(p, p + ImVec2(SIDE_MENU_W, MENU_H), c::bg, 16.f);
            // left rail: subtly darker vertical gradient, left corners rounded
            int v0 = draw->VtxBuffer.Size;
            draw->AddRectFilled(p, p + ImVec2(SIDEBAR_W, MENU_H), IM_COL32_WHITE, 16.f, ImDrawFlags_RoundCornersLeft);
            // neutral grey (equal R=G=B, no blue tint) so it matches c::bg (18,18,18);
            // a hair lighter at the top, darker at the bottom for subtle depth only.
            ShadeVertsLinearGradY(draw, v0, draw->VtxBuffer.Size, p, p + ImVec2(0.f, MENU_H),
                                  ImColor(21, 21, 21, 255), ImColor(16, 16, 16, 255));
            // divider between rail and content
            draw->AddLine(p + ImVec2(SIDEBAR_W, 10.f), p + ImVec2(SIDEBAR_W, MENU_H - 10.f), c::white6, 1.f);
        }
        else
        {
            // top bar
            draw->AddRectFilled(p + ImVec2(0.f, TOPBAR_Y), p + ImVec2(MENU_W, TOPBAR_Y + TOPBAR_H), c::bg, 14.f);
            // content panel
            draw->AddRectFilled(p + ImVec2(0.f, CONTENT_Y), p + ImVec2(MENU_W, MENU_H), c::bg, 16.f);
        }
    }

    void end() override
    {
        if (splash_done && unload_state == 0)   // panels aren't drawn during intro / unload splash
        {
            auto draw = ::GetWindowDrawList();
            const auto& p = ::GetWindowPos() + ImVec2(1.f, 1.f);

            if (side_mode())
            {
                draw->AddRect(p, p + ImVec2(SIDE_MENU_W, MENU_H), c::white6, 16.f);
            }
            else
            {
                draw->AddRect(p + ImVec2(0.f, TOPBAR_Y), p + ImVec2(MENU_W, TOPBAR_Y + TOPBAR_H), c::white6, 14.f);
                draw->AddRect(p + ImVec2(0.f, CONTENT_Y), p + ImVec2(MENU_W, MENU_H), c::white6, 16.f);
            }
        }

        ::End();
    }

    void destroy() override
    {
        delete this;
    }
private:
    int   screen = 0;
    float splash_t = 0.f;       // startup splash timer
    bool  splash_done = false;  // splash finished -> show menu
    int   unload_state = 0;     // unload splash: 0=none, 1=confirm, 2=exiting
    float unload_t = 0.f;       // unload splash timer / animation clock
    float prev_a = 0.f;         // esp-preview open/close animation
    int   disp_mode = 1;        // preview mode being displayed (kept during fade-out)
    float active_tab_cx = 0.f;
    bool  bar_init = false;
    float page_anim = 0.f;
    int   cur_page_id = -1;
    // weighted window-drag state
    bool   drag_init = false;
    bool   drag_active = false;
    ImVec2 drag_smooth = ImVec2(0.f, 0.f);
    ImVec2 drag_grab = ImVec2(0.f, 0.f);
    ID3D11Device* g_pd3dDevice = NULL;
};

c_menu* create_menu()
{
    return new _c_menu();
}
