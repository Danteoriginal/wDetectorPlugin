#include <Windows.h>
#include "FileInfo.h"

int PrintFileVersion(const wchar_t* pszFilePath)
{
	DWORD               dwSize = 0;
	BYTE                *pbVersionInfo = NULL;
	VS_FIXEDFILEINFO    *pFileInfo = NULL;
	UINT                puLenFileInfo = 0;
	int version = 0;

	// get the version info for the file requested
	dwSize = GetFileVersionInfoSizeW(pszFilePath, NULL);
	if (dwSize == 0)
		return version;

	pbVersionInfo = new BYTE[dwSize];

	if (GetFileVersionInfoW(pszFilePath, 0, dwSize, pbVersionInfo) == 0)
	{
		delete[] pbVersionInfo;
		return version;
	}

	if (VerQueryValueW(pbVersionInfo, L"\\", (LPVOID*)&pFileInfo, &puLenFileInfo) == 0)
	{
		delete[] pbVersionInfo;
		return version;
	}

	delete[] pbVersionInfo;
	version = (pFileInfo->dwFileVersionLS >> 16) & 0xff;
	return version;
}