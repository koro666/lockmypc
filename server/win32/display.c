#include <windows.h>
#include <commctrl.h>
#include <shellapi.h>
#include <tchar.h>
#include "display.h"

extern IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)

static const TCHAR LmpcUiClass[] = TEXT("LockMyPC_WndClass");

static HICON LmpcUiIcon;
static ATOM LmpcUiAtom;
static UINT LmpcUiTaskbarMessage;
static HWND LmpcUiWindow;

HRESULT LmpcUiInitialize(void)
{
	HRESULT hr = S_OK;

	if (FindWindowEx(HWND_MESSAGE, NULL, LmpcUiClass, NULL))
	{
		hr = HRESULT_FROM_WIN32(ERROR_ALREADY_EXISTS);
		goto leave;
	}

	static const INITCOMMONCONTROLSEX iccx =
	{
		.dwSize = sizeof(INITCOMMONCONTROLSEX),
		.dwICC = ICC_STANDARD_CLASSES
	};

	if (!InitCommonControlsEx(&iccx))
	{
		hr = E_FAIL;
		goto leave;
	}

	WNDCLASSEX wcx =
	{
		.cbSize = sizeof(WNDCLASSEX),
		.lpfnWndProc = LmpcUiWndProc,
		.hInstance = THIS_HINSTANCE,
		.lpszClassName = LmpcUiClass
	};

	LmpcUiAtom = RegisterClassEx(&wcx);
	if (!LmpcUiAtom)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	LmpcUiTaskbarMessage = RegisterWindowMessage(TEXT("TaskbarCreated"));
	if (!LmpcUiTaskbarMessage)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	hr = LoadIconMetric(
		THIS_HINSTANCE,
		MAKEINTRESOURCE(1),
		LIM_SMALL,
		&LmpcUiIcon);

	if (FAILED(hr))
		goto leave;
	else
		hr = S_OK;

	LmpcUiWindow = CreateWindowEx(
		0,
		MAKEINTATOM(LmpcUiAtom),
		NULL,
		WS_POPUP,
		0, 0, 0, 0,
		HWND_MESSAGE,
		NULL,
		THIS_HINSTANCE,
		NULL);

	if (!LmpcUiWindow)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

leave:
	if (FAILED(hr))
		LmpcUiFinalize();

	return hr;
}

HRESULT LmpcUiFinalize(void)
{
	if (LmpcUiWindow)
	{
		DestroyWindow(LmpcUiWindow);
		LmpcUiWindow = NULL;
	}

	if (LmpcUiIcon)
	{
		DestroyIcon(LmpcUiIcon);
		LmpcUiIcon = NULL;
	}

	if (LmpcUiAtom)
	{
		UnregisterClass(MAKEINTATOM(LmpcUiAtom), (HINSTANCE)& __ImageBase);
		LmpcUiAtom = 0;
	}

	return S_OK;
}

HRESULT LmpcUiRunLoop(void)
{
	MSG m;
	BOOL b;

	while ((b = GetMessage(&m, NULL, 0, 0)))
	{
		if (b == -1)
			return HRESULT_FROM_WIN32(GetLastError());

		TranslateMessage(&m);
		DispatchMessage(&m);
	}

	return S_OK;
}

LRESULT CALLBACK LmpcUiWndProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
			LmpcUiCreateNotifyIcon(hWnd);
			return 0;
		case WM_DESTROY:
			LmpcUiRemoveNotifyIcon(hWnd);
			return 0;
		case WM_CLOSE:
		case WM_ENDSESSION:
			PostQuitMessage(0);
			return 0;
		default:
			if (uMsg == LmpcUiTaskbarMessage)
			{
				LmpcUiCreateNotifyIcon(hWnd);
				return 0;
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

void LmpcUiCreateNotifyIcon(HWND hWnd)
{
	{
		NOTIFYICONDATA nid =
		{
			.cbSize = sizeof(NOTIFYICONDATA),
			.hWnd = hWnd,
			.uID = 0,
			.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP,
			.uCallbackMessage = WM_USER,
			.hIcon = LmpcUiIcon
		};

		if (!LoadString(THIS_HINSTANCE, 1, nid.szTip, ARRAYSIZE(nid.szTip)))
			return;

		if (!Shell_NotifyIcon(NIM_ADD, &nid))
			return;
	}

	{
		NOTIFYICONDATA nid =
		{
			.cbSize = sizeof(NOTIFYICONDATA),
			.hWnd = hWnd,
			.uID = 0,
			.uVersion = NOTIFYICON_VERSION_4
		};

		Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}
}

void LmpcUiRemoveNotifyIcon(HWND hWnd)
{
	NOTIFYICONDATA nid =
	{
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = hWnd,
		.uID = 0
	};

	Shell_NotifyIcon(NIM_DELETE, &nid);
}
