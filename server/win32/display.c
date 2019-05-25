#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include <objbase.h>
#include <shellapi.h>
#include <tchar.h>
#include "display.h"
#include "resource.h"
#include "config.h"
#include "server.h"
#include "rtl.h"

extern IMAGE_DOS_HEADER __ImageBase;
#define THIS_HINSTANCE ((HINSTANCE)&__ImageBase)

struct _LMPC_UI
{
	HLMPC_CONFIG Config;
	HICON Icon;
	ATOM Atom;
	UINT TaskbarMessage;
	HWND Window;
	HWND Dialog;
};

static const TCHAR LmpcUiClass[] = TEXT("LockMyPC_WndClass");

HRESULT LmpcUiCreate(HLMPC_CONFIG config, HLMPC_UI* ui)
{
	if (!ui)
		return E_POINTER;

	*ui = NULL;
	if (!config)
		return E_INVALIDARG;

	HRESULT hr = S_OK;

	HLMPC_UI result = CoTaskMemAlloc(sizeof(LMPC_UI));
	if (!result)
		return E_OUTOFMEMORY;
	RtlZeroMemory(result, sizeof(LMPC_UI));

	result->Config = config;

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
		.lpfnWndProc = LmpcUiWndProcStatic,
		.cbWndExtra = sizeof(HLMPC_UI),
		.hInstance = THIS_HINSTANCE,
		.lpszClassName = LmpcUiClass
	};

	result->Atom = RegisterClassEx(&wcx);
	if (!result->Atom)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	result->TaskbarMessage = RegisterWindowMessage(TEXT("TaskbarCreated"));
	if (!result->TaskbarMessage)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	hr = LoadIconMetric(
		THIS_HINSTANCE,
		MAKEINTRESOURCEW(IDI_MAIN),
		LIM_SMALL,
		&result->Icon);

	if (FAILED(hr))
		goto leave;
	else
		hr = S_OK;

	HWND hWnd = CreateWindowEx(
		0,
		MAKEINTATOM(result->Atom),
		NULL,
		WS_POPUP,
		0, 0, 0, 0,
		HWND_MESSAGE,
		NULL,
		THIS_HINSTANCE,
		result);

	if (!hWnd)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

leave:
	if (FAILED(hr))
	{
		LmpcUiDestroy(result);
		result = NULL;
	}

	*ui = result;
	return hr;
}

HRESULT LmpcUiDestroy(HLMPC_UI ui)
{
	if (!ui)
		return S_FALSE;

	if (ui->Window)
		DestroyWindow(ui->Window);

	if (ui->Icon)
		DestroyIcon(ui->Icon);

	if (ui->Atom)
		UnregisterClass(MAKEINTATOM(ui->Atom), (HINSTANCE)& __ImageBase);

	CoTaskMemFree(ui);
	return S_OK;
}

HRESULT LmpcUiRunLoop(HLMPC_UI ui)
{
	if (!ui)
		return E_INVALIDARG;

	MSG m;
	BOOL b;

	while ((b = GetMessage(&m, NULL, 0, 0)))
	{
		if (b == -1)
			return HRESULT_FROM_WIN32(GetLastError());

		if (ui->Dialog && IsDialogMessage(ui->Dialog, &m))
			continue;

		TranslateMessage(&m);
		DispatchMessage(&m);
	}

	return S_OK;
}

LRESULT CALLBACK LmpcUiWndProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HLMPC_UI ui;
	if (uMsg == WM_NCCREATE)
	{
		ui = ((LPCREATESTRUCT)lParam)->lpCreateParams;
		SetWindowLongPtr(hWnd, 0, (LONG_PTR)ui);
		ui->Window = hWnd;
	}
	else
	{
		ui = (HLMPC_UI)GetWindowLongPtr(hWnd, 0);
	}

	LRESULT result;
	if (ui)
		result = LmpcUiWndProc(ui, uMsg, wParam, lParam);
	else
		result = DefWindowProc(hWnd, uMsg, wParam, lParam);

	if (uMsg == WM_NCDESTROY)
	{
		if (ui)
			ui->Window = NULL;
		SetWindowLongPtr(hWnd, 0, 0);
	}

	return result;
}

LRESULT LmpcUiWndProc(HLMPC_UI ui, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_CREATE:
			LmpcSrvStart(ui->Window, WM_USER+1);
			LmpcUiCreateNotifyIcon(ui);
			return 0;
		case WM_DESTROY:
			LmpcUiRemoveNotifyIcon(ui);
			LmpcSrvStop();
			return 0;
		case WM_CLOSE:
		case WM_ENDSESSION:
			PostQuitMessage(0);
			return 0;
		case WM_COMMAND:
			return LmpcUiHandleCommand(ui, wParam, lParam);
		case WM_USER:
			return LmpcUiHandleNotifyMessage(ui, wParam, lParam);
		case WM_USER+1:
			return LmpcSrvHandleSelect(wParam, lParam);
		default:
			if (uMsg == ui->TaskbarMessage)
			{
				LmpcUiCreateNotifyIcon(ui);
				return 0;
			}

			return DefWindowProc(ui->Window, uMsg, wParam, lParam);
	}
}

LRESULT LmpcUiHandleCommand(HLMPC_UI ui, WPARAM wParam, LPARAM lParam)
{
	switch (LOWORD(wParam))
	{
		case IDC_SETTINGS:
			if (ui->Dialog)
			{
				SetForegroundWindow(ui->Dialog);
			}
			else
			{
				HWND hDlg = CreateDialogParam(THIS_HINSTANCE, MAKEINTRESOURCE(IDD_SETTINGS), NULL, LmpcUiDlgProcStatic, (LPARAM)ui);
				if (hDlg)
					ShowWindow(hDlg, SW_SHOW);
			}
			break;
		case IDC_EXIT:
			if (ui->Dialog)
				SendMessage(ui->Dialog, WM_CLOSE, 0, 0);
			SendMessage(ui->Window, WM_CLOSE, 0, 0);
			break;
	}

	UNREFERENCED_PARAMETER(lParam);
	return 0;
}

LRESULT LmpcUiHandleNotifyMessage(HLMPC_UI ui, WPARAM wParam, LPARAM lParam)
{
	switch (lParam)
	{
		case MAKELPARAM(WM_CONTEXTMENU, 0):
			LmpcUiShowMenu(ui, GET_X_LPARAM(wParam), GET_Y_LPARAM(wParam));
			break;
		case MAKELPARAM(WM_LBUTTONDBLCLK, 0):
		case MAKELPARAM(NIN_KEYSELECT, 0):
			SendMessage(ui->Window, WM_COMMAND, MAKEWPARAM(IDC_SETTINGS, 0), 0);
			break;
	}

	return 0;
}

INT_PTR CALLBACK LmpcUiDlgProcStatic(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	HLMPC_UI ui;
	if (uMsg == WM_INITDIALOG)
	{
		ui = (HLMPC_UI)lParam;
		SetWindowLongPtr(hWnd, DWLP_USER, (LONG_PTR)ui);
		ui->Dialog = hWnd;
	}
	else
	{
		ui = (HLMPC_UI)GetWindowLongPtr(hWnd, DWLP_USER);
	}

	if (ui)
		return LmpcUiDlgProc(ui, uMsg, wParam, lParam);
	else
		return FALSE;
}

INT_PTR LmpcUiDlgProc(HLMPC_UI ui, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	switch (uMsg)
	{
		case WM_INITDIALOG:
		{
			WCHAR buffer[256];

			SendDlgItemMessage(ui->Dialog, IDC_SETTINGS_HOST, EM_LIMITTEXT, 256, 0);
			if (LoadStringW(THIS_HINSTANCE, IDS_HOST_DEFAULT, buffer, ARRAYSIZE(buffer)))
				SendDlgItemMessage(ui->Dialog, IDC_SETTINGS_HOST, EM_SETCUEBANNER, TRUE, (LPARAM)buffer);
			LmpcCfgFieldToControl(ui->Config, LMPC_CFG_ADDRESS, ui->Dialog, IDC_SETTINGS_HOST);

			SendDlgItemMessage(ui->Dialog, IDC_SETTINGS_PORT, EM_LIMITTEXT, 5, 0);
			LmpcCfgFieldToControl(ui->Config, LMPC_CFG_PORT, ui->Dialog, IDC_SETTINGS_PORT);

			SendDlgItemMessage(ui->Dialog, IDC_SETTINGS_PORT_SPIN, UDM_SETRANGE32, 1, 65535);

			LmpcCfgFieldToControl(ui->Config, LMPC_CFG_SECRET, ui->Dialog,IDC_SETTINGS_SECRET);
			return TRUE;
		}
		case WM_COMMAND:
		{
			switch (wParam)
			{
				case MAKEWPARAM(IDOK, BN_CLICKED):
				{
					LmpcCfgFieldFromControl(ui->Config, LMPC_CFG_ADDRESS, ui->Dialog, IDC_SETTINGS_HOST);
					LmpcCfgFieldFromControl(ui->Config, LMPC_CFG_PORT, ui->Dialog, IDC_SETTINGS_PORT);
					LmpcCfgFieldFromControl(ui->Config, LMPC_CFG_SECRET, ui->Dialog, IDC_SETTINGS_SECRET);
					LmpcCfgSave(ui->Config);
					LmpcSrvStop();
					LmpcSrvStart(ui->Window, WM_USER+1);
					// fallthrough
				}
				case MAKEWPARAM(IDCANCEL, BN_CLICKED):
				{
					DestroyWindow(ui->Dialog);
					ui->Dialog = NULL;
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
					.hMonitor = MonitorFromWindow(ui->Dialog, MONITOR_DEFAULTTONEAREST)
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

void LmpcUiCreateNotifyIcon(HLMPC_UI ui)
{
	{
		NOTIFYICONDATA nid =
		{
			.cbSize = sizeof(NOTIFYICONDATA),
			.hWnd = ui->Window,
			.uID = 0,
			.uFlags = NIF_MESSAGE | NIF_ICON | NIF_TIP | NIF_SHOWTIP,
			.uCallbackMessage = WM_USER,
			.hIcon = ui->Icon
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
			.hWnd = ui->Window,
			.uID = 0,
			.uVersion = NOTIFYICON_VERSION_4
		};

		Shell_NotifyIcon(NIM_SETVERSION, &nid);
	}
}

void LmpcUiRemoveNotifyIcon(HLMPC_UI ui)
{
	NOTIFYICONDATA nid =
	{
		.cbSize = sizeof(NOTIFYICONDATA),
		.hWnd = ui->Window,
		.uID = 0
	};

	Shell_NotifyIcon(NIM_DELETE, &nid);
}

void LmpcUiShowMenu(HLMPC_UI ui, int x, int y)
{
	HMENU hMenu = LoadMenu(THIS_HINSTANCE, MAKEINTRESOURCE(IDR_MENU));
	if (!hMenu)
		return;

	HMENU hSubMenu = GetSubMenu(hMenu, 0);
	SetMenuDefaultItem(hSubMenu, IDC_SETTINGS, FALSE);

	SetForegroundWindow(ui->Window);
	TrackPopupMenu(hSubMenu, 0, x, y, 0, ui->Window, NULL);
	DestroyMenu(hMenu);
}
