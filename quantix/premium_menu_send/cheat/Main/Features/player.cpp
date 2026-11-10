#include "player.hpp"
#include "../Options.hpp"
#include <thread>
#include <Main/FivemSDK/simplemath.h>

bool TPVehicle = false;
uint64_t TPModelInfo = NULL;
uint64_t TPObject = NULL;
uint64_t TPNavigation = NULL;
Vector3D TPPosition = Vector3D(0, 0, 0);

namespace Cheat
{
	void Noclip(Vector3D CameraPos)
	{
	}

	void Player::TeleportToWaypoint()
	{
		for (int i = 0; i < 2000; i++)
		{
			uint64_t Blip = M_base::mem::Read<uint64_t>(C_Five.GetBlipListAddress() + (i * 8));
			if (!Blip)
				continue;

			int BlipIcon = M_base::mem::Read<int>(Blip + 0x40);
			int BlipColor = M_base::mem::Read<int>(Blip + 0x48);

			if ((BlipColor != BlipColor::ColorWaypoint) || (BlipIcon != BlipSprite::SpriteWaypoint))
				continue;

			Vector2D WaypointPos = M_base::mem::Read<Vector2D>(Blip + 0x10);

			if (WaypointPos.x != 0 && WaypointPos.y != 0)
			{
				uint64_t Object = NULL;
				uint64_t Navigation = NULL;
				uint64_t ModelInfo = NULL;

				if (C_Five.GetLocalPlayerInfo().Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* Vehicle = C_Five.GetLocalPlayerInfo().Ped->GetLastVehicle();
					if (Vehicle)
					{
						Object = (uint64_t)Vehicle;
						Navigation = Vehicle->GetNavigation();
						ModelInfo = Vehicle->GetModelInfo();
					}
				}

				if (!Object || !Navigation || !ModelInfo)
				{
					Object = (uint64_t)C_Five.GetLocalPlayerInfo().Ped;
					Navigation = C_Five.GetLocalPlayerInfo().Ped->GetNavigation();
					ModelInfo = C_Five.GetLocalPlayerInfo().Ped->GetModelInfo();
				}

				Vector3D TeleportPos = Vector3D(WaypointPos.x, WaypointPos.y, -210.f);

				C_Five.TeleportFunc(Object, Navigation, ModelInfo, TeleportPos, TeleportPos, true);
			}
		}
	}

	void Player::TeleportToCoord(Vector3D Coord)
	{
		uint64_t Object = NULL;
		uint64_t Navigation = NULL;
		uint64_t ModelInfo = NULL;

		if (C_Five.GetLocalPlayerInfo().Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
		{
			CVehicle* Vehicle = C_Five.GetLocalPlayerInfo().Ped->GetLastVehicle();
			if (Vehicle)
			{
				Object = (uint64_t)Vehicle;
				Navigation = Vehicle->GetNavigation();
				ModelInfo = Vehicle->GetModelInfo();
			}
		}

		if (!Object || !Navigation || !ModelInfo)
		{
			Object = (uint64_t)C_Five.GetLocalPlayerInfo().Ped;
			Navigation = C_Five.GetLocalPlayerInfo().Ped->GetNavigation();
			ModelInfo = C_Five.GetLocalPlayerInfo().Ped->GetModelInfo();
		}

		Vector3D TeleportPos = Vector3D(Coord.x, Coord.y, Coord.z);

		C_Five.TeleportFunc(Object, Navigation, ModelInfo, TeleportPos, TeleportPos, true);
	}

	void Player::SaveCurrentLocation()
	{
		auto localPlayer = C_Five.GetLocalPlayerInfo();
		if (localPlayer.Ped)
		{
			g_Options.Player.SavedLocation = localPlayer.WorldPos;
			g_Options.Player.HasSavedLocation = true;
		}
	}

	void Player::TeleportToSavedLocation()
	{
		if (g_Options.Player.HasSavedLocation)
		{
			TeleportToCoord(g_Options.Player.SavedLocation);
		}
	}

	// Ghost Peek - Save current position
	void Player::GhostPeekSetMark()
	{
		auto localplayer = C_Five.GetLocalPlayerInfo();
		if (localplayer.Ped)
		{
			g_Options.Player.GhostPeekPosition = localplayer.WorldPos;
			g_Options.Player.HasGhostPeekPosition = true;
		}
	}

	// Ghost Peek - Return to saved position
	void Player::GhostPeekReturnToMark()
	{
		if (g_Options.Player.HasGhostPeekPosition)
		{
			TeleportToCoord(g_Options.Player.GhostPeekPosition);
		}
	}

	// Ghost Peek - Handle key presses
	void Player::HandleGhostPeek()
	{
		if (!g_Options.Player.GhostPeekEnabled)
			return;

		static bool markKeyWasPressed = false;
		static bool returnKeyWasPressed = false;

		if (g_Options.Player.GhostPeekMarkKey != 0)
		{
			bool markKeyPressed = (GetAsyncKeyState(g_Options.Player.GhostPeekMarkKey) & 0x8000) != 0;
			if (markKeyPressed && !markKeyWasPressed)
			{
				GhostPeekSetMark();
			}
			markKeyWasPressed = markKeyPressed;
		}

		if (g_Options.Player.GhostPeekReturnKey != 0)
		{
			bool returnKeyPressed = (GetAsyncKeyState(g_Options.Player.GhostPeekReturnKey) & 0x8000) != 0;
			if (returnKeyPressed && !returnKeyWasPressed)
			{
				GhostPeekReturnToMark();
			}
			returnKeyWasPressed = returnKeyPressed;
		}
	}

	// Ghost Peek was never polled anywhere -> keys did nothing. This loop drives it.
	void Player::GhostPeekLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			HandleGhostPeek();
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void Player::FreecamTeleport()
	{
		// Get current camera position and teleport player there
		if (g_Options.Player.Freecam)
		{
			// Get current camera position
			CCamGameplayDirector* pCamGameplayDirector = (CCamGameplayDirector*)M_base::mem::Read<uint64_t>(C_Five.Camera);
			if (pCamGameplayDirector)
			{
				uintptr_t CamFollowPedCamera = M_base::mem::Read<uintptr_t>(pCamGameplayDirector + 0x2C0);
				if (CamFollowPedCamera)
				{
					Vector3D CameraPos = M_base::mem::Read<Vector3D>(CamFollowPedCamera + 0x60);
					// Teleport player to camera position
					TeleportToCoord(CameraPos);
				}
			}
		}
	}

	void Player::FixVehicle()
	{
		CVehicle* CurrentVehicle = C_Five.GetLocalPlayerInfo().Ped->GetLastVehicle();
		CurrentVehicle->FixVehicle();
	}

	void Player::SetFuelToZero()
	{
		auto lplayer = C_Five.GetLocalPlayerInfo();
		if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
		{
			CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
			if (CurrentVehicle)
			{
				uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
				M_base::mem::WriteMemory<float>(vehiclePtr + 0x844, 100.0f);
			}
		}
	}

	void Player::GodModeLoop()
	{
		static bool passiveKeyPressed = false;
		static bool godKeyPressed = false;
		static bool semiGodKeyPressed = false;

		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();

			// Keybinds — classic press-to-toggle (edge detection)
			if (g_Options.Player.PassiveKey != 0)
			{
				bool down = (GetAsyncKeyState(g_Options.Player.PassiveKey) & 0x8000) != 0;
				if (down && !passiveKeyPressed) g_Options.Player.Passive = !g_Options.Player.Passive;
				passiveKeyPressed = down;
			}
			if (g_Options.Player.GodKey != 0)
			{
				bool down = (GetAsyncKeyState(g_Options.Player.GodKey) & 0x8000) != 0;
				if (down && !godKeyPressed) g_Options.Player.God = !g_Options.Player.God;
				godKeyPressed = down;
			}
			if (g_Options.Player.SemiGodKey != 0)
			{
				bool down = (GetAsyncKeyState(g_Options.Player.SemiGodKey) & 0x8000) != 0;
				if (down && !semiGodKeyPressed) g_Options.Player.SemiGod = !g_Options.Player.SemiGod;
				semiGodKeyPressed = down;
			}

			if (g_Options.Player.God) {
				if (lplayer.Ped->GetHealth() < 197) lplayer.Ped->SetHealth(199);
				if (lplayer.Ped->GetArmor() < 97) lplayer.Ped->SetArmor(99);
			}
			else if (g_Options.Player.SemiGod) {
				// Semi-God Mode: Health won't go below threshold (default 70)
				// Armor can be depleted normally, but health is protected
				float currentHealth = lplayer.Ped->GetHealth() - 100.0f; // Subtract 100 because GTA adds 100 to health
				float threshold = g_Options.Player.SemiGodHealth;
				
				if (currentHealth <= threshold && currentHealth > 0) {
					lplayer.Ped->SetHealth(threshold + 100.0f);
				}
			}
			else if (g_Options.Player.Absorption && lplayer.Ped->GetHealth() < 195) {
				lplayer.Ped->SetHealth(199);
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void Player::HealthLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.Health)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();

				if (GetAsyncKeyState(g_Options.Player.HealthKey))
				{
					lplayer.Ped->SetHealth(g_Options.Player.HealthAmmount + 100);
				}
			}

			// Short sleep for fast response time
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void Player::ArmorLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.Armor)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();

				if (GetAsyncKeyState(g_Options.Player.ArmorKey))
				{
					lplayer.Ped->SetArmor(g_Options.Player.ArmorAmmount);
				}
			}

			// Short sleep for fast response time
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	// Slower loops for weapon related features
	// Applies a weapon-stat modifier and restores the ORIGINAL value once the toggle is off
	// (or the weapon changes), so turning it off returns the weapon to default.
	static void ApplyWeaponModifier(bool enabled, uint64_t offset, float value,
		CWeaponInfo*& modWi, float& orig)
	{
		auto lplayer = C_Five.GetLocalPlayerInfo();
		CWeaponManager* wm = lplayer.Ped ? lplayer.Ped->GetWeaponManager() : nullptr;
		CWeaponInfo* wi = wm ? wm->GetWeaponInfo() : nullptr;

		if (enabled && wi)
		{
			if (modWi != wi)   // first enable or weapon switched: restore previous, cache new original
			{
				if (modWi) M_base::mem::WriteMemory<float>(modWi + offset, orig);
				orig = M_base::mem::Read<float>(wi + offset);
				modWi = wi;
			}
			M_base::mem::WriteMemory<float>(wi + offset, value);
		}
		else if (modWi)        // disabled: put the original value back once
		{
			M_base::mem::WriteMemory<float>(modWi + offset, orig);
			modWi = nullptr;
		}
	}

	void Player::NoRecoilLoop()
	{
		static CWeaponInfo* modWi = nullptr; static float orig = 0.f;
		while (!g_Options.General.ShutDown)
		{
			ApplyWeaponModifier(g_Options.Player.NoRecoil, 0x2F4, g_Options.Player.NoRecoilValue, modWi, orig);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::NoReloadLoop()
	{
		static CWeaponInfo* modWi = nullptr; static float orig = 0.f;
		while (!g_Options.General.ShutDown)
		{
			ApplyWeaponModifier(g_Options.Player.NoReload, 0x134, g_Options.Player.NoReloadValue, modWi, orig);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::NoSpreadLoop()
	{
		static CWeaponInfo* modWi = nullptr; static float orig = 0.f;
		while (!g_Options.General.ShutDown)
		{
			ApplyWeaponModifier(g_Options.Player.NoSpread, 0x84, g_Options.Player.NoSpreadValue, modWi, orig);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::FixVehicleKey()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.FixVehicleKey != 0)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();

				CWeaponManager* WeaponManager = lplayer.Ped->GetWeaponManager();
				CWeaponInfo* WeaponInfo = WeaponManager->GetWeaponInfo();

				if (GetAsyncKeyState(g_Options.Player.FixVehicleKey))
				{
					Cheat::Player::FixVehicle();
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void Player::RangeModifyLoop()
	{
		static CWeaponInfo* modWi = nullptr; static float orig = 0.f;
		while (!g_Options.General.ShutDown)
		{
			ApplyWeaponModifier(g_Options.Player.RangeModify, 0x28c, g_Options.Player.RangeModifyValue, modWi, orig);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::DamageBoostLoop()
	{
		// Uses the shared modifier helper so the ORIGINAL weapon damage is cached and
		// restored the moment the toggle is turned off (or the weapon is switched).
		static CWeaponInfo* modWi = nullptr; static float orig = 0.f;
		while (!g_Options.General.ShutDown)
		{
			ApplyWeaponModifier(g_Options.Player.DamageBoost, 0xB0, g_Options.Player.DamageAmount, modWi, orig);
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::SafeDamageBoostLoop()
	{
		static CWeaponInfo* LWInfo = nullptr;
		static uint32_t origVal = 0;   // original weapon value at +0x120, restored on disable
		static bool RapidActive = false;

		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();
			CWeaponManager* WeaponManager = lplayer.Ped ? lplayer.Ped->GetWeaponManager() : nullptr;
			CWeaponInfo* WeaponInfo = WeaponManager ? WeaponManager->GetWeaponInfo() : nullptr;

			// Check for Safe Damage Boost toggle key
			if (GetAsyncKeyState(g_Options.Player.SDamageBoostKey) & 1)
			{
				g_Options.Player.SDamageBoost = !g_Options.Player.SDamageBoost;
			}

			bool SafeDamageBoost = g_Options.Player.SDamageBoost;
			uint32_t SDamageVal = (uint32_t)g_Options.Player.SDamageBoostValue;

			if (SafeDamageBoost && WeaponInfo)
			{
				if (LWInfo != WeaponInfo)   // first enable or weapon switched: restore old, cache new original
				{
					if (LWInfo) M_base::mem::WriteMemory<uint32_t>(LWInfo + 0x120, origVal);
					origVal = M_base::mem::Read<uint32_t>(WeaponInfo + 0x120);
					LWInfo = WeaponInfo;
				}
				M_base::mem::WriteMemory<uint32_t>(WeaponInfo + 0x120, SDamageVal);
				RapidActive = true;
			}
			else if (!SafeDamageBoost && RapidActive && LWInfo)
			{
				M_base::mem::WriteMemory<uint32_t>(LWInfo + 0x120, origVal);   // restore ORIGINAL, not a hardcoded 1
				LWInfo = nullptr;
				RapidActive = false;
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}

	// Vehicle-related features
	void Player::VehicleSpeedLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.VehicleSpeed)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					if (CurrentVehicle)
					{
						CurrentVehicle->set_acceleration(reinterpret_cast<uint64_t>(CurrentVehicle), g_Options.Player.VehicleSpeedAmount);
					}
				}
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::VehicleGodLoop()
	{
		static bool VehGodState = false;

		while (!g_Options.General.ShutDown)
		{
			bool vehgod = g_Options.Player.VehicleGod;

			if (vehgod != VehGodState)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				lplayer.Ped->Godmode(vehgod);
				VehGodState = vehgod;
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	

	// Player movement and camera features
	void Player::PlayerSpeedLoop()
	{
		static bool wasOn = false;
		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();
			if (g_Options.Player.PSpeed)
			{
				if (lplayer.Ped) { auto pi = lplayer.Ped->GetPlayerInfo(); if (pi) pi->SetSpeed(g_Options.Player.PSpeedAmount); }
				wasOn = true;
			}
			else if (wasOn)
			{
				// restore normal move rate once when turned off
				if (lplayer.Ped) { auto pi = lplayer.Ped->GetPlayerInfo(); if (pi) pi->SetSpeed(1.0f); }
				wasOn = false;
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::CamFovLoop()
	{
		static float OriginalCamDistance = 0.0f;
		static uint64_t CurrentModelinfo = 0;

		while (!g_Options.General.ShutDown)
		{
			bool fov = g_Options.Player.CamFovModify;

			if (fov)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				uint64_t ModelInfo = lplayer.Ped->GetModelInfo();

				if (ModelInfo != CurrentModelinfo)
				{
					CurrentModelinfo = ModelInfo;
					OriginalCamDistance = M_base::mem::Read<float>(ModelInfo + 0x38);
				}

				float FinalValue = -5.f + (g_Options.Player.CamFovAmount / 100.f) * (1.f - -5.f);
				M_base::mem::WriteMemory(ModelInfo + 0x38, FinalValue);
			}
			else if (CurrentModelinfo != 0)
			{
				// Restore original value when disabled
				M_base::mem::WriteMemory(CurrentModelinfo + 0x38, OriginalCamDistance);
			}

			// Longer sleep for camera modifications
			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	// Ammo and collision features
	void Player::InfiniteAmmoLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.InfiniteAmmo)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				CWeaponManager* WeaponManager = lplayer.Ped->GetWeaponManager();
				CWeaponInfo* WeaponInfo = WeaponManager->GetWeaponInfo();

				uintptr_t AmmoInfo = M_base::mem::Read<uintptr_t>(WeaponInfo + 0x60);
				uintptr_t AmmoCount = M_base::mem::Read<uintptr_t>(AmmoInfo + 0x8);
				uintptr_t AmmoCount2 = M_base::mem::Read<uintptr_t>(AmmoCount + 0x0);

				M_base::mem::WriteMemory<float>(AmmoCount2 + 0x18, 30);
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}

	void Player::FreecamLoop()
	{
		static bool patch_applied = false;
		static Vector3D savedPlayerPosition = { 0, 0, 0 };
		static bool positionSaved = false;

		static bool freecamKeyWas = false;
		while (!g_Options.General.ShutDown)
		{
			// Keybind — classic press-to-toggle (edge detection), works while menu is closed
			if (g_Options.Player.FreecamKey != 0)
			{
				bool down = (GetAsyncKeyState(g_Options.Player.FreecamKey) & 0x8000) != 0;
				if (down && !freecamKeyWas) g_Options.Player.Freecam = !g_Options.Player.Freecam;
				freecamKeyWas = down;
			}

			if (g_Options.Player.Freecam) {
				// On builds without a hardcoded Freecam_offset, resolve it once at runtime with a
				// specific 30-byte signature of the three camera-position writes (movss [rdi+0x60/64/68]).
				// The 6 wildcard bytes cover the per-build load operands. If not found, offset stays 0
				// and the patch below is skipped (freecam just won't detach — never a bad write).
				if (Freecam_offset == 0)
				{
					static bool freecamScanned = false;
					if (!freecamScanned)
					{
						freecamScanned = true;
						uint64_t base = C_Five.GetModuleBase();
						// Exact freecam camera-write signature (the same bytes the restore constants use):
						// movss [rdi+0x60],xmm0 / movss [rdi+0x64],xmm1 / movss [rdi+0x68],xmm0 with the
						// interleaved loads wildcarded. Fixed disps + xmm regs keep it specific enough to
						// avoid the false matches a looser pattern hits; a match sits exactly at +0/+0xB/+0x16
						// so the existing patch bytes line up. (3258/3407/3570 are hardcoded; this covers the rest.)
						uint64_t found = M_base::mem::FindPattern(base,
							"F3 0F 11 47 60 F3 0F 10 ?? ?? ?? F3 0F 11 4F 64 F3 0F 10 ?? ?? ?? F3 0F 11 47 68 F3 0F 11");
						if (found > base)
						{
							Freecam_offset = found - base;
							PatternScanLog::Log("Freecam offset resolved at 0x" + std::to_string(Freecam_offset));
						}
						else
						{
							PatternScanLog::Log("Freecam pattern NOT found - offset unresolved");
						}
					}
				}

				// Save player position when entering freecam (only once)
				if (!positionSaved)
				{
					auto localPlayer = C_Five.GetLocalPlayerInfo();
					if (localPlayer.Ped)
					{
						savedPlayerPosition = localPlayer.Ped->GetCoordinate();
						positionSaved = true;
					}
				}

				uintptr_t ModelInfo = C_Five.GetLocalPlayerInfo().Ped->GetModelInfo();
				if (ModelInfo)
				{
					M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, 0.f);
				}
				// Freecam_offset is only defined for some game builds; if it's 0 the patch
				// would corrupt the image base, so skip it (freecam camera won't detach on
				// unsupported builds until the correct offset is added for that build).
				if (!patch_applied && Freecam_offset != 0)
				{
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset, 0x100FF39090909090);
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset + 0xB, 0x100FF39090909090);
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset + 0x16, 0x100FF39090909090);
					patch_applied = true;
				}
				CCamGameplayDirector* pCamGameplayDirector = (CCamGameplayDirector*)M_base::mem::Read<uint64_t>(C_Five.Camera);

				uintptr_t CamFollowPedCamera = M_base::mem::Read<uintptr_t>(pCamGameplayDirector + 0x2C0);
				if (!CamFollowPedCamera)
				{
					std::this_thread::sleep_for(std::chrono::milliseconds(100));
					continue;
				}

				Vector3D CameraPos = M_base::mem::Read<Vector3D>(CamFollowPedCamera + 0x60);
				Vector3D ViewAngles = M_base::mem::Read<Vector3D>(CamFollowPedCamera + 0x40);


				float cam_speed = g_Options.Player.FreecamSpeed;

				if (GetAsyncKeyState(VK_SHIFT) & 0x8000)
				{
					cam_speed *= 5.f; // Boost
				}

				Vector3D NewPos = CameraPos;

				if (GetAsyncKeyState('W') & 0x8000)
				{
					NewPos.x += ViewAngles.x * cam_speed;
					NewPos.y += ViewAngles.y * cam_speed;
					NewPos.z += ViewAngles.z * cam_speed;
				}
				if (GetAsyncKeyState('S') & 0x8000)
				{
					NewPos.x -= ViewAngles.x * cam_speed;
					NewPos.y -= ViewAngles.y * cam_speed;
					NewPos.z -= ViewAngles.z * cam_speed;
				}

				if (GetAsyncKeyState(VK_SPACE) & 0x8000)
				{
					NewPos.z += cam_speed;
				}
				if (GetAsyncKeyState(VK_LCONTROL) & 0x8000)
				{
					NewPos.z -= cam_speed;
				}

				M_base::mem::WriteMemory<Vector3D>(CamFollowPedCamera + 0x60, NewPos);
			}
			else
			{
				if (patch_applied)
				{
					// Restore original memory patches
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset, 0x100FF36047110FF3);
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset + 0xB, 0x100FF3644F110FF3);
					M_base::mem::WriteMemory<uintptr_t>(C_Five.GetModuleBase() + Freecam_offset + 0x16, 0x110FF36847110FF3);
					patch_applied = false;
					
					uintptr_t ModelInfo = C_Five.GetLocalPlayerInfo().Ped->GetModelInfo();
					if (ModelInfo)
					{
						M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, 1.f);
					}

					// Teleport player back to saved position when exiting freecam
					if (positionSaved)
					{
						TeleportToCoord(savedPlayerPosition);
						positionSaved = false;
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}


	void Player::NocollisionLoop()
	{
		static bool nocollisionActive = false;

		while (!g_Options.General.ShutDown)
		{
			// Noclip also needs collision off (ported 1:1 from MYNX)
			bool nocollision = g_Options.Player.Nocollision || g_Options.Player.noclip;
			auto lplayer = C_Five.GetLocalPlayerInfo();

			if (nocollision)
			{
				uint64_t navig = M_base::mem::Read<uint64_t>(lplayer.Ped + 0x30);

				uint64_t addr1 = navig;
				if (addr1) {
					uint64_t addr2 = M_base::mem::Read<uint64_t>(addr1 + 0x10);
					if (addr2) {
						uint64_t addr3 = M_base::mem::Read<uint64_t>(addr2 + 0x20);
						if (addr3) {
							uint64_t addr4 = M_base::mem::Read<uint64_t>(addr3 + 0x70);
							if (addr4) {
								uint64_t addr5 = M_base::mem::Read<uint64_t>(addr4 + 0x0);
								if (addr5) {
									M_base::mem::WriteMemory<float>(addr5 + 0x2C, -1.0f);
									nocollisionActive = true;
								}
							}
						}
					}
				}
			}
			else if (!nocollision && nocollisionActive)
			{
				uint64_t navig = M_base::mem::Read<uint64_t>(lplayer.Ped + 0x30);

				uint64_t addr1 = navig;
				if (addr1) {
					uint64_t addr2 = M_base::mem::Read<uint64_t>(addr1 + 0x10);
					if (addr2) {
						uint64_t addr3 = M_base::mem::Read<uint64_t>(addr2 + 0x20);
						if (addr3) {
							uint64_t addr4 = M_base::mem::Read<uint64_t>(addr3 + 0x70);
							if (addr4) {
								uint64_t addr5 = M_base::mem::Read<uint64_t>(addr4 + 0x0);
								if (addr5) {
									M_base::mem::WriteMemory<float>(addr5 + 0x2C, 0.25f);
									nocollisionActive = false;
								}
							}
						}
					}
				}
			}

			// Medium sleep time
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}


	void Player::AbsorptionLoop()
	{
		static float previousHealth = 0.0f;
		static float previousArmor = 0.0f;
		static bool initialized = false;

		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.Absorption)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				float currentHealth = lplayer.Ped->GetHealth();
				float currentArmor = lplayer.Ped->GetArmor();
				float maxHealth = lplayer.Ped->GetMaxHealth();
				float maxArmor = 100.0f; // Standard max armor

				// Initialize previous values on first run or if player respawned
				if (!initialized || currentHealth > previousHealth + 50.0f)
				{
					previousHealth = currentHealth;
					previousArmor = currentArmor;
					initialized = true;
				}
				else
				{
					bool damageTaken = false;
					float totalAbsorption = 0.0f;

					// Check for health damage
					if (currentHealth < previousHealth)
					{
						float healthDamage = previousHealth - currentHealth;
						float healthAbsorption = healthDamage * (g_Options.Player.AbsorptionPercent / 100.0f);

						float newHealth = currentHealth + healthAbsorption;
						if (newHealth > maxHealth)
						{
							newHealth = maxHealth;
						}

						lplayer.Ped->SetHealth(newHealth);
						previousHealth = newHealth;
						totalAbsorption += healthAbsorption;
						damageTaken = true;
					}

					// Check for armor damage
					if (currentArmor < previousArmor)
					{
						float armorDamage = previousArmor - currentArmor;
						float armorAbsorption = armorDamage * (g_Options.Player.AbsorptionPercent / 100.0f);

						float newArmor = currentArmor + armorAbsorption;
						if (newArmor > maxArmor)
						{
							newArmor = maxArmor;
						}

						lplayer.Ped->SetArmor(newArmor);
						previousArmor = newArmor;
						totalAbsorption += armorAbsorption;
						damageTaken = true;
					}

					// Update previous values if no damage was taken
					if (!damageTaken)
					{
						previousHealth = currentHealth;
						previousArmor = currentArmor;
					}
				}
			}
			else if (initialized)
			{
				// Reset when absorption is disabled
				auto lplayer = C_Five.GetLocalPlayerInfo();
				previousHealth = lplayer.Ped->GetHealth();
				previousArmor = lplayer.Ped->GetArmor();
			}

			// Fast response time for damage detection
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
	}
	
	void Player::test()
	{
		 while (!g_Options.General.ShutDown)
        {
            if (g_Options.Exploit.Test)
            {
                auto lplayer = C_Five.GetLocalPlayerInfo();
                if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
                {
                    CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
                    if (CurrentVehicle)
                    {
                        uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
                        M_base::mem::WriteMemory<float>(vehiclePtr + 0x80, g_Options.Exploit.TestValue);
                    }
                }
            }

            // Medium sleep time
            std::this_thread::sleep_for(std::chrono::milliseconds(200));
        }
	}

	void Player::FreecamTeleportLoop()
	{
		// Drives ALL three teleport keybinds (waypoint / saved / freecam) with proper
		// edge detection. None of these except freecam were polled before.
		static bool wpWas = false, svWas = false, fcWas = false;
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.TeleportToWaypointKey != 0)
			{
				bool d = (GetAsyncKeyState(g_Options.Player.TeleportToWaypointKey) & 0x8000) != 0;
				if (d && !wpWas) Cheat::Player::TeleportToWaypoint();
				wpWas = d;
			}
			if (g_Options.Player.TeleportToSavedLocationKey != 0)
			{
				bool d = (GetAsyncKeyState(g_Options.Player.TeleportToSavedLocationKey) & 0x8000) != 0;
				if (d && !svWas) Cheat::Player::TeleportToSavedLocation();
				svWas = d;
			}
			if (g_Options.Player.FreecamTeleportKey != 0)
			{
				bool d = (GetAsyncKeyState(g_Options.Player.FreecamTeleportKey) & 0x8000) != 0;
				if (d && !fcWas) Cheat::Player::FreecamTeleport();
				fcWas = d;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void Player::RocketBoostLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.rocket_boost)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					if (CurrentVehicle)
					{
						uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
						uint64_t model_info = M_base::mem::Read<uint64_t>(vehiclePtr + 0x20);
						if (model_info)
						{
							M_base::mem::WriteMemory<uint16_t>(model_info + 0x058B, 1 << 6);
						}
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::JumpLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.jump)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					if (CurrentVehicle)
					{
						uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
						uint64_t model_info = M_base::mem::Read<uint64_t>(vehiclePtr + 0x20);
						if (model_info)
						{
							M_base::mem::WriteMemory<uint16_t>(model_info + 0x058B, 1 << 5);
						}
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::ParachuteLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.parachute)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					if (CurrentVehicle)
					{
						uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
						uint64_t model_info = M_base::mem::Read<uint64_t>(vehiclePtr + 0x20);
						if (model_info)
						{
							M_base::mem::WriteMemory<uint16_t>(model_info + 0x058B, 1 << 8);
						}
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}
	void Player::VehicleGravityLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.Gravity)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					if (CurrentVehicle)
					{
						uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
						M_base::mem::WriteMemory<float>(vehiclePtr + gravity, g_Options.Exploit.GravityCount);
					}
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	void Player::SetFuelToZeroLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.FuelToZeroKey != 0)
			{
				if (GetAsyncKeyState(g_Options.Exploit.FuelToZeroKey) & 1)
				{
					Cheat::Player::SetFuelToZero();
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	void Player::ReduceEngineHealth()
	{
		auto lplayer = C_Five.GetLocalPlayerInfo();
		CVehicle* CurrentVehicle = nullptr;
		if (lplayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
		{
			CurrentVehicle = lplayer.Ped->GetLastVehicle();
		}
		if (CurrentVehicle)
		{
			uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
			M_base::mem::WriteMemory<float>(vehiclePtr + 0x910, 1000.0f);
		}
	}

	void Player::NoFallDamageLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Player.NoFallDamage)
			{
				auto lplayer = C_Five.GetLocalPlayerInfo();
				if (lplayer.Ped)
				{
					uintptr_t pedPtr = reinterpret_cast<uintptr_t>(lplayer.Ped);
					M_base::mem::WriteMemory<float>(pedPtr + 0x15C0, -110.0f);
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(200));
		}
	}

	// Shrink — scales the local player's model down via the ModelInfo scale multiplier
	// (ModelInfo + 0x2C). 1.0 = normal size, lower = smaller. Restores to 1.0 when disabled.
	// NOTE: ModelInfo is the shared model archetype, so peds using the exact same model
	// can be affected too; this is the only ped-scale primitive available from an external base.
	void Player::ShrinkLoop()
	{
		static bool applied = false;
		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();

			if (g_Options.Player.Shrink && lplayer.Ped)
			{
				uint64_t ModelInfo = lplayer.Ped->GetModelInfo();
				if (ModelInfo)
				{
					float amount = g_Options.Player.ShrinkAmount;
					if (amount < 0.05f) amount = 0.05f;
					if (amount > 1.0f)  amount = 1.0f;
					M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, amount);
					applied = true;
				}
			}
			else if (applied && lplayer.Ped)
			{
				uint64_t ModelInfo = lplayer.Ped->GetModelInfo();
				if (ModelInfo)
					M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, 1.0f);
				applied = false;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	// Noclip — ported 1:1 from MYNX (the version that "works full"). The key difference from the
	// old buggy version: it keeps a PERSISTENT accumulator position (noclipPos) instead of re-reading
	// the ped coordinate every tick (which fought gravity/physics and caused stutter/falling), and it
	// zeroes the model gravity multiplier (ModelInfo + 0x2C) while active. Collision-off is driven by
	// NocollisionLoop (which now also triggers on noclip). Classic press-to-toggle keybind on top.
	void Player::NoclipLoop()
	{
		static Vector3D noclipPos;
		static bool wasNoclip = false;
		static bool keyWas = false;

		while (!g_Options.General.ShutDown)
		{
			// Keybind — classic press-to-toggle (edge detection)
			if (g_Options.Player.NoclipKey != 0)
			{
				bool down = (GetAsyncKeyState(g_Options.Player.NoclipKey) & 0x8000) != 0;
				if (down && !keyWas) g_Options.Player.noclip = !g_Options.Player.noclip;
				keyWas = down;
			}

			if (g_Options.Player.noclip)
			{
				auto localPlayer = C_Five.GetLocalPlayerInfo();
				if (localPlayer.Ped)
				{
					uint64_t Object = (uint64_t)localPlayer.Ped;
					uint64_t Navigation = localPlayer.Ped->GetNavigation();
					uint64_t ModelInfo = localPlayer.Ped->GetModelInfo();

					if (localPlayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
					{
						CVehicle* Vehicle = localPlayer.Ped->GetLastVehicle();
						if (Vehicle)
						{
							Object = (uint64_t)Vehicle;
							Navigation = Vehicle->GetNavigation();
							ModelInfo = Vehicle->GetModelInfo();
						}
					}

					if (!wasNoclip)
					{
						noclipPos = localPlayer.Ped->GetCoordinate();
						wasNoclip = true;
					}

					// Zero gravity/air resistance for the model to prevent falling
					if (ModelInfo)
						M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, 0.0f);

					CCamGameplayDirector* pCamGameplayDirector = (CCamGameplayDirector*)M_base::mem::Read<uint64_t>(C_Five.Camera);
					uintptr_t CamFollowPedCamera = M_base::mem::Read<uintptr_t>(pCamGameplayDirector + 0x2C0);

					if (CamFollowPedCamera)
					{
						Vector3D forward = M_base::mem::Read<Vector3D>(CamFollowPedCamera + 0x40);
						float speed = g_Options.Player.noclipspeed / 50.0f; // Scale speed for 10ms loop

						if (GetAsyncKeyState(VK_SHIFT) & 0x8000) speed *= 5.0f;

						if (GetAsyncKeyState('W') & 0x8000) noclipPos = noclipPos + (forward * speed);
						if (GetAsyncKeyState('S') & 0x8000) noclipPos = noclipPos - (forward * speed);

						if (GetAsyncKeyState('A') & 0x8000 || GetAsyncKeyState('D') & 0x8000)
						{
							Vector3D up(0, 0, 1);
							Vector3D right = forward.Cross(up);
							right.Normalize();

							if (GetAsyncKeyState('A') & 0x8000) noclipPos = noclipPos - (right * speed);
							if (GetAsyncKeyState('D') & 0x8000) noclipPos = noclipPos + (right * speed);
						}

						if (GetAsyncKeyState(VK_SPACE) & 0x8000) noclipPos.z += speed;
						if (GetAsyncKeyState(VK_LCONTROL) & 0x8000) noclipPos.z -= speed;

						// Constantly write position to game offsets for butter-smooth movement
						M_base::mem::WriteMemory(Object + 0x90, noclipPos);
						M_base::mem::WriteMemory(Navigation + 0x50, noclipPos);
					}
				}
			}
			else if (wasNoclip)
			{
				// Restore default gravity multiplier when noclip is turned off
				auto localPlayer = C_Five.GetLocalPlayerInfo();
				if (localPlayer.Ped)
				{
					uint64_t ModelInfo = localPlayer.Ped->GetModelInfo();
					if (ModelInfo)
						M_base::mem::WriteMemory<float>(ModelInfo + 0x2C, 1.0f);
				}
				wasNoclip = false;
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(10));
		}
	}

	// Strafe macro — ported 1:1 from MYNX (ProcessStrafeMacro). While the bind key is HELD it cycles
	// W -> A -> S -> D via SendInput scancodes; StrafeMacroDelay controls the switch speed. Requires a
	// bind key set (matches MYNX: no bind -> macro does nothing). Releases all keys on release/disable.
	void Player::StrafeMacroLoop()
	{
		static int strafeIndex = 0; // 0=W, 1=A, 2=S, 3=D
		static auto lastStrafeTime = std::chrono::steady_clock::now();
		static WORD strafeScanCodes[] = { 0x11, 0x1E, 0x1F, 0x20 }; // W, A, S, D
		static bool strafeWasActive = false;

		auto releaseAllStrafe = []() {
			INPUT input = { 0 }; input.type = INPUT_KEYBOARD;
			input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
			WORD codes[] = { 0x11, 0x1E, 0x1F, 0x20 };
			for (int i = 0; i < 4; i++) { input.ki.wScan = codes[i]; SendInput(1, &input, sizeof(INPUT)); }
		};

		while (!g_Options.General.ShutDown)
		{
			// Enabled globally (checkbox) AND a key is bound AND that key is held
			if (g_Options.Exploit.StrafeMacro && g_Options.Exploit.StrafeMacroBind != 0)
			{
				bool keyHeld = (GetAsyncKeyState(g_Options.Exploit.StrafeMacroBind) & 0x8000) != 0;

				if (keyHeld)
				{
					auto now = std::chrono::steady_clock::now();
					auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - lastStrafeTime).count();

					if (elapsed >= g_Options.Exploit.StrafeMacroDelay)
					{
						// Release previous key
						INPUT input = { 0 }; input.type = INPUT_KEYBOARD;
						input.ki.dwFlags = KEYEVENTF_SCANCODE | KEYEVENTF_KEYUP;
						input.ki.wScan = strafeScanCodes[strafeIndex];
						SendInput(1, &input, sizeof(INPUT));

						// Move to next key in the W-A-S-D cycle
						strafeIndex++;
						if (strafeIndex > 3) strafeIndex = 0;

						// Press new key
						input.ki.dwFlags = KEYEVENTF_SCANCODE;
						input.ki.wScan = strafeScanCodes[strafeIndex];
						SendInput(1, &input, sizeof(INPUT));

						lastStrafeTime = now;
					}
					strafeWasActive = true;
				}
				else if (strafeWasActive)
				{
					releaseAllStrafe();
					strafeWasActive = false;
					strafeIndex = 0;
				}
			}
			else if (strafeWasActive)
			{
				releaseAllStrafe();
				strafeWasActive = false;
				strafeIndex = 0;
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(1));
		}
	}

	void Player::SuperHandlingLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			if (g_Options.Exploit.SuperHandling)
			{
				auto localPlayer = C_Five.GetLocalPlayerInfo();
				if (localPlayer.Ped && localPlayer.Ped->HasConfigFlag(CPED_CONFIG_FLAG_InVehicle))
				{
					auto lplayer = C_Five.GetLocalPlayerInfo();
					CVehicle* CurrentVehicle = lplayer.Ped->GetLastVehicle();
					uintptr_t vehiclePtr = reinterpret_cast<uintptr_t>(CurrentVehicle);
					CVehicle* vehicle = localPlayer.Ped->GetLastVehicle();
					if (vehicle)
					{
						// Set handling value
						M_base::mem::WriteMemory<float>(vehicle->GetHandling() + 0x4C, 50.f);
						// Set gravity to 25
						M_base::mem::WriteMemory<float>(vehiclePtr + gravity, 25.f);
					}
				}
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(300));
		}
	}

	void Player::StealCarLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();
			if (lplayer.Ped)
			{
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_NotAllowedToJackAnyPlayers, !g_Options.Exploit.stealcar);
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_PlayerCanJackFriendlyPlayers, g_Options.Exploit.stealcar);
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_WillJackAnyPlayer, g_Options.Exploit.stealcar);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(500));
		}
	}
	void Player::KediciShadowsLoop()
	{
		static bool invisibleKeyPressed = false;
		static bool bWasInvisible = false;

		while (!g_Options.General.ShutDown)
		{
			// Keybind — classic press-to-toggle (edge detection)
			if (g_Options.Exploit.KediciShadowKey != 0)
			{
				bool currentKeyState = (GetAsyncKeyState(g_Options.Exploit.KediciShadowKey) & 0x8000) != 0;
				if (currentKeyState && !invisibleKeyPressed)
					g_Options.Exploit.KediciShadow = !g_Options.Exploit.KediciShadow;
				invisibleKeyPressed = currentKeyState;
			}

			auto lpPlayer = C_Five.GetLocalPlayerInfo();
			
			// Only write when state changes
			if (g_Options.Exploit.KediciShadow && !bWasInvisible && lpPlayer.Ped)
			{
				uint64_t moduleBase = C_Five.GetModuleBase();
				if (moduleBase && Cheat::invisible)
				{
					M_base::mem::WriteMemory<uintptr_t>(moduleBase + Cheat::invisible, 0x18598B4822100FF3);
					bWasInvisible = true;
				}
			}
			else if (!g_Options.Exploit.KediciShadow && bWasInvisible && lpPlayer.Ped)
			{
				uint64_t moduleBase = C_Five.GetModuleBase();
				if (moduleBase && Cheat::invisible)
				{
					M_base::mem::WriteMemory<uintptr_t>(moduleBase + Cheat::invisible, 0x18598B480A100FF3);
					bWasInvisible = false;
				}
			}

			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	// Immortality ("no one can shoot at you") — flips the ped's friendly-targeting config flags.
	// The Passive keybind toggle is handled in GodModeLoop; this loop only applies the state.
	void Player::ImmortalityLoop()
	{
		while (!g_Options.General.ShutDown)
		{
			auto lplayer = C_Five.GetLocalPlayerInfo();
			if (lplayer.Ped)
			{
				bool on = g_Options.Player.Passive;
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_DisablePlayerLockon, on);
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_AllowPlayerLockOnIfFriendly, on);
				lplayer.Ped->SetConfigFlag(CPED_CONFIG_FLAG_TreatAsFriendlyForTargetingAndDamage, on);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(100));
		}
	}

	void Player::StartFeatureThreads()
	{
		std::thread godModeThread(GodModeLoop);
		godModeThread.detach();

		std::thread immortalityThread(ImmortalityLoop);
		immortalityThread.detach();

		std::thread healthThread(HealthLoop);
		healthThread.detach();

		std::thread armorThread(ArmorLoop);
		armorThread.detach();

		std::thread absorptionThread(AbsorptionLoop);
		absorptionThread.detach();

		std::thread noRecoilThread(NoRecoilLoop);
		noRecoilThread.detach();

		std::thread noReloadThread(NoReloadLoop);
		noReloadThread.detach();

		std::thread FixVehicleThread(FixVehicleKey);
		FixVehicleThread.detach();

		std::thread noSpreadThread(NoSpreadLoop);
		noSpreadThread.detach();

		std::thread rangeModifyThread(RangeModifyLoop);
		rangeModifyThread.detach();

		std::thread damageBoostThread(DamageBoostLoop);
		damageBoostThread.detach();

		std::thread SafeDamageBoostThread(SafeDamageBoostLoop);
		SafeDamageBoostThread.detach();

		std::thread vehicleSpeedThread(VehicleSpeedLoop);
		vehicleSpeedThread.detach();

		std::thread vehicleGodThread(VehicleGodLoop);
		vehicleGodThread.detach();

	
		std::thread playerSpeedThread(PlayerSpeedLoop);
		playerSpeedThread.detach();

		std::thread camFovThread(CamFovLoop);
		camFovThread.detach();

		std::thread infiniteAmmoThread(InfiniteAmmoLoop);
		infiniteAmmoThread.detach();

		std::thread freecamThread(FreecamLoop);
		freecamThread.detach();

		std::thread freecamTeleportThread(FreecamTeleportLoop);
		freecamTeleportThread.detach();

		std::thread nocollisionThread(NocollisionLoop);
		nocollisionThread.detach();

		std::thread rocketBoostThread(RocketBoostLoop);
		rocketBoostThread.detach();

		std::thread jumpThread(JumpLoop);
		jumpThread.detach();

		std::thread parachuteThread(ParachuteLoop);
		parachuteThread.detach();
		std::thread vehicleGravityThread(VehicleGravityLoop);
		vehicleGravityThread.detach();

		std::thread setFuelToZeroThread(SetFuelToZeroLoop);
		setFuelToZeroThread.detach();

		std::thread superHandlingThread(SuperHandlingLoop);
		superHandlingThread.detach();

		std::thread stealCarThread(StealCarLoop);
		stealCarThread.detach();

		std::thread noFallDamageThread(NoFallDamageLoop);
		noFallDamageThread.detach();

		std::thread shrinkThread(ShrinkLoop);
		shrinkThread.detach();

		std::thread noclipThread(NoclipLoop);
		noclipThread.detach();

		std::thread ghostPeekThread(GhostPeekLoop);
		ghostPeekThread.detach();

		std::thread strafeMacroThread(StrafeMacroLoop);
		strafeMacroThread.detach();

		std::thread KediciShadowsThread(KediciShadowsLoop);
		KediciShadowsThread.detach();

		std::thread testThread(test);
		testThread.detach();
		
	}
}
