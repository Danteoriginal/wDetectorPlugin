//wDetector Plugin by xboi209
#include "FileInfo.h"
#include "Inject.h"
#include "Process.h"
#include "SeDebugPrivilege.h"
#include "Source.h"
#include "wLog.h"

#include <Windows.h>
#include <process.h>
#include <tlhelp32.h>

#include <cwchar>
#include <cstdlib>

struct ExchangeData
{
	int  iPluginAPI;
	int  iStarCraftBuild;
	BOOL bNotSCBWmodule;
	BOOL bConfigDialog;
};

extern "C" __declspec(dllexport) void GetPluginAPI(ExchangeData &Data)
{
	// BWL Gets version from Resource - VersionInfo
	Data.iPluginAPI = BWLAPI;  			//4 == BWL4
	Data.iStarCraftBuild = STARCRAFTBUILD; 		//13 == StarCraft 1.16.1
	Data.bConfigDialog = FALSE;		//TRUE == Allow Config button
	Data.bNotSCBWmodule = FALSE;	//Inform user that closing BWL will shut down your plugin(?)
}


extern "C" __declspec(dllexport) void GetData(char* name, char* description, char* updateurl)
{
	size_t i;
	wchar_t* name0 = L"wDetector";
	wchar_t* description0 = L"Injects and patches wDetector\r\n\r\nwDetector by Won Soon-cheol\r\nwDetector offsets by DyS- and mca64\r\nwDetector Plugin by iCCup.xboi209";
	wchar_t* updateurl0 = L"http://mjr896.net/techguy/wDetector/";
	char *pMBBuffer = (char *)malloc(65536);

	//https://github.com/MasterOfChaos/Chaoslauncher/blob/88c889c203e9fe47880fa1661657f2428ffa736e/Source/Launcher/Launcher/Plugins_CHL.pas#L82
	wcstombs_s(&i, pMBBuffer, 256, name0, wcslen(name0) + 1);
	strcpy_s(name, strlen(pMBBuffer) + 1, pMBBuffer);

	wcstombs_s(&i, pMBBuffer, 65536, description0, wcslen(description0) + 1);
	strcpy_s(description, strlen(pMBBuffer) + 1, pMBBuffer);

	wcstombs_s(&i, pMBBuffer, 1024, updateurl0, wcslen(updateurl0) + 1);
	strcpy_s(updateurl, strlen(pMBBuffer) + 1, pMBBuffer);

	free(pMBBuffer);
}


// Called when user clicks Config button
extern "C" __declspec(dllexport) bool OpenConfig()
{
	return true;
}


//Called before StarCraft is completely loaded
extern "C" __declspec(dllexport) bool ApplyPatchSuspended(HANDLE, DWORD)
{
	if (GetFileAttributesW(WLAUNCHER) == 0xFFFFFFFF)
	{
		MessageBoxW(NULL, L"wLauncher.exe not found!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
	}
	else if (GetFileAttributesW(WDETECTOR) == 0xFFFFFFFF)
	{
		MessageBoxW(NULL, L"wDetector.w not found!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
	}
	else if (PrintFileVersion(WDETECTOR) != 32)
	{
		MessageBoxW(NULL, L"wDetector's version is incorrect!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
	}
	else
	{
		STARTUPINFOW info = { sizeof(info) };

		if (CreateProcessW(WLAUNCHER, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo) == 0)
		{
			MessageBoxW(NULL, L"Failed to start wLauncher.exe!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
			return false;
		}
	}

	return true;
}


//Called after StarCraft is completely loaded
extern "C" __declspec(dllexport) bool ApplyPatch(HANDLE hProcess, DWORD dwProcessID)
{
	wchar_t msgtemp[255];
	wchar_t dll[MAX_PATH];

	GetFullPathNameW(WDETECTOR, MAX_PATH, dll, NULL);

	wLog(LOG_INFO, L"Logging started");

	//Start array
	/*
	LPVOID addresses[] = {
		//"0x3F230",
	};
	BYTE values[] = {
		//{ 0x00 },
	};
	char
	*/

	//Get SeDebugPrivilege
	if (SetDebugPrivilege(TRUE) == true)
	{
		wLog(LOG_INFO, L"Obtained SeDebugPrivilege");
	}
	else
	{
		wLog(LOG_ERROR, L"Unable to obtain SeDebugPrivilege");
		return false;
	}

	//Inject wDetector.w
	if (CreateRemoteThreadInject(dwProcessID, dll) == true)
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"Injected %ls into %d", dll, dwProcessID);
		wLog(LOG_INFO, msgtemp);
	}
	else
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"Could not inject %ls into %d", dll, dwProcessID);
		wLog(LOG_ERROR, msgtemp);
		return false;
	}

	//Kill wLauncher.exe
	Sleep(1500);
	if (TerminateProcess(processInfo.hProcess, 0) != 0) //Kill wLauncher.exe
	{
		wLog(LOG_INFO, L"Killed wLauncher.exe");
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
	}
	else
	{
		wLog(LOG_INFO, L"Could not kill wLauncher.exe");
		return false;
	}

	//Get base address of wDetector.w module
	DWORD wDetectorBaseAddress = 0;
	MODULEENTRY32W lpModuleEntry = { 0 };
	HANDLE hSnapShot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, PobierzIdProcesu(L"starcraft.exe"));
	if (hSnapShot == INVALID_HANDLE_VALUE)
	{
		wLog(LOG_ERROR, L"Could not get handle for StarCraft.exe");
		return false;
	}

	lpModuleEntry.dwSize = sizeof(lpModuleEntry);
	BOOL bModule = Module32FirstW(hSnapShot, &lpModuleEntry);

	while (bModule)
	{
		if (wcscmp(lpModuleEntry.szModule, WDETECTOR) == 0)
		{
			wDetectorBaseAddress = (DWORD)lpModuleEntry.modBaseAddr;
			swprintf_s(msgtemp, sizeof(msgtemp), L"wDetector's base address is %d", wDetectorBaseAddress);
			wLog(LOG_INFO, msgtemp);
			bModule = TRUE;
			CloseHandle(hSnapShot);
			break;
		}

		bModule = Module32NextW(hSnapShot, &lpModuleEntry);
	}

	if (bModule == FALSE)
	{
		wLog(LOG_ERROR, L"Could not get wDetector's base address!");
		return false;
	}

	//Patch wDetector
	BYTE ADD[1] = { 0x00 };
	DWORD wDetectorActivate = wDetectorBaseAddress + 0x3F230;
	//DWORD wDetectorRefresh = wDetectorBaseAddress + 0x40D5C;

	//Activation
	WriteProcessMemory(hProcess, (LPVOID)wDetectorActivate, &ADD, sizeof(ADD), NULL);
	swprintf_s(msgtemp, sizeof(msgtemp), L"[ACTIVATION] WriteProcessMemory %d, address %d", GetLastError(), wDetectorActivate);
	wLog(LOG_INFO, msgtemp);

	//Lobby
	//WriteProcessMemory(hProcess, (LPVOID)wDetectorRefresh, "<wDetector 3.32 - Refreshing Lobby>", 36, NULL);
	//swprintf_s(msgtemp, sizeof(msgtemp), L"[LOBBY] WriteProcessMemory %d, address %d", GetLastError(), wDetectorRefresh);
	//wLog(LOG_INFO, msgtemp);

	wLog(LOG_INFO, L"Logging ended");

	return true;
}