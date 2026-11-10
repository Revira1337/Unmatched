#include "Auth.hpp"
#include <sstream>
#include <fstream>
#include <thread>
#include <atomic>
#include <chrono>
#include <intrin.h>
#include <iphlpapi.h>
#include <comdef.h>
#include <Wbemidl.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "libcurl.lib")

using json = nlohmann::json;

namespace Auth
{
    // Auth server URL (VDS'nizdeki server adresi)
    static const std::string AUTH_SERVER = "http://localhost:3001"; // BUNU DEĞİŞTİR!
    
    // Heartbeat thread kontrolü
    static std::atomic<bool> g_HeartbeatRunning(false);
    static std::thread g_HeartbeatThread;
    
    // Curl yazma callback
    static size_t WriteCallback(void* contents, size_t size, size_t nmemb, void* userp)
    {
        ((std::string*)userp)->append((char*)contents, size * nmemb);
        return size * nmemb;
    }
    
    // HWID oluşturma fonksiyonu (CPU + Motherboard + MAC)
    std::string GetHWID()
    {
        std::stringstream hwid;
        
        // CPU ID al
        int cpuInfo[4] = { 0 };
        __cpuid(cpuInfo, 0);
        hwid << std::hex << cpuInfo[1] << cpuInfo[3] << cpuInfo[2];
        
        // MAC Address al
        IP_ADAPTER_INFO adapterInfo[16];
        DWORD dwBufLen = sizeof(adapterInfo);
        DWORD dwStatus = GetAdaptersInfo(adapterInfo, &dwBufLen);
        
        if (dwStatus == ERROR_SUCCESS)
        {
            PIP_ADAPTER_INFO pAdapterInfo = adapterInfo;
            if (pAdapterInfo)
            {
                for (UINT i = 0; i < pAdapterInfo->AddressLength; i++)
                {
                    hwid << std::hex << (int)pAdapterInfo->Address[i];
                }
            }
        }
        
        // Motherboard Serial Number al (WMI)
        HRESULT hres;
        hres = CoInitializeEx(0, COINIT_MULTITHREADED);
        if (SUCCEEDED(hres))
        {
            hres = CoInitializeSecurity(
                NULL, -1, NULL, NULL,
                RPC_C_AUTHN_LEVEL_DEFAULT,
                RPC_C_IMP_LEVEL_IMPERSONATE,
                NULL, EOAC_NONE, NULL
            );
            
            if (SUCCEEDED(hres))
            {
                IWbemLocator* pLoc = NULL;
                hres = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)&pLoc);
                
                if (SUCCEEDED(hres))
                {
                    IWbemServices* pSvc = NULL;
                    hres = pLoc->ConnectServer(
                        _bstr_t(L"ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc
                    );
                    
                    if (SUCCEEDED(hres))
                    {
                        hres = CoSetProxyBlanket(
                            pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL,
                            RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE
                        );
                        
                        if (SUCCEEDED(hres))
                        {
                            IEnumWbemClassObject* pEnumerator = NULL;
                            hres = pSvc->ExecQuery(
                                bstr_t("WQL"),
                                bstr_t("SELECT SerialNumber FROM Win32_BaseBoard"),
                                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                                NULL, &pEnumerator
                            );
                            
                            if (SUCCEEDED(hres))
                            {
                                IWbemClassObject* pclsObj = NULL;
                                ULONG uReturn = 0;
                                
                                while (pEnumerator)
                                {
                                    HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);
                                    if (0 == uReturn) break;
                                    
                                    VARIANT vtProp;
                                    hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
                                    if (SUCCEEDED(hr) && vtProp.vt == VT_BSTR)
                                    {
                                        _bstr_t serialNum(vtProp.bstrVal, false);
                                        hwid << (char*)serialNum;
                                    }
                                    VariantClear(&vtProp);
                                    pclsObj->Release();
                                }
                                pEnumerator->Release();
                            }
                        }
                        pSvc->Release();
                    }
                    pLoc->Release();
                }
            }
            CoUninitialize();
        }
        
        // Hash olarak döndür (SHA256 benzeri basit hash)
        std::string hwidStr = hwid.str();
        std::hash<std::string> hasher;
        size_t hash = hasher(hwidStr);
        
        std::stringstream finalHwid;
        finalHwid << std::hex << hash;
        
        return finalHwid.str();
    }
    
    // Discord ID input dialog
    std::string GetDiscordID()
    {
        // Discord ID'yi kaydetmek için dosya kontrolü
        std::ifstream configFile("discord_config.dat");
        if (configFile.is_open())
        {
            std::string savedDiscordId;
            std::getline(configFile, savedDiscordId);
            configFile.close();
            
            if (!savedDiscordId.empty() && savedDiscordId.length() >= 17)
            {
                return savedDiscordId;
            }
        }
        
        // Discord ID input penceresi
        char discordIdBuffer[256] = { 0 };
        
        HWND hwndInput = CreateWindowExA(
            WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
            "Edit",
            "",
            WS_VISIBLE | WS_CHILD | ES_NUMBER,
            10, 10, 250, 25,
            NULL, NULL, GetModuleHandle(NULL), NULL
        );
        
        // Basit input dialog (gerçek uygulamada daha iyi bir UI kullanılmalı)
        MessageBoxA(NULL, 
            "Please enter your Discord ID in the next window.\nYou can find it by enabling Developer Mode in Discord settings.",
            "Discord ID Required", MB_OK | MB_ICONINFORMATION);
        
        // Discord ID'yi al (bu basit bir örnek - gerçek uygulamada custom dialog kullanılmalı)
        // Burada InputBox benzeri bir fonksiyon gerekli
        
        // Şimdilik MessageBox ile basit bir çözüm
        // NOT: Gerçek uygulamada ImGui veya custom dialog kullanılmalı!
        
        std::string discordId;
        
        // Dosyadan oku veya kullanıcıdan al
        // Bu kısım ImGui entegrasyonu ile daha güzel yapılabilir
        
        return discordId;
    }
    
    // Auth doğrulama
    bool ValidateAuth(const std::string& hwid, const std::string& discord_id, UserInfo& outUserInfo, std::string& outError)
    {
        CURL* curl = curl_easy_init();
        if (!curl)
        {
            outError = "Failed to initialize HTTP client";
            return false;
        }
        
        std::string response;
        std::string url = AUTH_SERVER + "/api/hwid/validate";
        
        // JSON payload oluştur
        json payload;
        payload["hwid"] = hwid;
        payload["discord_id"] = discord_id;
        std::string jsonStr = payload.dump();
        
        // HTTP headers
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // DEV için, production'da 1L olmalı
        
        CURLcode res = curl_easy_perform(curl);
        long httpCode = 0;
        curl_easy_getinfo(curl, CURLINFO_RESPONSE_CODE, &httpCode);
        
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK)
        {
            outError = "Failed to connect to auth server: " + std::string(curl_easy_strerror(res));
            return false;
        }
        
        try
        {
            json responseJson = json::parse(response);
            
            if (!responseJson["success"].get<bool>())
            {
                outError = responseJson.value("error", "Authentication failed");
                return false;
            }
            
            // User bilgilerini doldur
            if (responseJson.contains("user"))
            {
                json user = responseJson["user"];
                outUserInfo.discord_id = user.value("discord_id", "");
                outUserInfo.discord_username = user.value("discord_username", "Unknown");
                outUserInfo.discord_avatar = user.value("discord_avatar", "");
                outUserInfo.plan = user.value("plan", "Unknown");
                outUserInfo.expires_at = user.value("expires_at", "Never");
                outUserInfo.remaining = user.value("remaining", "N/A");
                outUserInfo.status = user.value("status", "unknown");
                outUserInfo.remaining_ms = user.value("remaining_ms", 0);
                outUserInfo.is_valid = true;
            }
            
            return true;
        }
        catch (const json::exception& e)
        {
            outError = "Failed to parse server response: " + std::string(e.what());
            return false;
        }
    }
    
    // Heartbeat gönder
    bool SendHeartbeat(const std::string& hwid, const std::string& discord_id)
    {
        CURL* curl = curl_easy_init();
        if (!curl) return false;
        
        std::string response;
        std::string url = AUTH_SERVER + "/api/hwid/heartbeat";
        
        json payload;
        payload["hwid"] = hwid;
        payload["discord_id"] = discord_id;
        std::string jsonStr = payload.dump();
        
        struct curl_slist* headers = NULL;
        headers = curl_slist_append(headers, "Content-Type: application/json");
        
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, jsonStr.c_str());
        curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headers);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, WriteCallback);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &response);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);
        
        CURLcode res = curl_easy_perform(curl);
        curl_slist_free_all(headers);
        curl_easy_cleanup(curl);
        
        if (res != CURLE_OK) return false;
        
        try
        {
            json responseJson = json::parse(response);
            
            if (!responseJson["success"].get<bool>())
            {
                // License artık geçersiz (banned, frozen, expired)
                // Program kapatılmalı
                MessageBoxA(NULL, 
                    "Your license is no longer valid. Application will close.", 
                    "License Error", MB_OK | MB_ICONERROR);
                exit(0);
                return false;
            }
            
            return true;
        }
        catch (...)
        {
            return false;
        }
    }
    
    // Heartbeat thread
    void HeartbeatThreadFunc(std::string hwid, std::string discord_id)
    {
        while (g_HeartbeatRunning.load())
        {
            SendHeartbeat(hwid, discord_id);
            
            // 60 saniye bekle
            for (int i = 0; i < 60 && g_HeartbeatRunning.load(); i++)
            {
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    }
    
    void StartHeartbeatThread(const std::string& hwid, const std::string& discord_id)
    {
        if (g_HeartbeatRunning.load()) return;
        
        g_HeartbeatRunning.store(true);
        g_HeartbeatThread = std::thread(HeartbeatThreadFunc, hwid, discord_id);
        g_HeartbeatThread.detach();
    }
    
    void StopHeartbeatThread()
    {
        g_HeartbeatRunning.store(false);
    }
    
    // Auth dialog göster
    bool ShowAuthDialog(UserInfo& outUserInfo)
    {
        // HWID al
        std::string hwid = GetHWID();
        
        // Discord ID'yi dosyadan oku veya kullanıcıdan al
        std::string discordId;
        std::ifstream configFile("discord_config.dat");
        if (configFile.is_open())
        {
            std::getline(configFile, discordId);
            configFile.close();
        }
        
        // Eğer Discord ID yoksa kullanıcıdan iste
        bool usePlaceholder = false;
        if (discordId.empty())
        {
            // Discord ID yoksa uygulama yine de başlasın (test/dev için)
            // Gerçek uygulamada burada input dialog gösterilmeli
            discordId = "000000000000000000"; // Placeholder ID
            usePlaceholder = true;
        }
        
        // Auth doğrulama yap (placeholder ID ise atla)
        std::string error;
        if (!usePlaceholder && !ValidateAuth(hwid, discordId, outUserInfo, error))
        {
            MessageBoxA(NULL, error.c_str(), "Authentication Failed", MB_OK | MB_ICONERROR);
            return false;
        }
        
        // Placeholder ID kullanılıyorsa fake user info oluştur
        if (usePlaceholder)
        {
            outUserInfo.discord_id = discordId;
            outUserInfo.discord_username = "Dev User";
            outUserInfo.discord_avatar = "";
            outUserInfo.plan = "Dev";
            outUserInfo.expires_at = "Never";
            outUserInfo.remaining = "Unlimited";
            outUserInfo.status = "active";
            outUserInfo.remaining_ms = 0;
            outUserInfo.is_valid = true;
        }
        
        // Discord ID'yi kaydet (placeholder değilse)
        if (!usePlaceholder)
        {
            std::ofstream saveFile("discord_config.dat");
            if (saveFile.is_open())
            {
                saveFile << discordId;
                saveFile.close();
            }
            
            // Heartbeat başlat
            StartHeartbeatThread(hwid, discordId);
        }
        
        return true;
    }
}
