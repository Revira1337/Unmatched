#pragma once

#include <Windows.h>
#include <functional>
#include <d3d11.h>
#include <dwmapi.h>

namespace M_base
{
    namespace Overlay
    {
        void Setup(DWORD TargetPid);
        void Initialize();
        void ShutDown();
        void UpdateWindowPos();
        void SetupWindowProcHook(std::function<void(HWND, UINT, WPARAM, LPARAM)> Funtion);
        bool IsSettuped();
        bool IsInitialized();
        HWND GetOverlayWindow();
        HWND GetTargetWindow();
        void dxInitialize();
        void dxRefresh();
        void dxShutDown();
        void dxCreateRenderTarget();
        void dxCleanupRenderTarget();
        void dxCleanupDeviceD3D();
        ID3D11Device* dxGetDevice();
        ID3D11DeviceContext* dxGetDeviceContext();
        IDXGISwapChain* dxGetSwapChain();
        ID3D11RenderTargetView* dxGetRenderTarget();
        
        // Click-through control - when true, clicks pass through to game
        void SetClickThrough(bool clickThrough);
    }
}
