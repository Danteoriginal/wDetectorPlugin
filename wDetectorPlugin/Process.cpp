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
#include <tlhelp32.h>

int PobierzIdProcesu(wchar_t* pProcessName)
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