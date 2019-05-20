#pragma once
#include <windows.h>

typedef struct
{
	DWORD Signature;
	UINT Time;
	BYTE Hash[32];
} LMPC_PACKET, *PLMPC_PACKET;

HRESULT LmpcSrvInitialize(void);
HRESULT LmpcSrvFinalize(void);

HRESULT LmpcSrvStart(HWND, UINT);
HRESULT LmpcSrvStop(void);

LRESULT LmpcSrvHandleSelect(WPARAM, LPARAM);
HRESULT LmpcSrvHandlePacket(const LMPC_PACKET*, SIZE_T);
HRESULT LmpcSrvCheckPacket(const LMPC_PACKET*, SIZE_T);

ULONG LmpcSrvGetUnixTime(void);
