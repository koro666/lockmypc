#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "config.h"
#include "rtl.h"

static const TCHAR LmpcCfgName[] = TEXT("LockMyPC");
static const TCHAR LmpcCfgFile[] = TEXT("lmpc.ini");

static const TCHAR LmpcCfgKeyAddress[] = TEXT("Address");
static const TCHAR LmpcCfgKeyPort[] = TEXT("Port");
static const TCHAR LmpcCfgKeySecret[] = TEXT("Secret");

static const TCHAR LmpcCfgDefaultAddress[] = TEXT("");
static const TCHAR LmpcCfgDefaultPort[] = TEXT("43666");
static const TCHAR LmpcCfgDefaultSecret[] = TEXT("default");

static TCHAR LmpcCfgPath[MAX_PATH];

LMPC_CONFIG LmpcCfgCurrent;

HRESULT LmpcCfgInitialize(void)
{
	if (!GetModuleFileName(NULL, LmpcCfgPath, MAX_PATH))
		return HRESULT_FROM_WIN32(GetLastError());

	if (!PathRemoveFileSpec(LmpcCfgPath))
		return E_FAIL;

	if (!PathAppend(LmpcCfgPath, LmpcCfgFile))
		return E_FAIL;

	if (GetFileAttributes(LmpcCfgPath) != INVALID_FILE_ATTRIBUTES)
		return S_OK;

	HRESULT hr = SHGetFolderPath(
		NULL,
		CSIDL_LOCAL_APPDATA,
		NULL,
		SHGFP_TYPE_CURRENT,
		LmpcCfgPath);

	if (FAILED(hr))
		return hr;

	if (!PathAppend(LmpcCfgPath, LmpcCfgName))
		return E_FAIL;

	if (!CreateDirectory(LmpcCfgPath, NULL))
	{
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	if (!PathAppend(LmpcCfgPath, LmpcCfgFile))
		return E_FAIL;

	hr = LmpcCfgLoad();
	if (FAILED(hr))
	{
		LmpcCfgFinalize();
		return hr;
	}

	return S_OK;
}

HRESULT LmpcCfgFinalize(void)
{
	LmpcCfgStringFree(&LmpcCfgCurrent.Secret);
	LmpcCfgStringFree(&LmpcCfgCurrent.Port);
	LmpcCfgStringFree(&LmpcCfgCurrent.Address);

	return S_OK;
}

HRESULT LmpcCfgLoad(void)
{
	HRESULT hr = LmpcCfgStringRead(LmpcCfgKeyAddress, &LmpcCfgCurrent.Address, LmpcCfgDefaultAddress);
	if (FAILED(hr))
		return hr;

	hr = LmpcCfgStringRead(LmpcCfgKeyPort, &LmpcCfgCurrent.Port, LmpcCfgDefaultPort);
	if (FAILED(hr))
		return hr;

	hr = LmpcCfgStringRead(LmpcCfgKeySecret, &LmpcCfgCurrent.Secret, LmpcCfgDefaultSecret);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT LmpcCfgSave(void)
{
	HRESULT hr = LmpcCfgStringWrite(LmpcCfgKeyAddress, LmpcCfgCurrent.Address);
	if (FAILED(hr))
		return hr;

	hr = LmpcCfgStringWrite(LmpcCfgKeyPort, LmpcCfgCurrent.Port);
	if (FAILED(hr))
		return hr;

	hr = LmpcCfgStringWrite(LmpcCfgKeySecret, LmpcCfgCurrent.Secret);
	if (FAILED(hr))
		return hr;

	return S_OK;
}

HRESULT LmpcCfgStringRead(LPCTSTR key, LPWSTR* storage, LPCTSTR fallback)
{
	TCHAR buffer[1024];
	DWORD count = GetPrivateProfileString(LmpcCfgName, key, fallback, buffer, ARRAYSIZE(buffer), LmpcCfgPath);
	SIZE_T size = (((SIZE_T)count) + 1) * sizeof(TCHAR);

	*storage = CoTaskMemRealloc(*storage, size);
	RtlMoveMemory(*storage, buffer, size);

	return S_OK;
}

HRESULT LmpcCfgStringWrite(LPCTSTR key, LPCTSTR value)
{
#if UNICODE
	HANDLE hFile = CreateFile(LmpcCfgPath, GENERIC_WRITE, 0, NULL,CREATE_NEW, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		static const BYTE bom[2] = { 0xff, 0xfe };

		DWORD written;
		WriteFile(hFile, bom, sizeof(bom), &written, NULL);

		CloseHandle(hFile);
	}
#endif

	if (!WritePrivateProfileString(LmpcCfgName, key, value, LmpcCfgPath))
		return HRESULT_FROM_WIN32(GetLastError());

	return S_OK;
}

HRESULT LmpcCfgStringSet(HWND hDlg, int item, LPTSTR* storage)
{
	HWND hCtl = GetDlgItem(hDlg, item);
	if (!hCtl)
		return HRESULT_FROM_WIN32(GetLastError());

	LRESULT count = SendMessage(hCtl, WM_GETTEXTLENGTH, 0, 0);
	SIZE_T size = (((SIZE_T)count) + 1) * sizeof(TCHAR);

	*storage = CoTaskMemRealloc(*storage, size);
	SendMessage(hCtl, WM_GETTEXT, count + 1, (LPARAM)*storage);

	return S_OK;
}

void LmpcCfgStringFree(LPWSTR* s)
{
	CoTaskMemFree(*s);
	*s = NULL;
}
