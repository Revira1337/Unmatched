#pragma once

#include "../FivemSDK/Fivem.hpp"
#include "../FivemSDK/GTADefines.hpp"
#include <string>
#include <unordered_map>
#include <mutex>

namespace Cheat
{
    namespace PlayerNames
    {
        // Player name cache structure
        struct PlayerNameInfo
        {
            int playerId;
            std::string name;
            bool isValid;
            uint64_t lastUpdate;
        };

        // Initialize the player names system
        bool Initialize();

        // Get player name by Ped reference
        std::string GetPedName(CPed* ped);

        // Get player name by player ID
        std::string GetPlayerNameById(int playerId);

        // Update player names cache
        void UpdatePlayerNamesCache();

        // Clear the cache
        void ClearCache();

        // Check if the system is initialized
        bool IsInitialized();

        // Get all cached player names
        std::unordered_map<int, std::string> GetAllPlayerNames();

        // Manual cache refresh
        void RefreshCache();

        // Background update thread function
        void PlayerNamesUpdateLoop();

        // Start the background update thread
        void StartUpdateThread();

        // Stop the background update thread
        void StopUpdateThread();

        // Trigger background update (internal function)
        void TriggerBackgroundUpdate();
    }
}
