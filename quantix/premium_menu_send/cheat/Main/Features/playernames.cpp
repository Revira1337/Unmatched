#include "playernames.hpp"
#include "Main/Options.hpp"
#include <Utils/Utilities/Memory.hpp>
#include <thread>
#include <chrono>

namespace Cheat
{
    namespace PlayerNames
    {
        // High-performance cache variables
        static uintptr_t citizenbase_address = 0;
        static std::unordered_map<int, std::string> fastNameCache; // O(1) lookup cache
        static std::unordered_map<int, std::string> stagingCache;  // Background update cache
        static std::mutex cacheSwapMutex; // Only for cache swapping
        static bool systemInitialized = false;
        static bool updateThreadRunning = false;
        static std::thread updateThread;
        static uint64_t lastCacheUpdate = 0;
        static uint64_t cacheUpdateInterval = 2000; // 2 seconds
        static int lastKnownPlayerCount = 0;

        // Helper function to get module base address
        uintptr_t GetBaseAddress(const char* moduleName)
        {
            std::wstring wModuleName(moduleName, moduleName + strlen(moduleName));
            return M_base::mem::GetModuleBaseByName(C_Five.GetPid(), wModuleName);
        }
        std::string ReadRemoteStdString(uint64_t address)
        {
            uint64_t chunk0 = M_base::mem::Read<uint64_t>(address + 0x00);
            uint64_t chunk1 = M_base::mem::Read<uint64_t>(address + 0x08);
            uint64_t length = M_base::mem::Read<uint64_t>(address + 0x10);

            if (length <= 15) {
                std::string result;
                result.reserve(length);

                for (size_t i = 0; i < length; ++i) {
                    char c;
                    if (i < 8) {
                        c = reinterpret_cast<char*>(&chunk0)[i];
                    } else {
                        c = reinterpret_cast<char*>(&chunk1)[i - 8];
                    }
                    result.push_back(c);
                }
                return result;
            } else {
                uint64_t heap_ptr = chunk0;
                return M_base::mem::ReadProcessMemoryString(heap_ptr, length);
            }
        }

        bool Initialize()
        {
            if (systemInitialized)
                return true;

            citizenbase_address = GetBaseAddress("citizen-playernames-five.dll");
            if (citizenbase_address == 0)
            {
                return false;
            }

            systemInitialized = true;
            return true;
        }

        std::string GetPedName(CPed* ped)
        {
            if (!ped)
            {
                return "Unknown";
            }

            CPlayerInfo* playerInfo = ped->GetPlayerInfo();
            if (!playerInfo)
            {
                return "Unknown";
            }

            // Get NetID
            int playerNetId = playerInfo->GetPlayerID();
            if (playerNetId <= 0)
            {
                return "Unknown";
            }

            // Check our fast cache first
            auto it = fastNameCache.find(playerNetId);
            if (it != fastNameCache.end())
            {
                return it->second;
            }

            auto serverIt = C_Five.PlayerIdToName.find(playerNetId);
            if (serverIt != C_Five.PlayerIdToName.end() && !serverIt->second.empty())
            {
                fastNameCache[playerNetId] = serverIt->second;
                return serverIt->second;
            }

            TriggerBackgroundUpdate();
            return "Unknown";
        }

        std::string GetPlayerNameById(int playerId)
        {
            if (!systemInitialized)
            {
                return "Unknown";
            }

            // ULTRA-FAST O(1) cache lookup - same as GetPedName
            auto it = fastNameCache.find(playerId);
            if (it != fastNameCache.end())
            {
                return it->second; // Instant return from cache
            }

            // If not in cache, trigger background update
            TriggerBackgroundUpdate();
            return "Unknown";
        }

        // Trigger background update - ultra lightweight
        void TriggerBackgroundUpdate()
        {
            uint64_t currentTime = GetTickCount64();
            if (currentTime - lastCacheUpdate > cacheUpdateInterval)
            {
                lastCacheUpdate = currentTime;
                // Background thread will handle the actual update
            }
        }

        // Background cache update - runs in separate thread
        void UpdatePlayerNamesInBackground()
        {
            if (!systemInitialized || citizenbase_address == 0)
                return;

            // Use correct offsets from forum post
            uintptr_t namesList = M_base::mem::Read<uintptr_t>(citizenbase_address + 0x30D38);
            int namesListSize = M_base::mem::Read<int>(citizenbase_address + 0x30D40);

            if (!namesList || namesListSize == 0)
                return;

            // Build new cache in staging area (no locks on main cache)
            std::unordered_map<int, std::string> newCache;
            newCache.reserve(namesListSize); // Pre-allocate for performance

            uintptr_t current = M_base::mem::Read<uintptr_t>(namesList);

            for (int i = 0; i < namesListSize && current != 0; ++i)
            {
                uint16_t netId = M_base::mem::Read<uint16_t>(current + 0x10);
                if (netId > 0)
                {
                    std::string name = ReadRemoteStdString(current + 0x18);
                    if (!name.empty() && name != "Unknown")
                    {
                        newCache[netId] = std::move(name); // Move for performance
                    }
                }
                current = M_base::mem::Read<uintptr_t>(current);
            }

            // Ultra-fast cache swap - minimal lock time
            {
                std::lock_guard<std::mutex> lock(cacheSwapMutex);
                fastNameCache = std::move(newCache); // Atomic swap
            }

            lastKnownPlayerCount = namesListSize;
        }

        // Legacy function for compatibility
        void UpdatePlayerNamesCache()
        {
            UpdatePlayerNamesInBackground();
        }

        void ClearCache()
        {
            std::lock_guard<std::mutex> lock(cacheSwapMutex);
            fastNameCache.clear();
        }

        bool IsInitialized()
        {
            return systemInitialized;
        }

        std::unordered_map<int, std::string> GetAllPlayerNames()
        {
            std::lock_guard<std::mutex> lock(cacheSwapMutex);
            return fastNameCache;
        }

        void RefreshCache()
        {
            UpdatePlayerNamesInBackground();
        }

        void PlayerNamesUpdateLoop()
        {
            while (updateThreadRunning && !g_Options.General.ShutDown)
            {
                if (systemInitialized)
                {
                    UpdatePlayerNamesInBackground(); // Use the ultra-fast background updater
                }
                
                // Update every 2 seconds for optimal performance
                std::this_thread::sleep_for(std::chrono::milliseconds(2000));
            }
        }

        void StartUpdateThread()
        {
            if (updateThreadRunning)
                return;

            updateThreadRunning = true;
            updateThread = std::thread(PlayerNamesUpdateLoop);
            updateThread.detach();
        }

        void StopUpdateThread()
        {
            updateThreadRunning = false;
            if (updateThread.joinable())
            {
                updateThread.join();
            }
        }
    }
}
