#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <objbase.h>
#include "server.h"
#include "config.h"
#include "rtl.h"

static BOOL LmpcSrvWsaInitialized;
static HCRYPTPROV LmpcSrvCryptProvider;
static SOCKET LmpcSrvSocket;
static LPSTR LmpcSrvSecret;

HRESULT LmpcSrvInitialize(void)
{
	HRESULT hr = S_OK;

	WSADATA wd;
	int error = WSAStartup(MAKEWORD(2, 2), &wd);
	if (error)
	{
		hr = HRESULT_FROM_WIN32(error);
		goto exit;
	}

	LmpcSrvWsaInitialized = TRUE;

	if (!CryptAcquireContext(&LmpcSrvCryptProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		LmpcSrvCryptProvider = 0;
		goto exit;
	}

exit:
	if (FAILED(hr))
		LmpcSrvFinalize();
	return hr;
}

HRESULT LmpcSrvFinalize(void)
{
	if (LmpcSrvCryptProvider)
	{
		CryptReleaseContext(LmpcSrvCryptProvider, 0);
		LmpcSrvCryptProvider = 0;
	}

	if (LmpcSrvWsaInitialized)
	{
		WSACleanup();
		LmpcSrvWsaInitialized = FALSE;
	}

	return S_OK;
}

HRESULT LmpcSrvStart(HWND hWnd, UINT uMsg)
{
	if (!hWnd || !uMsg)
		return E_INVALIDARG;

	if (LmpcSrvSocket)
		return S_FALSE;

	static const ADDRINFOT hints =
	{
		.ai_flags = AI_NUMERICHOST | AI_PASSIVE | AI_ADDRCONFIG,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM
	};

	HRESULT hr = S_OK;
	LPWSTR wsecret = NULL;
	ADDRINFOT * addr = NULL;
	int result;

#if UNICODE
	wsecret = LmpcCfgCurrent.Secret;
#else
	result = MultiByteToWideChar(CP_ACP, 0, LmpcCfgCurrent.Secret, -1, NULL, 0);
	if (!result)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	wsecret = CoTaskMemAlloc(result * sizeof(WCHAR));
	if (!wsecret)
	{
		hr = E_OUTOFMEMORY;
		goto leave;
	}

	result = MultiByteToWideChar(CP_ACP, 0, LmpcCfgCurrent.Secret, -1, wsecret, result);
	if (!result)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}
#endif

	result = WideCharToMultiByte(CP_UTF8, 0, wsecret, -1, NULL, 0, NULL, NULL);
	if (!result)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	LmpcSrvSecret = CoTaskMemAlloc(result * sizeof(CHAR));
	if (!LmpcSrvSecret)
	{
		hr = E_OUTOFMEMORY;
		goto leave;
	}

	result = WideCharToMultiByte(CP_UTF8, 0, wsecret, -1, LmpcSrvSecret, result, NULL, NULL);
	if (!result)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	result = GetAddrInfo(*LmpcCfgCurrent.Address ? LmpcCfgCurrent.Address : NULL, LmpcCfgCurrent.Port, &hints, &addr);
	if (result)
	{
		hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, result);
		goto leave;
	}

	LmpcSrvSocket = WSASocket(addr->ai_family, addr->ai_socktype, addr->ai_protocol, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (!LmpcSrvSocket)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

	result = bind(LmpcSrvSocket, addr->ai_addr, (int)addr->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

	result = WSAAsyncSelect(LmpcSrvSocket, hWnd, uMsg, FD_READ);
	if (result == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

leave:
	if (FAILED(hr))
		LmpcSrvStop();
	if (addr)
		FreeAddrInfo(addr);
#if !UNICODE
	if (wsecret)
		CoTaskMemFree(wsecret);
#endif
	return hr;
}

HRESULT LmpcSrvStop(void)
{
	if (LmpcSrvSocket)
	{
		closesocket(LmpcSrvSocket);
		LmpcSrvSocket = 0;
	}

	if (LmpcSrvSecret)
	{
		CoTaskMemFree(LmpcSrvSecret);
		LmpcSrvSecret = NULL;
	}

	return S_OK;
}

LRESULT LmpcSrvHandleSelect(WPARAM wParam, LPARAM lParam)
{
	if (!LmpcSrvSocket)
		return 1;
	if (LmpcSrvSocket != (SOCKET)wParam)
		return 1;
	if (WSAGETSELECTERROR(lParam))
		return 1;
	if (WSAGETSELECTEVENT(lParam) != FD_READ)
		return 0;

	LMPC_PACKET packet;

	int length = recv(LmpcSrvSocket, (char*)&packet, sizeof(packet), 0);
	if (length == SOCKET_ERROR)
		return 1;

	return FAILED(LmpcSrvHandlePacket(&packet, length));
}

HRESULT LmpcSrvHandlePacket(const LMPC_PACKET* packet, SIZE_T length)
{
	HRESULT hr = LmpcSrvCheckPacket(packet, length);
	if (SUCCEEDED(hr))
		LockWorkStation();
	return hr;
}

HRESULT LmpcSrvCheckPacket(const LMPC_PACKET* packet, SIZE_T length)
{
	if (length != sizeof(LMPC_PACKET))
		return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

	if (ntohl(packet->Signature) != 0x4C4F434B)
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	ULONG now = LmpcSrvGetUnixTime();
	ULONG then = ntohl(packet->Time);
	LONG diff = (LONG)(then - now);

	if (diff <= -60 || diff >= 60)
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	HRESULT hr = S_OK;
	HCRYPTHASH hash;
	if (!CryptCreateHash(LmpcSrvCryptProvider, CALG_SHA_256, 0, 0, &hash))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		hash = 0;
		goto leave;
	}

	if (!CryptHashData(hash, (LPBYTE)&packet->Time, sizeof(packet->Time), 0))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	if (!CryptHashData(hash, (LPBYTE)LmpcSrvSecret, lstrlenA(LmpcSrvSecret), 0))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	BYTE md[32];
	DWORD size = sizeof(md);
	if (!CryptGetHashParam(hash, HP_HASHVAL, md, &size, 0))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	if (size != sizeof(md))
	{
		hr = E_FAIL;
		goto leave;
	}

	if (RtlCompareMemory(packet->Hash, md, sizeof(packet->Hash)) != sizeof(packet->Hash))
	{
		hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATA);
		goto leave;
	}

leave:
	if (hash)
		CryptDestroyHash(hash);
	return hr;
}

ULONG LmpcSrvGetUnixTime(void)
{
	union
	{
		FILETIME ft;
		LARGE_INTEGER li;
	} u;

	GetSystemTimeAsFileTime(&u.ft);

	ULONG result;
	if (RtlTimeToSecondsSince1970(&u.li, &result))
		return result;
	else
		return 0xFFFFFFFFu;
}
