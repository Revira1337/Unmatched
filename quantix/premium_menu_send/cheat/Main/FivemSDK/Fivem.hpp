#pragma once
#include <cstdint>

#include "Classes.hpp"

#include "../../Utils/FrameWork.hpp"
#include <mutex>

namespace Cheat
{
	enum eGAME_VERSION
	{
		GAME_VERSION_GAME_b2060,
		GAME_VERSION_GTA_b2060,
		GAME_VERSION_GAME_b2189,
		GAME_VERSION_GTA_b2189,
		GAME_VERSION_GAME_b2372,
		GAME_VERSION_GTA_b2372,
		GAME_VERSION_GAME_b2545,
		GAME_VERSION_GTA_b2545,
		GAME_VERSION_GAME_b2612,
		GAME_VERSION_GTA_b2612,
		GAME_VERSION_GAME_b2699,
		GAME_VERSION_GTA_b2699,
		GAME_VERSION_GAME_b2802,
		GAME_VERSION_GTA_b2802,
		GAME_VERSION_GAME_b2944,
		GAME_VERSION_GTA_b2944,
		GAME_VERSION_GAME_b3095,
		GAME_VERSION_GTA_b3095,
		GAME_VERSION_GAME_b3258,
		GAME_VERSION_GTA_b3258,
		GAME_VERSION_GAME_b3323,
		GAME_VERSION_GTA_b3323,
		GAME_VERSION_GAME_b3407,
		GAME_VERSION_GTA_b3407,
		GAME_VERSION_GAME_b3570,
		GAME_VERSION_GTA_b3570,
		
	};

	struct PedStaticInfo
	{
		CPed* Ped;
		int iIndex;
		int NetId;
		bool bIsLocalPlayer = false;
		bool bIsNPC = false;
		std::string Name;
		uint64_t crSkeletonData;
		std::unordered_map<unsigned int, unsigned int> MaskToBoneId;

		bool IsFriend = false;

	};

	struct Entity
	{
		PedStaticInfo StaticInfo;

		Vector3D Cordinates;
		bool Visible;
		ImVec2 PedLocation;
		ImVec2 HeadPos;
		ImVec2 LeftFootPos;
		ImVec2 RightFootPos;
		ImVec2 NeckPos;
		ImVec2 ChestPos;

		Vector3D HeadVEC;
		Vector3D LeftFootVEC;
		Vector3D RightFootVEC;
		Vector3D NeckVEC;
		Vector3D ChestVEC;

		// Cached skeleton bones for ESP (optimized + smoothed)
		Vector3D PelvisVEC;
		Vector3D L_UpperArmVEC;
		Vector3D L_ForearmVEC;
		Vector3D L_HandVEC;
		Vector3D R_UpperArmVEC;
		Vector3D R_ForearmVEC;
		Vector3D R_HandVEC;
		Vector3D L_ThighVEC;
		Vector3D L_CalfVEC;
		Vector3D R_ThighVEC;
		Vector3D R_CalfVEC;

		// Previous frame bones for interpolation (anti-jitter)
		Vector3D PrevHeadVEC;
		Vector3D PrevNeckVEC;
		Vector3D PrevPelvisVEC;
		Vector3D PrevL_FootVEC;
		Vector3D PrevR_FootVEC;
		bool HasPrevBones = false;

		float Health;
		int MaxHealth;

		int Distance;
		
		float Armor = 0;

		bool IsFriend = false;

		bool Dead = false;
		std::vector<std::pair<Vector3D, Vector3D>> Bones;
	};

	struct OFFSET {
		CReplayInterface* pReplayInterface;
		CPedInterface* pPedInterface;
		CCamGameplayDirector* pCamGameplayDirector;
		CWorld* pWorld;
		CPed* pLocalPlayer;
	};

	struct LocalPEDInfo
	{
		CPed* Ped;
		int iIndex;
		ImVec2 ScreenPos;
		Vector3D WorldPos;
		int WeaponDamage;
		uintptr_t PInfo;
	};

struct VehicleInfo
{
	CVehicle* Vehicle;
	std::string Name;
	uint64_t ModelInfo;
	int iIndex = -1;
	Vector3D Cordinates;
	int Distance;
	float Health;
	bool LockState;
	bool Driver;
	int randomId = -1;
};

	class CFiveFramework
	{
	public:
		OFFSET Cache;

		int GetGTAPID();
		void Initialize();
		std::string GetProcessName(DWORD processID);
		uintptr_t GetRemoteModuleHandle(DWORD dwProcessId, const wchar_t* szModuleName);
		int GetBuild(int Pid);
		int GetBuild2(int Pid);

		void InitializeServerInfo();

		float ImVec2Distance(const ImVec2& a, const ImVec2& b);


		DWORD GetPid() { return Pid; }
		uint64_t GetModuleBase() { return ModuleBase; };
		std::string GetModuleName() { return ModuleName; };
		std::vector<Entity>& GetEntitiyList() {
			return EntityList;
		}
		std::vector<VehicleInfo>& GetVehicleList() {
			return VehicleList;
		}
		ImVec2 WorldToScreen(Vector3D Pos);
		// Per-frame cached projection: call CacheW2S() once per frame, then
		// WorldToScreenC() reuses the snapshot (no per-call view-matrix read).
		void   CacheW2S();
		ImVec2 WorldToScreenC(Vector3D Pos);
		void UpdateLocalPlayerInfo(const Entity& ent, uintptr_t PInfo, int index);
		std::tuple<Vector3D, Vector3D, Vector3D, Vector3D, Vector3D> GetBoneVectors(const Entity& ent);
		ImVec2 WorldToScreen2(Vector3D Pos);
		LocalPEDInfo GetLocalPlayerInfo() { return LocalPlayerInfo; }
		Entity BuildEntity(CPed* Ped, uintptr_t PInfo, int index, const Vector3D& localPos);
		void ValuesGet();
		bool UpdateEntityList();
		Vector3D GetClosestHitBox(Entity Ped);
		bool FindClosestEntity(float Fov, int MaxDistance, bool vischeck, Entity* Output, bool ignoreDead = true, bool ignoreInvisible = true, bool ignoreFriends = true, bool ignoreNPC = true);
		Vector3D GetBone(Entity Ped, int Bone);
		void SpectatePed(uint64_t Ped, bool Toggle);
		void HumanizedAim(Vector3D WorldPosition);
		Vector3D GetPt(Vector3D A, Vector3D B, float t);
		Vector3D GetBonePosVec3(Entity Ped, unsigned int Mask);
		bool GetPedBoneIndex(Entity Ped, unsigned int Mask, unsigned int& newIdx);
		uint64_t GetHandleBullet() { return HandleBullet; }
		uint64_t GetCamera() { return Camera; }
		uint64_t GetBlipListAddress() { return BlipList; }



		void CameraMove(Vector3D WorldPosition, int SmoothHorizontal, int SmoothVertical);
		void TeleportFunc(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3D Position, Vector3D VisualPosition, bool Stop);
		void UpdateNamesThread();
		nlohmann::json GetServerInfo2();
		bool CheckServerInfo();
		uintptr_t SyncTreeGetter(const std::string& className);
		nlohmann::json GetServerInfo();
		bool IsOnScreen(ImVec2 Pos);
		CCamGameplayDirector* GetCamGameplayDirector() { return pCamGameplayDirector; }


		uint64_t World;
		uint64_t ReplayInterface;
		uint64_t ViewPort;
		uint64_t Camera;
		uint64_t bIsPlayerAiming;
		uint64_t PlayerAimingAt;
		uint64_t HandleBullet;
		uint64_t BlipList;
		uint64_t GetSyncTree() { return SyncTree; }
		std::mutex LockLists;
		std::mutex& GetLockLists() { return LockLists; }
		std::mutex& GetLockLists2() { return LockLists2; }

		DWORD Pid;
		uint64_t ModuleBase;
		std::string ModuleName;
		bool IsInitialized() { return Build_Found; }
		eGAME_VERSION GameBuild;
		int buildNumber;
		bool Build_Found = false;

		std::unordered_map<CPed*, bool> FriendList;
		std::unordered_map<int, bool> NetIdFriendList;
		
		// Player name mapping (public for access from playernames system)
		std::unordered_map<int, std::string> PlayerIdToName;
		// Server identifiers (discord id / steam hex) keyed by server player id, for the Players panel
		std::unordered_map<int, std::string> PlayerIdToDiscord;
		std::unordered_map<int, std::string> PlayerIdToSteam;
		// Name-keyed bridge (in-game NetId often != server "id", so fall back to matching by name)
		std::unordered_map<std::string, std::string> PlayerNameToDiscord;
		std::unordered_map<std::string, std::string> PlayerNameToSteam;

	private:
	
		std::string FivemFolder;
		std::string CrashoMetryLocation;
		std::string iniPath;
		std::string ServerIp;
		std::string ServerPort;
		nlohmann::json PlayersInfo;
		std::unordered_map<int, std::string> PlayerNameToId;

		bool LanGame;
		

	private:
		CWorld* pWorld;
		CPed* pLocalPlayer;
		CReplayInterface* pReplayInterface;
		CPedInterface* pPedInterface;
		CCamGameplayDirector* pCamGameplayDirector;

		uint64_t pViewPort;

		// cached view-projection rows for WorldToScreenC (refreshed once per frame)
		float m_w2s[12];
		bool  m_w2sReady = false;

	private:
		std::unordered_map<CPed*, PedStaticInfo> CachedStaticInfoList;
		LocalPEDInfo LocalPlayerInfo;
		//std::mutex LockLists;
		std::mutex LockLists2;
		std::mutex NameMutex;
		std::vector<Entity> EntityList;
		std::vector<VehicleInfo> VehicleList;
		std::unordered_map<CVehicle*, int> VehicleIdMap; // Map to store vehicle pointer to random ID mapping


	};

	inline CFiveFramework C_Five;
}
