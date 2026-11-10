// Include premium menu GUI FIRST (before other includes)
#include <fstream>
#include <ctime>
#include <iomanip>
#include <sstream>
#include "../../../framework/headers/includes.h"
#include "../../../framework/headers/functions.h"
#include "../../../framework/headers/sections.h"
#include "../../../framework/settings/variables.h"
#include "../../../framework/settings/elements.h"
#include "../../../framework/settings/colors.h"
#include "../../../framework/headers/font.h"
#include "../../../framework/data/fonts.h"
#include "../../../framework/data/images.h"
#include "../../../framework/image_loaders/image_dx11.h"

#include "Interface.hpp"
#include "../Main/Options.hpp"
#include "Main/xorstring.h"
#include <thread>
#include "kaanlar.h"
#include <chrono>
#include <string>
#include <sstream>
#include <wincrypt.h>
#include "Main/FivemSDK/Fivem.hpp"
#include "Main/FivemSDK/GTADefines.hpp"
#include "Main/Features/player.hpp"
#include <filesystem>
#include <vector>

M_base::Interface interfaceObject;

#pragma comment(lib, "crypt32.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

bool LeftMouseDown = false;
ImVec2 LastMousePos = ImVec2(0, 0);
ImRect WindowRect = ImRect();

using namespace ImGui;

// Data
static ID3D11Device* g_pd3dDevice = nullptr;
static ID3D11DeviceContext* g_pd3dDeviceContext = nullptr;
static IDXGISwapChain* g_pSwapChain = nullptr;
static bool g_SwapChainOccluded = false;
static UINT g_ResizeWidth = 0, g_ResizeHeight = 0;
static ID3D11RenderTargetView* g_mainRenderTargetView = nullptr;
static bool g_FontsInitialized = false;
static bool g_ImagesInitialized = false;

DWORD picker_flags = ImGuiColorEditFlags_NoSidePreview | ImGuiColorEditFlags_AlphaBar | ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_AlphaPreview | ImGuiColorEditFlags_DisplayHex;

namespace M_base {

void Interface::Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext)
{
    hWindow = Window;
    hTargetWindow = TargetWindow;
    IDevice = Device;
    g_pd3dDevice = Device;
    g_pd3dDeviceContext = DeviceContext;

    // Fonts are already initialized in Cheat.cpp before Interface is created
    // Images are also initialized in Cheat.cpp
    g_FontsInitialized = true;
    g_ImagesInitialized = true;
    
    // Set screen size for the menu
    var->window.screen_size = ImVec2((float)GetSystemMetrics(SM_CXSCREEN), (float)GetSystemMetrics(SM_CYSCREEN));

    bIsMenuOpen = true;
}

// Debug log for Interface
static std::ofstream g_InterfaceLog;
void InterfaceLog(const char* msg)
{
    if (!g_InterfaceLog.is_open())
        g_InterfaceLog.open("interface_debug.log", std::ios::out | std::ios::trunc);
    if (g_InterfaceLog.is_open())
    {
        g_InterfaceLog << msg << std::endl;
        g_InterfaceLog.flush();
    }
}

void Interface::RenderGui()
{
    static int frameNum = 0;
    frameNum++;
    
    // IMPORTANT: Process keybinds EVERY frame, even when menu is closed
    // This ensures toggle keybinds (Freecam, Godmode, etc.) work when menu is hidden
    try {
        if (keybinds) {
            keybinds->process_binds();
        }
    } catch (...) {}

    static bool injectionUiDone = false;
    static bool injectionUiStarted = false;
    static std::chrono::steady_clock::time_point injectionUiStartTime;

    if (!injectionUiDone)
    {
        if (!injectionUiStarted) {
            injectionUiStarted = true;
            injectionUiStartTime = std::chrono::steady_clock::now();
        }

        constexpr float totalSeconds = 15.0f;
        constexpr float fadeInSeconds = 0.35f;

        const auto now = std::chrono::steady_clock::now();
        float elapsed = std::chrono::duration<float>(now - injectionUiStartTime).count();
        if (elapsed < 0.0f) elapsed = 0.0f;

        float progress = elapsed / totalSeconds;
        if (progress < 0.0f) progress = 0.0f;
        if (progress > 1.0f) progress = 1.0f;

        float alpha = 1.0f;
        if (elapsed < fadeInSeconds)
            alpha = elapsed / fadeInSeconds;
        if (alpha < 0.0f) alpha = 0.0f;
        if (alpha > 1.0f) alpha = 1.0f;

        const float uiScale = (var ? var->gui.stored_dpi : 1.0f);
        ImGuiIO& io = ImGui::GetIO();
        const ImVec2 screen = io.DisplaySize;

        ImDrawList* drawList = ImGui::GetForegroundDrawList();
        if (drawList)
            drawList->AddRectFilled(ImVec2(0, 0), screen, ImColor(10, 10, 10, (int)(230.0f * alpha)));

        ImGui::PushStyleVar(ImGuiStyleVar_Alpha, alpha);
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.f, 0.f, 0.f, 0.f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.f, 0.f));
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(screen);

        const ImGuiWindowFlags flags =
            ImGuiWindowFlags_NoDecoration |
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoSavedSettings |
            ImGuiWindowFlags_NoBringToFrontOnFocus |
            ImGuiWindowFlags_NoInputs;

        if (ImGui::Begin("##injection_loading", nullptr, flags))
        {
            const char* logoName = "logo_bg_removed_white";
            ImTextureID logoTex = 0;
            ImVec2 logoDrawSize(0.f, 0.f);
            try {
                logoTex = image->get(logoName);
                const ImVec2 logoReal = image->get_real_size(logoName);
                const float maxLogoW = screen.x * 0.35f;
                const float maxLogoH = screen.y * 0.18f;
                float s = 1.0f;
                if (logoReal.x > 0.f && logoReal.y > 0.f) {
                    const float sw = maxLogoW / logoReal.x;
                    const float sh = maxLogoH / logoReal.y;
                    s = (sw < sh ? sw : sh);
                    if (s > 1.0f) s = 1.0f;
                    if (s < 0.0f) s = 0.0f;
                }
                logoDrawSize = ImVec2(logoReal.x * s, logoReal.y * s);
            } catch (...) {
                logoTex = 0;
            }

            const float topPadding = 30.0f * uiScale;
            if (logoTex && logoDrawSize.x > 0.f && logoDrawSize.y > 0.f)
            {
                ImGui::SetCursorPos(ImVec2((screen.x - logoDrawSize.x) * 0.5f, topPadding));
                ImGui::Image(logoTex, logoDrawSize);
            }

            const char* loadingText = "Loading";
            ImFont* loadingFont = nullptr;
            try {
                loadingFont = (font ? font->get("semibold14") : nullptr);
            } catch (...) {
                loadingFont = nullptr;
            }
            if (loadingFont)
                ImGui::PushFont(loadingFont);

            const ImVec2 textSize = ImGui::CalcTextSize(loadingText);
            float contentStartY = topPadding + (logoDrawSize.y > 0.f ? (logoDrawSize.y + 70.0f * uiScale) : (screen.y * 0.35f));
            if (contentStartY > screen.y * 0.65f) contentStartY = screen.y * 0.65f;

            ImGui::SetCursorPos(ImVec2((screen.x - textSize.x) * 0.5f, contentStartY));
            ImGui::TextUnformatted(loadingText);

            if (loadingFont)
                ImGui::PopFont();

            const float barWidth = (screen.x * 0.35f < 420.0f * uiScale ? screen.x * 0.35f : 420.0f * uiScale);
            const float barHeight = 10.0f * uiScale;
            const float barY = contentStartY + textSize.y + 14.0f * uiScale;

            ImGui::SetCursorPos(ImVec2((screen.x - barWidth) * 0.5f, barY));
            ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(1.f, 1.f, 1.f, 0.08f * alpha));
            ImGui::PushStyleColor(ImGuiCol_PlotHistogram, ImVec4(1.f, 1.f, 1.f, 0.85f * alpha));
            ImGui::ProgressBar(progress, ImVec2(barWidth, barHeight), "");
            ImGui::PopStyleColor(2);
        }
        ImGui::End();

        ImGui::PopStyleVar();
        ImGui::PopStyleColor(2);
        ImGui::PopStyleVar();

        if (elapsed >= totalSeconds) {
            injectionUiDone = true;
            bIsMenuOpen = true;
        }

        return;
    }
    
    if (!bIsMenuOpen)
        return;
    
    // Debug: check all pointers
    if (frameNum <= 5) {
        char buf[256];
        sprintf(buf, "Frame %d: gui=%p var=%p elements=%p col=%p font=%p", 
            frameNum, (void*)gui.get(), (void*)var.get(), (void*)elements.get(), (void*)col.get(), (void*)font.get());
        InterfaceLog(buf);
    }
    
    // Try premium menu gui->render() first, fallback to simple menu on crash
    static bool guiCrashed = false;
    
    if (!guiCrashed) {
        if (!gui) { InterfaceLog("gui is NULL!"); }
        else if (!var) { InterfaceLog("var is NULL!"); }
        else if (!elements) { InterfaceLog("elements is NULL!"); }
        else if (!col) { InterfaceLog("col is NULL!"); }
        else {
            if (frameNum <= 3) InterfaceLog("All pointers OK, calling gui->render()...");
            try {
                gui->render();
                if (frameNum <= 3) InterfaceLog("gui->render() OK!");
                return; // Success, don't show fallback menu
            } catch (...) {
                InterfaceLog("gui->render() CRASHED! Switching to fallback menu.");
                guiCrashed = true;
            }
        }
    }
    
    // Fallback simple menu (only shown if gui->render crashes or pointers are null)
    if (frameNum <= 3) InterfaceLog("Rendering fallback menu");
    
    // Premium-style dark theme
    ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.08f, 0.08f, 0.08f, 0.95f));
    ImGui::PushStyleColor(ImGuiCol_TitleBg, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_TitleBgActive, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.15f, 0.15f, 0.15f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgHovered, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_FrameBgActive, ImVec4(0.25f, 0.25f, 0.25f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.2f, 0.2f, 0.2f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.3f, 0.3f, 0.3f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.4f, 0.4f, 0.4f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.2f, 0.4f, 0.8f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderHovered, ImVec4(0.3f, 0.5f, 0.9f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_HeaderActive, ImVec4(0.4f, 0.6f, 1.0f, 0.8f));
    ImGui::PushStyleColor(ImGuiCol_CheckMark, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.3f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.4f, 0.7f, 1.0f, 1.0f));
    ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 4.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(15, 15));
    
    ImGui::SetNextWindowPos(ImVec2(100, 100), ImGuiCond_FirstUseEver);
    ImGui::SetNextWindowSize(ImVec2(500, 450), ImGuiCond_FirstUseEver);
    
    if (ImGui::Begin("Kaitha Kaitha.cc", nullptr, ImGuiWindowFlags_NoCollapse))
    {
        // Tab bar
        if (ImGui::BeginTabBar("MainTabs"))
        {
            // AIMBOT TAB
            if (ImGui::BeginTabItem("Aimbot"))
            {
                ImGui::Checkbox("Enable Aimbot", &g_Options.Aimbot.enable);
                ImGui::SliderInt("FOV##aim", &g_Options.Aimbot.fov, 1, 300);
                ImGui::SliderInt("Distance##aim", &g_Options.Aimbot.distance, 1, 500);
                ImGui::SliderFloat("X Smooth", &g_Options.Aimbot.xsmooth, 0.0f, 50.0f);
                ImGui::SliderFloat("Y Smooth", &g_Options.Aimbot.ysmooth, 0.0f, 50.0f);
                
                const char* bones[] = { "Head", "Neck", "Chest", "Closest" };
                ImGui::Combo("Bone##aim", &g_Options.Aimbot.bone, bones, 4);
                
                ImGui::Checkbox("Visible Check##aim", &g_Options.Aimbot.cVisible);
                ImGui::Checkbox("Draw FOV##aim", &g_Options.Aimbot.drawFov);
                ImGui::Checkbox("Ignore Dead", &g_Options.Aimbot.IgnoreDead);
                ImGui::Checkbox("Ignore Friends", &g_Options.Aimbot.IgnoreFriends);
                ImGui::EndTabItem();
            }
            
            // SILENT TAB
            if (ImGui::BeginTabItem("Silent"))
            {
                ImGui::Checkbox("Enable Silent", &g_Options.Silent.enable);
                ImGui::SliderInt("FOV##silent", &g_Options.Silent.fov, 1, 300);
                ImGui::SliderInt("Distance##silent", &g_Options.Silent.distance, 1, 500);
                ImGui::SliderInt("Miss Chance %", &g_Options.Silent.misschance, 0, 100);
                
                const char* bones[] = { "Head", "Neck", "Chest", "Closest" };
                ImGui::Combo("Bone##silent", &g_Options.Silent.bone, bones, 4);
                
                ImGui::Checkbox("Visible Check##silent", &g_Options.Silent.cVisible);
                ImGui::Checkbox("Draw FOV##silent", &g_Options.Silent.drawFov);
                ImGui::Checkbox("Magic Bullet", &g_Options.Silent.Magic);
                ImGui::EndTabItem();
            }
            
            // ESP TAB
            if (ImGui::BeginTabItem("ESP"))
            {
                ImGui::Checkbox("Enable ESP", &g_Options.ESP.enable);
                ImGui::SliderInt("Max Distance", &g_Options.ESP.MaxDistance, 1, 500);
                ImGui::Separator();
                ImGui::Checkbox("Box", &g_Options.ESP.Box);
                ImGui::Checkbox("Skeleton", &g_Options.ESP.Skeleton);
                ImGui::Checkbox("Name", &g_Options.ESP.Name);
                ImGui::Checkbox("Distance", &g_Options.ESP.DistanceESP);
                ImGui::Checkbox("Health Bar", &g_Options.ESP.HealthBar);
                ImGui::Checkbox("Armor Bar", &g_Options.ESP.ArmorBar);
                ImGui::Checkbox("Snaplines", &g_Options.ESP.SnapLines);
                ImGui::Checkbox("Visible Check##esp", &g_Options.ESP.visible);
                ImGui::EndTabItem();
            }
            
            // PLAYER TAB
            if (ImGui::BeginTabItem("Player"))
            {
                ImGui::Checkbox("God Mode", &g_Options.Player.God);
                ImGui::Checkbox("Semi God", &g_Options.Player.SemiGod);
                if (g_Options.Player.SemiGod) {
                    ImGui::SliderFloat("Min Health", &g_Options.Player.SemiGodHealth, 1.0f, 200.0f);
                }
                ImGui::Separator();
                ImGui::Checkbox("Health Regen", &g_Options.Player.Health);
                if (g_Options.Player.Health) {
                    ImGui::SliderFloat("Health Amount", &g_Options.Player.HealthAmmount, 1.0f, 200.0f);
                }
                ImGui::Checkbox("Armor Regen", &g_Options.Player.Armor);
                if (g_Options.Player.Armor) {
                    ImGui::SliderFloat("Armor Amount", &g_Options.Player.ArmorAmmount, 1.0f, 100.0f);
                }
                ImGui::Separator();
                ImGui::Checkbox("No Fall Damage", &g_Options.Player.NoFallDamage);
                ImGui::Checkbox("Invisible", &g_Options.Exploit.KediciShadow);
                ImGui::EndTabItem();
            }
            
            // WEAPON TAB
            if (ImGui::BeginTabItem("Weapon"))
            {
                ImGui::Checkbox("No Recoil", &g_Options.Player.NoRecoil);
                ImGui::Checkbox("No Spread", &g_Options.Player.NoSpread);
                ImGui::Checkbox("No Reload", &g_Options.Player.NoReload);
                ImGui::Checkbox("Infinite Ammo", &g_Options.Player.InfiniteAmmo);
                ImGui::Separator();
                ImGui::Checkbox("Damage Boost", &g_Options.Player.DamageBoost);
                if (g_Options.Player.DamageBoost) {
                    ImGui::SliderFloat("Damage Amount", &g_Options.Player.DamageAmount, 1.0f, 100.0f);
                }
                ImGui::Checkbox("Range Modify", &g_Options.Player.RangeModify);
                if (g_Options.Player.RangeModify) {
                    ImGui::SliderFloat("Range Value", &g_Options.Player.RangeModifyValue, 1.0f, 1000.0f);
                }
                ImGui::EndTabItem();
            }
            
            // VEHICLE TAB
            if (ImGui::BeginTabItem("Vehicle"))
            {
                ImGui::Checkbox("Vehicle God", &g_Options.Player.VehicleGod);
                ImGui::Checkbox("Vehicle Speed", &g_Options.Player.VehicleSpeed);
                if (g_Options.Player.VehicleSpeed) {
                    ImGui::SliderFloat("Speed Amount", &g_Options.Player.VehicleSpeedAmount, 1.0f, 500.0f);
                }
                ImGui::Separator();
                ImGui::Checkbox("Super Handling", &g_Options.Exploit.SuperHandling);
                ImGui::Checkbox("Rocket Boost", &g_Options.Exploit.rocket_boost);
                ImGui::Checkbox("Vehicle Jump", &g_Options.Exploit.jump);
                ImGui::EndTabItem();
            }
            
            // MISC TAB
            if (ImGui::BeginTabItem("Misc"))
            {
                ImGui::Checkbox("Strafe Macro", &g_Options.Exploit.StrafeMacro);
                if (g_Options.Exploit.StrafeMacro) {
                    ImGui::SliderInt("Strafe Delay (ms)", &g_Options.Exploit.StrafeMacroDelay, 10, 500);
                }
                ImGui::Separator();
                ImGui::Checkbox("Radar", &g_Options.Radar.enable);
                if (g_Options.Radar.enable) {
                    ImGui::SliderInt("Radar Size", &g_Options.Radar.size, 100, 400);
                    ImGui::SliderInt("Radar Range", &g_Options.Radar.range, 50, 500);
                }
                ImGui::Separator();
                ImGui::Checkbox("Stream Proof", &g_Options.General.StreamProof);
                ImGui::Checkbox("Hide on Alt-Tab", &g_Options.General.HideOnAltTab);
                ImGui::EndTabItem();
            }
            
            ImGui::EndTabBar();
        }
        
        ImGui::Separator();
        ImGui::TextColored(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), "Press INSERT to toggle menu");
    }
    ImGui::End();
    
    ImGui::PopStyleVar(3);
    ImGui::PopStyleColor(15);
    
    
}

void Interface::RenderWatermark()
{
    // Check if watermark is enabled
    if (!var->cfg.get<bool>("menu_watermark"))
        return;
    
    ImDrawList* drawList = ImGui::GetForegroundDrawList();
    if (!drawList)
        return;
    
    // Get current time
    std::time_t now = std::time(nullptr);
    std::tm localTime;
    localtime_s(&localTime, &now);
    
    std::ostringstream timeStream;
    timeStream << std::setfill('0') << std::setw(2) << localTime.tm_hour << ":"
               << std::setfill('0') << std::setw(2) << localTime.tm_min;
    std::string timeStr = timeStream.str();
    
    // Watermark text parts
    const char* brandText = "Kaitha";
    const char* separator = " | ";
    const char* premiumText = "Kaitha.cc";
    
    // Colors matching menu theme
    ImU32 bgColor = IM_COL32(15, 15, 15, 230);
    ImU32 borderColor = IM_COL32(50, 50, 50, 255);
    ImU32 brandColor = IM_COL32(255, 255, 255, 255);     // White for brand
    ImU32 separatorColor = IM_COL32(80, 80, 80, 255);    // Gray for separators
    ImU32 premiumColor = IM_COL32(255, 200, 100, 255);   // Gold for Premium
    ImU32 timeColor = IM_COL32(150, 150, 150, 255);      // Light gray for time
    
    // Get fonts
    ImFont* textFont = font->get("medium12");
    if (!textFont) textFont = ImGui::GetFont();
    
    // Calculate sizes for each part
    ImVec2 brandSize = textFont->CalcTextSizeA(textFont->FontSize, FLT_MAX, 0.0f, brandText);
    ImVec2 sep1Size = textFont->CalcTextSizeA(textFont->FontSize, FLT_MAX, 0.0f, separator);
    ImVec2 premiumSize = textFont->CalcTextSizeA(textFont->FontSize, FLT_MAX, 0.0f, premiumText);
    ImVec2 sep2Size = textFont->CalcTextSizeA(textFont->FontSize, FLT_MAX, 0.0f, separator);
    ImVec2 timeSize = textFont->CalcTextSizeA(textFont->FontSize, FLT_MAX, 0.0f, timeStr.c_str());
    
    // Total width calculation
    float totalTextWidth = brandSize.x + sep1Size.x + premiumSize.x + sep2Size.x + timeSize.x;
    
    // Watermark position (top left)
    float padding = 10.0f;
    float boxPaddingX = 12.0f;
    float boxPaddingY = 8.0f;
    
    float boxHeight = textFont->FontSize + boxPaddingY * 2;
    float boxWidth = totalTextWidth + boxPaddingX * 2;
    
    ImVec2 boxPos = ImVec2(padding, padding);
    ImVec2 boxEnd = ImVec2(boxPos.x + boxWidth, boxPos.y + boxHeight);
    
    // Draw background with rounded corners
    drawList->AddRectFilled(boxPos, boxEnd, bgColor, 6.0f);
    drawList->AddRect(boxPos, boxEnd, borderColor, 6.0f, 0, 1.0f);
    
    // Calculate vertical center for text
    float textY = boxPos.y + (boxHeight - textFont->FontSize) * 0.5f;
    
    // Starting X position
    float currentX = boxPos.x + boxPaddingX;
    
    // Draw brand text "Vicious"
    drawList->AddText(textFont, textFont->FontSize, ImVec2(currentX, textY), brandColor, brandText);
    currentX += brandSize.x;
    
    // Draw separator " | "
    drawList->AddText(textFont, textFont->FontSize, ImVec2(currentX, textY), separatorColor, separator);
    currentX += sep1Size.x;
    
    // Draw "Premium"
    drawList->AddText(textFont, textFont->FontSize, ImVec2(currentX, textY), premiumColor, premiumText);
    currentX += premiumSize.x;
    
    // Draw separator " | "
    drawList->AddText(textFont, textFont->FontSize, ImVec2(currentX, textY), separatorColor, separator);
    currentX += sep2Size.x;
    
    // Draw time
    drawList->AddText(textFont, textFont->FontSize, ImVec2(currentX, textY), timeColor, timeStr.c_str());
}

void Interface::WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    ImGui_ImplWin32_WndProcHandler(hWnd, uMsg, wParam, lParam);
}

void Interface::HandleWindowDrag()
{
    // Window dragging is handled by premium_menu_send
}

void Interface::HandleMenuKey()
{
    static bool MenuKeyDown = false;

    if (GetAsyncKeyState(g_Options.General.MenuKey) & 0x8000)
    {
        if (!MenuKeyDown)
        {
            bIsMenuOpen = !bIsMenuOpen;
            
            // Set click-through based on menu state
            M_base::Overlay::SetClickThrough(!bIsMenuOpen);
            
            MenuKeyDown = true;
        }
    }
    else
    {
        MenuKeyDown = false;
    }
}

void Interface::ShutDown()
{
    hWindow = nullptr;
    hTargetWindow = nullptr;
    IDevice = nullptr;
    g_FontsInitialized = false;
    g_ImagesInitialized = false;
}

} // namespace M_base
