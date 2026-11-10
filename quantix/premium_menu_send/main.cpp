// Dear ImGui: standalone example application for DirectX 11
// Modified to launch overlay cheat system with new menu + AUTH SYSTEM

#define IMGUI_DEFINE_MATH_OPERATORS
#include "cheat/Main/Cheat.hpp"
#include <thread>

// Windows headers for WinMain
#include <Windows.h>

// Main code - launches cheat overlay system with authentication
int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    // Hide console window if it exists
    HWND consoleWindow = GetConsoleWindow();
    if (consoleWindow != NULL)
    {
        ShowWindow(consoleWindow, SW_HIDE);
    }
    
    // ============================================
    // CHEAT SYSTEM START
    // ============================================
    
    // Start cheat system in separate thread
    std::thread cheatThread([]() {
        Cheat::C_Start();
    });
    
    // Wait for cheat thread
    cheatThread.join();
    
    // Cleanup
    
    return 0;
}