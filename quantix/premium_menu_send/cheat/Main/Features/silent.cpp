#include <imgui.h>
#include "silent.hpp"
#include "../Options.hpp"
#include "../../../framework/settings/variables.h"
#include "../../Utils/Utilities/Memory.hpp"
#include <fstream>
#include <ctime>
#include <mutex>
#include <cmath>

// WaveShield Debug Log
namespace WaveShieldLog
{
	static void Log(const std::string& message)
	{
		std::ofstream logFile("waveshield_debug.log", std::ios::app);
		if (logFile.is_open())
		{
			time_t now = time(0);
			char timeStr[26];
			ctime_s(timeStr, sizeof(timeStr), &now);
			timeStr[24] = '\0'; // Remove newline
			logFile << "[" << timeStr << "] " << message << std::endl;
			logFile.close();
		}
	}
}

// WaveShield Bypass - Unlimited FOV Silent Aim
namespace WaveShieldBypass
{
	static bool weaponHooked = false;
	static uint64_t originalBytes = 0;
	static uint32_t originalBytes2 = 0;
	static uint64_t hookAddress = 0;
	
	// Code cave that returns a fake weapon hash (bypasses weapon check)
	// mov eax, 0xA2719263 (fake weapon hash - WEAPON_PISTOL)
	// ret
	static const uint8_t bypass_code[] = {
		0xB8, 0x63, 0x92, 0x71, 0xA2,  // mov eax, 0xA2719263
		0xC3                            // ret
	};
	
	void HookWeaponCheck()
	{
		if (weaponHooked)
		{
			WaveShieldLog::Log("WaveShield Hook: Already hooked, skipping");
			return;
		}
			
		uint64_t moduleBase = Cheat::C_Five.GetModuleBase();
		if (moduleBase == 0)
		{
			WaveShieldLog::Log("WaveShield Hook: FAILED - Module base is 0");
			return;
		}
		
		WaveShieldLog::Log("WaveShield Hook: Module base found at 0x" + std::to_string(moduleBase));
		
		// Use a code cave location near the module base
		uint64_t caveAddr = moduleBase + 0x600;
		
		int buildNumber = Cheat::C_Five.buildNumber;
		WaveShieldLog::Log("WaveShield Hook: FiveM Build Number = " + std::to_string(buildNumber));
		
		// Pattern Scanner - WaveShield bypass pattern
		// Pattern: 48 83 EC 28 E8 ? ? ? ? 48 85 C0 74 11 48 8B 80 B8 10 00 00
		const std::string waveshieldPattern = "48 83 EC 28 E8 ? ? ? ? 48 85 C0 74 11 48 8B 80 B8 10 00 00";
		
		WaveShieldLog::Log("WaveShield Hook: Scanning for pattern: " + waveshieldPattern);
		
		hookAddress = M_base::mem::FindPattern(moduleBase, waveshieldPattern);
		
		if (hookAddress != 0)
		{
			WaveShieldLog::Log("WaveShield Hook: Pattern found at 0x" + std::to_string(hookAddress));
		}
		else
		{
			WaveShieldLog::Log("WaveShield Hook: Pattern not found, trying fallback offsets...");
			
			// Fallback to hardcoded offsets if pattern not found
			switch (buildNumber)
			{
			case 2372:
				hookAddress = moduleBase + 0xFE7DA4;
				break;
			case 2612:
				hookAddress = moduleBase + 0xFF340C;
				break;
			case 2699:
				hookAddress = moduleBase + 0xFF9D90;
				break;
			case 2802:
				hookAddress = moduleBase + 0xFE2154;
				break;
			case 2944:
				hookAddress = moduleBase + 0xFF716C;
				break;
			case 3095:
				hookAddress = moduleBase + 0xFCE6EC;
				break;
			case 3258:
				hookAddress = moduleBase + 0xFF1B40;
				break;
			case 3323:
				hookAddress = moduleBase + 0x1003F80;
				break;
			case 3407:
				hookAddress = moduleBase + 0x100F5A4;
				break;
			case 3570:
				hookAddress = moduleBase + 0x101A660;
				break;
			default:
				hookAddress = moduleBase + 0x102D550;
				WaveShieldLog::Log("WaveShield Hook: WARNING - Unknown build, using default offset 0x102D550");
				break;
			}
			WaveShieldLog::Log("WaveShield Hook: Using fallback offset, address = 0x" + std::to_string(hookAddress));
		}
		
		if (hookAddress == 0)
		{
			WaveShieldLog::Log("WaveShield Hook: FAILED - Hook address is 0");
			return;
		}
		
		// Save original bytes
		originalBytes = M_base::mem::Read<uint64_t>(hookAddress);
		originalBytes2 = M_base::mem::Read<uint32_t>(hookAddress + 8);
		
		// Write bypass code to code cave
		M_base::mem::WriteProcessMemoryImpl(caveAddr, (void*)bypass_code, sizeof(bypass_code));
		
		// Create jump to code cave (12 bytes: mov rax, addr; jmp rax)
		uint8_t jumpCode[12] = {
			0x48, 0xB8,                                     // mov rax,
			0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // address (8 bytes)
			0xFF, 0xE0                                      // jmp rax
		};
		
		memcpy(&jumpCode[2], &caveAddr, sizeof(uint64_t));
		M_base::mem::WriteProcessMemoryImpl(hookAddress, jumpCode, sizeof(jumpCode));
		
		weaponHooked = true;
		WaveShieldLog::Log("WaveShield Hook: SUCCESS - Hook installed for build " + std::to_string(buildNumber));
	}
	
	void UnhookWeaponCheck()
	{
		if (!weaponHooked || hookAddress == 0)
		{
			WaveShieldLog::Log("WaveShield Unhook: Not hooked, skipping");
			return;
		}
		
		// Restore original bytes
		M_base::mem::WriteMemory<uint64_t>(hookAddress, originalBytes);
		M_base::mem::WriteMemory<uint32_t>(hookAddress + 8, originalBytes2);
		
		weaponHooked = false;
		hookAddress = 0;
		WaveShieldLog::Log("WaveShield Unhook: SUCCESS - Hook removed");
	}
	
	bool IsHooked()
	{
		return weaponHooked;
	}
}

namespace Cheat
{
	static std::vector<int> GetSelectedSilentBones()
	{
		std::vector<int> selected;
		// Use bone array from g_Options.Silent.bones[] (synced with preview)
		for (int i = 0; i < 14; i++)
		{
			if (g_Options.Silent.bones[i])
				selected.push_back(i);
		}
		return selected;
	}

	static Vector3D GetSilentBonePosition(const Entity& entity, int boneIndex)
	{
		switch (boneIndex)
		{
		case 0:  return entity.HeadVEC;                                 // Head
		case 1:  return entity.NeckVEC;                                 // Neck
		case 2:  return entity.ChestVEC;                                // Chest
		case 3:  return C_Five.GetBonePosVec3(entity, SKEL_L_UpperArm); // L Shoulder
		case 4:  return C_Five.GetBonePosVec3(entity, SKEL_L_Forearm);  // L Elbow
		case 5:  return C_Five.GetBonePosVec3(entity, SKEL_L_Hand);     // L Hand
		case 6:  return C_Five.GetBonePosVec3(entity, SKEL_R_UpperArm); // R Shoulder
		case 7:  return C_Five.GetBonePosVec3(entity, SKEL_R_Forearm);  // R Elbow
		case 8:  return C_Five.GetBonePosVec3(entity, SKEL_R_Hand);     // R Hand
		case 9:  return C_Five.GetBonePosVec3(entity, SKEL_Pelvis);     // Pelvis
		case 10: { Vector3D v = entity.L_CalfVEC; if (v.x == 0 && v.y == 0 && v.z == 0) v = C_Five.GetBonePosVec3(entity, SKEL_L_Calf); return v; } // L Knee
		case 11: return entity.LeftFootVEC;                            // L Foot (always cached)
		case 12: { Vector3D v = entity.R_CalfVEC; if (v.x == 0 && v.y == 0 && v.z == 0) v = C_Five.GetBonePosVec3(entity, SKEL_R_Calf); return v; } // R Knee
		case 13: return entity.RightFootVEC;                           // R Foot (always cached)
		default: return Vector3D(0, 0, 0);                             // never force head
		}
	}

	static Vector3D GetClosestSelectedSilentBone(const Entity& entity, const std::vector<int>& selectedBones)
	{
		if (selectedBones.empty()) return entity.HeadVEC;
		if (selectedBones.size() == 1) return GetSilentBonePosition(entity, selectedBones[0]);
		Vector3D closestPos = { 0, 0, 0 };
		float closestDist = FLT_MAX;
		int screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
		int screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2;
		for (int boneIndex : selectedBones) {
			Vector3D bonePos = GetSilentBonePosition(entity, boneIndex);
			if (bonePos.x == 0 && bonePos.y == 0 && bonePos.z == 0) continue;
			ImVec2 screenPos = C_Five.WorldToScreen(bonePos);
			if (screenPos.x != 0 || screenPos.y != 0) {
				float dist = sqrtf((screenPos.x - screenCenterX) * (screenPos.x - screenCenterX) + (screenPos.y - screenCenterY) * (screenPos.y - screenCenterY));
				if (dist < closestDist) { closestDist = dist; closestPos = bonePos; }
			}
		}
		if (closestPos.x == 0 && closestPos.y == 0 && closestPos.z == 0) closestPos = GetSilentBonePosition(entity, selectedBones[0]);
		return closestPos;
	}

	void RestoreSilent()
	{
		static const std::vector<uint8_t> ReWriteTable = { 0xF3, 0x41, 0x0F, 0x10, 0x19, 0xF3, 0x41, 0x0F, 0x10, 0x41, 0x04, 0xF3, 0x41, 0x0F, 0x10, 0x51, 0x08 };
		static const std::vector<uint8_t> AngleReWriteTable = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
		M_base::mem::WriteProcessMemoryImpl(C_Five.GetHandleBullet(), (void*)ReWriteTable.data(), ReWriteTable.size());
		M_base::mem::WriteProcessMemoryImpl(C_Five.GetModuleBase() + 0x34E, (void*)AngleReWriteTable.data(), AngleReWriteTable.size());
	}

	void ApplySilent(const Vector3D& EndBulletPos)
	{
		static uint64_t HandleBulletAddress = C_Five.GetHandleBullet();
		static uint64_t AllocPtr = C_Five.GetModuleBase() + 0x34E;
		static auto CalculateRelativeOffset = [](uint64_t CurrentAddress, uint64_t TargetAddress, int Offset = 5) {
			return static_cast<uint32_t>(static_cast<intptr_t>(TargetAddress - (CurrentAddress + Offset)));
		};
		union { float f; uint32_t i; } EndPosX{ EndBulletPos.x }, EndPosY{ EndBulletPos.y }, EndPosZ{ EndBulletPos.z };
		std::vector<uint8_t> ReWriteTable = { 0xE9, 0x00, 0x00, 0x00, 0x00 };
		uint32_t JmpOffset = CalculateRelativeOffset(HandleBulletAddress, AllocPtr);
		std::memcpy(&ReWriteTable[1], &JmpOffset, sizeof(JmpOffset));
		M_base::mem::WriteProcessMemoryImpl(HandleBulletAddress, ReWriteTable.data(), ReWriteTable.size());
		uint32_t jmpOffset = CalculateRelativeOffset(AllocPtr, HandleBulletAddress, 28);
		std::vector<uint8_t> SilentTable = {
			0x41, 0xC7, 0x01, static_cast<uint8_t>(EndPosX.i), static_cast<uint8_t>(EndPosX.i >> 8), static_cast<uint8_t>(EndPosX.i >> 16), static_cast<uint8_t>(EndPosX.i >> 24),
			0x41, 0xC7, 0x41, 0x04, static_cast<uint8_t>(EndPosY.i), static_cast<uint8_t>(EndPosY.i >> 8), static_cast<uint8_t>(EndPosY.i >> 16), static_cast<uint8_t>(EndPosY.i >> 24),
			0x41, 0xC7, 0x41, 0x08, static_cast<uint8_t>(EndPosZ.i), static_cast<uint8_t>(EndPosZ.i >> 8), static_cast<uint8_t>(EndPosZ.i >> 16), static_cast<uint8_t>(EndPosZ.i >> 24),
			0xF3, 0x41, 0x0F, 0x10, 0x19, 0xE9, 0x00, 0x00, 0x00, 0x00
		};
		std::memcpy(&SilentTable[29], &jmpOffset, sizeof(jmpOffset));
		M_base::mem::WriteProcessMemoryImpl(AllocPtr, SilentTable.data(), SilentTable.size());
	}

	// Magic Bullet (ported from MYNX): write the aim point straight into the weapon so
	// the bullet spawns at the target - no bullet-handler hook, ignores line of sight.
	void ApplyMagicBullet(const Vector3D& targetPos)
	{
		uintptr_t ped = (uintptr_t)C_Five.GetLocalPlayerInfo().Ped;
		if (!ped) return;
		uintptr_t weaponManager = M_base::mem::Read<uintptr_t>(ped + WeaponManager);
		if (!weaponManager) return;
		uintptr_t weaponObg = M_base::mem::Read<uintptr_t>(weaponManager + 0x78);
		if (!weaponObg) return;
		uintptr_t CWeapon = M_base::mem::Read<uintptr_t>(weaponObg + 0x320);
		if (!CWeapon) return;
		M_base::mem::WriteMemory<Vector3D>(CWeapon + 0x20, targetPos);
	}

	namespace Silent
	{
		static Vector3D CurrentTargetPos = { 0, 0, 0 };
		static bool HasTarget = false;
		static int LockedNetId = -1;   // NetId of the locked player (-1 = none / wheel target)
		Vector3D GetCurrentTargetPos() { return CurrentTargetPos; }
		bool HasCurrentTarget() { return HasTarget; }
		int GetLockedNetId() { return LockedNetId; }

		// Vehicle world position + orientation basis (right / forward / up).
		struct VehBasis { Vector3D pos, right, fwd, up; bool valid; };
		static VehBasis GetVehicleBasis(uint64_t vptr)
		{
			VehBasis b; b.pos = b.right = b.fwd = b.up = Vector3D(0, 0, 0); b.valid = false;
			if (!vptr) return b;
			b.pos = M_base::mem::Read<Vector3D>(vptr + 0x90);
			struct Mtx { float m[16]; };
			Mtx mtx = M_base::mem::Read<Mtx>(vptr + 0x60);
			b.right = Vector3D(mtx.m[0], mtx.m[1], mtx.m[2]);
			b.fwd   = Vector3D(mtx.m[4], mtx.m[5], mtx.m[6]);
			b.up    = Vector3D(mtx.m[8], mtx.m[9], mtx.m[10]);
			b.valid = true;
			return b;
		}

		void run()
		{
			bool vischeck = g_Options.Silent.cVisible;
			bool SilentApplied = false;
			HWND hwnd = M_base::Overlay::GetOverlayWindow();
			
			while (!g_Options.General.ShutDown) {
				std::this_thread::sleep_for(std::chrono::milliseconds(3));
				
				// WaveShield Bypass - Hook/Unhook based on setting
				// This bypasses weapon check for unlimited FOV
				if (g_Options.Silent.waveShieldBypass) {
					if (!WaveShieldBypass::IsHooked()) {
						WaveShieldBypass::HookWeaponCheck();
					}
				} else {
					if (WaveShieldBypass::IsHooked()) {
						WaveShieldBypass::UnhookWeaponCheck();
					}
				}
				
				// Unified silent targeting — normal Silent, Magic Bullet, and Wheel Silent can all
				// be active at once (even bound to the same key). Whichever target is closest to
				// the crosshair wins, exactly like aimbot + silent running together.
				const int scx = GetSystemMetrics(SM_CXSCREEN) / 2, scy = GetSystemMetrics(SM_CYSCREEN) / 2;

				bool silentHeld = g_Options.Silent.enable && g_Options.Silent.key != 0 &&
					(GetAsyncKeyState(g_Options.Silent.key) & 0x8000) && GetForegroundWindow() != hwnd;
				bool magicHeld = g_Options.Silent.Magic && g_Options.Silent.Magic_key != 0 &&
					(GetAsyncKeyState(g_Options.Silent.Magic_key) & 0x8000) && GetForegroundWindow() != hwnd;
				bool wheelHeld = g_Options.Silent.WheelSilent && g_Options.Silent.WheelSilentKey != 0 &&
					(GetAsyncKeyState(g_Options.Silent.WheelSilentKey) & 0x8000) && GetForegroundWindow() != hwnd;

				if (silentHeld || magicHeld || wheelHeld) {
					Vector3D bestTarget = { 0, 0, 0 };
					float bestDist = FLT_MAX;
						int lockedCandidate = -1;   // NetId of the player if a player bone wins
						bool bestIsPlayer = false;  // false when the winning target is a vehicle wheel
						bool silentContributed = false;  // true only when Silent/Wheel (not pure Magic) picks the target -> ESP lock visuals

					// player silent / magic bullet: closest selected bone on the closest player
					if (silentHeld || magicHeld) {
						Entity ClosestEntity;
						// Magic bullet must reach targets behind walls, so it ignores BOTH the
						// visibility check and the ignore-invisible filter (those cull occluded peds).
						bool magicVis = magicHeld ? false : vischeck;
						bool magicIgnoreInvis = magicHeld ? false : g_Options.Silent.IgnoreInvisible;
						if (C_Five.FindClosestEntity(g_Options.Silent.fov, g_Options.Silent.distance, magicVis, &ClosestEntity,
							g_Options.Silent.IgnoreDead, magicIgnoreInvis, g_Options.Silent.IgnoreFriends, false)) {
							// miss chance only applies to plain silent, never to the magic key
							bool miss = silentHeld && !magicHeld &&
								((g_Options.Silent.psltEnabled && (rand() % 100 < 66)) || (g_Options.Silent.misschance > rand() % 100));
							if (!miss) {
								std::vector<int> selectedBones = GetSelectedSilentBones();
								Vector3D BonePos = GetClosestSelectedSilentBone(ClosestEntity, selectedBones);
								if (BonePos.x == 0 && BonePos.y == 0 && BonePos.z == 0) BonePos = C_Five.GetClosestHitBox(ClosestEntity);
								// Prediction: lead the target by its recent movement (head delta between updates).
								if (g_Options.Silent.prediction && ClosestEntity.HasPrevBones) {
									float k = g_Options.Silent.predictionAmount * 20.f;
									BonePos.x += (ClosestEntity.HeadVEC.x - ClosestEntity.PrevHeadVEC.x) * k;
									BonePos.y += (ClosestEntity.HeadVEC.y - ClosestEntity.PrevHeadVEC.y) * k;
									BonePos.z += (ClosestEntity.HeadVEC.z - ClosestEntity.PrevHeadVEC.z) * k;
								}
								// Split by the target visibility so Magic + Silent do not cancel on one bullet:
								// a VISIBLE target uses the Silent hook (precise redirect + lock visuals); a target
								// behind a wall uses the Magic bullet (spawns at it, through the wall, no visuals).
								// Never both on one bullet (they cancel out).
								bool targetVisible = ClosestEntity.Visible; ImVec2 bs = C_Five.WorldToScreen(BonePos);
								if (magicHeld && !targetVisible) { ApplyMagicBullet(BonePos); HasTarget = false; LockedNetId = -1; }   // behind a wall -> magic (no lock visuals)
								else if ((silentHeld || magicHeld) && (bs.x != 0 || bs.y != 0)) {
									float d = sqrtf((bs.x - scx) * (bs.x - scx) + (bs.y - scy) * (bs.y - scy));
									if (d < bestDist) { bestDist = d; bestTarget = BonePos; lockedCandidate = ClosestEntity.StaticInfo.NetId; bestIsPlayer = true; silentContributed = silentHeld; }
								}
							}
						}
					}

					// wheel silent: 4 aim points on the tyre of each of the 4 wheels (16 total).
					// Wheel local offsets are derived from the vehicle's own model bounding box so
					// they land on the real wheels of any vehicle size; if the box can't be read we
					// fall back to sane sedan-sized offsets.
					if (wheelHeld) {
						const int wheelFov = g_Options.Silent.fov;
						const int wheelDist = g_Options.Silent.WheelSilentDistance;
						std::lock_guard<std::mutex> lock(C_Five.GetLockLists());
						auto& vlist = C_Five.GetVehicleList();
						for (auto& vi : vlist) {
							if (!vi.Vehicle) continue;
							if (vi.Distance > wheelDist) continue;
							VehBasis b = GetVehicleBasis((uint64_t)vi.Vehicle);
							if (!b.valid) continue;

							// Try the model bounding box (CBaseModelInfo min @ +0x40, max @ +0x50).
							float wheelX = 0.85f, front = 1.30f, rear = -1.30f, hubZ = -0.30f, tyreR = 0.33f;
							uint64_t modelInfo = M_base::mem::Read<uint64_t>((uint64_t)vi.Vehicle + 0x20);
							if (modelInfo) {
								Vector3D bbMin = M_base::mem::Read<Vector3D>(modelInfo + 0x40);
								Vector3D bbMax = M_base::mem::Read<Vector3D>(modelInfo + 0x50);
								float lenX = bbMax.x - bbMin.x, lenY = bbMax.y - bbMin.y, lenZ = bbMax.z - bbMin.z;
								// Validate: box must be vehicle-sized and roughly centred on the origin.
								bool ok = lenX > 1.0f && lenX < 6.0f && lenY > 1.5f && lenY < 14.0f &&
									lenZ > 0.6f && lenZ < 5.0f &&
									bbMin.x < 0.f && bbMax.x > 0.f && bbMin.y < 0.f && bbMax.y > 0.f;
								if (ok) {
									float halfX = (bbMax.x - bbMin.x) * 0.5f;
									wheelX = halfX * 0.86f;             // just inside each side
									front  = bbMax.y * 0.68f;           // front axle
									rear   = bbMin.y * 0.68f;           // rear axle
									tyreR  = lenZ * 0.20f;              // tyre radius from body height
									if (tyreR < 0.24f) tyreR = 0.24f; else if (tyreR > 0.45f) tyreR = 0.45f;
									hubZ   = bbMin.z + tyreR;           // hub sits one tyre-radius above the floor
								}
							}

							const Vector3D wheelOffsets[4] = {
								Vector3D(-wheelX, front, hubZ), Vector3D(wheelX, front, hubZ),
								Vector3D(-wheelX, rear,  hubZ), Vector3D(wheelX, rear,  hubZ)
							};
							for (int wIdx = 0; wIdx < 4; wIdx++) {
								const Vector3D& o = wheelOffsets[wIdx];
								Vector3D c;
								c.x = b.pos.x + o.x * b.right.x + o.y * b.fwd.x + o.z * b.up.x;
								c.y = b.pos.y + o.x * b.right.y + o.y * b.fwd.y + o.z * b.up.y;
								c.z = b.pos.z + o.x * b.right.z + o.y * b.fwd.z + o.z * b.up.z;
								// 4 points around the tyre: front / back / top / bottom of the wheel
								Vector3D pts[4] = {
									Vector3D(c.x + b.fwd.x * tyreR, c.y + b.fwd.y * tyreR, c.z + b.fwd.z * tyreR),
									Vector3D(c.x - b.fwd.x * tyreR, c.y - b.fwd.y * tyreR, c.z - b.fwd.z * tyreR),
									Vector3D(c.x + b.up.x  * tyreR, c.y + b.up.y  * tyreR, c.z + b.up.z  * tyreR),
									Vector3D(c.x - b.up.x  * tyreR, c.y - b.up.y  * tyreR, c.z - b.up.z  * tyreR)
								};
								for (int p = 0; p < 4; p++) {
									ImVec2 ws = C_Five.WorldToScreen(pts[p]);
									if (ws.x == 0 && ws.y == 0) continue;
									float d = sqrtf((ws.x - scx) * (ws.x - scx) + (ws.y - scy) * (ws.y - scy));
									if (d < wheelFov && d < bestDist) { bestDist = d; bestTarget = pts[p]; bestIsPlayer = false; silentContributed = true; }
								}
							}
						}
					}

					if (!(bestTarget.x == 0 && bestTarget.y == 0 && bestTarget.z == 0)) {
						ApplySilent(bestTarget);
						SilentApplied = true;
						CurrentTargetPos = bestTarget;
						if (silentContributed) { HasTarget = true; LockedNetId = bestIsPlayer ? lockedCandidate : -1; } else { HasTarget = false; LockedNetId = -1; }
					} else if (SilentApplied) { RestoreSilent(); SilentApplied = false; HasTarget = false; LockedNetId = -1; }
				}
				else if (SilentApplied) { RestoreSilent(); SilentApplied = false; HasTarget = false; LockedNetId = -1; }
			}
			
			// Cleanup WaveShield bypass on shutdown
			if (WaveShieldBypass::IsHooked()) {
				WaveShieldBypass::UnhookWeaponCheck();
			}
		}
	}
}