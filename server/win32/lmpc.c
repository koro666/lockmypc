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

	HLMPC_CONFIG config;
	hr = LmpcCfgLoad(&config);
	if (FAILED(hr))
		goto clean_com;

	HLMPC_SERVER server;
	hr = LmpcSrvCreate(config, &server);
	if (FAILED(hr))
		goto clean_cfg;

	hr = RegisterApplicationRestart(NULL, 0);
	if (FAILED(hr))
		goto clean_server;

	HLMPC_UI ui;
	hr = LmpcUiCreate(config, server, &ui);
	if (FAILED(hr))
		goto clean_restart;

	hr = LmpcUiRunLoop(ui);

//clean_ui:
	LmpcUiDestroy(ui);

clean_restart:
	UnregisterApplicationRestart();

clean_server:
	LmpcSrvDestroy(server);

clean_cfg:
	LmpcCfgDestroy(config);

clean_com:
	CoUninitialize();

exit:
	ExitProcess(SUCCEEDED(hr) ? 0 : hr);
}
