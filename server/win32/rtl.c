#include <windows.h>
#include "rtl.h"

void* __cdecl memset(void* dest, int c, size_t count)
{
	RtlFillMemory(dest, count, (UCHAR)c);
	return dest;
}
