#include "triggerbot.hpp"
#include "../Options.hpp"
#include <chrono>
#include <thread>
#include <fstream>

namespace Cheat
{
	namespace Triggerbot
	{
		static bool g_TargetInCrosshair = false;
		static std::chrono::steady_clock::time_point g_LastTargetTime;
		static bool g_WaitingForDelay = false;
		static std::ofstream g_DebugLog;

		bool IsTargetInCrosshair()
		{
			return g_TargetInCrosshair;
		}

		void DebugLog(const char* msg)
		{
			if (!g_DebugLog.is_open())
				g_DebugLog.open("triggerbot_debug.log", std::ios::out | std::ios::trunc);
			if (g_DebugLog.is_open())
			{
				g_DebugLog << msg << std::endl;
				g_DebugLog.flush();
			}
		}

		void SimulateMouseClick()
		{
			DebugLog("SimulateMouseClick called!");
			// Mouse down
			mouse_event(MOUSEEVENTF_LEFTDOWN, 0, 0, 0, 0);
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
			// Mouse up
			mouse_event(MOUSEEVENTF_LEFTUP, 0, 0, 0, 0);
		}

		void run()
		{
			DebugLog("Triggerbot thread started!");
			
			HWND hwnd = M_base::Overlay::GetOverlayWindow();
			HWND targetHwnd = M_base::Overlay::GetTargetWindow();

			while (!g_Options.General.ShutDown)
			{
				std::this_thread::sleep_for(std::chrono::milliseconds(5));

				// Check if triggerbot is enabled
				if (!g_Options.Triggerbot.enable)
				{
					g_TargetInCrosshair = false;
					g_WaitingForDelay = false;
					continue;
				}

				// Check if key is active (Hold / Press mode honoured via kb::)
				if (g_Options.Triggerbot.key != 0)
				{
					if (!(GetAsyncKeyState(g_Options.Triggerbot.key) & 0x8000))
					{
						g_TargetInCrosshair = false;
						g_WaitingForDelay = false;
						continue;
					}
				}

				// Don't trigger when overlay/menu is focused
				HWND foreground = GetForegroundWindow();
				if (foreground == hwnd)
				{
					g_TargetInCrosshair = false;
					g_WaitingForDelay = false;
					continue;
				}

				// Get screen center (crosshair position)
				float screenCenterX = (float)GetSystemMetrics(SM_CXSCREEN) / 2.0f;
				float screenCenterY = (float)GetSystemMetrics(SM_CYSCREEN) / 2.0f;

				// Get entity list
				auto entityList = C_Five.GetEntitiyList();
				auto localPlayer = C_Five.GetLocalPlayerInfo();

				static int logCounter = 0;
				logCounter++;
				if (logCounter % 200 == 0) // Log every 200 iterations (about 1 second)
				{
					char buf[256];
					sprintf(buf, "Entity count: %d, Screen center: %.0f, %.0f", (int)entityList.size(), screenCenterX, screenCenterY);
					DebugLog(buf);
				}

				bool foundTarget = false;

				for (size_t i = 0; i < entityList.size(); i++)
				{
					const auto& entity = entityList[i];

					// Check distance
					float distance = localPlayer.WorldPos.DistTo(entity.Cordinates);

					// Use screen positions from entity (already calculated)
					ImVec2 headPos = entity.HeadPos;
					ImVec2 footPos = entity.LeftFootPos;

					// Log ALL entities to debug
					if (logCounter % 200 == 0)
					{
						char buf[512];
						sprintf(buf, "Entity %d: HeadPos(%.0f, %.0f), FootPos(%.0f, %.0f), Distance: %.0f, Health: %.0f, Dead: %d, Visible: %d, IsFriend: %d", 
							(int)i, headPos.x, headPos.y, footPos.x, footPos.y, distance, entity.Health, entity.Dead ? 1 : 0, entity.Visible ? 1 : 0, entity.IsFriend ? 1 : 0);
						DebugLog(buf);
					}

					// Skip dead if option enabled (only check Dead flag, Health is unreliable)
					if (g_Options.Triggerbot.IgnoreDead && entity.Dead)
						continue;

					// Skip invisible if option enabled
					if (g_Options.Triggerbot.IgnoreInvisible && !entity.Visible)
						continue;

					// Skip friends if option enabled
					if (g_Options.Triggerbot.IgnoreFriends && entity.IsFriend)
						continue;

					// Check distance
					if (distance > g_Options.Triggerbot.distance)
						continue;

					// Visibility check
					if (g_Options.Triggerbot.cVisible && !entity.Visible)
						continue;

					// Skip if not on screen
					if (headPos.x <= 0 || headPos.y <= 0)
						continue;

					// Calculate box dimensions from head to feet
					float boxTop = headPos.y;
					float boxBottom = headPos.y;
					float boxHeight = 0;

					if (footPos.y > 0)
					{
						boxBottom = footPos.y;
						boxHeight = boxBottom - boxTop;
					}
					else
					{
						// Fallback - estimate height based on distance
						boxHeight = 80.0f;
						if (distance > 0)
							boxHeight = 4000.0f / distance;
						if (boxHeight > 500) boxHeight = 500;
						if (boxHeight < 20) boxHeight = 20;
						boxBottom = headPos.y + boxHeight;
					}

					// Calculate box width (roughly 1/3 of height)
					float boxWidth = (boxBottom - boxTop) * 0.4f;
					if (boxWidth < 20.0f) boxWidth = 20.0f;
					if (boxWidth > 200.0f) boxWidth = 200.0f;

					float boxCenterX = headPos.x;
					float boxLeft = boxCenterX - boxWidth / 2.0f;
					float boxRight = boxCenterX + boxWidth / 2.0f;

					// Log box info occasionally
					if (logCounter % 200 == 0)
					{
						char buf[256];
						sprintf(buf, "Box: Left=%.0f, Right=%.0f, Top=%.0f, Bottom=%.0f | Crosshair: %.0f, %.0f", 
							boxLeft, boxRight, boxTop, boxBottom, screenCenterX, screenCenterY);
						DebugLog(buf);
					}

					// Check if crosshair is inside the box
					if (screenCenterX >= boxLeft && screenCenterX <= boxRight &&
						screenCenterY >= boxTop && screenCenterY <= boxBottom)
					{
						DebugLog("TARGET FOUND IN CROSSHAIR!");
						foundTarget = true;
						break;
					}
				}

				if (foundTarget)
				{
					g_TargetInCrosshair = true;

					// Start delay timer if not already waiting
					if (!g_WaitingForDelay)
					{
						g_LastTargetTime = std::chrono::steady_clock::now();
						g_WaitingForDelay = true;
						DebugLog("Starting delay timer...");
					}

					// Check if delay has passed
					auto now = std::chrono::steady_clock::now();
					auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - g_LastTargetTime).count();

					if (elapsed >= g_Options.Triggerbot.delay)
					{
						// Auto shoot
						if (g_Options.Triggerbot.autoShoot)
						{
							DebugLog("SHOOTING!");
							SimulateMouseClick();

							// Reset delay for next shot
							g_WaitingForDelay = false;

							// Small cooldown after shooting
							std::this_thread::sleep_for(std::chrono::milliseconds(60));
						}
					}
				}
				else
				{
					g_TargetInCrosshair = false;
					g_WaitingForDelay = false;
				}
			}
			
			DebugLog("Triggerbot thread ended!");
			if (g_DebugLog.is_open())
				g_DebugLog.close();
		}
	}
}