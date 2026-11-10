#pragma once

#include <Windows.h>
#include <string>
#include "FivemSDK/GTADefines.hpp"
#include "../Utils/Math/Vectors/Vector3D.hpp"

namespace Cheat
{
	class Options
	{
	public:
		struct General
		{
			bool ShutDown = false;
			int MenuKey = 0x2D; // INSERT key code
			float ThreadDelay = 0;
			bool StreamProof = true;
			bool HideOnAltTab = true; // Hide ESP/FOV when FiveM is not focused
			bool ShowActiveFeatures = true; // show the on-screen active-features list
		}General;
		struct Player {
			// enable option removed as requested
			bool God = false;
			int GodKey = 0;
			bool SemiGod = false;
			int SemiGodKey = 0;
			float SemiGodHealth = 70.0f;
			bool Passive = false;
			int PassiveKey = 0;
			bool NoRecoil = false;
			float NoRecoilValue = 0.f;
			bool NoReload = false;
			float NoReloadValue = 1000.f;
			bool NoSpread = false;
			float NoSpreadValue = 0.f;
			bool RangeModify = false;
			float RangeModifyValue = 1000.f;
			bool DamageBoost = false;
			float DamageAmount;
			bool SpeedModify = false;
			float SpeedAmount;
			bool VehicleSpeed = false;
			float VehicleSpeedAmount;
			bool VehicleGod = false;
			bool Health = false;
			float HealthAmmount;
			int HealthKey = 0;
			bool Armor = false;
			float ArmorAmmount;
			int ArmorKey = 0;
			bool PSpeed = false;
			float PSpeedAmount = 1.0f;
			bool CamFovModify = false;
			float CamFovAmount;
			bool SDamageBoost = false;
			int SDamageBoostKey = 0;
			float SDamageBoostValue;
			bool InfiniteAmmo = false;
			bool bIsSpectatingEntity = false;
			uint64_t SpectatingEntity;
			bool godtest = false;
			bool Invisible = false;
			bool Nocollision = false;
			bool beastmode = false;
			bool noclip = false;
			int NoclipKey = 0;
			float noclipspeed = 100;
			bool Absorption = false;
			float AbsorptionPercent = 50.0f;
			bool NoFallDamage = false;
			bool Shrink = false;
			float ShrinkAmount = 0.7f; // ModelInfo scale multiplier (1.0 = normal, lower = smaller)
			bool Freecam = false;
			int FreecamKey = 0;
			float FreecamSpeed = 5.0f;
			int FixVehicleKey = 0;
			
			// Teleportation options
			bool TeleportToWaypoint = false;
			int TeleportToWaypointKey = 0;
			bool TeleportToSavedLocation = false;
			int TeleportToSavedLocationKey = 0;
			bool FreecamTeleport = false;
			int FreecamTeleportKey = 0;
			Vector3D SavedLocation;
			bool HasSavedLocation;
			
			// Ghost Peek
			bool GhostPeekEnabled = false;
			int GhostPeekMarkKey = 0;
			int GhostPeekReturnKey = 0;
			Vector3D GhostPeekPosition;
			bool HasGhostPeekPosition = false;
		}Player;
		struct Vehicle {
			bool enablevehicleesp = false;
			bool drawlocalvehicle = false;
			bool drawenemyvehicle = false;
			bool vehiclemarker = false;
			bool vehiclehealth = false;
			bool vehiclesnapline = false;
			bool vehicledistance = false;
		}Vehicle;
		struct Radar {
			bool enable = false;
			bool showself = false;
			int range = 100;
			int size = 200;
			int position = 0; // 0 = top-left, 1 = top-right, 2 = bottom-left, 3 = bottom-right
			float customPosX = 18.0f; // default: aligned under the watermark
			float customPosY = 74.0f; // default: just below the watermark (18,18 + ~46h + gap)
			bool isDragging = false; // Track if radar is being dragged
			bool useCustomPos = true;  // open below the watermark by default (still draggable)
			bool showPlayers = true;
			bool showVehicles = false;   // default off (only Waves + Players on)
			bool showNames = false;
			bool showDistance = false;
			bool showDirection = true;
			bool circleStyle = false; // Set to square style
			bool showWaves = true; // Toggle concentric circles/waves
			bool animateWaves = false; // default off (only Waves + Players on)
			float backgroundColor[4] = { 0.2f, 0.2f, 0.2f, 0.6f }; // Fully opaque grey background
			float borderColor[4] = { 0.5f, 0.5f, 0.5f, 0.0f }; // Border completely transparent (effectively removed)
			float playerColor[4] = { 10.f, 1.f, 1.f, 1.f };
			float enemyColor[4] = { 1.f, 0.f, 0.f, 1.f };
			float vehicleColor[4] = { 0.5f, 0.8f, 1.f, 1.f }; // Light blue for vehicles
			float friendColor[4] = { 0.f, 1.f, 0.f, 1.f };
		}Radar;
		
		struct PlayersAround {
			bool enable = false;
			int distance = 100; // metre
			bool showDistance = true;
			float posX = 18.0f;   // default: aligned under the radar
			float posY = 284.0f;  // default: below the radar (74 + 200 size + gap)
			bool isDragging = false;
		}PlayersAround;
		
		struct Other {
			bool watermark = true;
			float watermarkPosX = -1.0f; // -1 means auto (top-right)
			float watermarkPosY = 10.0f;
			bool watermarkDragging = false;
			bool enableAnimations = true; // Toggle for page transitions and switch animations
		}Other;
		struct Crosshair {
			bool enable = false;
			int style = 0;                 // 0 = cross, 1 = cross + dot, 2 = dot only, 3 = circle
			float color[4] = { 1.f, 1.f, 1.f, 1.f };
			float size = 8.f;
			float thickness = 2.f;
			float gap = 4.f;
		}Crosshair;
		struct Aimbot {
			bool enable = false;
			bool prediction = false;
			float predictionAmount = 0.15f;
			bool humanizedAim = false;
			float AimbotFovColor[4] = { 1.f, 1.f, 1.f, 1.f };
			int fov=70;
			float xsmooth=0;
			float ysmooth=0;
			int distance =100;
			bool drawFov = false;
			int bone=3; // 0=Head, 1=Neck, 2=Chest, 3=Closest
			int key=0;
			bool cVisible = false;
			// Extra options
			bool IgnoreDead = true;
			bool IgnoreInvisible = true;
			bool IgnoreFriends = true;
			// Bone selections (synced with preview)
			bool bones[14] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true }; // All bones enabled by default
		}Aimbot;
		struct Silent {
			bool enable = false;
			bool prediction = false;
			float predictionAmount = 0.15f;
			int fov = 70;
			int distance = 300;
			bool undetectablesilent = false;
			float SilentFovColor[4] = { 1.f, 1.f, 1.f, 1.f };
			bool drawFov = false;
			int bone = 3; // 0=Head, 1=Neck, 2=Chest, 3=Closest
			int key = 0;
			int Magic_key = 0;
			bool WheelSilent = false;
			int WheelSilentKey = 0;
			int WheelSilentDistance = 300;
			bool psltEnabled = false;
			int misschance = 0;
			bool cVisible = false;
			bool Magic = false;
			bool showLockedBone = true; // Show locked bone indicator
			// Extra options
			bool IgnoreDead = true;
			bool IgnoreInvisible = true;
			bool IgnoreFriends = true;
			// WaveShield Bypass - Unlimited FOV
			bool waveShieldBypass = false;
			// Bone selections (synced with preview)
			bool bones[14] = { true, true, true, true, true, true, true, true, true, true, true, true, true, true }; // All bones enabled by default
		}Silent;
		struct ESP
		{
			bool enable = false;
			bool ShowDamage = false;   // floating damage number above the last player we hit
			int  ShowDamageSize = 1;   // damage text scale, 1 = current size ... 10 = 10x
			bool HitMarker = false;    // crosshair X flash when we land a hit
			bool showself = false;
			bool visible = false;
			int MaxDistance = 200;
			bool ShowFriends = false;
			float FriendColor[4] = { 0.f, 1.f, 0.f, 1.f };
			bool FriendAddMode = false;
			int FriendAddKey = 0;


			bool Box = false;
			bool BoxDashed = true; // false = Normal (full), true = Dashed (corners only) — Advanced default
			float BoxColor[4] = { 1.f, 1.f, 1.f, 1.f };
			int Boxbold = 2;
			int BoxSizeIncrease = 2;

			bool CorneredBox = false;
			float CorneredBoxColor[4] = { 1.f, 1.f, 1.f, 1.f };

			struct DrawBox {
				bool Enabled = false;
				float Color[4] = { 1.f, 1.f, 1.f, 1.f };
				float Size = 1.0f;
				int SelectedType = 0;
				const char* Type[3] = { "Full Box", "Corner Box", "3D Box" };
				bool GradientEnabled = false;
				float GradientIntensity = 1.0f;
				bool UseCustomGradient = false;
				float BoxGradientTopColor[4] = { 1.f, 1.f, 1.f, 0.3f };
				float BoxGradientBottomColor[4] = { 0.f, 0.f, 0.f, 0.5f };
				float Box3DDepth = 10.0f;
			} DrawBox;

			bool Skeleton = false;
			float SkeletonColor[4] = { 1.f, 1.f, 1.f, 1.f };

			bool Name = false;
			float NameColor[4] = { 1.f, 1.f, 1.f, 1.f };

			// Show server/network ID
			bool ShowID = false;
			float IDColor[4] = { 1.f, 1.f, 1.f, 1.f };

			// Show ped gender (Male / Female by model hash)
			bool ShowGender = false;
			float MaleColor[4]   = { 0.40f, 0.70f, 1.00f, 1.f };
			float FemaleColor[4] = { 1.00f, 0.45f, 0.75f, 1.f };

			bool HeadCircle = false;
			float HeadCircleColor[4] = { 1.f, 1.f, 1.f, 1.f };
			float HeadCircleRadius = 15.0f;

			// Filled box (chams-like 2D fill overlay)
			bool FilledBox = false;
			float FilledBoxColor[4] = { 1.f, 1.f, 1.f, 0.15f };

			// "Aiming at you" alert
			bool AimingAtYou = false;
			float AimingAtYouColor[4] = { 1.f, 0.25f, 0.25f, 1.f };

			// Vehicle ESP
			bool VehicleESP = false;
			bool VehicleBox = true;
			bool VehicleName = true;
			bool VehicleDistance = true;
			bool VehicleHealthBar = false;
			int  VehicleMaxDistance = 300;
			float VehicleColor[4] = { 1.f, 1.f, 1.f, 1.f };

			bool HealthBar = false;
			float HealthBarColor[4] = { 0.f, 1.f, 0.f, 1.f };
			bool HealthBarp = false;
			float HealthBarColor2[4] = { 1.f, 1.f, 1.f, 1.f };

			bool ArmorBar = false;
			bool ArmorBarp = false;
			float ArmorBarColor[4] = { 0.f, 0.450f, 1.f, 1.f };
			float ArmorBarColor2[4] = { 1.f, 1.f, 1.f, 1.f };
			int baropt = 0;
			
			// Bar positions — MUST match the ESP preview encoding: 0=Bottom, 1=Left, 2=Right (never Top)
			int HealthBarPosition = 0; // Default: Bottom (Advanced)
			int ArmorBarPosition = 0;  // Default: Bottom (Advanced)

			// Text positions — MUST match the ESP preview slots:
			// 0=head-top, 1=upper-left, 2=upper-right, 3=lower-left, 4=lower-right, 5=bottom
			int NamePosition = 0;     // Default: head-top
			int DistancePosition = 2; // Default: upper-right
			int WeaponPosition = 5;   // Default: bottom
			int IDPosition = 4;       // Default: lower-right (Show ID)

			bool WeaponName = false;
			bool Weapon_Misc = false;
			float WeaponNameColor[4] = { 1.f, 1.f, 1.f, 1.f };


			bool Arrow = false;
			float ArrowColor[4] = { 1.f, 1.f, 1.f, 1.f };

			int ArrowRadius = 10;
            
            // Offscreen ESP settings
            bool OffscreenESP = false;
            float OffscreenESPColor[4] = { 1.f, 1.f, 0.f, 1.f }; // Yellow default color
            float OffscreenESPRange = 300.0f; // Default range value
            bool OffscreenESPPulse = true; // Pulsating effect

			bool DistanceESP = false;
			float DistanceColor[4] = { 1.f, 1.f, 1.f, 1.f };

			bool SnapLines = false;
			float SnaplinesColor[4] = { 1.f, 1.f, 1.f, 1.f };

			float InvisibleColor[4] = { 1.f, 0.f, 0.f, 1.f };

			// Colour applied to a player while Silent Aim is locked onto them
			// (the whole ESP turns this colour). Default a strong purple (#A826E6),
			// deliberately magenta-leaning so it can't read as blue.
			float LockedColor[4] = { 0.659f, 0.149f, 0.902f, 1.f };

		}ESP;
		struct Triggerbot {
			bool enable = false;
			int key = 0;
			int delay = 50; // ms delay before shooting
			int fov = 5; // FOV for triggerbot (smaller than aimbot)
			int distance = 200;
			int bone = 3; // 0=Head, 1=Neck, 2=Chest, 3=Closest
			bool cVisible = true; // Visibility check
			bool IgnoreDead = true;
			bool IgnoreInvisible = true;
			bool IgnoreFriends = true;
			bool autoShoot = true; // Auto shoot when target in crosshair
		}Triggerbot;
		
		struct Exploit {
			bool rocket_boost = false;
			bool jump = false;
			bool parachute = false;
			bool Gravity = false;
			float GravityCount = 1.0f;
			bool stealcar = false;
			int FuelToZeroKey = 0;
			bool SuperHandling;
			bool KediciShadow =false;
			int KediciShadowKey = 0;

					// strafe macro

					bool StrafeMacro = false;

					int StrafeMacroBind = 0;

					int StrafeMacroDelay = 100;


			bool Test = false;
			float TestValue = 1.0f;

		}Exploit;
	};
}

inline Cheat::Options g_Options;
