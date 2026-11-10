#include <imgui.h>
#include "aim.hpp"
#include "../Options.hpp"
#include "../../../framework/settings/variables.h"

namespace Cheat
{
	static std::vector<int> GetSelectedAimbotBones()
	{
		std::vector<int> selected;
		// Use bone array from g_Options.Aimbot.bones[] (synced with preview)
		for (int i = 0; i < 14; i++)
		{
			if (g_Options.Aimbot.bones[i])
				selected.push_back(i);
		}
		return selected;
	}

	// Maps the 14 preview bones (HB[] order) to the exact same joints the ESP skeleton draws.
	static Vector3D GetBonePosition(const Entity& entity, int boneIndex)
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

	static Vector3D GetClosestSelectedBone(const Entity& entity, const std::vector<int>& selectedBones)
	{
		if (selectedBones.empty()) return entity.HeadVEC;
		if (selectedBones.size() == 1) return GetBonePosition(entity, selectedBones[0]);
		Vector3D closestPos = { 0, 0, 0 };
		float closestDist = FLT_MAX;
		int screenCenterX = GetSystemMetrics(SM_CXSCREEN) / 2;
		int screenCenterY = GetSystemMetrics(SM_CYSCREEN) / 2;
		for (int boneIndex : selectedBones) {
			Vector3D bonePos = GetBonePosition(entity, boneIndex);
			if (bonePos.x == 0 && bonePos.y == 0 && bonePos.z == 0) continue;
			ImVec2 screenPos = C_Five.WorldToScreen(bonePos);
			if (screenPos.x != 0 || screenPos.y != 0) {
				float dist = sqrtf((screenPos.x - screenCenterX) * (screenPos.x - screenCenterX) + (screenPos.y - screenCenterY) * (screenPos.y - screenCenterY));
				if (dist < closestDist) { closestDist = dist; closestPos = bonePos; }
			}
		}
		if (closestPos.x == 0 && closestPos.y == 0 && closestPos.z == 0) closestPos = GetBonePosition(entity, selectedBones[0]);
		return closestPos;
	}

	void Aim::run()
	{
		auto localplayer = C_Five.GetLocalPlayerInfo();
		auto localpos = localplayer.WorldPos;
		int maxdistance = g_Options.Aimbot.distance;
		while (!g_Options.General.ShutDown) {
			bool vischeck = g_Options.Aimbot.cVisible;
			std::this_thread::sleep_for(std::chrono::milliseconds(5));
			if (!g_Options.Aimbot.enable) continue;
			Entity ClosestEntity;
			if (C_Five.FindClosestEntity(g_Options.Aimbot.fov, maxdistance, vischeck, &ClosestEntity, g_Options.Aimbot.IgnoreDead, g_Options.Aimbot.IgnoreInvisible, g_Options.Aimbot.IgnoreFriends, false)) {
				if (!GetAsyncKeyState(g_Options.Aimbot.key)) continue;
				std::vector<int> selectedBones = GetSelectedAimbotBones();
				Vector3D BonePos = GetClosestSelectedBone(ClosestEntity, selectedBones);
				if (BonePos.x == 0 && BonePos.y == 0 && BonePos.z == 0) BonePos = C_Five.GetClosestHitBox(ClosestEntity);
				// Prediction: lead the target by its recent movement (head delta between updates).
				if (g_Options.Aimbot.prediction && ClosestEntity.HasPrevBones) {
					float k = g_Options.Aimbot.predictionAmount * 20.f;
					BonePos.x += (ClosestEntity.HeadVEC.x - ClosestEntity.PrevHeadVEC.x) * k;
					BonePos.y += (ClosestEntity.HeadVEC.y - ClosestEntity.PrevHeadVEC.y) * k;
					BonePos.z += (ClosestEntity.HeadVEC.z - ClosestEntity.PrevHeadVEC.z) * k;
				}
				C_Five.CameraMove(BonePos, g_Options.Aimbot.xsmooth, g_Options.Aimbot.ysmooth);
			}
		}
	}
}
