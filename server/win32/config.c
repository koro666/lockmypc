#include <windows.h>
#include <objbase.h>
#include <shlobj.h>
#include <shlwapi.h>
#include "config.h"
#include "utility.h"
#include "rtl.h"

struct _LMPC_CONFIG
{
	LPTSTR Path;
	LPTSTR Address;
	LPTSTR Port;
	LPTSTR Secret;
};

struct _LMPC_FIELD
{
	LPCTSTR Key;
	SIZE_T Offset;
	LPCTSTR Default;
};

typedef struct _LMPC_FIELD LMPC_FIELD;
typedef const LMPC_FIELD* PCLMPC_FIELD;

static const TCHAR LmpcCfgName[] = TEXT("LockMyPC");
static const TCHAR LmpcCfgFile[] = TEXT("lmpc.ini");

static const LMPC_FIELD LmpcCfgFields[] =
{
	{ TEXT("Address"), FIELD_OFFSET(LMPC_CONFIG, Address), TEXT("") },
	{ TEXT("Port"), FIELD_OFFSET(LMPC_CONFIG, Port), TEXT("43666") },
	{ TEXT("Secret"), FIELD_OFFSET(LMPC_CONFIG, Secret), TEXT("default") }
};

#define LMPC_GET_FIELD(config, field) (((LPTSTR*)(((LPBYTE)(config)) + (field)->Offset)))

HRESULT LmpcCfgGetPath(LPTSTR* path)
{
	if (!path)
		return E_POINTER;

	TCHAR buffer[MAX_PATH];
	*path = NULL;

	if (!GetModuleFileName(NULL, buffer, MAX_PATH))
		return HRESULT_FROM_WIN32(GetLastError());

	if (!PathRemoveFileSpec(buffer))
		return E_FAIL;

	if (!PathAppend(buffer, LmpcCfgFile))
		return E_FAIL;

	if (GetFileAttributes(buffer) != INVALID_FILE_ATTRIBUTES)
		return CoStrDup(buffer, path);

	HRESULT hr = SHGetFolderPath(
		NULL,
		CSIDL_LOCAL_APPDATA,
		NULL,
		SHGFP_TYPE_CURRENT,
		buffer);

	if (FAILED(hr))
		return hr;

	if (!PathAppend(buffer, LmpcCfgName))
		return E_FAIL;

	if (!CreateDirectory(buffer, NULL))
	{
		DWORD error = GetLastError();
		if (error != ERROR_ALREADY_EXISTS)
			return HRESULT_FROM_WIN32(error);
	}

	if (!PathAppend(buffer, LmpcCfgFile))
		return E_FAIL;

	return CoStrDup(buffer, path);
}

HRESULT LmpcCfgLoad(HLMPC_CONFIG* config)
{
	if (!config)
		return E_POINTER;

	*config = NULL;

	LPTSTR path;
	HRESULT hr = LmpcCfgGetPath(&path);
	if (FAILED(hr))
		return hr;

	hr = LmpcCfgLoadFromPath(path, config);
	CoTaskMemFree(path);
	return hr;
}

HRESULT LmpcCfgLoadFromPath(LPCTSTR path, HLMPC_CONFIG* config)
{
	if (!config)
		return E_POINTER;

	*config = NULL;
	if (!path)
		return E_INVALIDARG;

	HLMPC_CONFIG result = CoTaskMemAlloc(sizeof(LMPC_CONFIG));
	if (!result)
		return E_OUTOFMEMORY;

	RtlZeroMemory(result, sizeof(LMPC_CONFIG));

	HRESULT hr = CoStrDup(path, &result->Path);
	if (FAILED(hr))
	{
		CoTaskMemFree(result);
		return hr;
	}

	for (DWORD i = 0; i < ARRAYSIZE(LmpcCfgFields); ++i)
	{
		hr = LmpcCfgFieldRead(result, i);
		if (FAILED(hr))
		{
			LmpcCfgDestroy(result);
			return hr;
		}
	}

	*config = result;
	return S_OK;
}

HRESULT LmpcCfgSave(HLMPC_CONFIG config)
{
	if (!config)
		return E_INVALIDARG;

	for (DWORD i = 0; i < ARRAYSIZE(LmpcCfgFields); ++i)
	{
		HRESULT hr = LmpcCfgFieldWrite(config, i);
		if (FAILED(hr))
			return hr;
	}

	return S_OK;
}

HRESULT LmpcCfgFieldRead(HLMPC_CONFIG config, DWORD fid)
{
	if (!config)
		return E_INVALIDARG;
	if (fid >= ARRAYSIZE(LmpcCfgFields))
		return E_INVALIDARG;

	PCLMPC_FIELD field = LmpcCfgFields + fid;
	LPTSTR* storage = LMPC_GET_FIELD(config, field);

	TCHAR buffer[1024];

	SetLastError(ERROR_SUCCESS);
	if (!GetPrivateProfileString(LmpcCfgName, field->Key, field->Default, buffer, ARRAYSIZE(buffer), config->Path))
	{
		DWORD error = GetLastError();
		if (error != ERROR_SUCCESS && error != ERROR_FILE_NOT_FOUND)
			return HRESULT_FROM_WIN32(error);
	}

	LPTSTR old = *storage;
	*storage = NULL;

	HRESULT hr = CoStrDup(buffer, storage);
	if (FAILED(hr))
	{
		*storage = old;
		return hr;
	}

	CoTaskMemFree(old);
	return S_OK;
}

HRESULT LmpcCfgFieldWrite(HLMPC_CONFIG config, DWORD fid)
{
	if (!config)
		return E_INVALIDARG;
	if (fid >= ARRAYSIZE(LmpcCfgFields))
		return E_INVALIDARG;

	PCLMPC_FIELD field = LmpcCfgFields + fid;
	LPTSTR* storage = LMPC_GET_FIELD(config, field);

#if UNICODE
	HANDLE hFile = CreateFile(config->Path, GENERIC_WRITE, 0, NULL,CREATE_NEW, 0, NULL);
	if (hFile != INVALID_HANDLE_VALUE)
	{
		static const BYTE bom[2] = { 0xff, 0xfe };

		DWORD written;
		WriteFile(hFile, bom, sizeof(bom), &written, NULL);

		CloseHandle(hFile);
	}
#endif

	if (!WritePrivateProfileString(LmpcCfgName, field->Key, *storage, config->Path))
		return HRESULT_FROM_WIN32(GetLastError());

	return S_OK;
}

HRESULT LmpcCfgFieldGet(HLMPC_CONFIG config, DWORD fid, LPTSTR* value)
{
	if (!value)
		return E_POINTER;

	*value = NULL;

	if (!config)
		return E_INVALIDARG;
	if (fid >= ARRAYSIZE(LmpcCfgFields))
		return E_INVALIDARG;

	PCLMPC_FIELD field = LmpcCfgFields + fid;
	LPTSTR* storage = LMPC_GET_FIELD(config, field);

	return CoStrDup(*storage, value);
}

HRESULT LmpcCfgFieldToWindow(HLMPC_CONFIG config, DWORD fid, HWND hWnd)
{
	if (!config)
		return E_INVALIDARG;
	if (fid >= ARRAYSIZE(LmpcCfgFields))
		return E_INVALIDARG;
	if (!hWnd)
		return E_INVALIDARG;

	PCLMPC_FIELD field = LmpcCfgFields + fid;
	LPTSTR* storage = LMPC_GET_FIELD(config, field);

	if (!SetWindowText(hWnd, *storage))
		return HRESULT_FROM_WIN32(GetLastError());

	return S_OK;
}

HRESULT LmpcCfgFieldFromWindow(HLMPC_CONFIG config, DWORD fid, HWND hWnd)
{
	if (!config)
		return E_INVALIDARG;
	if (fid >= ARRAYSIZE(LmpcCfgFields))
		return E_INVALIDARG;
	if (!hWnd)
		return E_INVALIDARG;

	PCLMPC_FIELD field = LmpcCfgFields + fid;
	LPTSTR* storage = LMPC_GET_FIELD(config, field);

	SetLastError(ERROR_SUCCESS);
	LRESULT count = SendMessage(hWnd, WM_GETTEXTLENGTH, 0, 0);
	if (!count)
	{
		DWORD error = GetLastError();
		if (error != ERROR_SUCCESS)
			return HRESULT_FROM_WIN32(error);
	}

	SIZE_T size = (((SIZE_T)count) + 1) * sizeof(TCHAR);
	LPTSTR buffer = CoTaskMemAlloc(size);
	if (!buffer)
		return E_OUTOFMEMORY;

	SetLastError(ERROR_SUCCESS);
	count = SendMessage(hWnd, WM_GETTEXT, count + 1, (LPARAM)buffer);
	if (!count)
	{
		DWORD error = GetLastError();
		if (error != ERROR_SUCCESS)
		{
			CoTaskMemFree(buffer);
			return HRESULT_FROM_WIN32(error);
		}
	}

	buffer[count] = 0;
	CoTaskMemFree(*storage);
	*storage = buffer;
	return S_OK;
}

HRESULT LmpcCfgDestroy(HLMPC_CONFIG config)
{
	if (!config)
		return S_FALSE;

	CoTaskMemFree(config->Path);

	for (DWORD i = 0; i < ARRAYSIZE(LmpcCfgFields); ++i)
	{
		LPTSTR* storage = LMPC_GET_FIELD(config, LmpcCfgFields + i);
		CoTaskMemFree(*storage);
	}

	CoTaskMemFree(config);
	return S_OK;
}
