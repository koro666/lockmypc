#include <windows.h>
#include <winsock2.h>
#include "server.h"

HRESULT LmpcSrvInitialize(void)
{
	WSADATA wd;

	int error = WSAStartup(MAKEWORD(2, 2), &wd);
	if (error)
		return HRESULT_FROM_WIN32(error);

	return S_OK;
}

HRESULT LmpcSrvFinalize(void)
{
	int error = WSACleanup();
	return error ? HRESULT_FROM_WIN32(error) : S_OK;
}
