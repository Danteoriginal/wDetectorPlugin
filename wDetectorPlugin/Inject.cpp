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