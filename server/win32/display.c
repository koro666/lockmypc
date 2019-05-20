#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <shellapi.h>
#include <tchar.h>
#include "display.h"
#include "resource.h"
#include "config.h"
#include "server.h"

extern IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)

static const TCHAR LmpcUiClass[] = TEXT("LockMyPC_WndClass");

static HICON LmpcUiIcon;
static ATOM LmpcUiAtom;
static UINT LmpcUiTaskbarMessage;
static HWND LmpcUiWindow;
static HWND LmpcUiDialog;

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
		.dwICC =  ICC_UPDOWN_CLASS | ICC_STANDARD_CLASSES | ICC_LINK_CLASS
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
		MAKEINTRESOURCEW(IDI_MAIN),
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

		if (LmpcUiDialog && IsDialogMessage(LmpcUiDialog, &m))
			continue;

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
			LmpcSrvStart(hWnd, WM_USER+1);
			LmpcUiCreateNotifyIcon(hWnd);
			return 0;
		case WM_DESTROY:
			LmpcUiRemoveNotifyIcon(hWnd);
			LmpcSrvStop();
			return 0;
		case WM_CLOSE:
		case WM_ENDSESSION:
			PostQuitMessage(0);
			return 0;
		case WM_COMMAND:
			return LmpcUiHandleCommand(hWnd, wParam, lParam);
		case WM_USER:
			return LmpcUiHandleNotifyMessage(hWnd, wParam, lParam);
		case WM_USER+1:
			return LmpcSrvHandleSelect(wParam, lParam);
		default:
			if (uMsg == LmpcUiTaskbarMessage)
			{
				LmpcUiCreateNotifyIcon(hWnd);
				return 0;
			}

			return DefWindowProc(hWnd, uMsg, wParam, lParam);
	}
}

LRESULT LmpcUiHandleCommand(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDC_SETTINGS:
			if (LmpcUiDialog)
			{
				SetForegroundWindow(LmpcUiDialog);
			}
			else
			{
				LmpcUiDialog = CreateDialogParam(THIS_HINSTANCE, MAKEINTRESOURCE(IDD_SETTINGS), NULL, LmpcUiDlgProc, 0);
				ShowWindow(LmpcUiDialog, SW_SHOW);
			}
			break;
		case IDC_EXIT:
			if (LmpcUiDialog)
				SendMessage(LmpcUiDialog, WM_CLOSE, 0, 0);
			SendMessage(hWnd, WM_CLOSE, 0, 0);
			break;
	}

	UNREFERENCED_PARAMETER(lParam);
	return 0;
}

LRESULT LmpcUiHandleNotifyMessage(HWND hWnd, WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
		case MAKELPARAM(WM_CONTEXTMENU, 0):
			LmpcUiShowMenu(hWnd, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			break;
		case MAKELPARAM(WM_LBUTTONDBLCLK, 0):
		case MAKELPARAM(NIN_KEYSELECT, 0):
			SendMessage(hWnd, WM_COMMAND, MAKEWPARAM(IDC_SETTINGS, 0), 0);
			break;
	}

	return 0;
}

INT_PTR LmpcUiDlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			WCHAR buffer[256];

			SendDlgItemMessage(hWnd, IDC_SETTINGS_HOST, EM_LIMITTEXT, 256, 0);
			if (LoadStringW(THIS_HINSTANCE, IDS_HOST_DEFAULT, buffer, ARRAYSIZE(buffer)))
				SendDlgItemMessage(hWnd, IDC_SETTINGS_HOST, EM_SETCUEBANNER, TRUE, (LPARAM)buffer);
			SetDlgItemText(hWnd, IDC_SETTINGS_HOST, LmpcCfgCurrent.Address);

			SendDlgItemMessage(hWnd, IDC_SETTINGS_PORT, EM_LIMITTEXT, 5, 0);
			SetDlgItemText(hWnd, IDC_SETTINGS_PORT, LmpcCfgCurrent.Port);

			SendDlgItemMessage(hWnd, IDC_SETTINGS_PORT_SPIN, UDM_SETRANGE32, 1, 65535);

			SetDlgItemText(hWnd, IDC_SETTINGS_SECRET, LmpcCfgCurrent.Secret);
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case MAKEWPARAM(IDOK, BN_CLICKED):
				{
					LmpcCfgStringSet(hWnd, IDC_SETTINGS_HOST, &LmpcCfgCurrent.Address);
					LmpcCfgStringSet(hWnd, IDC_SETTINGS_PORT, &LmpcCfgCurrent.Port);
					LmpcCfgStringSet(hWnd, IDC_SETTINGS_SECRET, &LmpcCfgCurrent.Secret);
					LmpcCfgSave();
					LmpcSrvStop();
					LmpcSrvStart(LmpcUiWindow, WM_USER+1);
					// fallthrough
				}
				case MAKEWPARAM(IDCANCEL, BN_CLICKED):
				{
					DestroyWindow(hWnd);
					LmpcUiDialog = NULL;
					return TRUE;
				}
			}
			return FALSE;
		}
		case WM_NOTIFY:
		{
			LPNMHDR nmhdr = (LPNMHDR)lParam;
			if (nmhdr->idFrom == IDC_SETTINGS_HLINK &&
				(nmhdr->code == NM_CLICK || nmhdr->code == NM_RETURN))
			{
				PNMLINK nmlink = (PNMLINK)lParam;

				SHELLEXECUTEINFOW sei =
				{
					.cbSize = sizeof(SHELLEXECUTEINFOW),
					.fMask = SEE_MASK_NOASYNC | SEE_MASK_FLAG_NO_UI | SEE_MASK_HMONITOR,
					.lpVerb = L"open",
					.lpFile = nmlink->item.szUrl,
					.nShow = SW_SHOW,
					.hMonitor = MonitorFromWindow(hWnd, MONITOR_DEFAULTTONEAREST)
				};

				ShellExecuteExW(&sei);
				return TRUE;
			}
			return FALSE;
		}
		default:
		{
			return FALSE;
		}
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
			.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP,
			.uCallbackMessage = WM_USER,
			.hIcon = LmpcUiIcon
		};

		if (!LoadString(THIS_HINSTANCE, IDS_APPNAME, nid.szTip, ARRAYSIZE(nid.szTip)))
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

void LmpcUiShowMenu(HWND hWnd, int x, int y)
{
	HMENU hMenu = LoadMenu(THIS_HINSTANCE, MAKEINTRESOURCE(IDR_MENU));
	if (!hMenu)
		return;

	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	SetMenuDefaultItem(hSubMenu, IDC_SETTINGS, FALSE);

	SetForegroundWindow(hWnd);
	TrackPopupMenu(hSubMenu, 0, x, y, 0, hWnd, NULL);
	DestroyMenu(hMenu);
}
