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

#include <array>
#include <cwchar>
#include <cstdint>
#include <cstdlib>
#include <string>
#include <thread>

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
	char* description0 = "Injects and patches wDetector 3.36\r\n\r\nwDetector by Won Soon-cheol\r\nwDetector offsets by DyS- and mca64\r\nwDetector Plugin by iCCup.xboi209";
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

	if (wcscmp(FileVersion(WDETECTOR), L"3.36") != 0)
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

	wLog(LOG_INFO, L"~Logging started~");

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
	std::thread wLauncher(KillProc, processInfo.hProcess);

	//Wait for wDetector.w
	std::this_thread::sleep_for(std::chrono::milliseconds(250));

	//Get base address of wDetector.w module
	uint32_t wDetectorBaseAddress = 0;
	if (FindModuleBaseAddress(WDETECTOR, wDetectorBaseAddress) == true)
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"wDetector's base address is %d", wDetectorBaseAddress);
		wLog(LOG_INFO, msgtemp);
	}
	else
	{
		wLog(LOG_ERROR, L"Could not get wDetector's base address!");
		return false;
	}
	
	//Wait for wLauncher to be killed
	wLauncher.join();
	
	//Patch wDetector
	int8_t activate = { 0x12 };
	WriteProcessMemory(hProcess, (LPVOID)(wDetectorBaseAddress + (uint32_t)0x5AD94), &activate, sizeof(activate), NULL);
	wLog(LOG_INFO, L"wDetector activated!");

	std::array<uint32_t, 15> offset = {
		0x429E4,		//Refresh game message
		0x43CB4,		//toggle automatic refresh - enable
		0x43CAC,		//toggle automatic refresh - disable
		0x41B7C,	//ago
		0x41B74,	//min
		0x41B78,	//sec
		0x41AA0,
		0x418C3,	//mission briefing
		0x43D93,	//time off
		0x43D83,	//time on
		0x43CBD,	//toggle automatic refresh
		0x4297D,	//Automatic game refresh disable
		0x4299F,	//3 minutes passed)
		0x429C8,	//seconds until refreshing.
		0x41C0B
	};

	std::array<std::string, 15> vals = {
		"Refreshing", //<wDetector 3.35 - Refreshing>
		"enabled", //toggle automatic refresh
		"disable", //toggle automatic refresh
		"ago",
		"min",
		"sec",
		" min %u sec",
		"Players Ready", //mission briefing
		"Time off",
		"Time on",
		"Automatic refresh %s", //toggle automatic refresh
		"Automatic game refresh disable" //msg after 3 mins
		"3 minutes passed)", //msg after 3 mins
		"seconds until refreshing.", //F5
		"English"

	};

	{
		char buff[50] = "";
		for (std::size_t i{ 0 }; i < offset.size(); ++i)
		{
			strcpy_s(buff, sizeof(buff), vals.at(i).c_str());
			WriteProcessMemory(hProcess, (LPVOID)(wDetectorBaseAddress + offset.at(i)), buff, strlen(buff) + 1, NULL);
		}
	}

	wLog(LOG_INFO, L"wDetector translated");

	return true;
}
