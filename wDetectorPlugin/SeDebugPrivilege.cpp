#include "SeDebugPrivilege.h"

bool SetPrivilege(HANDLE  hToken, LPCTSTR lpPrivilege, BOOL	bEnablePrivilege)
{
	TOKEN_PRIVILEGES	tkp = { 0 };
	LUID				luid = { 0 };
	TOKEN_PRIVILEGES	tkpPrevious = { 0 };
	DWORD			   cbPrevious = 0;

	if ((!hToken) || (!lpPrivilege))
		return false;

	if (!LookupPrivilegeValue(NULL, lpPrivilege, &luid))
		return false;

	tkp.PrivilegeCount = 1;
	tkp.Privileges[0].Luid = luid;
	tkp.Privileges[0].Attributes = 0;
	cbPrevious = sizeof(TOKEN_PRIVILEGES);
	AdjustTokenPrivileges(hToken, FALSE, &tkp, sizeof(TOKEN_PRIVILEGES), &tkpPrevious, &cbPrevious);

	if (GetLastError() != ERROR_SUCCESS)
		return false;

	tkpPrevious.PrivilegeCount = 1;
	tkpPrevious.Privileges[0].Luid = luid;

	if (bEnablePrivilege)
		tkpPrevious.Privileges[0].Attributes |= (SE_PRIVILEGE_ENABLED);
	else
		tkpPrevious.Privileges[0].Attributes ^= (SE_PRIVILEGE_ENABLED & tkpPrevious.Privileges[0].Attributes);

	AdjustTokenPrivileges(hToken, FALSE, &tkpPrevious, cbPrevious, NULL, NULL);

	if (GetLastError() != ERROR_SUCCESS)
		return false;

	return true;
}

bool SetDebugPrivilege(BOOL	bEnable)
{
	HANDLE hToken = NULL;

	if (!OpenProcessToken(GetCurrentProcess(), TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY, &hToken))
		return false;

	if (!SetPrivilege(hToken, SE_DEBUG_NAME, bEnable))
	{
		CloseHandle(hToken);
		return false;
	}

	CloseHandle(hToken);
	return true;
}