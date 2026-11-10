#pragma once

#include "../FrameWork.hpp"
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <ctime>
#include <iomanip>

// Pattern Scanner Log
namespace PatternScanLog
{
	inline void Log(const std::string& message)
	{
		std::ofstream logFile("waveshield_debug.log", std::ios::app);
		if (logFile.is_open())
		{
			time_t now = time(0);
			char timeStr[26];
			ctime_s(timeStr, sizeof(timeStr), &now);
			timeStr[24] = '\0';
			logFile << "[" << timeStr << "] [PatternScanner] " << message << std::endl;
			logFile.close();
		}
	}
}

class KEKELER
{
public:
	 HANDLE pubHANDLE = NULL;
};
inline KEKELER k_Kekeler;
namespace M_base
{
	namespace mem
	{
		HWND GetWindowHandleByPID(int Pid);
		HWND GetWindowHandleByName(std::wstring WindowName);
		DWORD GetProcessPidByName(std::wstring ProcessName);
		uint64_t GetModuleBaseByName(DWORD Pid, std::wstring ModuleName);

		void AttachProces(DWORD Pid);
		void DetachProcess();

		void ReadProcessMemoryImpl(uint64_t ReadAddress, LPVOID Read, SIZE_T Size);
		bool WriteProcessMemoryImpl(uint64_t WriteAddress, LPVOID Write, SIZE_T Size);

		std::string ReadProcessMemoryString(uint64_t ReadAddress, SIZE_T StringSize = 256);

		template <typename T, typename B>
		T Read(B ReadAddress)
		{
			T Read;
			ReadProcessMemoryImpl((uint64_t)ReadAddress, &Read, sizeof(T));
			return Read;
		}

		template <typename T, typename B>
		bool WriteMemory(B WriteAddress, T Value)
		{
			return WriteProcessMemoryImpl((uint64_t)WriteAddress, &Value, sizeof(T));
		}

		// Pattern Scanner - IDA style pattern (e.g., "48 83 EC 28 E8 ? ? ? ? 48 85 C0")
		inline std::vector<int> ParsePattern(const std::string& pattern)
		{
			std::vector<int> bytes;
			std::istringstream iss(pattern);
			std::string byte;
			
			while (iss >> byte)
			{
				if (byte == "?" || byte == "??")
				{
					bytes.push_back(-1); // Wildcard
				}
				else
				{
					bytes.push_back(std::stoi(byte, nullptr, 16));
				}
			}
			return bytes;
		}

		inline uint64_t FindPattern(uint64_t moduleBase, size_t moduleSize, const std::string& pattern)
		{
			PatternScanLog::Log("=== Pattern Scan Started ===");
			PatternScanLog::Log("Module Base: 0x" + std::to_string(moduleBase));
			PatternScanLog::Log("Scan Size: " + std::to_string(moduleSize) + " bytes (" + std::to_string(moduleSize / 1024 / 1024) + " MB)");
			PatternScanLog::Log("Pattern: " + pattern);
			
			std::vector<int> patternBytes = ParsePattern(pattern);
			if (patternBytes.empty())
			{
				PatternScanLog::Log("ERROR: Pattern parsing failed - empty pattern");
				return 0;
			}
			
			PatternScanLog::Log("Pattern Length: " + std::to_string(patternBytes.size()) + " bytes");

			// Read module memory in chunks for efficiency
			const size_t chunkSize = 0x10000; // 64KB chunks
			std::vector<uint8_t> buffer(chunkSize);
			
			int chunksScanned = 0;
			PatternScanLog::Log("Scanning memory in 64KB chunks...");

			for (size_t offset = 0; offset < moduleSize; offset += chunkSize)
			{
				size_t readSize = min(chunkSize, moduleSize - offset);
				ReadProcessMemoryImpl(moduleBase + offset, buffer.data(), readSize);
				chunksScanned++;

				// Search within this chunk
				for (size_t i = 0; i < readSize - patternBytes.size(); i++)
				{
					bool found = true;
					for (size_t j = 0; j < patternBytes.size(); j++)
					{
						if (patternBytes[j] != -1 && buffer[i + j] != patternBytes[j])
						{
							found = false;
							break;
						}
					}
					
					if (found)
					{
						uint64_t foundAddress = moduleBase + offset + i;
						PatternScanLog::Log("SUCCESS: Pattern found!");
						PatternScanLog::Log("Address: 0x" + std::to_string(foundAddress));
						PatternScanLog::Log("Offset from base: 0x" + std::to_string(offset + i));
						PatternScanLog::Log("Chunks scanned: " + std::to_string(chunksScanned));
						PatternScanLog::Log("=== Pattern Scan Completed ===");
						return foundAddress;
					}
				}
			}
			
			PatternScanLog::Log("FAILED: Pattern not found after scanning " + std::to_string(chunksScanned) + " chunks");
			PatternScanLog::Log("=== Pattern Scan Completed ===");
			return 0;
		}

		// Overload with default module size (scan first 50MB)
		inline uint64_t FindPattern(uint64_t moduleBase, const std::string& pattern)
		{
			return FindPattern(moduleBase, 0x3200000, pattern); // ~50MB scan range
		}
	}
}
