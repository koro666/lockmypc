#include <windows.h>

__declspec(noreturn) void WinMainCRTStartup(void)
{
	SetDefaultDllDirectories(LOAD_LIBRARY_SEARCH_SYSTEM32);
	// TODO:
	ExitProcess(0);
}
