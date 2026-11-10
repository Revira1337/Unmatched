// Alternative Auth implementation using WinHTTP (no external dependencies)
// Use this if you don't want to install libcurl

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
#include <winhttp.h>

#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "iphlpapi.lib")
#pragma comment(lib, "winhttp.lib")

using json = nlohmann::json;

namespace Auth
{
    // Auth server URL (Ngrok tunnel)
    // Ngrok çalıştırdıktan sonra URL'i buraya yaz (örn: "abc123xyz.ngrok-free.app")
    static const std::string AUTH_SERVER_HOST = "NGROK_URL_BURAYA"; // Ngrok URL'ini yaz
    static const int AUTH_SERVER_PORT = 443; // HTTPS port
    static const bool USE_HTTPS = true; // Ngrok HTTPS kullanır
    
    // Heartbeat thread kontrolü
    static std::atomic<bool> g_HeartbeatRunning(false);
    static std::thread g_HeartbeatThread;
    
    // WinHTTP Helper - HTTP POST request
    bool HttpPost(const std::string& host, int port, const std::string& path, const std::string& jsonData, std::string& response, bool useHttps = false)
    {
        HINTERNET hSession = NULL;
        HINTERNET hConnect = NULL;
        HINTERNET hRequest = NULL;
        bool success = false;
        
        try
        {
            // Session oluştur
            hSession = WinHttpOpen(
                L"PremiumMenu/1.0",
                WINHTTP_ACCESS_TYPE_DEFAULT_PROXY,
                WINHTTP_NO_PROXY_NAME,
                WINHTTP_NO_PROXY_BYPASS,
                0
            );
            
            if (!hSession) return false;
            
            // Connection oluştur
            std::wstring wHost(host.begin(), host.end());
            hConnect = WinHttpConnect(
                hSession,
                wHost.c_str(),
                port,
                0
            );
            
            if (!hConnect) goto cleanup;
            
            // Request oluştur
            std::wstring wPath(path.begin(), path.end());
            hRequest = WinHttpOpenRequest(
                hConnect,
                L"POST",
                wPath.c_str(),
                NULL,
                WINHTTP_NO_REFERER,
                WINHTTP_DEFAULT_ACCEPT_TYPES,
                useHttps ? WINHTTP_FLAG_SECURE : 0
            );
            
            if (!hRequest) goto cleanup;
            
            // HTTPS için SSL hatalarını ignore et (Ngrok self-signed cert için)
            if (useHttps)
            {
                DWORD dwFlags = SECURITY_FLAG_IGNORE_UNKNOWN_CA |
                               SECURITY_FLAG_IGNORE_CERT_WRONG_USAGE |
                               SECURITY_FLAG_IGNORE_CERT_CN_INVALID |
                               SECURITY_FLAG_IGNORE_CERT_DATE_INVALID;
                
                WinHttpSetOption(hRequest, WINHTTP_OPTION_SECURITY_FLAGS, &dwFlags, sizeof(dwFlags));
            }
            
            // Headers
            std::wstring headers = L"Content-Type: application/json\r\n";
            
            // Request gönder
            if (!WinHttpSendRequest(
                hRequest,
                headers.c_str(),
                -1,
                (LPVOID)jsonData.c_str(),
                jsonData.length(),
                jsonData.length(),
                0
            ))
            {
                goto cleanup;
            }
            
            // Response al
            if (!WinHttpReceiveResponse(hRequest, NULL))
                goto cleanup;
            
            // Response'u oku
            DWORD dwSize = 0;
            DWORD dwDownloaded = 0;
            std::string tempResponse;
            
            do
            {
                dwSize = 0;
                if (!WinHttpQueryDataAvailable(hRequest, &dwSize))
                    break;
                
                if (dwSize == 0)
                    break;
                
                char* buffer = new char[dwSize + 1];
                ZeroMemory(buffer, dwSize + 1);
                
                if (WinHttpReadData(hRequest, buffer, dwSize, &dwDownloaded))
                {
                    tempResponse.append(buffer, dwDownloaded);
                }
                
                delete[] buffer;
                
            } while (dwSize > 0);
            
            response = tempResponse;
            success = true;
        }
        catch (...)
        {
            success = false;
        }
        
    cleanup:
        if (hRequest) WinHttpCloseHandle(hRequest);
        if (hConnect) WinHttpCloseHandle(hConnect);
        if (hSession) WinHttpCloseHandle(hSession);
        
        return success;
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
        
        // Hash olarak döndür
        std::string hwidStr = hwid.str();
        std::hash<std::string> hasher;
        size_t hash = hasher(hwidStr);
        
        std::stringstream finalHwid;
        finalHwid << std::hex << hash;
        
        return finalHwid.str();
    }
    
    // Auth doğrulama
    bool ValidateAuth(const std::string& hwid, const std::string& discord_id, UserInfo& outUserInfo, std::string& outError)
    {
        try
        {
            // JSON payload oluştur
            json payload;
            payload["hwid"] = hwid;
            payload["discord_id"] = discord_id;
            std::string jsonStr = payload.dump();
            
            // HTTP request
            std::string response;
            if (!HttpPost(AUTH_SERVER_HOST, AUTH_SERVER_PORT, "/api/hwid/validate", jsonStr, response, USE_HTTPS))
            {
                outError = "Failed to connect to auth server";
                return false;
            }
            
            // Parse response
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
        catch (...)
        {
            outError = "Unknown error during authentication";
            return false;
        }
    }
    
    // Heartbeat gönder
    bool SendHeartbeat(const std::string& hwid, const std::string& discord_id)
    {
        try
        {
            json payload;
            payload["hwid"] = hwid;
            payload["discord_id"] = discord_id;
            std::string jsonStr = payload.dump();
            
            std::string response;
            if (!HttpPost(AUTH_SERVER_HOST, AUTH_SERVER_PORT, "/api/hwid/heartbeat", jsonStr, response, USE_HTTPS))
                return false;
            
            json responseJson = json::parse(response);
            
            if (!responseJson["success"].get<bool>())
            {
                // License artık geçersiz
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
        
        // Discord ID'yi dosyadan oku
        std::string discordId;
        std::ifstream configFile("discord_config.dat");
        if (configFile.is_open())
        {
            std::getline(configFile, discordId);
            configFile.close();
        }
        
        // Eğer Discord ID yoksa hata
        if (discordId.empty())
        {
            MessageBoxA(NULL, 
                "Discord ID not found. Please create a discord_config.dat file with your Discord ID.", 
                "Setup Required", MB_OK | MB_ICONWARNING);
            return false;
        }
        
        // Auth doğrulama yap
        std::string error;
        if (!ValidateAuth(hwid, discordId, outUserInfo, error))
        {
            MessageBoxA(NULL, error.c_str(), "Authentication Failed", MB_OK | MB_ICONERROR);
            return false;
        }
        
        // Heartbeat başlat
        StartHeartbeatThread(hwid, discordId);
        
        return true;
    }
}
