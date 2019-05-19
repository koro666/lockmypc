#pragma once
#include <windows.h>

HRESULT LmpcUiInitialize(void);
HRESULT LmpcUiFinalize(void);
HRESULT LmpcUiRunLoop(void);

LRESULT CALLBACK LmpcUiWndProc(HWND, UINT, WPARAM, LPARAM);

void LmpcUiCreateNotifyIcon(HWND);
void LmpcUiRemoveNotifyIcon(HWND);
