#pragma once

#include "Offsets.hpp"

#include "../../Utils/FrameWork.hpp"
#include "../../Utils/Render/kaanlar.h"
#include "GTADefines.hpp"

enum eCarLockState
{
	CARLOCK_NONE = 0,
	CARLOCK_UNLOCKED = 1,
	CARLOCK_LOCKED = 2,
	CARLOCK_LOCKOUT_PLAYER_ONLY = 3,
	CARLOCK_LOCKED_PLAYER_INSIDE = 4,
	CARLOCK_LOCKED_INITIALLY = 5,
	CARLOCK_FORCE_SHUT_DOORS = 6,
	CARLOCK_LOCKED_BUT_CAN_BE_DAMAGED = 7,
	CARLOCK_LOCKED_BUT_BOOT_UNLOCKED = 8,
	CARLOCK_LOCKED_NO_PASSENGERS = 9,
};


namespace Cheat
{
	class CVehicle
	{
	public:
		uint64_t GetModelInfo()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x20);
		}

		uint64_t GetDriver()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + Driver);
		}
		uintptr_t GetHandling()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uintptr_t>(this + 0x960);
		}
		uint64_t GetNavigation()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x30);
		}

		Vector3D GetCoordinate()
		{
			if (!this)
				return Vector3D{ 0,0,0 };

			return M_base::mem::Read<Vector3D>(this + 0x90);
		}
		

		void SetDoorLock(int Lock)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + DoorLock, Lock);
		}

		bool Is_Locked(uint64_t c_vehicle)
		{
			if (!this)
				return 0;

			uint32_t m_door_lock_status = M_base::mem::Read<uint32_t>(c_vehicle + DoorLock);

			return  m_door_lock_status != CARLOCK_UNLOCKED;
		}

		bool Has_Driver(uint64_t c_vehicle)
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint32_t>(c_vehicle + 0xC98);
		}

		float GetHealth()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<float>(this + 0x280);
		}

		void SetHealth(float Health)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0x280, Health);
		}

		float GetMaxHealth()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<float>(this + MaxHealth);
		}
		void FixVehicle()
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + VehicleState, (BYTE)0x17);
		}

		void SetInvencible(bool Toggle)
		{
			if (!this)
				return;

			uint64_t OldFlag = M_base::mem::Read<uint64_t>(this + 0x188);
			M_base::mem::WriteMemory(this + 0x188, Toggle ? (OldFlag | (1 << 8)) : (OldFlag & ~(1 << 8)));
		}

		uint64_t get_handling(uint64_t c_vehicle)
		{
			return M_base::mem::Read<uintptr_t>(c_vehicle + Handling);
		}

		void set_acceleration(uint64_t c_vehicle, float acceleration)
		{
			uint64_t handling = get_handling(c_vehicle);
			M_base::mem::WriteMemory<float>(handling + 0x90, acceleration);
			
			M_base::mem::WriteMemory<float>(handling + 0x4C, acceleration);
			M_base::mem::WriteMemory<float>(handling + 0x6C, acceleration);
			M_base::mem::WriteMemory<float>(handling + 0x14, acceleration);

		}
	};

	class CPlayerInfo
	{
	public:
		int GetPlayerID()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<int>(this + PlayerNetID);
		}

		void SetStamina(float Stamina)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0xCF0, Stamina);
		}

		void SetSpeed(float Speed)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory<float>(this + PlayerSpeed, Speed);
		}
	};

	class CWeaponInfo
	{
	public:
		std::string GetWeaponName()
		{
			//if (!this)
			//	return NULL;

			return M_base::mem::ReadProcessMemoryString(M_base::mem::Read<uint64_t>(this + 0x05F0));
		}
	};

	class CWeaponManager
	{
	public:
		CWeaponInfo* GetWeaponInfo()
		{
			if (!this)
				return 0;

			return (CWeaponInfo*)M_base::mem::Read<uint64_t>(this + 0x20);
		}

	};

	class CPed
	{
	public:
		CPlayerInfo* GetPlayerInfo()
		{
			if (!this)
				return 0;

			return (CPlayerInfo*)M_base::mem::Read<uint64_t>(this + PlayerInfo);
		}

		CWeaponManager* GetWeaponManager()
		{
			if (!this)
				return 0;

			return (CWeaponManager*)M_base::mem::Read<uint64_t>(this + WeaponManager);
		}

		CVehicle* GetLastVehicle()
		{
			if (!this)
				return 0;

			return (CVehicle*)M_base::mem::Read<uint64_t>(this + LastVehicle);
		}

		uint64_t GetModelInfo()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x20);
		}

		void godmode(bool Toggle)
		{
			M_base::mem::WriteMemory(this + 0x2C, Toggle);
		}

		void invisible(bool Toggle)
		{
			uint8_t value = Toggle ? 0x1 : 0x37;
			M_base::mem::WriteMemory(this + 0x189, value);
		}

		uint64_t GetNavigation()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x30);
		}

		Vector3D GetCoordinate()
		{
			if (!this)
				return Vector3D{ 0,0,0 };

			return M_base::mem::Read<Vector3D>(this + 0x90);
		}

		bool IsGodMode()
		{
			if (!this)
				return false;

			return M_base::mem::Read<bool>(this + 0x189);
		}

		float GetHealth()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<float>(this + 0x280);
		}

		void SetHealth(float NewHealth)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0x280, NewHealth);
		}

		float GetMaxHealth()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<float>(this + MaxHealth);
		}

		float GetArmor()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<float>(this + Armor);
		}

		void SetArmor(float NewArmor)
		{
			if (!this)
				return;

			uintptr_t address = reinterpret_cast<uintptr_t>(this) + Armor;
			M_base::mem::WriteMemory<float>(address, NewArmor);
		}

		uint32_t GetPedType()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint32_t>(this + EntityType);
		}

		bool IsVisible()
		{
			if (!this)
				return false;

			BYTE VisibilityFlag = M_base::mem::Read<BYTE>(this + VisibleFlag);

			if (VisibilityFlag == 36 || VisibilityFlag == 0 || VisibilityFlag == 4)
				return false;

			return true;
		}

		bool IsNPC()
		{
			uint32_t PedType = GetPedType();

			if (!PedType)
				return false;

			PedType = PedType << 11 >> 25;

			if (PedType != 2)
				return true;

			return false;
		}

		bool IsAnimal()
		{
			uint32_t PedType = GetPedType();

			if (!PedType)
				return false;

			PedType = PedType << 11 >> 25;

			if (PedType > 27)
				return true;

			return false;
		}

		void SetConfigFlag(ePedConfigFlags Flag, bool Value)
		{
			int v1 = (int)Flag;
			if (!this || v1 >= 0x1CA)
				return;

			auto v2 = 1 << (v1 & 0x1F);
			auto v3 = v1 >> 5;
			auto v4 = (uint64_t)this + 4 * v3 + ConfigFlags;
			auto v5 = M_base::mem::Read<long>(v4);

			if (Value != ((v2 & v5) != 0))
			{
				auto v6 = v2 & (v5 ^ -(uint8_t)(Value ? 1 : 0));
				v5 ^= v6;

				M_base::mem::WriteMemory(v4, v5);
			}
		}

		bool HasConfigFlag(ePedConfigFlags Flag)
		{
			int v1 = (int)Flag;
			if (!this || v1 >= 0x1CA)
				return false;

			auto v2 = 1 << (v1 & 0x1F);
			auto v3 = v1 >> 5;
			auto v4 = (uint64_t)this + 4 * v3 + ConfigFlags;
			auto v5 = M_base::mem::Read<long>(v4);

			return (v2 & v5) != 0;
		}
		void Invisible_ON()
		{
			if (!this)
				return;
			M_base::mem::WriteMemory(this + 0xAC, 0);
		}

		void Invisible_Off()
		{
			if (!this)
				return;
			M_base::mem::WriteMemory(this + 0xAC, 1);
		}

		void InfiniteAmmo(bool Toggle)
		{
			uint64_t weapon_manager = M_base::mem::Read<uint64_t>(this + WeaponManager);
			uint64_t weapon_info = M_base::mem::Read<uint64_t>(weapon_manager + 0x20);
			uint64_t ammo_info = M_base::mem::Read<uint64_t>(weapon_info + 0x60);
			uint64_t ammo_count = M_base::mem::Read<uint64_t>(ammo_info + 0x8);
			uint64_t ammo_count2 = M_base::mem::Read<uint64_t>(ammo_count + 0x0);
			M_base::mem::WriteMemory<int>(ammo_count2 + 0x18, 30);

		}

		void Godmode(bool Toggle)
		{
			uint64_t cur_vehicle = M_base::mem::Read<uint64_t>(this + LastVehicle);

			M_base::mem::WriteMemory(this + 0x189, Toggle);
		}
	};

	class CPedList
	{
	public:
		CPed* Ped(int Index)
		{
			if (!this)
				return 0;

			return (CPed*)M_base::mem::Read<uint64_t>(this + (Index * 0x10));
		}
	};

	class CPedInterface
	{
	public:
		CPedList* PedList()
		{
			if (!this)
				return 0;

			return (CPedList*)M_base::mem::Read<uint64_t>(this + 0x100);
		}

		uint64_t PedMaximum()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x108);
		}
	};

	class CVehicleList
	{
	public:
		CVehicle* Vehicle(int Index)
		{
			if (!this)
				return 0;

			return (CVehicle*)M_base::mem::Read<uint64_t>(this + (Index * 0x10));
		}
	};

	class CVehicleInterface
	{
	public:
		CVehicleList* VehicleList()
		{
			if (!this)
				return 0;

			return (CVehicleList*)M_base::mem::Read<uint64_t>(this + 0x180);
		}

		uint64_t VehicleMaximum()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x188);
		}

		uint64_t VehiclesAtList()
		{
			if (!this)
				return 0;

			return M_base::mem::Read<uint64_t>(this + 0x190);
		}
	};

	class CReplayInterface
	{
	public:
		CVehicleInterface* VehicleInterface()
		{
			if (!this)
				return 0;

			return (CVehicleInterface*)M_base::mem::Read<uint64_t>(this + 0x10);
		}

		CPedInterface* PedInterface()
		{
			if (!this)
				return 0;

			return (CPedInterface*)M_base::mem::Read<uint64_t>(this + 0x18);
		}
	};

	class CCamFollowPedCamera
	{
	public:
		Vector3D GetViewAngles()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return M_base::mem::Read<Vector3D>(this + 0x40);
		}

		void SetViewAngles(Vector3D Angles)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0x40, Angles);
		}

		Vector3D GetCrosshairPosition()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return M_base::mem::Read<Vector3D>(this + 0x60);
		}

		void SetCrosshairPosition(Vector3D Position)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0x60, Position);
		}

		Vector3D GetThirdpersonViewAngles()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return M_base::mem::Read<Vector3D>(this + 0x3d0);
		}

		void SetThirdpersonViewAngles(Vector3D ViewAngles)
		{
			if (!this)
				return;

			M_base::mem::WriteMemory(this + 0x3d0, ViewAngles);
		}


		Vector3D GetCameraPosition()
		{
			if (!this)
				return Vector3D(0, 0, 0);

			return M_base::mem::Read<Vector3D>(this + 0x3F0);
		}
	};

	class CCamGameplayDirector
	{
	public:
		CCamFollowPedCamera* GetFollowPedCamera()
		{
			if (!this)
				return 0;

			return (CCamFollowPedCamera*)M_base::mem::Read<uint64_t>(this + 0x3C0);
		}
	};

	class CWorld
	{
	public:
		CPed* LocalPlayer()
		{
			if (!this)
				return 0;

			return (CPed*)M_base::mem::Read<uint64_t>(this + 0x8);
		}
	};
}