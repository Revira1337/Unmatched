#pragma once

#include "../FrameWork.hpp"

#include <d3d11.h>
// Forward declaration
class c_menu;


namespace M_base
{

	class Interface
	{
	public:
		Interface() { }
		Interface(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext) { Initialize(Window, TargetWindow, Device, DeviceContext); }
		~Interface() { ShutDown(); }

		void Initialize(HWND Window, HWND TargetWindow, ID3D11Device* Device, ID3D11DeviceContext* DeviceContext);

		void RenderGui();
		void RenderWatermark();

		void WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

		void HandleWindowDrag();
		void HandleMenuKey();

		void ShutDown();

		bool GetMenuOpen() { return bIsMenuOpen; }

	private:
		HWND hWindow;
		HWND hTargetWindow;
		ID3D11Device* IDevice;
		c_menu* menuInstance;

		int CurrentTab = 1;

		bool bIsMenuOpen = true;

	public:
		UINT ResizeWidht;
		UINT ResizeHeight;
	};
}

extern M_base::Interface interfaceObject;