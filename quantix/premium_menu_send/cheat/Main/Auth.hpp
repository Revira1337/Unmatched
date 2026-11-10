#pragma once

#include <string>
#include <Windows.h>
#include "nlohmann/json.hpp"

namespace Auth
{
    struct UserInfo
    {
        std::string discord_id;
        std::string discord_username;
        std::string discord_avatar;
        std::string plan;
        std::string expires_at;
        std::string remaining;
        std::string status;
        int64_t remaining_ms;
        bool is_valid;
    };

    // HWID alma fonksiyonu
    std::string GetHWID();
    
    // Discord ID alma fonksiyonu (kullanıcıdan input alınacak)
    std::string GetDiscordID();
    
    // Auth server'a doğrulama isteği gönder
    bool ValidateAuth(const std::string& hwid, const std::string& discord_id, UserInfo& outUserInfo, std::string& outError);
    
    // Heartbeat gönder (her 60 saniyede bir)
    bool SendHeartbeat(const std::string& hwid, const std::string& discord_id);
    
    // Auth dialog göster ve doğrulama yap
    bool ShowAuthDialog(UserInfo& outUserInfo);
    
    // Heartbeat thread başlat
    void StartHeartbeatThread(const std::string& hwid, const std::string& discord_id);
    
    // Heartbeat thread durdur
    void StopHeartbeatThread();
}
