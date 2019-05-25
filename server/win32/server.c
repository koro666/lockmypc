#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <wincrypt.h>
#include <objbase.h>
#include "server.h"
#include "config.h"
#include "utility.h"
#include "rtl.h"

struct _LMPC_SERVER
{
	HLMPC_CONFIG Config;

	BOOL WsaInitialized;
	HCRYPTPROV CryptProvider;

	HWND Window;
	UINT Message;

	SOCKET Socket;
	LPSTR Secret;
};

HRESULT LmpcSrvCreate(HLMPC_CONFIG config, HLMPC_SERVER* server)
{
	if (!server)
		return E_POINTER;

	*server = NULL;
	if (!config)
		return E_INVALIDARG;

	HLMPC_SERVER result = CoTaskMemAlloc(sizeof(LMPC_SERVER));
	if (!result)
		return E_OUTOFMEMORY;
	RtlZeroMemory(result, sizeof(LMPC_SERVER));

	HRESULT hr = S_OK;
	result->Config = config;

	WSADATA wd;
	int error = WSAStartup(MAKEWORD(2, 2), &wd);
	if (error)
	{
		hr = HRESULT_FROM_WIN32(error);
		goto leave;
	}

	result->WsaInitialized = TRUE;

	if (!CryptAcquireContext(&result->CryptProvider, NULL, NULL, PROV_RSA_AES, CRYPT_VERIFYCONTEXT))
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		result->CryptProvider = 0;
		goto leave;
	}

leave:
	if (FAILED(hr))
	{
		LmpcSrvDestroy(result);
		result = NULL;
	}

	*server = result;
	return hr;
}

HRESULT LmpcSrvDestroy(HLMPC_SERVER server)
{
	if (!server)
		return S_FALSE;

	if (server->CryptProvider)
		CryptReleaseContext(server->CryptProvider, 0);

	if (server->WsaInitialized)
		WSACleanup();

	CoTaskMemFree(server);
	return S_OK;
}

HRESULT LmpcSrvSetCallback(HLMPC_SERVER server, HWND hWnd, UINT uMsg)
{
	if (!server)
		return E_INVALIDARG;

	server->Window = hWnd;
	server->Message = uMsg;

	return S_OK;
}

HRESULT LmpcSrvStart(HLMPC_SERVER server)
{
	if (!server)
		return E_INVALIDARG;

	if (!(server->Window && server->Message))
		return E_FAIL;

	if (server->Socket)
		return S_FALSE;

	static const ADDRINFOT hints =
	{
		.ai_flags = AI_NUMERICHOST | AI_PASSIVE | AI_ADDRCONFIG,
		.ai_family = AF_UNSPEC,
		.ai_socktype = SOCK_DGRAM
	};

	HRESULT hr = S_OK;
	LPTSTR secret = NULL, host = NULL, port = NULL;
	LPWSTR wsecret = NULL;
	ADDRINFOT * addr = NULL;
	int result;

	hr = LmpcCfgFieldGet(server->Config, LMPC_CFG_SECRET, &secret);
	if (FAILED(hr))
		goto leave;
	hr = S_OK;

#if UNICODE
	wsecret = secret;
#else
	result = MultiByteToWideChar(CP_ACP, 0, secret, -1, NULL, 0);
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

	result = MultiByteToWideChar(CP_ACP, 0, secret, -1, wsecret, result);
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

	server->Secret = CoTaskMemAlloc(result * sizeof(CHAR));
	if (!server->Secret)
	{
		hr = E_OUTOFMEMORY;
		goto leave;
	}

	result = WideCharToMultiByte(CP_UTF8, 0, wsecret, -1, server->Secret, result, NULL, NULL);
	if (!result)
	{
		hr = HRESULT_FROM_WIN32(GetLastError());
		goto leave;
	}

	hr = LmpcCfgFieldGet(server->Config, LMPC_CFG_ADDRESS, &host);
	if (FAILED(hr))
		goto leave;
	hr = S_OK;

	hr = LmpcCfgFieldGet(server->Config, LMPC_CFG_PORT, &port);
	if (FAILED(hr))
		goto leave;
	hr = S_OK;

	result = GetAddrInfo(*host ? host : NULL, port, &hints, &addr);
	if (result)
	{
		hr = MAKE_HRESULT(SEVERITY_ERROR, FACILITY_ITF, result);
		goto leave;
	}

	server->Socket = WSASocket(addr->ai_family, addr->ai_socktype, addr->ai_protocol, NULL, 0, WSA_FLAG_NO_HANDLE_INHERIT);
	if (!server->Socket)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

	result = bind(server->Socket, addr->ai_addr, (int)addr->ai_addrlen);
	if (result == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

	result = WSAAsyncSelect(server->Socket, server->Window, server->Message, FD_READ);
	if (result == SOCKET_ERROR)
	{
		hr = HRESULT_FROM_WIN32(WSAGetLastError());
		goto leave;
	}

leave:
	if (FAILED(hr))
		LmpcSrvStop(server);
	if (addr)
		FreeAddrInfo(addr);
#if !UNICODE
	CoTaskMemFree(wsecret);
#endif
	CoTaskMemFree(port);
	CoTaskMemFree(host);
	CoTaskMemFree(secret);
	return hr;
}

HRESULT LmpcSrvStop(HLMPC_SERVER server)
{
	if (!server)
		return E_INVALIDARG;

	if (server->Socket)
	{
		closesocket(server->Socket);
		server->Socket = 0;
	}

	if (server->Secret)
	{
		CoTaskMemFree(server->Secret);
		server->Secret = NULL;
	}

	return S_OK;
}

LRESULT LmpcSrvHandleSelect(HLMPC_SERVER server, WPARAM wParam, LPARAM lParam)
{
	if (!server->Socket)
		return 1;
	if (server->Socket != (SOCKET)wParam)
		return 1;
	if (WSAGETSELECTERROR(lParam))
		return 1;
	if (WSAGETSELECTEVENT(lParam) != FD_READ)
		return 0;

	LMPC_PACKET packet;

	int length = recv(server->Socket, (char*)&packet, sizeof(packet), 0);
	if (length == SOCKET_ERROR)
		return 1;

	return FAILED(LmpcSrvHandlePacket(server, &packet, length));
}

HRESULT LmpcSrvHandlePacket(HLMPC_SERVER server, PCLMPC_PACKET packet, SIZE_T length)
{
	HRESULT hr = LmpcSrvCheckPacket(server, packet, length);
	if (SUCCEEDED(hr))
		LockWorkStation();
	return hr;
}

HRESULT LmpcSrvCheckPacket(HLMPC_SERVER server, PCLMPC_PACKET packet, SIZE_T length)
{
	if (length != sizeof(LMPC_PACKET))
		return HRESULT_FROM_WIN32(ERROR_INVALID_PARAMETER);

	if (ntohl(packet->Signature) != 0x4C4F434B)
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	ULONG now = GetSystemTimeAsUnixTime();
	ULONG then = ntohl(packet->Time);
	LONG diff = (LONG)(then - now);

	if (diff <= -60 || diff >= 60)
		return HRESULT_FROM_WIN32(ERROR_INVALID_DATA);

	HRESULT hr = S_OK;
	HCRYPTHASH hash;
	if (!CryptCreateHash(server->CryptProvider, CALG_SHA_256, 0, 0, &hash))
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

	if (!CryptHashData(hash, (LPBYTE)server->Secret, lstrlenA(server->Secret), 0))
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
