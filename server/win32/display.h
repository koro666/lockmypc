#pragma once
#include <windows.h>
#include "config.h"

struct _LMPC_UI;
typedef struct _LMPC_UI LMPC_UI, *HLMPC_UI;

HRESULT LmpcUiCreate(HLMPC_CONFIG, HLMPC_UI*);
HRESULT LmpcUiDestroy(HLMPC_UI);

HRESULT LmpcUiRunLoop(HLMPC_UI);

LRESULT CALLBACK LmpcUiWndProcStatic(HWND, UINT, WPARAM, LPARAM);
LRESULT LmpcUiWndProc(HLMPC_UI, UINT, WPARAM, LPARAM);

LRESULT LmpcUiHandleCommand(HLMPC_UI, WPARAM, LPARAM);
LRESULT LmpcUiHandleNotifyMessage(HLMPC_UI, WPARAM, LPARAM);

INT_PTR CALLBACK LmpcUiDlgProcStatic(HWND, UINT, WPARAM, LPARAM);
INT_PTR LmpcUiDlgProc(HLMPC_UI, UINT, WPARAM, LPARAM);

void LmpcUiCreateNotifyIcon(HLMPC_UI);
void LmpcUiRemoveNotifyIcon(HLMPC_UI);

void LmpcUiShowMenu(HLMPC_UI, int, int);
