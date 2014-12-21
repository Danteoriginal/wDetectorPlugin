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