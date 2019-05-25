#pragma once
#include <windows.h>
#include <shlwapi.h>

#if UNICODE
#define CoStrDup SHStrDup
#else
HRESULT CoStrDup(LPCTSTR, LPTSTR*);
#endif
