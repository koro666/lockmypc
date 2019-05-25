#include <windows.h>
#include "utility.h"
#include "rtl.h"

#if !UNICODE
HRESULT CoStrDup(LPCTSTR input, LPTSTR* result)
{
	*result = NULL;

	if (!input)
		return E_OUTOFMEMORY;

	int count = lstrlen(input);
	size_t size = (count + 1) * sizeof(TCHAR);

	*result = CoTaskMemAlloc(size);
	if (!*result)
		return E_OUTOFMEMORY;

	RtlMoveMemory(*result, input, size);
	return S_OK;
}
#endif

ULONG GetSystemTimeAsUnixTime(void)
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
