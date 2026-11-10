#include "Misc.hpp"

#include "../FrameWork.hpp"
#include <algorithm>
#include <winhttp.h>
#include <sstream>
#include <tchar.h>
#include "../Render/kaanlar.h"

namespace M_base
{
	int Misc::RandomInt(int Lower, int Max)
	{
		return (rand() % (Max - Lower + 1)) + Lower;
	}

	std::wstring Misc::RandomString(size_t Length)
	{
		auto Randchar = []() -> char
			{
				const char* Charset = xorstr_("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
				const size_t MaxIndex = (sizeof(Charset) - 1);
				return Charset[rand() % MaxIndex];
			};

		std::wstring Str(Length, 0);
		std::generate_n(Str.begin(), Length, Randchar);
		return Str;
	}

	std::string Misc::Wstring2String(std::wstring Input)
	{
		return std::string(Input.begin(), Input.end());
	}

	std::wstring Misc::String2WString(std::string Input)
	{
		return std::wstring(Input.begin(), Input.end());
	}

	void Misc::InitializeConsole()
	{
		SafeCall(AllocConsole)();
		freopen(xorstr_("CONIN$"), xorstr_("r"), stdin);
		freopen(xorstr_("CONOUT$"), xorstr_("w"), stdout);
		SafeCall(SetConsoleTitleA)(xorstr_("acfxck3r"));
	}

	void Misc::ShutDownConsole()
	{
		SafeCall(FreeConsole)();
	}

	std::string Misc::DownloadServerInfo(std::wstring IP, int PORT)
	{
		HINTERNET hSession = WinHttpOpen(xorstr_(L"Fivem/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

		if (hSession)
		{
			HINTERNET hConnect = WinHttpConnect(hSession, IP.c_str(), PORT, 0);

			if (hConnect)
			{
				HINTERNET hRequest = WinHttpOpenRequest(hConnect, xorstr_(L"GET"), xorstr_(L"/players.json"), NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

				if (hRequest)
				{
					if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
					{
						if (WinHttpReceiveResponse(hRequest, NULL))
						{
							DWORD dwSize = 0;
							DWORD dwDownloaded = 0;
							LPSTR pszOutBuffer;
							BOOL  bResults = FALSE;
							std::string Response;

							do
							{
								dwSize = 0;
								if (WinHttpQueryDataAvailable(hRequest, &dwSize))
								{
									pszOutBuffer = new char[dwSize + 1];
									if (!pszOutBuffer)
									{
										dwSize = 0;
									}
									else
									{
										ZeroMemory(pszOutBuffer, dwSize + 1);

										if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
											Response.append(pszOutBuffer, dwDownloaded);
									}

									delete[] pszOutBuffer;
								}

							} while (dwSize > 0);

							return Response;
						}
					}

					WinHttpCloseHandle(hRequest);
				}

				WinHttpCloseHandle(hConnect);
			}

			WinHttpCloseHandle(hSession);
		}

		return xorstr_("");
	}

    //std::string ServerID = "";

    //bool Misc::GetServerIDFromHeaders(std::wstring IP, int PORT)
    //{
    //    HINTERNET hSession = WinHttpOpen(xorstr_(L"Fivem/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);

    //    if (hSession)
    //    {
    //        HINTERNET hConnect = WinHttpConnect(hSession, IP.c_str(), PORT, 0);

    //        if (hConnect)
    //        {
    //            HINTERNET hRequest = WinHttpOpenRequest(hConnect, xorstr_(L"GET"), NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, 0);

    //            if (hRequest)
    //            {
    //                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    //                {
    //                    if (WinHttpReceiveResponse(hRequest, NULL))
    //                    {
    //                        DWORD dwSize = 0;
    //                        DWORD dwDownloaded = 0;
    //                        LPSTR pszOutBuffer;
    //                        BOOL bResults = FALSE;

    //                        WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX, NULL, &dwSize, WINHTTP_NO_HEADER_INDEX);

    //                        if (dwSize > 0)
    //                        {
    //                            pszOutBuffer = new char[dwSize + 1];
    //                            if (pszOutBuffer)
    //                            {
    //                                if (WinHttpQueryHeaders(hRequest, WINHTTP_QUERY_LOCATION, WINHTTP_HEADER_NAME_BY_INDEX, pszOutBuffer, &dwSize, WINHTTP_NO_HEADER_INDEX))
    //                                {
    //                                    std::string locationHeader(pszOutBuffer);

    //                                    size_t pos = locationHeader.find_last_of('/');
    //                                    if (pos != std::string::npos)
    //                                    {
    //                                        ServerID = locationHeader.substr(pos + 1);
    //                                    }
    //                                }

    //                                delete[] pszOutBuffer;
    //                            }
    //                        }
    //                    }
    //                    WinHttpCloseHandle(hRequest);
    //                }

    //                WinHttpCloseHandle(hConnect);
    //            }

    //            WinHttpCloseHandle(hSession);
    //        }
    //    }

    //    return !ServerID.empty();
    //}

    //std::string Misc::GetServerInfoFromID()
    //{

    //    std::string serverInfoUrl = "https://servers-frontend.fivem.net/api/servers/single/" + ServerID;
    //    std::string RawServerInfo = DownloadServerInfoFromUrl(serverInfoUrl);
    //    return RawServerInfo;
    //}

    //std::string Misc::DownloadServerInfoFromUrl(const std::string& url)
    //{
    //    HINTERNET hSession = WinHttpOpen(xorstr_(L"Fivem/1.0"), WINHTTP_ACCESS_TYPE_DEFAULT_PROXY, WINHTTP_NO_PROXY_NAME, WINHTTP_NO_PROXY_BYPASS, 0);
    //    if (hSession)
    //    {
    //        HINTERNET hConnect = WinHttpConnect(hSession, std::wstring(url.begin(), url.end()).c_str(), 443, 0);

    //        if (hConnect)
    //        {
    //            HINTERNET hRequest = WinHttpOpenRequest(hConnect, xorstr_(L"GET"), NULL, NULL, WINHTTP_NO_REFERER, WINHTTP_DEFAULT_ACCEPT_TYPES, WINHTTP_FLAG_SECURE);

    //            if (hRequest)
    //            {
    //                if (WinHttpSendRequest(hRequest, WINHTTP_NO_ADDITIONAL_HEADERS, 0, WINHTTP_NO_REQUEST_DATA, 0, 0, 0))
    //                {
    //                    if (WinHttpReceiveResponse(hRequest, NULL))
    //                    {
    //                        DWORD dwSize = 0;
    //                        DWORD dwDownloaded = 0;
    //                        LPSTR pszOutBuffer;
    //                        BOOL  bResults = FALSE;
    //                        std::string Response;

    //                        do
    //                        {
    //                            dwSize = 0;
    //                            if (WinHttpQueryDataAvailable(hRequest, &dwSize))
    //                            {
    //                                pszOutBuffer = new char[dwSize + 1];
    //                                if (!pszOutBuffer)
    //                                {
    //                                    dwSize = 0;
    //                                }
    //                                else
    //                                {
    //                                    ZeroMemory(pszOutBuffer, dwSize + 1);

    //                                    if (WinHttpReadData(hRequest, (LPVOID)pszOutBuffer, dwSize, &dwDownloaded))
    //                                        Response.append(pszOutBuffer, dwDownloaded);
    //                                }

    //                                delete[] pszOutBuffer;
    //                            }

    //                        } while (dwSize > 0);

    //                        WinHttpCloseHandle(hRequest);
    //                        WinHttpCloseHandle(hConnect);
    //                        WinHttpCloseHandle(hSession);

    //                        return Response;
    //                    }
    //                }

    //                WinHttpCloseHandle(hRequest);
    //            }

    //            WinHttpCloseHandle(hConnect);
    //        }

    //        WinHttpCloseHandle(hSession);
    //    }

    //    return xorstr_("");
    //}
}
