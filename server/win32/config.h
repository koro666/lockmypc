#pragma once
#include <windows.h>
#include <tchar.h>

typedef struct
{
	LPTSTR Address;
	LPTSTR Port;
	LPTSTR Secret;
} LMPC_CONFIG, *PLMPC_CONFIG;

extern LMPC_CONFIG LmpcCfgCurrent;

HRESULT LmpcCfgInitialize(void);
HRESULT LmpcCfgFinalize(void);

HRESULT LmpcCfgLoad(void);
HRESULT LmpcCfgSave(void);

HRESULT LmpcCfgStringRead(LPCTSTR, LPWSTR*, LPCTSTR);
HRESULT LmpcCfgStringWrite(LPCTSTR, LPCTSTR);

HRESULT LmpcCfgStringSet(HWND, int, LPTSTR*);
void LmpcCfgStringFree(LPWSTR*);
