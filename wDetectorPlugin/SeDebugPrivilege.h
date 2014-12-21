#include "Source.h"

bool SetPrivilege(HANDLE  hToken, LPCTSTR lpPrivilege, BOOL	bEnablePrivilege);
bool SetDebugPrivilege(BOOL	bEnable);