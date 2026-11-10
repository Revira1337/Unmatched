#include "Memory.hpp"
#include "../Render/kaanlar.h"

struct TGetWindowHandleData
{
	DWORD Pid;
	std::wstring WindowName;
	HWND hWnd;
};

HANDLE AttachedProcessHandle;
DWORD AttachedProcessPid;

BOOL CALLBACK EnumWindowsCallback(HWND Handle, LPARAM lParam)
{
	TGetWindowHandleData& Data = *(TGetWindowHandleData*)lParam;

	// Searching For Name
	if (Data.Pid == 0)
	{
		int Length = SafeCall(GetWindowTextLength)(Handle);
		if (Length == 0)
			return true;

		std::wstring Buffer(Length + 1, L'\0');

		SafeCall(GetWindowText)(Handle, &Buffer[0], Length + 1);

		if (Data.WindowName != Buffer)
			return true;

		Data.hWnd = Handle;
		return false;
	}
	else // Searching for Pid
	{
		DWORD Pid;
		SafeCall(GetWindowThreadProcessId)(Handle, &Pid);

		if (Data.Pid != Pid)
			return true;

		Data.hWnd = Handle;
		return false;
	}

	return true;
}

namespace M_base
{
	HWND mem::GetWindowHandleByPID(int Pid)
	{
		TGetWindowHandleData HandleData;
		HandleData.Pid = Pid;
		HandleData.WindowName = xorstr_(L"");
		HandleData.hWnd = NULL;

		SafeCall(EnumWindows)(EnumWindowsCallback, (LPARAM)&HandleData);
		return HandleData.hWnd;
	}

	HWND mem::GetWindowHandleByName(std::wstring WindowName)
	{
		TGetWindowHandleData HandleData;
		HandleData.Pid = 0;
		HandleData.WindowName = WindowName;
		HandleData.hWnd = NULL;

		SafeCall(EnumWindows)(EnumWindowsCallback, (LPARAM)&HandleData);
		return HandleData.hWnd;
	}

	DWORD mem::GetProcessPidByName(std::wstring ProcessName)
	{
		HANDLE hSnapshot = SafeCall(CreateToolhelp32Snapshot)(TH32CS_SNAPPROCESS, 0);
		if (!hSnapshot || hSnapshot == INVALID_HANDLE_VALUE || hSnapshot == ((HANDLE)(LONG_PTR)ERROR_BAD_LENGTH))
		{

			return 0;
		}

		DWORD Pid;
		PROCESSENTRY32 ProcessEntry;
		ProcessEntry.dwSize = sizeof(ProcessEntry);
		if (SafeCall(Process32First)(hSnapshot, &ProcessEntry))
		{
			while (_wcsicmp(ProcessEntry.szExeFile, ProcessName.c_str()))
			{
				if (!SafeCall(Process32Next)(hSnapshot, &ProcessEntry))
				{
					SafeCall(CloseHandle)(hSnapshot);
					return 0;
				}
			}

			Pid = ProcessEntry.th32ProcessID;
		}
		else
		{
			SafeCall(CloseHandle)(hSnapshot);
			return 0;
		}

		SafeCall(CloseHandle)(hSnapshot);
		return Pid;
	}

	uint64_t mem::GetModuleBaseByName(DWORD Pid, std::wstring ModuleName)
	{
		HANDLE hSnapshot = SafeCall(CreateToolhelp32Snapshot)(TH32CS_SNAPMODULE, Pid);
		if (!hSnapshot || hSnapshot == INVALID_HANDLE_VALUE || hSnapshot == ((HANDLE)(LONG_PTR)ERROR_BAD_LENGTH))
		{

			return 0;
		}

		uint64_t ModuleBase;
		MODULEENTRY32 ModuleEntry;
		ModuleEntry.dwSize = sizeof(ModuleEntry);
		if (SafeCall(Module32First)(hSnapshot, &ModuleEntry))
		{
			while (_wcsicmp(ModuleEntry.szModule, ModuleName.c_str()))
			{
				if (!SafeCall(Module32Next)(hSnapshot, &ModuleEntry))
				{
					SafeCall(CloseHandle)(hSnapshot);
					return 0;
				}
			}

			ModuleBase = (uint64_t)ModuleEntry.modBaseAddr;
		}
		else
		{
			SafeCall(CloseHandle)(hSnapshot);
			return 0;
		}

		SafeCall(CloseHandle)(hSnapshot);
		return ModuleBase;
	}

	void mem::AttachProces(DWORD Pid)
	{
		AttachedProcessHandle = OpenProcess(PROCESS_ALL_ACCESS, false, Pid);
		AttachedProcessPid = Pid;
	}

	void mem::DetachProcess()
	{
		CloseHandle(AttachedProcessHandle);
		AttachedProcessHandle = nullptr;
		AttachedProcessPid = 0x0;
	}

	void mem::ReadProcessMemoryImpl(uint64_t ReadAddress, LPVOID Read, SIZE_T Size)
	{
		if (AttachedProcessHandle && AttachedProcessPid)
		{
			if (ReadProcessMemory(AttachedProcessHandle, (LPVOID)ReadAddress, Read, Size, NULL))
			{
				return;
			}
		}
	}

	bool mem::WriteProcessMemoryImpl(uint64_t WriteAddress, LPVOID Value, SIZE_T Size)
	{
		if (AttachedProcessHandle && AttachedProcessPid)
		{
			if (WriteProcessMemory(AttachedProcessHandle, (LPVOID)WriteAddress, Value, Size, NULL))
			{
				return true;
			}
		}

		return false;
	}

	std::string mem::ReadProcessMemoryString(uint64_t ReadAddress, SIZE_T StringSize)
	{
		const int BufferSize = 256;

		char Buffer[BufferSize];

		int BytesRead = 0;

		while (BytesRead < BufferSize && BytesRead < StringSize)
		{
			char Character;
			ReadProcessMemoryImpl((uint64_t)ReadAddress + BytesRead, &Character, sizeof(char));
			Buffer[BytesRead] = Character;

			if (Character == '\0')
				break;

			BytesRead++;
		}

		return std::string(Buffer);
	}
}