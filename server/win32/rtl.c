#include <windows.h>

#undef RtlFillMemory
WINBASEAPI VOID WINAPI RtlFillMemory(VOID UNALIGNED* Destination, SIZE_T Length, UCHAR Fill);

void* __cdecl memset(void* dest, int c, size_t count)
{
	RtlFillMemory(dest, count, (UCHAR)c);
	return dest;
}
