/*
*	wDetector plugin for Chaoslauncher, activates and translates wDetector
*	Copyright (C) 2015  xboi209 xboi209@gmail.com
*
*	This program is free software: you can redistribute it and/or modify
*	it under the terms of the GNU General Public License as published by
*	the Free Software Foundation, either version 3 of the License, or
*	(at your option) any later version.
*
*	This program is distributed in the hope that it will be useful,
*	but WITHOUT ANY WARRANTY; without even the implied warranty of
*	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*	GNU General Public License for more details.
*
*	You should have received a copy of the GNU General Public License
*	along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#define BWLAPI 4
#define STARCRAFTBUILD 13
#define WLAUNCHER L"wLauncher.exe"
#define WDETECTOR L"wDetector.w"

#include "FileInfo.h"
#include "Inject.h"
#include "Process.h"
#include "SeDebugPrivilege.h"
#include "wLog.h"

#include <Windows.h>
#include <process.h>
#include <tlhelp32.h>

#include <cwchar>
#include <cstdint>
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
	char* name0 = "wDetector";
	char* description0 = "Injects and patches wDetector 3.35\r\n\r\nwDetector by Won Soon-cheol\r\nwDetector offsets by DyS- and mca64\r\nwDetector Plugin by iCCup.xboi209";
	char* updateurl0 = "";

	//https://github.com/MasterOfChaos/Chaoslauncher/blob/88c889c203e9fe47880fa1661657f2428ffa736e/Source/Launcher/Launcher/Plugins_CHL.pas#L82
	strcpy_s(name, strlen(name0) + 1, name0);
	strcpy_s(description, strlen(description0) + 1, description0);
	strcpy_s(updateurl, strlen(updateurl0) + 1, updateurl0);
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

	if (GetFileAttributesW(WDETECTOR) == 0xFFFFFFFF)
	{
		MessageBoxW(NULL, L"wDetector.w not found!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
	}

	if (wcscmp(FileVersion(WDETECTOR), L"3.35") != 0)
	{
		MessageBoxW(NULL, L"wDetector's version is incorrect!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
	}

	STARTUPINFOW info = { sizeof(info) };

	if (!CreateProcessW(WLAUNCHER, NULL, NULL, NULL, TRUE, 0, NULL, NULL, &info, &processInfo))
	{
		MessageBoxW(NULL, L"Failed to start wLauncher.exe!", L"wDetector Plugin", MB_OK | MB_ICONERROR);
		return false;
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

	/*
	DWORD offsets[] = {
	0x3F260,	//Activation
	0x40E04		//Refresh game message
	};

	int8_t vals[][37] = {
	{ 0x00, 0x00, 0xAA, 0xB4, 0xC2, 0x20, 0xC7, 0xD1, 0xB1, 0xB9, 0xC0, 0xCE, 0x00, 0x06, 0x25, 0x73, 0x03, 0xB0, 0xA1, 0x20, 0xB4, 0xE7, 0xBD, 0xC5, 0xC0, 0xBB, 0x20, 0xB0, 0xAD, 0xC5, 0xF0, 0xC7, 0xCF, 0xBF, 0xB4, 0xBD, 0xC0 },
	{ 0x3C, 0x77, 0x44, 0x65, 0x74, 0x65, 0x63, 0x74, 0x6F, 0x72, 0x20, 0x33, 0x2E, 0x33, 0x34, 0x20, 0x2D, 0x20, 0x52, 0x65, 0x66, 0x72, 0x65, 0x73, 0x68, 0x69, 0x6E, 0x67, 0x20, 0x47, 0x61, 0x6D, 0x65, 0x3E, 0x00, 0x00, 0x00 } // <wDetector 3.34 - Refreshing Game>
	};

	for (auto i = 0; i < (sizeof(offsets) / sizeof(*offsets)); i++)
	{
	WriteProcessMemory(hProcess, (LPVOID)(wDetectorBaseAddress + offsets[i]), &vals[i], sizeof(vals[i]), NULL);
	swprintf_s(msgtemp, sizeof(msgtemp), L"Address: %d, Value: %d", wDetectorBaseAddress + offsets[i], vals[i]);
	wLog(LOG_INFO, msgtemp);
	}
	*/
	int8_t activate = { 0x12 };
	WriteProcessMemory(hProcess, (LPVOID)(wDetectorBaseAddress + (DWORD)0x5AD94), &activate, sizeof(activate), NULL);
	wLog(LOG_INFO, L"wDetector activated!");
	WriteProcessMemory(hProcess, (LPVOID)(wDetectorBaseAddress + (DWORD)0x429C4), "<wDetector 3.35 - Refreshing Game>", 35, NULL);

	wLog(LOG_INFO, L"Logging ended");

	return true;
}
