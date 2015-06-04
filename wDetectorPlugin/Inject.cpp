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

#include "Inject.h"
#include "wLog.h"
#include <cstdio>

bool CreateRemoteThreadInject(DWORD ID, const wchar_t* dll)
{
	HANDLE Process;
	LPVOID Memory;
	LPVOID LoadLibrary;
	wchar_t msgtemp[255];

	if (!ID || ID < 0)
	{
		wLog(LOG_ERROR, L"Invalid PID");
		return false;
	}

	Process = OpenProcess(PROCESS_CREATE_THREAD | PROCESS_QUERY_INFORMATION | PROCESS_VM_READ | PROCESS_VM_WRITE | PROCESS_VM_OPERATION, FALSE, ID);
	if (Process == NULL)
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"OpenProcess returned %d", GetLastError());
		wLog(LOG_ERROR, msgtemp);
		return false;
	}

	LoadLibrary = (LPVOID)GetProcAddress(GetModuleHandleA("kernel32.dll"), "LoadLibraryA");
	if (LoadLibrary == NULL)
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"GetProcAddress returned %d", GetLastError());
		wLog(LOG_ERROR, msgtemp);
		CloseHandle(Process);
		return false;
	}

	char mbs[MAX_PATH];
	WideCharToMultiByte(CP_ACP, WC_COMPOSITECHECK, dll, -1, mbs, sizeof(mbs), NULL, NULL);

	Memory = (LPVOID)VirtualAllocEx(Process, NULL, strlen(mbs) + 1, MEM_RESERVE | MEM_COMMIT, PAGE_READWRITE);
	if (Memory == NULL)
	{
		swprintf_s(msgtemp, sizeof(msgtemp), L"VirtualAllocEx returned %d", GetLastError());
		wLog(LOG_ERROR, msgtemp);
		CloseHandle(Process);
		return false;
	}

	WriteProcessMemory(Process, (LPVOID)Memory, mbs, strlen(mbs) + 1, NULL);

	CreateRemoteThread(Process, NULL, NULL, (LPTHREAD_START_ROUTINE)LoadLibrary, (LPVOID)Memory, NULL, NULL);

	CloseHandle(Process);

	VirtualFreeEx(Process, (LPVOID)Memory, 0, MEM_RELEASE);

	return true;
}