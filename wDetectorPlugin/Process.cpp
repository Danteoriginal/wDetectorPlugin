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

#include "Process.h"
#include "wLog.h"
#include <tlhelp32.h>
#include <Windows.h>

#include <chrono>
#include <cstdint>
#include <thread>

DWORD PobierzIdProcesu(wchar_t* pProcessName)
{
	HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	PROCESSENTRY32 ProcessStruct;
	ProcessStruct.dwSize = sizeof(PROCESSENTRY32);
	wchar_t* temp = ProcessStruct.szExeFile;

	if (hSnap == INVALID_HANDLE_VALUE)
		return 0;

	if (Process32First(hSnap, &ProcessStruct) == FALSE)
	{
		CloseHandle(hSnap);
		return 0;
	}

	do
	{
		if (_wcsicmp(temp, pProcessName) == 0)
		{
			CloseHandle(hSnap);
			return ProcessStruct.th32ProcessID;
			break;
		}
	}

	while (Process32Next(hSnap, &ProcessStruct));

	CloseHandle(hSnap);

	return 0;
}

void KillProc(HANDLE hProcess)
{
	/*
	wchar_t msgtemp[255];
	DWORD ret = WaitForInputIdle(hProcess, 5000);
	swprintf_s(msgtemp, sizeof(msgtemp), L"WaitForInputIdle(): %d", ret);
	wLog(LOG_INFO, msgtemp);
	*/
	std::this_thread::sleep_for(std::chrono::milliseconds(1500));
	if (TerminateProcess(hProcess, 0))
	{
		CloseHandle(processInfo.hProcess);
		CloseHandle(processInfo.hThread);
		wLog(LOG_INFO, L"Killed wLauncher.exe");
	}
	else
	{
		wLog(LOG_ERROR, L"Could not kill wLauncher.exe");
	}
}

bool FindModuleBaseAddress(const wchar_t *module, uint32_t &wDetectorBaseAddress)
{
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
		if (wcscmp(lpModuleEntry.szModule, (const wchar_t *)module) == 0)
		{
			wDetectorBaseAddress = (uint32_t)lpModuleEntry.modBaseAddr;
			bModule = TRUE;
			CloseHandle(hSnapShot);
			break;
		}

		bModule = Module32NextW(hSnapShot, &lpModuleEntry);
	}

	if (bModule == FALSE)
		return false;

	return true;
}