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

#include <cwchar>
#include <Windows.h>
#include "FileInfo.h"

wchar_t * FileVersion(const wchar_t* pszFilePath)
{
	DWORD  verHandle = NULL;
	UINT   size = 0;
	LPBYTE lpBuffer = NULL;
	DWORD  verSize = GetFileVersionInfoSize(pszFilePath, &verHandle);
	static wchar_t buff[30] = L"";

	if (verSize != NULL)
	{
		LPSTR verData = new char[verSize];

		if (GetFileVersionInfo(pszFilePath, verHandle, verSize, verData))
		{
			if (VerQueryValue(verData, L"\\", (VOID FAR* FAR*)&lpBuffer, &size))
			{
				if (size)
				{
					VS_FIXEDFILEINFO *verInfo = (VS_FIXEDFILEINFO *)lpBuffer;
					if (verInfo->dwSignature == 0xfeef04bd)
					{
						swprintf(buff, sizeof(buff), L"%d.%d",
							(verInfo->dwFileVersionMS >> 16) & 0xffff,
							(verInfo->dwFileVersionLS >> 16) & 0xffff
							);
					}
				}
			}
		}
		delete[] verData;
	}

	return buff;
}