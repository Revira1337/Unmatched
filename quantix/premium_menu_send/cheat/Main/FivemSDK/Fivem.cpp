#include <windows.h>
#include <tlhelp32.h>
#include "Fivem.hpp"
#include <fstream>
#include <filesystem>
#include <unordered_map>
#include <sstream>
#include <psapi.h>
#include <regex>
#include <thread>
// DirectX included via FrameWork
#include "../../Utils/Math/Vectors/Vector3D.hpp"
#include "simplemath.h"
#include "GTADefines.hpp"
#include "../Features/esp.hpp"
#include <cmath>
#include <iostream>
#include "../Options.hpp"
#include <unordered_set>
#include <algorithm>
#include <wininet.h>
#include <Main/Features/playernames.hpp>

#pragma comment(lib, "wininet.lib")

struct PedBarFix {
	int id;
	float health;   // last-known-good (>0) health
	float armor;    // last-known-good armor
	int hZero = 0;  // consecutive zero-health reads (transient-bad-read filter)
	int aZero = 0;  // consecutive zero-armor reads
};

std::vector<PedBarFix> pedBarFix;


namespace Cheat
{
	uint64_t EntityType;
	uint64_t Armor;
	uint64_t BoneManager;
	uint64_t PlayerInfo;
	uint64_t PlayerNetID;
	uint64_t WeaponManager;
	uint64_t FragInsNmGTA;
	uint64_t ConfigFlags;
	uint64_t MaxHealth;
	uint64_t LastVehicle;
	uint64_t Driver;
	uint64_t DoorLock;
	uint64_t Handling;
	uint64_t VehicleState;
	uint64_t Stamina;
	uint64_t VisibleFlag;
	uint64_t PlayerSpeed;
	uint64_t SyncTree;
	uint64_t Freecam_offset;
	uint64_t Id;
	uint64_t gravity;
	uint64_t invisible;


	int initializetime = 0;

	int ModuleBase = 0;

	void CFiveFramework::Initialize()
	{
		if (Build_Found)
			return;

		HKEY hKey;
		WCHAR Buffer[MAX_PATH];
		DWORD BufferSize = sizeof(Buffer);
		if (SafeCall(RegOpenKeyEx)(HKEY_CURRENT_USER, xorstr_(L"SOFTWARE\\CitizenFX\\FiveM"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
		{

			if (RegQueryValueEx(hKey, xorstr_(L"Last Run Location"), NULL, NULL, (LPBYTE)Buffer, &BufferSize) == ERROR_SUCCESS)
			{
				std::string BasePath = M_base::Misc::Wstring2String(std::wstring(Buffer));
				if (!BasePath.empty())
				{
					FivemFolder = BasePath;
					iniPath = BasePath + xorstr_("CitizenFX.ini");

				}
			}
			SafeCall(RegCloseKey)(hKey);
		}
		std::wifstream iniFile(iniPath);
		if (!iniFile.is_open())
		{
			return;
		}

		std::wstring line;
		std::wstring savedBuildNumber;
		while (std::getline(iniFile, line))
		{
			if (line.find(xorstr_(L"SavedBuildNumber")) != std::wstring::npos)
			{
				size_t pos = line.find(L"=");
				if (pos != std::wstring::npos)
				{
					savedBuildNumber = line.substr(pos + 1);
					break;
				}
			}
		}
		iniFile.close();

		if (savedBuildNumber.empty())
		{
			return;
		}

		std::string CitizenfxPath = std::string(getenv(xorstr_("LOCALAPPDATA"))) + xorstr_("\\FiveM\\FiveM.app\\CitizenFX.ini");

		int replaceExecutableValue = 0;

		std::ifstream CitizenfxFile(CitizenfxPath);
		if (CitizenfxFile.is_open()) {
			std::string line;
			while (std::getline(CitizenfxFile, line)) {
				if (line.find(xorstr_("ReplaceExecutable=")) != std::string::npos) {
					replaceExecutableValue = std::stoi(line.substr(line.find('=') + 1));
					break;
				}
			}
			CitizenfxFile.close();
		}

		//int buildNumber = std::stoi(savedBuildNumber);

		int ppid = GetGTAPID();
		auto name = GetProcessName(ppid);
		std::string nameLower = name;
		std::transform(nameLower.begin(), nameLower.end(), nameLower.begin(), ::tolower);
		//if (nameLower != "fivem_gtaprocess.exe" && nameLower != "fivem_gameprocess.exe") 
		//{
		//	buildNumber = GetBuild(ppid);
		//}
		//else
		//{
		//	int builds = GetBuild(ppid);
		//	if (builds == 0)
		//	{
		//		buildNumber = 3258;
		//	}
		//	else
		//	{
		//		buildNumber = builds;
		//	}
		//}

		buildNumber = GetBuild(ppid);


		int bonebuild = buildNumber;

		std::cout << buildNumber << std::endl;


		if (buildNumber == 2372)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2372;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));

			if (ModuleBase)
			{
				World = ModuleBase + 0x252DCD8;
				ReplayInterface = ModuleBase + 0x1F05208;
				ViewPort = ModuleBase + 0x1F9E9F0;
				Camera = ModuleBase + 0x1F9F898;
				bIsPlayerAiming = ModuleBase + 0x2BF7CF0;
				PlayerAimingAt = ModuleBase + 0x1FACDE0;
				HandleBullet = ModuleBase + 0xFE7EF8;
				BlipList = ModuleBase + 0x1F9FFA0;

				EntityType = 0x10B8;
				Armor = 0x14E0;
				BoneManager = 0x430;
				PlayerInfo = 0x10C8;
				PlayerNetID = 0x88;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1400;
				ConfigFlags = 0x1414;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD30;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCD4;
				VisibleFlag = 0x142C;
				PlayerSpeed = 0x0CF0;

				SyncTree = 0x1120D30;

				buildNumber == 2372;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2612)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2612;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x2567DB0;
				ReplayInterface = ModuleBase + 0x1F77EF0;
				ViewPort = ModuleBase + 0x1FD8570;
				Camera = ModuleBase + 0x1FD9418;
				bIsPlayerAiming = ModuleBase + 0x2C33DB0;
				PlayerAimingAt = ModuleBase + 0x1FE6960;
				HandleBullet = ModuleBase + 0xFF340C;
				BlipList = ModuleBase + 0x1FDDD20;

				EntityType = 0x10B8;
				Armor = 0x1530;
				BoneManager = 0x430;
				PlayerInfo = 0x10C8;
				PlayerNetID = 0x88;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1450;
				ConfigFlags = 0x1464;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD30;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCF4;
				VisibleFlag = 0x147C;
				PlayerSpeed = 0x0CF0;

				SyncTree = 0x1120D30;

				buildNumber == 2612;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2699)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2699;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x26684D8;
				ReplayInterface = ModuleBase + 0x20304C8;
				ViewPort = ModuleBase + 0x20D8C90;
				Camera = ModuleBase + 0x20D9B38;
				bIsPlayerAiming = ModuleBase + 0x2DA1950;
				PlayerAimingAt = ModuleBase + 0x20E6CC0;
				HandleBullet = ModuleBase + 0xFF9D90;
				BlipList = ModuleBase + 0x20E1420;

				EntityType = 0x10B8;
				Armor = 0x1530;
				BoneManager = 0x430;
				PlayerInfo = 0x10C8;
				PlayerNetID = 0x88;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1450;
				ConfigFlags = 0x1464;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD30;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCF4;
				VisibleFlag = 0x147C;
				PlayerSpeed = 0x0CF0;
				invisible = 0x6A46EE;

				SyncTree = 0x1120D30;


				buildNumber == 2699;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2189)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2189;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x24E6D90;
				ReplayInterface = ModuleBase + 0x1EE18A8;
				ViewPort = ModuleBase + 0x1F888C0;
				Camera = ModuleBase + 0x1F89768;
				bIsPlayerAiming = ModuleBase + 0x2BC0FCC;
				PlayerAimingAt = ModuleBase + 0x1F7CBD0;
				HandleBullet = ModuleBase + 0xFE2154;
				BlipList = ModuleBase + 0x1F6EF80;

				EntityType = 0x10B8;
				Armor = 0x14E0;
				BoneManager = 0x430;
				PlayerInfo = 0x10C8;
				PlayerNetID = 0x78;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1400;
				ConfigFlags = 0x1414;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD30;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCD4;
				VisibleFlag = 0x142C;
				PlayerSpeed = 0x0CF0;

				SyncTree = 0x1120D30;

				buildNumber == 2189;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2802)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2802;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x254D448;
				ReplayInterface = ModuleBase + 0x1F5B820;
				ViewPort = ModuleBase + 0x1FBC100;
				Camera = ModuleBase + 0x1FBCFA8;
				bIsPlayerAiming = ModuleBase + 0x2CA3594;
				PlayerAimingAt = ModuleBase + 0x1FCA160;
				HandleBullet = ModuleBase + 0xFF716C;
				BlipList = ModuleBase + 0x1FBD6E0;

				EntityType = 0x1098;
				Armor = 0x1530;
				BoneManager = 0x410;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0x88;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC48;
				DoorLock = 0x1370;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0x0CF0;
				invisible = 0x6A46EE;

				SyncTree = 0x1120D30;

				buildNumber == 2802;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2060)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2060;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x24C8858;
				ReplayInterface = ModuleBase + 0x1EC3828;
				ViewPort = ModuleBase + 0x1F6A7E0;
				Camera = ModuleBase + 0x1F6B940;
				bIsPlayerAiming = ModuleBase + 0x2B958CC;
				PlayerAimingAt = ModuleBase + 0x1F5EB20;
				HandleBullet = ModuleBase + 0xFCE6EC;
				BlipList = ModuleBase + 0x1F4F940;

				EntityType = 0x10B8;
				Armor = 0x14E0;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0x78;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1400;
				ConfigFlags = 0x1414;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD28;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCD4;
				VisibleFlag = 0x142C;
				PlayerSpeed = 0x0CF0;

				SyncTree = 0x118786C;

				buildNumber == 2060;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2545)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2545;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x25667E8;
				ReplayInterface = ModuleBase + 0x1F2E7A8;
				ViewPort = ModuleBase + 0x1FD6F70;
				Camera = ModuleBase + 0x1FD7E18;
				bIsPlayerAiming = ModuleBase + 0x2C32630;
				PlayerAimingAt = ModuleBase + 0x1FE5360;
				HandleBullet = ModuleBase + 0xFF1B40;
				BlipList = ModuleBase + 0x1F4F940;

				EntityType = 0x10B8;
				Armor = 0x1530;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0x88;
				WeaponManager = 0x10D8;
				FragInsNmGTA = 0x1450;
				ConfigFlags = 0x1464;
				MaxHealth = 0x2A0;
				LastVehicle = 0xD30;
				Driver = 0xC68;
				DoorLock = 0x1390;
				Handling = 0x938;
				VehicleState = 0x94A;
				Stamina = 0xCF4;
				VisibleFlag = 0x147C;
				PlayerSpeed = 0x0CF0;

				SyncTree = 0x118786C;

				buildNumber == 2545;
				Build_Found = true;
			}
		}
		else if (buildNumber == 2944)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b2944;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x257BEA0;
				ReplayInterface = ModuleBase + 0x1F42068;
				ViewPort = ModuleBase + 0x1FEAAC0;
				Camera = ModuleBase + 0x1FEB968;
				bIsPlayerAiming = ModuleBase + 0x2CE2A94;
				PlayerAimingAt = ModuleBase + 0x1FF8AF0;
				HandleBullet = ModuleBase + 0x1003F80;
				BlipList = ModuleBase + 0x1FF3130;

				EntityType = 0x1098;
				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC48;
				DoorLock = 0x1370;
				Handling = 0x918;
				VehicleState = 0x92A;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0x0D40;
				invisible = 0x6A46EE;

				SyncTree = 0x118786C;

				buildNumber == 2944;
				Build_Found = true;
			}
		}
		else if (buildNumber == 3095)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b3095;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x2593320;
				ReplayInterface = ModuleBase + 0x1F58B58;
				ViewPort = ModuleBase + 0x20019E0;
				Camera = ModuleBase + 0x2002888;
				bIsPlayerAiming = ModuleBase + 0x2D3839C;
				PlayerAimingAt = ModuleBase + 0x200FA10;
				HandleBullet = ModuleBase + 0x100F5A4;
				BlipList = ModuleBase + 0x2002FA0;

				EntityType = 0x1098;
				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC90;
				DoorLock = 0x13C0;
				Handling = 0x960;
				VehicleState = 0x972;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0x0D40;
				invisible = 0x6A46EE;

				SyncTree = 0x118786C;

				buildNumber == 3095;
				Build_Found = true;
			}
		}
		else if (buildNumber == 3258)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b3258;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{

				World = ModuleBase + 0x25B14B0;
				ReplayInterface = ModuleBase + 0x1FBD4F0;
				ViewPort = ModuleBase + 0x201DBA0;
				Camera = ModuleBase + 0x201ED50;
				bIsPlayerAiming = ModuleBase + 0x2D3839C;
				PlayerAimingAt = ModuleBase + 0x202C8D0;
				HandleBullet = ModuleBase + 0x101A660;
				BlipList = ModuleBase + 0x2023400;

				EntityType = 0x1098;
				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC90;
				DoorLock = 0x13C0;
				Handling = 0x960;
				VehicleState = 0x972;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0xD50;
				gravity = 0xC8C;
				invisible = 0x6A46EE;

				SyncTree = 0x1198558;
				Freecam_offset = 0x2552741; // resolved via the freecam camera-write signature on 3258

				buildNumber == 3258;
				Build_Found = true;
			}
		}


		else if (buildNumber == 3570)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GTA_b3570;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{
				World = ModuleBase + 0x25EC580;
				ReplayInterface = ModuleBase + 0x1FB0418;
				ViewPort = ModuleBase + 0x2058BA0;
				Camera = ModuleBase + 0x2059A48;
				bIsPlayerAiming = ModuleBase + 0x2D3839C;
				PlayerAimingAt = ModuleBase + 0x200FA10;
				HandleBullet = ModuleBase + 0x102D550;
				BlipList = ModuleBase + 0x2061870;
				Freecam_offset = 0x27D653;

				EntityType = 0x1098;
				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				Id = 0xE8;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC90;
				DoorLock = 0x13C0;
				Handling = 0x960;
				VehicleState = 0x972;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0xd50;
				gravity = 0xc9c;
				SyncTree = 0x118786C;
				invisible = 0x6AE372;

				buildNumber == 3570;
				Build_Found = true;
			}
		}

		else if (buildNumber == 3323)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GAME_b3323;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{

				World = ModuleBase + 0x25C15B0;
				ReplayInterface = ModuleBase + 0x1F85458;
				ViewPort = ModuleBase + 0x202DC50;
				Camera = ModuleBase + 0x202EB48;
				bIsPlayerAiming = ModuleBase + 0x2D3839C;
				PlayerAimingAt = ModuleBase + 0x200FA10;
				HandleBullet = ModuleBase + 0x1026CB0;
				BlipList = ModuleBase + 0x20333E0;

				EntityType = 0x1098;

				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC90;
				DoorLock = 0x13C0;
				Handling = 0x960;
				VehicleState = 0x972;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0xD40;

				SyncTree = 0x1198558;
				invisible = 0x6A51DA;

				buildNumber == 3323;
				Build_Found = true;
			}
		}

		else if (buildNumber == 3407)
		{
			Pid = GetGTAPID();
			if (Pid)
			{
				Pid = Pid;
				GameBuild = GAME_VERSION_GAME_b3407;
				ModuleName = GetProcessName(Pid);
			}

			ModuleBase = M_base::mem::GetModuleBaseByName(Pid, M_base::Misc::String2WString(ModuleName));
			if (ModuleBase)
			{

				World = ModuleBase + 0x25D7108;
				ReplayInterface = ModuleBase + 0x1F9A9D8;
				ViewPort = ModuleBase + 0x20431C0;
				Camera = ModuleBase + 0x20440C8;
				bIsPlayerAiming = ModuleBase + 0x2D3839C;
				PlayerAimingAt = ModuleBase + 0x200FA10;
				HandleBullet = ModuleBase + 0x102FF8C;


				BlipList = ModuleBase + 0x20440C8;


				EntityType = 0x1098;

				Armor = 0x150C;
				BoneManager = 0x430;
				PlayerInfo = 0x10A8;
				PlayerNetID = 0xE8;
				WeaponManager = 0x10B8;
				FragInsNmGTA = 0x1430;
				ConfigFlags = 0x1444;
				MaxHealth = 0x284;
				LastVehicle = 0xD10;
				Driver = 0xC90;
				DoorLock = 0x13C0;
				Handling = 0x960;
				VehicleState = 0x972;
				Stamina = 0xCF4;
				VisibleFlag = 0x145C;
				PlayerSpeed = 0xD50;
				gravity = 0xc9c;
				SyncTree = 0x1198558;
				Freecam_offset = 0x27BD43;
				invisible = 0x6AC2DA;

				buildNumber == 3407;
				Build_Found = true;
			}
		}

		if (Build_Found)
		{
			M_base::mem::AttachProces(Pid);
			InitializeServerInfo();
		}
		else
		{
			initializetime++;
			if (initializetime > 2)
			{
				exit(0);
			}
		}
	}

	std::string CFiveFramework::GetProcessName(DWORD processID) {
		HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processID);
		if (!hProcess) {
			return xorstr_("Unknown");
		}

		char processName[MAX_PATH] = "<Unknown>";
		if (GetModuleBaseNameA(hProcess, NULL, processName, sizeof(processName) / sizeof(char)) == 0) {
		}

		CloseHandle(hProcess);
		return std::string(processName);
	}

	uintptr_t CFiveFramework::GetRemoteModuleHandle(DWORD dwProcessId, const wchar_t* szModuleName) {
		HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, dwProcessId);
		if (hSnapshot == INVALID_HANDLE_VALUE)
			return 0;

		MODULEENTRY32 me32 = { sizeof(MODULEENTRY32) };

		if (Module32First(hSnapshot, &me32)) {
			do {
				if (_wcsicmp(me32.szModule, szModuleName) == 0) {
					CloseHandle(hSnapshot);
					return reinterpret_cast<uintptr_t>(me32.hModule);
				}
			} while (Module32Next(hSnapshot, &me32));
		}

		CloseHandle(hSnapshot);
		return 0;
	}


	int CFiveFramework::GetGTAPID() {
		HANDLE hProcessSnap;
		PROCESSENTRY32 pe32;
		int result = 0;

		hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
		if (hProcessSnap == INVALID_HANDLE_VALUE) {
			return 0;
		}

		pe32.dwSize = sizeof(PROCESSENTRY32);
		if (!Process32First(hProcessSnap, &pe32)) {
			CloseHandle(hProcessSnap);
			return 0;
		}

		do {
			char buffer[260] = { 0 };
			int converted = WideCharToMultiByte(CP_UTF8, 0, pe32.szExeFile, -1, buffer, 260, NULL, NULL);
			if (converted == 0) {
				continue;
			}

			std::string exeName = buffer;
			std::transform(exeName.begin(), exeName.end(), exeName.begin(), ::tolower);


			if (exeName.find("fivem") == 0 &&
				(exeName.find(xorstr_("gameprocess.exe")) != std::string::npos ||
					exeName.find(xorstr_("gtaprocess.exe")) != std::string::npos)) {
				result = static_cast<int>(pe32.th32ProcessID);
				break;
			}
		} while (Process32Next(hProcessSnap, &pe32));

		CloseHandle(hProcessSnap);

		return result;
	}

	int CFiveFramework::GetBuild(int Pid) {
		HANDLE hProcess = OpenProcess(PROCESS_VM_READ | PROCESS_QUERY_INFORMATION, FALSE, Pid);
		if (!hProcess) {
			return 0;
		}

		HMODULE hMods[1024];
		DWORD cbNeeded;
		uintptr_t baseAddress = 0;

		if (EnumProcessModules(hProcess, hMods, sizeof(hMods), &cbNeeded)) {
			baseAddress = reinterpret_cast<uintptr_t>(hMods[0]);
		}
		else {
			CloseHandle(hProcess);
			return 0;
		}

		MODULEINFO moduleInfo;
		if (!GetModuleInformation(hProcess, hMods[0], &moduleInfo, sizeof(moduleInfo))) {
			CloseHandle(hProcess);
			return 0;
		}

		std::string prefix = xorstr_("_SubProcess_game_");
		std::string suffix = xorstr_(".bin");

		const size_t CHUNK_SIZE = 4096;
		char* buffer = new char[CHUNK_SIZE + prefix.length()];

		const size_t ONE_MB = 1024 * 1024;
		const size_t HUNDRED_MB = 100 * ONE_MB;

		size_t maxSearchSize = HUNDRED_MB;
		if (static_cast<size_t>(moduleInfo.SizeOfImage) < maxSearchSize) {
			maxSearchSize = static_cast<size_t>(moduleInfo.SizeOfImage);
		}

		int result = 0;

		for (size_t offset = 0; offset < maxSearchSize; offset += CHUNK_SIZE) {
			SIZE_T bytesRead = 0;
			memset(buffer, 0, CHUNK_SIZE + prefix.length());

			if (!ReadProcessMemory(hProcess, (LPCVOID)(baseAddress + offset), buffer, CHUNK_SIZE, &bytesRead) || bytesRead == 0) {
				continue;
			}

			std::string chunk(buffer, bytesRead);

			size_t prefixPos = chunk.find(prefix);
			if (prefixPos != std::string::npos) {
				std::string extendedChunk = chunk;

				if (prefixPos + prefix.length() + 20 > bytesRead) {
					char extraBuffer[1024] = { 0 };
					SIZE_T extraBytesRead = 0;

					if (ReadProcessMemory(hProcess, (LPCVOID)(baseAddress + offset + bytesRead),
						extraBuffer, sizeof(extraBuffer), &extraBytesRead) && extraBytesRead > 0) {
						extendedChunk.append(extraBuffer, extraBytesRead);
					}
				}

				size_t suffixPos = extendedChunk.find(suffix, prefixPos);
				if (suffixPos != std::string::npos && suffixPos > prefixPos) {
					size_t versionStart = prefixPos + prefix.length();
					std::string versionStr = extendedChunk.substr(versionStart, suffixPos - versionStart);

					std::string numericPart;
					for (char c : versionStr) {
						if (std::isdigit(c)) {
							numericPart += c;
						}
						else {
							break;
						}
					}

					if (!numericPart.empty()) {
						try {
							result = std::stoi(numericPart);
							break;
						}
						catch (...) {}
					}
				}
			}
		}

		delete[] buffer;
		CloseHandle(hProcess);
		return result;
	}

	void CFiveFramework::InitializeServerInfo()
	{
		if (FivemFolder.empty() || CrashoMetryLocation.empty())
		{
			HKEY hKey;
			WCHAR Buffer[MAX_PATH];
			DWORD BufferSize = sizeof(Buffer);
			if (SafeCall(RegOpenKeyEx)(HKEY_CURRENT_USER, xorstr_(L"SOFTWARE\\CitizenFX\\FiveM"), 0, KEY_READ, &hKey) == ERROR_SUCCESS)
			{
				if (RegQueryValueEx(hKey, xorstr_(L"Last Run Location"), NULL, NULL, (LPBYTE)Buffer, &BufferSize) == ERROR_SUCCESS)
				{
					std::string BasePath = M_base::Misc::Wstring2String(std::wstring(Buffer));
					if (!BasePath.empty())
					{
						FivemFolder = BasePath;
						CrashoMetryLocation = BasePath + xorstr_("data\\cache\\crashometry");
					}
				}
				SafeCall(RegCloseKey)(hKey);
			}
		}

		if (!CrashoMetryLocation.empty())
		{
			std::ifstream Crashometry(CrashoMetryLocation, std::ios::binary);
			if (Crashometry.is_open())
			{
				std::string RawText((std::istreambuf_iterator<char>(Crashometry)), std::istreambuf_iterator<char>());
				if (!RawText.empty())
				{
					size_t StartPos = RawText.find(xorstr_("last_server_url"));
					if (StartPos != std::string::npos)
					{
						std::string TempServerIp = RawText.substr(StartPos + 15);

						if (TempServerIp.find(xorstr_("last_server_url")))
						{
							TempServerIp = TempServerIp.substr(TempServerIp.find(xorstr_("last_server_url")) + 15);
						}

						StartPos = TempServerIp.find(xorstr_("last_server"));

						TempServerIp = TempServerIp.substr(StartPos + 11);

						StartPos = TempServerIp.find(xorstr_(":"));

						ServerIp = TempServerIp.substr(0, StartPos);

						ServerPort = TempServerIp.substr(StartPos + 1, StartPos + 5);

					}
				}
			}
		}

		static bool isnameinit = false;
		if (ServerIp.size() > 4 && !isnameinit)
		{
			isnameinit = true;
			std::thread([&]() { this->UpdateNamesThread(); }).detach();
		}
	}

	float CFiveFramework::ImVec2Distance(const ImVec2& a, const ImVec2& b) {
		return sqrtf((a.x - b.x) * (a.x - b.x) + (a.y - b.y) * (a.y - b.y));
	}

	ImVec2 CFiveFramework::WorldToScreen2(Vector3D Pos)
	{
		if (!pViewPort)
		{
			pViewPort = M_base::mem::Read<uint64_t>(ViewPort);
			if (!pViewPort)
				return ImVec2(0, 0);
		}

		Matrix4x4 ViewMatrix = M_base::mem::Read<Matrix4x4>(pViewPort + 0x24C);
		ViewMatrix.TransposeThisMatrix();

		Vector4D VecX(ViewMatrix._21, ViewMatrix._22, ViewMatrix._23, ViewMatrix._24);
		Vector4D VecY(ViewMatrix._31, ViewMatrix._32, ViewMatrix._33, ViewMatrix._34);
		Vector4D VecZ(ViewMatrix._41, ViewMatrix._42, ViewMatrix._43, ViewMatrix._44);

		Vector3D ScreenPos;
		ScreenPos.x = (VecX.x * Pos.x) + (VecX.y * Pos.y) + (VecX.z * Pos.z) + VecX.w;
		ScreenPos.y = (VecY.x * Pos.x) + (VecY.y * Pos.y) + (VecY.z * Pos.z) + VecY.w;
		ScreenPos.z = (VecZ.x * Pos.x) + (VecZ.y * Pos.y) + (VecZ.z * Pos.z) + VecZ.w;

		ScreenPos.z = 1.0f / ScreenPos.z;
		ScreenPos.x *= ScreenPos.z;
		ScreenPos.y *= ScreenPos.z;


		ScreenPos.x += ImGui::GetIO().DisplaySize.x / 2 + float(0.5f * ScreenPos.x * ImGui::GetIO().DisplaySize.x + 0.5f);
		ScreenPos.y = ImGui::GetIO().DisplaySize.y / 2 - float(0.5f * ScreenPos.y * ImGui::GetIO().DisplaySize.y + 0.5f);

		return ImVec2(ScreenPos.x, ScreenPos.y);
	}

	ImVec2 CFiveFramework::WorldToScreen(Vector3D Pos)
	{
		if (!pViewPort)
		{
			pViewPort = M_base::mem::Read<uint64_t>(ViewPort);
			if (!pViewPort)
				return ImVec2(0, 0);
		}

		Matrix4x4 ViewMatrix = M_base::mem::Read<Matrix4x4>(pViewPort + 0x24C);

		ViewMatrix.TransposeThisMatrix();

		Vector4D VecX(ViewMatrix._21, ViewMatrix._22, ViewMatrix._23, ViewMatrix._24);
		Vector4D VecY(ViewMatrix._31, ViewMatrix._32, ViewMatrix._33, ViewMatrix._34);
		Vector4D VecZ(ViewMatrix._41, ViewMatrix._42, ViewMatrix._43, ViewMatrix._44);

		Vector3D ScreenPos;
		ScreenPos.x = (VecX.x * Pos.x) + (VecX.y * Pos.y) + (VecX.z * Pos.z) + VecX.w;
		ScreenPos.y = (VecY.x * Pos.x) + (VecY.y * Pos.y) + (VecY.z * Pos.z) + VecY.w;
		ScreenPos.z = (VecZ.x * Pos.x) + (VecZ.y * Pos.y) + (VecZ.z * Pos.z) + VecZ.w;

		if (ScreenPos.z <= 0.1f)
			return ImVec2(0, 0);

		ScreenPos.z = 1.0f / ScreenPos.z;
		ScreenPos.x *= ScreenPos.z;
		ScreenPos.y *= ScreenPos.z;

		ScreenPos.x += ImGui::GetIO().DisplaySize.x / 2 + float(0.5f * ScreenPos.x * ImGui::GetIO().DisplaySize.x + 0.5f);
		ScreenPos.y = ImGui::GetIO().DisplaySize.y / 2 - float(0.5f * ScreenPos.y * ImGui::GetIO().DisplaySize.y + 0.5f);

		return ImVec2(ScreenPos.x, ScreenPos.y);
	}

	// Snapshot the view-projection once per frame so bulk callers (vehicle ESP)
	// don't each read a 64-byte matrix from the game every single call.
	void CFiveFramework::CacheW2S()
	{
		m_w2sReady = false;
		if (!pViewPort)
		{
			pViewPort = M_base::mem::Read<uint64_t>(ViewPort);
			if (!pViewPort)
				return;
		}
		Matrix4x4 ViewMatrix = M_base::mem::Read<Matrix4x4>(pViewPort + 0x24C);
		ViewMatrix.TransposeThisMatrix();
		m_w2s[0] = ViewMatrix._21; m_w2s[1] = ViewMatrix._22; m_w2s[2] = ViewMatrix._23; m_w2s[3] = ViewMatrix._24;
		m_w2s[4] = ViewMatrix._31; m_w2s[5] = ViewMatrix._32; m_w2s[6] = ViewMatrix._33; m_w2s[7] = ViewMatrix._34;
		m_w2s[8] = ViewMatrix._41; m_w2s[9] = ViewMatrix._42; m_w2s[10] = ViewMatrix._43; m_w2s[11] = ViewMatrix._44;
		m_w2sReady = true;
	}

	ImVec2 CFiveFramework::WorldToScreenC(Vector3D Pos)
	{
		if (!m_w2sReady)
			return WorldToScreen(Pos);

		Vector3D s;
		s.x = m_w2s[0] * Pos.x + m_w2s[1] * Pos.y + m_w2s[2] * Pos.z + m_w2s[3];
		s.y = m_w2s[4] * Pos.x + m_w2s[5] * Pos.y + m_w2s[6] * Pos.z + m_w2s[7];
		s.z = m_w2s[8] * Pos.x + m_w2s[9] * Pos.y + m_w2s[10] * Pos.z + m_w2s[11];

		if (s.z <= 0.1f)
			return ImVec2(0, 0);

		s.z = 1.0f / s.z;
		s.x *= s.z;
		s.y *= s.z;

		s.x += ImGui::GetIO().DisplaySize.x / 2 + float(0.5f * s.x * ImGui::GetIO().DisplaySize.x + 0.5f);
		s.y = ImGui::GetIO().DisplaySize.y / 2 - float(0.5f * s.y * ImGui::GetIO().DisplaySize.y + 0.5f);
		return ImVec2(s.x, s.y);
	}

	float CalcDist(ImVec2 pos1, ImVec2 pos2)
	{
		float dx = pos2.x - pos1.x;
		float dy = pos2.y - pos1.y;
		return std::sqrt(dx * dx + dy * dy);
	}

	bool DeadCheck(Entity& Peds, Vector3D HeadPos, Vector3D ChestPos)
	{
		auto Ped = Peds.StaticInfo.Ped;

		float HDiff = std::abs(HeadPos.z - ChestPos.z);

		if (HDiff < 0.03f)
		{
			Peds.Dead = true;
			return true;
		}
		else
		{
			Peds.Dead = false;
			return false;
		}
	}

	// Resolves the ESP health/armor to display, filtering transient bad reads while still
	// showing 0 on real death. A >0 read is trusted and cached. A 0 read is only accepted as
	// real (bar -> 0) when the ped is ragdolled (deadPosture) OR it has stayed 0 for a few
	// consecutive reads; a single stray 0 (RPM read race) falls back to the last-good value.
	static void ResolveBars(int netId, float rawH, float rawA, bool deadPosture, float& outH, float& outA)
	{
		// Remote ped core health is 100-based (100 = 0 HP / dead, 200 = full). The SemiGod code
		// uses the same GetHealth()-100 convention. Convert to effective 0-100 HP for the bar.
		float effH = rawH - 100.f;

		PedBarFix* f = nullptr;
		for (auto& item : pedBarFix) if (item.id == netId) { f = &item; break; }
		if (!f) { pedBarFix.push_back({ netId, (effH > 0.f ? effH : 0.f), (rawA > 0.f ? rawA : 0.f), 0, 0 }); f = &pedBarFix.back(); }

		// effH > 0 = a trusted alive read. effH <= 0 is either a real death (posture / sustained)
		// or a one-off RPM read race -> fall back to the last-good value so the bar doesn't flicker.
		if (effH > 0.f) { f->health = effH; f->hZero = 0; outH = effH; }
		else { if (f->hZero < 1000000) f->hZero++; outH = (deadPosture || f->hZero >= 3) ? 0.f : f->health; }

		// Armor is already 0-100 (0 = no armor). Same debounce so armored players don't flicker.
		if (rawA > 0.f) { f->armor = rawA; f->aZero = 0; outA = rawA; }
		else { if (f->aZero < 1000000) f->aZero++; outA = (deadPosture || f->aZero >= 3) ? 0.f : f->armor; }
	}

	const float PI = 3.14159265358979323846f;

	float RAD2DEG(float radians) {
		return radians * (180.0f / PI);
	}

	int loop1 = 900;
	int loop2 = 1;
	int loop3 = 100;
	int loop4 = 10;
	int loop5 = 30;
	int loop6 = 900;


	int RefreshThreshold = 600;
	int RefreshThreshold2 = 80;

	int LastValidEntities = 0;
	bool vischeck = false;
	std::unordered_map<uint32_t, int> LastHealth;
	std::unordered_map<uint32_t, int> LastArmor;
	std::unordered_map<uint32_t, int> Azero;

	bool CFiveFramework::UpdateEntityList()
	{
		++loop1;
		++loop2;
		++loop3;
		++loop4;
		++loop5;

		if (loop1 > RefreshThreshold)
		{
			loop1 = 0;
			LockLists.lock();
			EntityList.clear();
			pReplayInterface = (CReplayInterface*)M_base::mem::Read<uint64_t>(ReplayInterface);
			pPedInterface = pReplayInterface->PedInterface();
			pCamGameplayDirector = (CCamGameplayDirector*)M_base::mem::Read<uint64_t>(Camera);
			pWorld = (CWorld*)M_base::mem::Read<uint64_t>(World);
			pLocalPlayer = pWorld->LocalPlayer();
			auto localplayer = C_Five.GetLocalPlayerInfo();
			auto localpos = localplayer.WorldPos;
			// Snapshot the view-projection ONCE for this whole rebuild so the per-ped
			// WorldToScreenC() calls below are pure math instead of each re-reading the
			// 64-byte view matrix from the game (that redundant read was the fps sink).
			CacheW2S();
			for (size_t i = 0; i < 256; i++)
			{
				CPed* Ped = pPedInterface->PedList()->Ped(i);
				if (!Ped) continue;

				uintptr_t PInfo = M_base::mem::Read<uintptr_t>(Ped + PlayerInfo);
				if (!PInfo) continue;

				PedStaticInfo StaticInfo;

				StaticInfo.Ped = Ped;
				StaticInfo.iIndex = i;
				StaticInfo.bIsLocalPlayer = (Ped == pLocalPlayer);

				auto NetId = Ped->GetPlayerInfo()->GetPlayerID();
				StaticInfo.NetId = NetId;

				Entity CurrentEntity;
				CurrentEntity.StaticInfo = StaticInfo;
				CurrentEntity.StaticInfo.Name = PlayerNames::GetPedName(Ped);
				CurrentEntity.Cordinates = Ped->GetCoordinate();
				CurrentEntity.PedLocation = WorldToScreenC(CurrentEntity.Cordinates);

				Vector3D HeadVEC = GetBonePosVec3(CurrentEntity, SKEL_Head);
				Vector3D LeftfootVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Foot);
				Vector3D RightfootVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Foot);
				Vector3D NeckVEC = GetBonePosVec3(CurrentEntity, SKEL_Neck_1);
				Vector3D ChestVEC = GetBonePosVec3(CurrentEntity, SKEL_Spine3);

				CurrentEntity.HeadVEC = HeadVEC;
				CurrentEntity.LeftFootVEC = LeftfootVEC;
				CurrentEntity.RightFootVEC = RightfootVEC;
				CurrentEntity.NeckVEC = NeckVEC;
				CurrentEntity.ChestVEC = ChestVEC;

				bool is_invisible = M_base::mem::Read<BYTE>(Ped + 0x2C) == 0x1;
				CurrentEntity.Visible = Ped->IsVisible() || is_invisible;

				CurrentEntity.HeadPos = WorldToScreenC(HeadVEC);
				CurrentEntity.LeftFootPos = WorldToScreenC(LeftfootVEC);
				CurrentEntity.RightFootPos = WorldToScreenC(RightfootVEC);
				CurrentEntity.NeckPos = WorldToScreenC(NeckVEC);
				CurrentEntity.ChestPos = WorldToScreenC(ChestVEC);

				// Seed health/armor through the shared resolver so the bars are stable from the
				// first frame (no flash to 0 on rebuild) and still read 0 on a real death.
				{
					bool deadP = DeadCheck(CurrentEntity, HeadVEC, ChestVEC);
					ResolveBars((int)NetId, Ped->GetHealth(), Ped->GetArmor(), deadP, CurrentEntity.Health, CurrentEntity.Armor);
				}
				CurrentEntity.Distance = localpos.DistTo(CurrentEntity.Cordinates);

				auto friendIt = NetIdFriendList.find(NetId);
				if (friendIt != NetIdFriendList.end()) {
					CurrentEntity.IsFriend = friendIt->second;
					CurrentEntity.StaticInfo.IsFriend = CurrentEntity.IsFriend;
					FriendList[Ped] = CurrentEntity.IsFriend;
				}
				else {
					CurrentEntity.IsFriend = FriendList[Ped];
					if (CurrentEntity.IsFriend) {
						NetIdFriendList[NetId] = true;
					}
					CurrentEntity.StaticInfo.IsFriend = CurrentEntity.IsFriend;
				}
				if (CurrentEntity.StaticInfo.bIsLocalPlayer) {
					LocalPlayerInfo.Ped = Ped;
					LocalPlayerInfo.iIndex = i;
					LocalPlayerInfo.PInfo = PInfo;
					LocalPlayerInfo.WorldPos = CurrentEntity.Cordinates;
					LocalPlayerInfo.ScreenPos = WorldToScreenC(CurrentEntity.Cordinates);
				}
				EntityList.push_back(CurrentEntity);
			}
					
			// Populate vehicle list.
			// Build into a LOCAL vector first (all the slow game reads happen with NO
			// lock held), then publish it with a single quick swap under LockLists2.
			// This keeps the render thread's vehicle-ESP draw from stalling behind the
			// ~256 per-vehicle memory reads done here — the cause of the fps drop.
			{
				std::vector<VehicleInfo> localVehicles;
				if (pReplayInterface) {
					CVehicleInterface* pVehicleInterface = pReplayInterface->VehicleInterface();
					if (pVehicleInterface) {
						CVehicleList* pVehicles = pVehicleInterface->VehicleList();
						if (pVehicles) {
							uint64_t vehicleCount = pVehicleInterface->VehicleMaximum();
							localVehicles.reserve(vehicleCount < 256 ? vehicleCount : 256);
							for (size_t i = 0; i < vehicleCount && i < 256; i++) {
								CVehicle* Vehicle = pVehicles->Vehicle(i);
								if (!Vehicle) continue;

								VehicleInfo CurrentVehicle;
								CurrentVehicle.Vehicle = Vehicle;
								CurrentVehicle.ModelInfo = Vehicle->GetModelInfo();
								CurrentVehicle.iIndex = i;
								CurrentVehicle.Cordinates = Vehicle->GetCoordinate();
								CurrentVehicle.Distance = localpos.DistTo(CurrentVehicle.Cordinates);
								CurrentVehicle.Health = Vehicle->GetHealth();
								CurrentVehicle.LockState = Vehicle->Is_Locked((uint64_t)Vehicle);
								CurrentVehicle.Driver = Vehicle->Has_Driver((uint64_t)Vehicle);
								CurrentVehicle.randomId = i;

								// Get vehicle name from model info
								if (CurrentVehicle.ModelInfo) {
									CurrentVehicle.Name = M_base::mem::ReadProcessMemoryString(CurrentVehicle.ModelInfo + 0x298, 24);
									if (CurrentVehicle.Name.size() > 24)
										continue;
								} else {
									CurrentVehicle.Name = "Unknown_Vehicle";
								}
								localVehicles.push_back(std::move(CurrentVehicle));
							}
						}
					}
				}
				LockLists2.lock();
				VehicleList.swap(localVehicles);
				LockLists2.unlock();
			}
					
			LockLists.unlock();
			return true;
		}

		if (loop4 > 10)
		{
			loop4 = 0;
			int validEntities = 0;
			LockLists.lock();
			for (const auto& CurrentEntity : EntityList)
			{
				if (CurrentEntity.StaticInfo.Ped && CurrentEntity.StaticInfo.NetId != -1)
					validEntities++;
			}
			// More aggressive thresholds for better performance
			if (validEntities >= 70) {
				RefreshThreshold = 700;
				RefreshThreshold2 = 120;
			}
			else if (validEntities >= 40) {
				RefreshThreshold = 400;
				RefreshThreshold2 = 70;
			}
			else if (validEntities >= 20) {
				RefreshThreshold = 200;
				RefreshThreshold2 = 40;
			}
			else if (validEntities >= 11) {
				RefreshThreshold = 150;
				RefreshThreshold2 = 25;
			}
			else {
				RefreshThreshold = 100;
				RefreshThreshold2 = 20;
			}
			if (LastValidEntities - validEntities >= 10) {
				loop1 = RefreshThreshold;
				loop3 = RefreshThreshold2;
			}
			LastValidEntities = validEntities;
			LockLists.unlock();
		}

		if (loop5 > 30)
		{
			auto localplayer = C_Five.GetLocalPlayerInfo();
			auto localpos = localplayer.WorldPos;
			loop5 = 0;
			LockLists2.lock();
			for (auto& CurrentVeh : VehicleList)
			{
				auto Vehicle = CurrentVeh.Vehicle;
				CurrentVeh.Cordinates = Vehicle->GetCoordinate();
				CurrentVeh.Distance = localpos.DistTo(CurrentVeh.Cordinates);
				CurrentVeh.Health = M_base::mem::Read<float>(Vehicle + 0x280);
			}
			LockLists2.unlock();
			LockLists.lock();
			for (auto& CurrentEntity : EntityList)
			{
				CurrentEntity.Distance = localpos.DistTo(CurrentEntity.Cordinates);
			}
			LockLists.unlock();
			return false;
		}

		if (loop3 > RefreshThreshold2)
		{
			loop3 = 0;
			LockLists.lock();
			for (auto& CurrentEntity : EntityList) {
				CPed* Ped = CurrentEntity.StaticInfo.Ped;
				Vector3D HVEC = GetBonePosVec3(CurrentEntity, SKEL_Head);
				Vector3D CVEC = GetBonePosVec3(CurrentEntity, SKEL_Spine3);
				bool t = DeadCheck(CurrentEntity, HVEC, CVEC);
				if (g_Options.ESP.ArmorBar || g_Options.ESP.HealthBar)
				{
					// Shared resolver: real value when alive, 0 on death, last-good on a stray 0 read.
					ResolveBars((int)CurrentEntity.StaticInfo.NetId, Ped->GetHealth(), Ped->GetArmor(), t, CurrentEntity.Health, CurrentEntity.Armor);
				}
				CurrentEntity.Cordinates = Ped->GetCoordinate();
				bool is_invisible = M_base::mem::Read<BYTE>(Ped + 0x2C) == 0x1;
				CurrentEntity.Visible = Ped->IsVisible() || is_invisible;
			}
			LockLists.unlock();
			return false;
		}

		if (loop2 > 0)
		{
			loop2 = 0;
			bool needSkeleton = g_Options.ESP.Skeleton;
			// One view-matrix read for the whole per-frame refresh (see loop1). Replaces
			// ~6 redundant matrix reads PER ped PER frame with cached pure-math projection.
			CacheW2S();
			LockLists.lock();
			for (auto& CurrentEntity : EntityList) {
				CPed* Ped = CurrentEntity.StaticInfo.Ped;
				if (!Ped) continue;
				CurrentEntity.Cordinates = Ped->GetCoordinate();
				CurrentEntity.PedLocation = WorldToScreenC(CurrentEntity.Cordinates);
				Vector3D HeadVEC = GetBonePosVec3(CurrentEntity, SKEL_Head);
				Vector3D LeftfootVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Foot);
				Vector3D RightfootVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Foot);
				
				// Only fetch neck/chest if needed
				Vector3D NeckVEC = GetBonePosVec3(CurrentEntity, SKEL_Neck_1);
				Vector3D ChestVEC = GetBonePosVec3(CurrentEntity, SKEL_Spine3);
				
				// Simplified interpolation - faster
				const float lerpFactor = 0.6f;
				auto LerpVec3 = [](const Vector3D& prev, const Vector3D& curr, float t) -> Vector3D {
					return Vector3D(
						prev.x + (curr.x - prev.x) * t,
						prev.y + (curr.y - prev.y) * t,
						prev.z + (curr.z - prev.z) * t
					);
				};
				
				// Apply interpolation if we have previous bones
				if (CurrentEntity.HasPrevBones) {
					CurrentEntity.HeadVEC = LerpVec3(CurrentEntity.PrevHeadVEC, HeadVEC, lerpFactor);
					CurrentEntity.NeckVEC = LerpVec3(CurrentEntity.PrevNeckVEC, NeckVEC, lerpFactor);
					CurrentEntity.LeftFootVEC = LerpVec3(CurrentEntity.PrevL_FootVEC, LeftfootVEC, lerpFactor);
					CurrentEntity.RightFootVEC = LerpVec3(CurrentEntity.PrevR_FootVEC, RightfootVEC, lerpFactor);
				} else {
					CurrentEntity.HeadVEC = HeadVEC;
					CurrentEntity.NeckVEC = NeckVEC;
					CurrentEntity.LeftFootVEC = LeftfootVEC;
					CurrentEntity.RightFootVEC = RightfootVEC;
				}
				
				// Store current as previous for next frame
				CurrentEntity.PrevHeadVEC = HeadVEC;
				CurrentEntity.PrevNeckVEC = NeckVEC;
				CurrentEntity.PrevL_FootVEC = LeftfootVEC;
				CurrentEntity.PrevR_FootVEC = RightfootVEC;
				CurrentEntity.HasPrevBones = true;
				
				CurrentEntity.ChestVEC = ChestVEC;
				CurrentEntity.HeadPos = WorldToScreenC(CurrentEntity.HeadVEC);
				CurrentEntity.LeftFootPos = WorldToScreenC(CurrentEntity.LeftFootVEC);
				CurrentEntity.RightFootPos = WorldToScreenC(CurrentEntity.RightFootVEC);
				CurrentEntity.NeckPos = WorldToScreenC(CurrentEntity.NeckVEC);
				CurrentEntity.ChestPos = WorldToScreenC(ChestVEC);
				
				// Only fetch skeleton bones if skeleton ESP is enabled - MAJOR OPTIMIZATION
				if (needSkeleton) {
					Vector3D pelvisRaw = GetBonePosVec3(CurrentEntity, SKEL_Pelvis);
					if (CurrentEntity.HasPrevBones && CurrentEntity.PrevPelvisVEC.x != 0) {
						CurrentEntity.PelvisVEC = LerpVec3(CurrentEntity.PrevPelvisVEC, pelvisRaw, lerpFactor);
					} else {
						CurrentEntity.PelvisVEC = pelvisRaw;
					}
					CurrentEntity.PrevPelvisVEC = pelvisRaw;
					
					CurrentEntity.L_UpperArmVEC = GetBonePosVec3(CurrentEntity, SKEL_L_UpperArm);
					CurrentEntity.L_ForearmVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Forearm);
					CurrentEntity.L_HandVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Hand);
					CurrentEntity.R_UpperArmVEC = GetBonePosVec3(CurrentEntity, SKEL_R_UpperArm);
					CurrentEntity.R_ForearmVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Forearm);
					CurrentEntity.R_HandVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Hand);
					CurrentEntity.L_ThighVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Thigh);
					CurrentEntity.L_CalfVEC = GetBonePosVec3(CurrentEntity, SKEL_L_Calf);
					CurrentEntity.R_ThighVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Thigh);
					CurrentEntity.R_CalfVEC = GetBonePosVec3(CurrentEntity, SKEL_R_Calf);
				}
				
				pLocalPlayer = pWorld->LocalPlayer();
				CurrentEntity.StaticInfo.bIsLocalPlayer = (Ped == pLocalPlayer);
				if (CurrentEntity.StaticInfo.bIsLocalPlayer) {
					LocalPlayerInfo.WorldPos = CurrentEntity.Cordinates;
					LocalPlayerInfo.ScreenPos = WorldToScreenC(CurrentEntity.Cordinates);
				}
			}
			LockLists.unlock();
			return false;
		}
		return false;
	}

	Vector3D CFiveFramework::GetClosestHitBox(Entity Ped)
	{
		Vector3D ClosestBone = { 0, 0, 0 };
		float ClosestDistance = FLT_MAX;

		auto FollowPedCamera = pCamGameplayDirector->GetFollowPedCamera();
		Vector3D CrossPos = FollowPedCamera->GetCrosshairPosition();
		ImVec2 CrosshairScr = WorldToScreen2(CrossPos);

		struct BoneData {
			Vector3D worldPos;
			unsigned int boneMask;
		};

		BoneData bones[] = {
			{ Ped.HeadVEC, SKEL_Head },
			{ Ped.NeckVEC, SKEL_Neck_1 },
			{ Ped.ChestVEC, SKEL_Spine3 },
			{ Ped.LeftFootVEC, SKEL_L_Foot },
			{ Ped.RightFootVEC, SKEL_R_Foot },
			{ GetBonePosVec3(Ped, SKEL_Pelvis), SKEL_Pelvis },
			{ GetBonePosVec3(Ped, SKEL_Spine_Root), SKEL_Spine_Root },
			{ GetBonePosVec3(Ped, SKEL_Spine0), SKEL_Spine0 },
			{ GetBonePosVec3(Ped, SKEL_Spine1), SKEL_Spine1 },
			{ GetBonePosVec3(Ped, SKEL_Spine2), SKEL_Spine2 },
			{ GetBonePosVec3(Ped, SKEL_L_Clavicle), SKEL_L_Clavicle },
			{ GetBonePosVec3(Ped, SKEL_R_Clavicle), SKEL_R_Clavicle },
			{ GetBonePosVec3(Ped, SKEL_L_UpperArm), SKEL_L_UpperArm },
			{ GetBonePosVec3(Ped, SKEL_R_UpperArm), SKEL_R_UpperArm },
			{ GetBonePosVec3(Ped, SKEL_L_Forearm), SKEL_L_Forearm },
			{ GetBonePosVec3(Ped, SKEL_R_Forearm), SKEL_R_Forearm },
			{ GetBonePosVec3(Ped, SKEL_L_Hand), SKEL_L_Hand },
			{ GetBonePosVec3(Ped, SKEL_R_Hand), SKEL_R_Hand },
			{ GetBonePosVec3(Ped, SKEL_L_Thigh), SKEL_L_Thigh },
			{ GetBonePosVec3(Ped, SKEL_R_Thigh), SKEL_R_Thigh },
			{ GetBonePosVec3(Ped, SKEL_L_Calf), SKEL_L_Calf },
			{ GetBonePosVec3(Ped, SKEL_R_Calf), SKEL_R_Calf },
			{ GetBonePosVec3(Ped, SKEL_L_Toe0), SKEL_L_Toe0 },
			{ GetBonePosVec3(Ped, SKEL_R_Toe0), SKEL_R_Toe0 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger00), SKEL_L_Finger00 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger01), SKEL_L_Finger01 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger02), SKEL_L_Finger02 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger10), SKEL_L_Finger10 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger11), SKEL_L_Finger11 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger12), SKEL_L_Finger12 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger20), SKEL_L_Finger20 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger21), SKEL_L_Finger21 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger22), SKEL_L_Finger22 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger30), SKEL_L_Finger30 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger31), SKEL_L_Finger31 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger32), SKEL_L_Finger32 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger40), SKEL_L_Finger40 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger41), SKEL_L_Finger41 },
			{ GetBonePosVec3(Ped, SKEL_L_Finger42), SKEL_L_Finger42 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger00), SKEL_R_Finger00 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger01), SKEL_R_Finger01 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger02), SKEL_R_Finger02 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger10), SKEL_R_Finger10 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger11), SKEL_R_Finger11 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger12), SKEL_R_Finger12 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger20), SKEL_R_Finger20 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger21), SKEL_R_Finger21 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger22), SKEL_R_Finger22 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger30), SKEL_R_Finger30 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger31), SKEL_R_Finger31 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger32), SKEL_R_Finger32 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger40), SKEL_R_Finger40 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger41), SKEL_R_Finger41 },
			{ GetBonePosVec3(Ped, SKEL_R_Finger42), SKEL_R_Finger42 },
			{ GetBonePosVec3(Ped, MH_L_Elbow), MH_L_Elbow },
			{ GetBonePosVec3(Ped, MH_R_Elbow), MH_R_Elbow },
			{ GetBonePosVec3(Ped, MH_L_Knee), MH_L_Knee },
			{ GetBonePosVec3(Ped, MH_R_Knee), MH_R_Knee },
			{ GetBonePosVec3(Ped, RB_L_ThighRoll), RB_L_ThighRoll },
			{ GetBonePosVec3(Ped, RB_R_ThighRoll), RB_R_ThighRoll },
			{ GetBonePosVec3(Ped, RB_L_ForeArmRoll), RB_L_ForeArmRoll },
			{ GetBonePosVec3(Ped, RB_R_ForeArmRoll), RB_R_ForeArmRoll },
			{ GetBonePosVec3(Ped, RB_L_ArmRoll), RB_L_ArmRoll },
			{ GetBonePosVec3(Ped, RB_R_ArmRoll), RB_R_ArmRoll },
			{ GetBonePosVec3(Ped, IK_L_Foot), IK_L_Foot },
			{ GetBonePosVec3(Ped, IK_R_Foot), IK_R_Foot },
			{ GetBonePosVec3(Ped, IK_L_Hand), IK_L_Hand },
			{ GetBonePosVec3(Ped, IK_R_Hand), IK_R_Hand },
			{ GetBonePosVec3(Ped, IK_Head), IK_Head },
			{ GetBonePosVec3(Ped, IK_Root), IK_Root },
			{ GetBonePosVec3(Ped, PH_L_Foot), PH_L_Foot },
			{ GetBonePosVec3(Ped, PH_R_Foot), PH_R_Foot },
			{ GetBonePosVec3(Ped, PH_L_Hand), PH_L_Hand },
			{ GetBonePosVec3(Ped, PH_R_Hand), PH_R_Hand },
			{ GetBonePosVec3(Ped, RB_Neck_1), RB_Neck_1 },
			{ GetBonePosVec3(Ped, FACIAL_facialRoot), FACIAL_facialRoot },
			{ GetBonePosVec3(Ped, FB_L_Brow_Out_000), FB_L_Brow_Out_000 },
			{ GetBonePosVec3(Ped, FB_R_Brow_Out_000), FB_R_Brow_Out_000 },
			{ GetBonePosVec3(Ped, FB_Brow_Centre_000), FB_Brow_Centre_000 },
			{ GetBonePosVec3(Ped, FB_L_Eye_000), FB_L_Eye_000 },
			{ GetBonePosVec3(Ped, FB_R_Eye_000), FB_R_Eye_000 },
			{ GetBonePosVec3(Ped, FB_L_CheekBone_000), FB_L_CheekBone_000 },
			{ GetBonePosVec3(Ped, FB_R_CheekBone_000), FB_R_CheekBone_000 },
			{ GetBonePosVec3(Ped, FB_Jaw_000), FB_Jaw_000 },
			{ GetBonePosVec3(Ped, FB_UpperLipRoot_000), FB_UpperLipRoot_000 },
			{ GetBonePosVec3(Ped, FB_UpperLip_000), FB_UpperLip_000 },
			{ GetBonePosVec3(Ped, FB_LowerLipRoot_000), FB_LowerLipRoot_000 },
			{ GetBonePosVec3(Ped, FB_LowerLip_000), FB_LowerLip_000 },
			{ GetBonePosVec3(Ped, FB_L_Lip_Top_000), FB_L_Lip_Top_000 },
			{ GetBonePosVec3(Ped, FB_R_Lip_Top_000), FB_R_Lip_Top_000 },
			{ GetBonePosVec3(Ped, FB_L_Lip_Bot_000), FB_L_Lip_Bot_000 },
			{ GetBonePosVec3(Ped, FB_R_Lip_Bot_000), FB_R_Lip_Bot_000 },
			{ GetBonePosVec3(Ped, FB_L_Lip_Corner_000), FB_L_Lip_Corner_000 },
			{ GetBonePosVec3(Ped, FB_R_Lip_Corner_000), FB_R_Lip_Corner_000 },
			{ GetBonePosVec3(Ped, FB_Tongue_000), FB_Tongue_000 }
		};

		for (const auto& bone : bones)
		{
			if (bone.worldPos.x == 0 && bone.worldPos.y == 0 && bone.worldPos.z == 0)
				continue;

			ImVec2 boneScr = WorldToScreen(bone.worldPos);
			if (boneScr.x <= 0 || boneScr.y <= 0)
				continue;

			float distance = CalcDist(CrosshairScr, boneScr);
			if (distance < ClosestDistance)
			{
				ClosestDistance = distance;
				ClosestBone = bone.worldPos;
			}
		}

		if (ClosestBone.x == 0 && ClosestBone.y == 0 && ClosestBone.z == 0)
		{
			ClosestBone = Ped.HeadVEC;
		}

		return ClosestBone;
	}


	bool CFiveFramework::FindClosestEntity(float Fov, int MaxDistance, bool vischeck, Entity* Output, bool ignoreDead, bool ignoreInvisible, bool ignoreFriends, bool ignoreNPC)
	{
		Entity Closest;
		float ClosestScreenDistance = FLT_MAX;
		bool Found = false;
		auto localplayer = C_Five.GetLocalPlayerInfo();

		Vector3D localPlayerPos = localplayer.WorldPos;

		ImVec2 Center(ImGui::GetIO().DisplaySize.x * 0.5f, ImGui::GetIO().DisplaySize.y * 0.5f);
		float displayW = ImGui::GetIO().DisplaySize.x;
		float displayH = ImGui::GetIO().DisplaySize.y;

		LockLists.lock();
		for (const Entity& Current : EntityList)
		{
			try {
				if (Current.StaticInfo.bIsLocalPlayer)
					continue;

				// Ignore options
				if (ignoreDead && Current.Dead)
					continue;

				if (ignoreFriends && Current.IsFriend)
					continue;

				if (ignoreNPC && Current.StaticInfo.bIsNPC)
					continue;

				if (ignoreInvisible && !Current.Visible)
					continue;

				if (vischeck && !Current.Visible)
					continue;

				float WorldDistance = Current.Cordinates.DistTo(localPlayerPos);

				if (WorldDistance > MaxDistance)
					continue;

				// First do a quick screen check with head position (already cached in entity)
				ImVec2 HeadScreen = WorldToScreen(Current.HeadVEC);
				
				// Quick bounds check
				if (HeadScreen.x <= 0 || HeadScreen.y <= 0 ||
					HeadScreen.x >= displayW || HeadScreen.y >= displayH)
					continue;

				// Quick FOV check with head first (cheaper than GetClosestHitBox)
				float QuickScreenDistance = ImVec2Distance(HeadScreen, Center);
				
				// If head is way outside FOV, skip this entity entirely
				if (QuickScreenDistance > Fov * 2.0f)
					continue;

				// Now do the expensive GetClosestHitBox only for entities that might be in FOV
				Vector3D ClosestBone = GetClosestHitBox(Current);
				ImVec2 ClosestBoneScreen = WorldToScreen(ClosestBone);

				if (ClosestBoneScreen.x <= 0 || ClosestBoneScreen.y <= 0 ||
					ClosestBoneScreen.x >= displayW || ClosestBoneScreen.y >= displayH)
					continue;

				float ScreenDistance = ImVec2Distance(ClosestBoneScreen, Center);

				if (ScreenDistance > Fov)
					continue;

				if (ScreenDistance < ClosestScreenDistance)
				{
					ClosestScreenDistance = ScreenDistance;
					Closest = Current;
					Found = true;
				}
			}
			catch (...) {
				continue;
			}
		}
		LockLists.unlock();

		if (Found)
			*Output = Closest;

		return Found;
	}

	Vector3D CFiveFramework::GetBone(Entity Ped, int Bone)
	{
		uintptr_t ped = reinterpret_cast<uintptr_t>(Ped.StaticInfo.Ped);
		if (!Ped.StaticInfo.Ped)
			return Vector3D(0, 0, 0);

		Matrix4x4 matrix = M_base::mem::Read<Matrix4x4>(ped + 0x60);

		if (Bone < 0)
			return Vector3D(0, 0, 0);

		Vector3D bone;

		// Bone offset'leri daha doğru hale getirildi
		if (GameBuild > 11) {
			// Yeni build'ler için daha doğru offset'ler
			switch (Bone) {
			case 0: // Head
				bone = M_base::mem::Read<Vector3D>(ped + 0x410);
				break;
			case 1: // Neck
				bone = M_base::mem::Read<Vector3D>(ped + 0x420);
				break;
			case 2: // Chest
				bone = M_base::mem::Read<Vector3D>(ped + 0x430);
				break;
			default:
				bone = M_base::mem::Read<Vector3D>(ped + (0x410 + 0x10 * Bone));
				break;
			}
		}
		else {
			// Eski build'ler için
			switch (Bone) {
			case 0: // Head
				bone = M_base::mem::Read<Vector3D>(ped + 0x430);
				break;
			case 1: // Neck
				bone = M_base::mem::Read<Vector3D>(ped + 0x440);
				break;
			case 2: // Chest
				bone = M_base::mem::Read<Vector3D>(ped + 0x450);
				break;
			default:
				bone = M_base::mem::Read<Vector3D>(ped + (0x430 + Bone * 0x10));
				break;
			}
		}

		// Manual matrix-vector transform (replaces D3DXVec3Transform)
		float x = bone.x * matrix._11 + bone.y * matrix._21 + bone.z * matrix._31 + matrix._41;
		float y = bone.x * matrix._12 + bone.y * matrix._22 + bone.z * matrix._32 + matrix._42;
		float z = bone.x * matrix._13 + bone.y * matrix._23 + bone.z * matrix._33 + matrix._43;
		return Vector3D(x, y, z);
	}


	Vector3D GetBonePosByInstFragAndID(uint64_t crSkeletonData, unsigned int BoneID)
	{
		Matrix4x4 v4 = M_base::mem::Read<Matrix4x4>(M_base::mem::Read<uint64_t>(crSkeletonData + 0x8));
		Matrix4x4 Result = M_base::mem::Read<Matrix4x4>(M_base::mem::Read<uint64_t>(crSkeletonData + 0x18) + (BoneID << 6));

		Vector3D vec1(v4._11, v4._12, v4._13);
		Vector3D vec2(v4._21, v4._22, v4._23);
		Vector3D vec3(v4._31, v4._32, v4._33);
		Vector3D vec4(v4._41, v4._42, v4._43);
		Vector3D vec5(Result._41, Result._42, Result._43);

		return Vector3D(
			vec1.x * vec5.x + vec4.x + vec2.x * vec5.y + vec3.x * vec5.z,
			vec1.y * vec5.x + vec4.y + vec2.y * vec5.y + vec3.y * vec5.z,
			vec1.z * vec5.x + vec4.z + vec2.z * vec5.y + vec3.z * vec5.z
		);
	}

	Vector3D CFiveFramework::GetBonePosVec3(Entity Ped, unsigned int Mask)
	{
		uint64_t FragInstNMGta = M_base::mem::Read<uint64_t>(Ped.StaticInfo.Ped + FragInsNmGTA);
		if (FragInstNMGta)
		{
			Ped.StaticInfo.crSkeletonData = M_base::mem::Read<uint64_t>(M_base::mem::Read<uint64_t>(FragInstNMGta + 0x68) + 0x178);

			auto it = Ped.StaticInfo.MaskToBoneId.find(Mask);
			if (it == Ped.StaticInfo.MaskToBoneId.end())
			{
				unsigned int BoneId = 0;
				if (GetPedBoneIndex(Ped, Mask, BoneId))
				{
					if (BoneId)
					{
						Ped.StaticInfo.MaskToBoneId[Mask] = BoneId;
						return GetBonePosByInstFragAndID(Ped.StaticInfo.crSkeletonData, BoneId);
					}
				}
			}
			else
			{
				return GetBonePosByInstFragAndID(Ped.StaticInfo.crSkeletonData, it->second);
			}
		}

		return Vector3D(0, 0, 0);
	}

	bool CFiveFramework::GetPedBoneIndex(Entity Ped, unsigned int Mask, unsigned int& newIdx)
	{
		uint64_t crSkeletonData = M_base::mem::Read<uint64_t>(Ped.StaticInfo.crSkeletonData);

		if (M_base::mem::Read<int16_t>(crSkeletonData + 0x1A))
		{
			uint16_t v1 = M_base::mem::Read<uint16_t>(crSkeletonData + 0x18);
			if (v1)
			{
				int64_t v2 = M_base::mem::Read<int64_t>(crSkeletonData + 0x10);
				int Count = 0;
				for (int64_t i = M_base::mem::Read<int64_t>(v2 + 0x8 * (Mask % v1)); ; i = M_base::mem::Read<int64_t>(i + 0x8))
				{
					Count++;
					if (!i || i >= 0xCCCCCCCCCCCCCC || Count > 3)
						return false;

					int v5 = M_base::mem::Read<int>(i);
					if (Mask == v5)
					{
						int v6 = M_base::mem::Read<int>(i + 0x4);
						newIdx = v6;
						return true;
					}
				}
			}
		}
		else if (Mask < M_base::mem::Read<uint64_t>(crSkeletonData + 0x5E))
		{
			newIdx = Mask;
			return true;
		}

		return false;
	}

	void CFiveFramework::CameraMove(Vector3D WorldPosition, int SmoothHorizontal, int SmoothVertical)
	{
		auto FollowPedCamera = pCamGameplayDirector->GetFollowPedCamera();

		Vector3D crosspos = FollowPedCamera->GetCrosshairPosition();
		Vector3D ViewAngles = FollowPedCamera->GetViewAngles();

		Vector3D Direction = WorldPosition - crosspos;
		float Distance = Direction.Length();

		Vector3D AimAngles = { Direction.x / Distance, Direction.y / Distance, Direction.z / Distance };

		Vector3D CamDelta = AimAngles - ViewAngles;
		Vector3D FinalAngles = ViewAngles;

		if (SmoothHorizontal > 1) {
			FinalAngles.x += CamDelta.x / SmoothHorizontal;
			FinalAngles.y += CamDelta.y / SmoothHorizontal;
		}
		else if (SmoothHorizontal == 0) {
			FinalAngles.x += CamDelta.x;
			FinalAngles.y += CamDelta.y;
		}

		if (SmoothVertical > 1) {
			FinalAngles.z += CamDelta.z / SmoothVertical;
		}
		else if (SmoothVertical == 0) {
			FinalAngles.z += CamDelta.z;
		}

		float AimbotFixZ = ViewAngles.z - FollowPedCamera->GetThirdpersonViewAngles().z;
		Vector3D ThirdPersonAngles = FinalAngles;
		ThirdPersonAngles.z -= AimbotFixZ;

		FollowPedCamera->SetThirdpersonViewAngles(ThirdPersonAngles);
		FollowPedCamera->SetViewAngles(FinalAngles);
	}

	int getPt(int n1, int n2, float perc) {
		return n1 + static_cast<int>((n2 - n1) * perc);
	}

	int getRnd(int min, int max) {
		return (rand() % (max - min)) + min;
	}

	Vector3D CFiveFramework::GetPt(Vector3D A, Vector3D B, float t) {
		return {
			A.x + t * (B.x - A.x),
			A.y + t * (B.y - A.y),
			A.z + t * (B.z - A.z)
		};
	}

	void CFiveFramework::HumanizedAim(Vector3D WorldPosition)
	{
		auto FollowPedCamera = pCamGameplayDirector->GetFollowPedCamera();

		Vector3D crosspos = FollowPedCamera->GetCrosshairPosition();
		Vector3D ViewAngles = FollowPedCamera->GetViewAngles();

		Vector3D Direction = WorldPosition - crosspos;
		float Distance = Direction.Length();

		Vector3D AimAngles = { Direction.x / Distance, Direction.y / Distance, Direction.z / Distance };
		Vector3D CamDelta = AimAngles - ViewAngles;

		Vector3D PInicial = ViewAngles;
		Vector3D PMedio1 = PInicial + (CamDelta * getRnd(0.2f, 0.4f));
		Vector3D PMedio2 = PInicial + (CamDelta * getRnd(0.5f, 0.8f));
		Vector3D PFinal = PInicial + CamDelta;

		float AimbotFixZ = ViewAngles.z - FollowPedCamera->GetThirdpersonViewAngles().z;

		for (float t = 0.0f; t <= 1.01f; t += 0.01f) {
			float easedT = t * t * (3 - 2 * t);

			Vector3D XA = GetPt(PInicial, PMedio1, easedT);
			Vector3D XB = GetPt(PMedio1, PMedio2, easedT);
			Vector3D XC = GetPt(PMedio2, PFinal, easedT);

			Vector3D XM = GetPt(XA, XB, easedT);
			Vector3D XN = GetPt(XB, XC, easedT);

			Vector3D TargetAngles = GetPt(XM, XN, easedT);

			Vector3D ThirdPersonAngles = TargetAngles;
			ThirdPersonAngles.z -= AimbotFixZ;

			FollowPedCamera->SetThirdpersonViewAngles(ThirdPersonAngles);
			FollowPedCamera->SetViewAngles(TargetAngles);

		}
	}

	void CFiveFramework::TeleportFunc(uintptr_t Object, uintptr_t Navigation, uintptr_t ModelInfo, Vector3D Position, Vector3D VisualPosition, bool Stop)
	{
		float BackupMagic = 0.f;
		if (Stop)
		{
			BackupMagic = M_base::mem::Read<float>(ModelInfo + 0x2C);
			M_base::mem::WriteMemory(ModelInfo + 0x2C, 0.f);
		}

		M_base::mem::WriteMemory(Object + 0x90, VisualPosition);
		M_base::mem::WriteMemory(Navigation + 0x50, Position);

		if (Stop)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(40));
			M_base::mem::WriteMemory(ModelInfo + 0x2C, BackupMagic);
		}
	}

	nlohmann::json previousServerInfo = nlohmann::json();

	nlohmann::json CFiveFramework::GetServerInfo()
	{
		if (!ServerIp.empty())
		{
			std::string RawServerInfo = M_base::Misc::DownloadServerInfo(M_base::Misc::String2WString(ServerIp), std::atoi(ServerPort.c_str()));

			if (RawServerInfo.empty())
			{
				return previousServerInfo;
			}

			try
			{
				nlohmann::json parsedData = nlohmann::json::parse(RawServerInfo);
				previousServerInfo = parsedData;
				return parsedData;
			}
			catch (const nlohmann::json::parse_error& ex)
			{
			}
		}

		return previousServerInfo.empty() ? nlohmann::json() : previousServerInfo;
	}

	void CFiveFramework::UpdateNamesThread()
	{
		while (!g_Options.General.ShutDown)
		{
			// Fetch server player info (names + identifiers). Runs regardless of ESP so the
			// Players panel has data; a sleep always follows (no CPU spin when ESP is off).
			try
			{
				PlayersInfo = GetServerInfo();

				for (const auto& Player : PlayersInfo)
				{
					int pid = Player[xorstr_("id")];
					std::string pname = Player[xorstr_("name")];
					PlayerIdToName[pid] = pname;

					if (Player.contains("identifiers"))
					{
						for (const auto& ident : Player["identifiers"])
						{
							std::string s = ident.get<std::string>();
							// Key by BOTH the server id and the player name. The in-game NetId
							// often differs from the server "id" field, so the name bridge lets
							// the Players panel resolve identifiers reliably.
							if (s.rfind("discord:", 0) == 0)    { PlayerIdToDiscord[pid] = s.substr(8); PlayerNameToDiscord[pname] = s.substr(8); }
							else if (s.rfind("steam:", 0) == 0) { PlayerIdToSteam[pid]   = s.substr(6); PlayerNameToSteam[pname]   = s.substr(6); }
						}
					}
				}
			}
			catch (...) {}

			std::this_thread::sleep_for(std::chrono::seconds(30));
		}
	}

	bool CFiveFramework::IsOnScreen(ImVec2 Pos)
	{
		if (Pos.x < 0.1f || Pos.y < 0.1 || Pos.x > ImGui::GetIO().DisplaySize.x || Pos.y > ImGui::GetIO().DisplaySize.y)
			return false;

		return true;
	}

}
