#include <windows.h>
#include <objbase.h>
#include "config.h"
#include "server.h"
#include "display.h"

__declspec(noreturn) void WinMainCRTStartup(void)
{
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);

	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
	if (FAILED(hr))
		goto exit;

	hr = LmpcCfgInitialize();
	if (FAILED(hr))
		goto clean_com;

	hr = LmpcSrvInitialize();
	if (FAILED(hr))
		goto clean_cfg;

	hr = RegisterApplicationRestart(NULL, 0);
	if (FAILED(hr))
		goto clean_server;

	hr = LmpcUiInitialize();
	if (FAILED(hr))
		goto clean_restart;

	hr = LmpcUiRunLoop();

//clean_ui:
	LmpcUiFinalize();

clean_restart:
	UnregisterApplicationRestart();

clean_server:
	LmpcSrvFinalize();

clean_cfg:
	LmpcCfgFinalize();

clean_com:
	CoUninitialize();

exit:
	ExitProcess(SUCCEEDED(hr) ? 0 : hr);
}
