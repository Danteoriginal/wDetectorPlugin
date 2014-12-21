#include <Windows.h>

int PobierzIdProcesu(wchar_t* pProcessName);

static PROCESS_INFORMATION processInfo = { 0 };