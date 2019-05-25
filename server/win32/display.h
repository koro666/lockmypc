#pragma once
#include <windows.h>
#include "config.h"

HRESULT LmpcUiInitialize(HLMPC_CONFIG);
HRESULT LmpcUiFinalize(void);
HRESULT LmpcUiRunLoop(void);

LRESULT CALLBACK LmpcUiWndProc(HWND, UINT, WPARAM, LPARAM);

LRESULT LmpcUiHandleCommand(HWND, WPARAM, LPARAM);
LRESULT LmpcUiHandleNotifyMessage(HWND, WPARAM, LPARAM);

INT_PTR LmpcUiDlgProc(HWND, UINT, WPARAM, LPARAM);

void LmpcUiCreateNotifyIcon(HWND);
void LmpcUiRemoveNotifyIcon(HWND);

void LmpcUiShowMenu(HWND, int, int);
