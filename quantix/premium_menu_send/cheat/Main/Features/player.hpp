#pragma once

#include "../FivemSDK/Fivem.hpp"
#include "../FivemSDK/GTADefines.hpp"

namespace Cheat
{
	namespace Player
	{
		void TeleportToWaypoint();
		void TeleportToCoord(Vector3D Coord);
		void SaveCurrentLocation();
		void TeleportToSavedLocation();
		void FreecamTeleport();
		void FreecamTeleportLoop();
		void FixVehicle();
		void SetFuelToZero();
		void SetFuelToZeroLoop();
		void ReduceEngineHealth();
		void test();
		
		// Ghost Peek
		void GhostPeekSetMark();
		void GhostPeekReturnToMark();
		void HandleGhostPeek();
		void GhostPeekLoop();
		
		void GodModeLoop();
		void HealthLoop();
		void ArmorLoop();
		void NoRecoilLoop();
		void NoReloadLoop();
		void NoSpreadLoop();
		void RangeModifyLoop();
		void DamageBoostLoop();
		void SafeDamageBoostLoop();
		void VehicleSpeedLoop();
		void VehicleGodLoop();
		void SuperHandlingLoop();
		void KediciShadowsLoop();
		void StealCarLoop();
		void ImmortalityLoop();
		void PlayerSpeedLoop();
		void CamFovLoop();
		void InfiniteAmmoLoop();
		void FreecamLoop();
		void NocollisionLoop();
		void AbsorptionLoop();
		void FixVehicleKey();
		void RocketBoostLoop();
		void JumpLoop();
		void ParachuteLoop();
		void VehicleGravityLoop();
		void NoFallDamageLoop();
		void ShrinkLoop();
		void NoclipLoop();
		void StrafeMacroLoop();
		void StartFeatureThreads();
	}
}
