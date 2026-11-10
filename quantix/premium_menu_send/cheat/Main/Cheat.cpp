// Cheat.cpp - Overlay system with NEW menu integration
#define IMGUI_DEFINE_MATH_OPERATORS
#include "Cheat.hpp"
#include "../Utils/Render/Interface.hpp"
#include "../Utils/Render/Overlay.hpp"
#include "Features/playernames.hpp"
#include "Features/player.hpp"
#include "Features/silent.hpp"
#include "Features/esp.hpp"
#include "Features/radar.hpp"
#include "Features/aim.hpp"
#include "Features/triggerbot.hpp"
#include "Options.hpp"

// NEW MENU INCLUDES
#include "../../menu_new/interface/menu_i.h"
#include "../../menu_new/interface/elements_manager.h"
#include "../../menu_new/interface/pages.h"
#include "../../menu_new/helpers/widgets.h"
#include "../../menu_new/helpers/notify.h"

// ImGui backends
#include "../../thirdparty/imgui/backends/imgui_impl_win32.h"
#include "../../thirdparty/imgui/backends/imgui_impl_dx11.h"

// Forward declare WndProc handler
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

#include <fstream>
#include <thread>
#include <chrono>

// Debug logging
static std::ofstream g_CheatLog;

void CheatLog(const char* msg)
{
    if (!g_CheatLog.is_open())
        g_CheatLog.open("cheat_debug.log", std::ios::out | std::ios::trunc);
    if (g_CheatLog.is_open())
    {
        g_CheatLog << msg << std::endl;
        g_CheatLog.flush();
    }
}

bool STATUS = true;

namespace Cheat
{
    static bool g_CheatInitialized = false;
    static bool g_FeaturesStarted = false;
    static c_menu* g_Menu = nullptr;

    void C_Start()
    {
        CheatLog("C_Start() called");
        
        g_Options.Exploit.rocket_boost = false;
        g_Options.Exploit.jump = false;
        g_Options.Exploit.parachute = false;
        
        CheatLog("Waiting for FiveM...");
        
        // Initialize FiveM SDK
        int attempts = 0;
        while (!C_Five.IsInitialized())
        {
            attempts++;
            char buf[64];
            sprintf_s(buf, "FiveM init attempt %d", attempts);
            CheatLog(buf);
            
            C_Five.Initialize();

            if (!C_Five.IsInitialized())
                std::this_thread::sleep_for(std::chrono::seconds(2));
        }
        
        CheatLog("FiveM initialized!");

        // Setup overlay
        CheatLog("Setting up overlay...");
        M_base::Overlay::Setup(C_Five.GetPid());
        CheatLog("Overlay setup done, initializing...");
        M_base::Overlay::Initialize();
        CheatLog("Overlay initialized!");

        // Start feature threads
        CheatLog("Starting feature threads...");
        
        std::thread([&]() { Cheat::Silent::run(); }).detach();
        CheatLog("Silent thread started!");
        
        std::thread([&]() { Cheat::Aim::run(); }).detach();
        CheatLog("Aim thread started!");
        
        std::thread([&]() { Cheat::Triggerbot::run(); }).detach();
        CheatLog("Triggerbot thread started!");

        Cheat::PlayerNames::Initialize();
        Cheat::PlayerNames::StartUpdateThread();
        CheatLog("PlayerNames initialized!");
        
        Cheat::Player::StartFeatureThreads();
        CheatLog("Player feature threads started!");
        
        g_FeaturesStarted = true;
        CheatLog("All feature threads started!");

        if (M_base::Overlay::IsInitialized())
        {
            CheatLog("Creating ImGui context...");
            ImGui::CreateContext();
            CheatLog("ImGui context created!");
            
            ImGuiIO& io = ImGui::GetIO();
            io.IniFilename = NULL;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
            io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;
            
            CheatLog("Initializing ImGui Win32 backend...");
            ImGui_ImplWin32_Init(M_base::Overlay::GetOverlayWindow());
            CheatLog("Win32 backend initialized!");
            
            CheatLog("Initializing ImGui DX11 backend...");
            ImGui_ImplDX11_Init(M_base::Overlay::dxGetDevice(), M_base::Overlay::dxGetDeviceContext());
            CheatLog("DX11 backend initialized!");
            
            // Initialize NEW MENU
            CheatLog("Creating NEW menu...");
            g_Menu = create_menu();
            g_Menu->setup_data(M_base::Overlay::dxGetDevice());
            g_Menu->setup_imgui();
            CheatLog("NEW menu created!");

            MGR->esp_p = true;

            // Setup menu tabs with NEW menu
            MGR->add_tab("Aim Assist", "aim");
            MGR->add_subtab("Aim Bot", pages::aimbot);
            MGR->add_subtab("Silent Aim", pages::silent);
            MGR->add_subtab("Trigger Bot", pages::trigger);

            MGR->add_tab("Visuals", "eye");
            MGR->add_subtab("Visuals", pages::visuals);
            MGR->add_subtab("Colors", pages::visuals_colors);
            MGR->add_subtab("Friends", pages::visuals_friends);

            MGR->add_tab("Player", "player");
            MGR->add_subtab("General", pages::player_general);
            MGR->add_subtab("Other", pages::player_freecam);
            MGR->add_subtab("Players", pages::player_players);

            MGR->add_tab("Vehicle", "car");
            MGR->add_subtab("Vehicle", pages::vehicle);
            MGR->add_subtab("Repair", pages::vehicle_repair);
            MGR->add_subtab("Vehicles", pages::vehicle_vehicles);

            MGR->add_tab("Weapon", "weapon");
            MGR->add_subtab("Modifiers", pages::weapon_modifiers);
            MGR->add_subtab("Weapon", pages::weapon_weapon);

            MGR->add_tab("Radar", "radar");
            MGR->add_subtab("Radar", pages::radar);
            MGR->add_subtab("Colors", pages::radar_colors);
            MGR->add_subtab("Around", pages::radar_around);

            MGR->add_tab("Settings", "settings");
            MGR->add_subtab("Settings", pages::settings);
            MGR->add_subtab("Config", pages::settings_config);

            CheatLog("Menu tabs configured!");
            
            M_base::Overlay::SetupWindowProcHook([](HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) -> LRESULT {
                return ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam);
            });
            
            // Set initial window style - menu starts visible (no click-through)
            SetWindowLong(M_base::Overlay::GetOverlayWindow(), GWL_EXSTYLE, 
                WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);

            g_CheatInitialized = true;
            CheatLog("Entering main render loop...");

            MSG Message;
            ZeroMemory(&Message, sizeof(Message));
            int frameCount = 0;
            bool menuVisible = true; // Menu starts visible
            
            while (Message.message != WM_QUIT)
            {
                frameCount++;
                if (frameCount <= 5) {
                    char buf[64];
                    sprintf_s(buf, "Frame %d start", frameCount);
                    CheatLog(buf);
                }
                
                // Convert FOV colors from g_Options float arrays to ImColor
                ImColor silentfovcolor(g_Options.Silent.SilentFovColor[0], g_Options.Silent.SilentFovColor[1], g_Options.Silent.SilentFovColor[2], g_Options.Silent.SilentFovColor[3]);
                ImColor aimfovcolor(g_Options.Aimbot.AimbotFovColor[0], g_Options.Aimbot.AimbotFovColor[1], g_Options.Aimbot.AimbotFovColor[2], g_Options.Aimbot.AimbotFovColor[3]);

                while (::PeekMessage(&Message, nullptr, 0U, 0U, PM_REMOVE))
                {
                    ::TranslateMessage(&Message);
                    ::DispatchMessage(&Message);
                    if (Message.message == WM_QUIT)
                        break;
                }

                // Check for menu toggle key (INSERT) — ignored until the intro splash has finished
                if ((GetAsyncKeyState(g_Options.General.MenuKey) & 1) && (!g_Menu || g_Menu->splash_finished())) {
                    menuVisible = !menuVisible;
                    
                    // Toggle click-through based on menu visibility
                    if (menuVisible) {
                        // Menu visible - disable click-through to allow interaction
                        SetWindowLong(M_base::Overlay::GetOverlayWindow(), GWL_EXSTYLE, 
                            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED);
                        SetForegroundWindow(M_base::Overlay::GetOverlayWindow());
                    } else {
                        // Menu hidden - enable click-through
                        SetWindowLong(M_base::Overlay::GetOverlayWindow(), GWL_EXSTYLE, 
                            WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_TRANSPARENT | WS_EX_LAYERED);
                        if (M_base::Overlay::GetTargetWindow())
                            SetForegroundWindow(M_base::Overlay::GetTargetWindow());
                    }
                }

                // Set mouse cursor visibility based on menu state
                ImGui::GetIO().MouseDrawCursor = menuVisible;

                // Stream proof toggle
                static bool streamon = false;
                if (g_Options.General.StreamProof != streamon)
                {
                    streamon = g_Options.General.StreamProof;
                    SetWindowDisplayAffinity(M_base::Overlay::GetOverlayWindow(), streamon ? WDA_EXCLUDEFROMCAPTURE : WDA_NONE);
                }

                M_base::Overlay::UpdateWindowPos();

                ImGui_ImplDX11_NewFrame();
                ImGui_ImplWin32_NewFrame();
                ImGui::NewFrame();
                
                // Update entity list
                C_Five.UpdateEntityList();
                
                if (!STATUS)
                    exit(0);
                else
                {
                    // Check if FiveM is focused (for Hide on Alt-Tab feature)
                    static bool shouldDrawOverlay = true;
                    static auto lastFocusCheck = std::chrono::steady_clock::now();
                    
                    if (g_Options.General.HideOnAltTab)
                    {
                        auto now = std::chrono::steady_clock::now();
                        if (std::chrono::duration_cast<std::chrono::milliseconds>(now - lastFocusCheck).count() > 100)
                        {
                            HWND foregroundWindow = GetForegroundWindow();
                            HWND fivemHwnd = M_base::Overlay::GetTargetWindow();
                            HWND overlayHwnd = M_base::Overlay::GetOverlayWindow();
                            shouldDrawOverlay = (foregroundWindow == fivemHwnd || foregroundWindow == overlayHwnd);
                            lastFocusCheck = now;
                        }
                    }
                    else
                    {
                        shouldDrawOverlay = true;
                    }

                    // Render NEW MENU (only if visible)
                    if (menuVisible)
                    {
                        if (frameCount <= 5) CheatLog("Drawing NEW menu...");
                        g_Menu->draw();
                        g_Menu->end();
                        if (frameCount <= 5) CheatLog("NEW menu drawn!");

                        if (MGR->close_requested)
                        {
                            CheatLog("Close requested via menu");
                            PostQuitMessage(0);
                            break;
                        }
                    }

                    // Watermark — drawn every frame, independent of the menu being open
                    // AND independent of Hide-on-Alt-Tab: it must stay visible even when
                    // FiveM loses focus, so it lives outside the shouldDrawOverlay gate.
                    if (g_Menu)
                        g_Menu->watermark();

                    // Feature toast notifications (top-right) — independent of menu open state
                    notify::watch();
                    notify::draw();

                    // Only draw ESP/FOV if FiveM is focused or HideOnAltTab is disabled
                    if (shouldDrawOverlay)
                    {
                        // ESP and Radar
                        Cheat::ESP::run();
                        Cheat::ESP::DrawActiveFeatures();
                        Cheat::ESP::DrawDamageHitmarker();
                        Cheat::Radar::run();

                        if (g_Options.Silent.drawFov && g_Options.Silent.enable)
                            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2((float)(GetSystemMetrics(SM_CXSCREEN)) / 2, (float)(GetSystemMetrics(SM_CYSCREEN)) / 2), (float)g_Options.Silent.fov, silentfovcolor, 500, 1.5f);

                        if (g_Options.Aimbot.drawFov && g_Options.Aimbot.enable)
                            ImGui::GetBackgroundDrawList()->AddCircle(ImVec2((float)(GetSystemMetrics(SM_CXSCREEN)) / 2, (float)(GetSystemMetrics(SM_CYSCREEN)) / 2), (float)g_Options.Aimbot.fov, aimfovcolor, 500, 1.5f);

                        // Draw locked bone indicator
                        if (g_Options.Silent.enable && g_Options.Silent.showLockedBone && Cheat::Silent::HasCurrentTarget())
                        {
                            Vector3D targetPos = Cheat::Silent::GetCurrentTargetPos();
                            ImVec2 screenPos = C_Five.WorldToScreen(targetPos);
                            if (screenPos.x > 0 && screenPos.y > 0 && screenPos.x < ImGui::GetIO().DisplaySize.x && screenPos.y < ImGui::GetIO().DisplaySize.y)
                            {
                                ImColor whiteColor(1.0f, 1.0f, 1.0f, 1.0f);
                                ImGui::GetBackgroundDrawList()->AddCircleFilled(screenPos, 4.0f, ImColor(0, 0, 0, 160), 24);
                                ImGui::GetBackgroundDrawList()->AddCircleFilled(screenPos, 3.0f, whiteColor, 24);
                            }
                        }
                    }
                }
                
                if (frameCount <= 5) CheatLog("Before EndFrame...");
                ImGui::EndFrame();
                ImGui::Render();

                if (frameCount <= 5) CheatLog("Before dxRefresh...");
                M_base::Overlay::dxRefresh();
                if (frameCount <= 5) CheatLog("Before RenderDrawData...");
                ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
                if (frameCount <= 5) CheatLog("Before Present...");
                M_base::Overlay::dxGetSwapChain()->Present(1, 0); // VSync enabled
                if (frameCount <= 5) CheatLog("Frame complete!");

                if (g_Options.General.ShutDown)
                {
                    CheatLog("Shutdown requested");
                    break;
                }

                // Only add extra delay if ThreadDelay is set
                if (g_Options.General.ThreadDelay > 0.1f)
                    std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(g_Options.General.ThreadDelay));
            }
            
            CheatLog("Exiting render loop");
        }
    }

    void C_Stop()
    {
        if (!g_Options.General.ShutDown)
            return;

        CheatLog("Stopping cheat...");
        
        // Cleanup ImGui
        ImGui_ImplDX11_Shutdown();
        ImGui_ImplWin32_Shutdown();
        ImGui::DestroyContext();
        
        M_base::Overlay::ShutDown();
        M_base::Overlay::dxShutDown();
        M_base::mem::DetachProcess();
        
        g_CheatInitialized = false;
        g_FeaturesStarted = false;
        
        CheatLog("Cheat stopped");
    }

    bool IsReady()
    {
        return g_CheatInitialized && C_Five.IsInitialized();
    }
}
