#pragma once

#include "../FivemSDK/Fivem.hpp"
#include "../FivemSDK/GTADefines.hpp"


namespace Cheat
{
	namespace Silent
	{
		void run();
		Vector3D GetCurrentTargetPos();
		bool HasCurrentTarget();
		// NetId of the player currently locked by Silent Aim (-1 if none / target is a vehicle wheel).
		// Read by the ESP to paint the locked player with g_Options.ESP.LockedColor.
		int GetLockedNetId();
	}
}