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

#include <Windows.h>
#include <cstdint>

DWORD PobierzIdProcesu(wchar_t* pProcessName);
void KillProc(HANDLE hProcess);
bool FindModuleBaseAddress(const wchar_t *module, uint32_t &wDetectorBaseAddress);

static PROCESS_INFORMATION processInfo = { 0 };