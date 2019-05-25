#pragma once
#include <winnt.h>

#undef RtlFillMemory
NTSYSAPI VOID NTAPI RtlFillMemory(VOID UNALIGNED* Destination, SIZE_T Length, UCHAR Fill);

#undef RtlZeroMemory
NTSYSAPI VOID NTAPI RtlZeroMemory(VOID UNALIGNED* Destination, SIZE_T Length);

#undef RtlMoveMemory
NTSYSAPI VOID NTAPI RtlMoveMemory(VOID UNALIGNED* Destination, const VOID UNALIGNED* Source, SIZE_T Length);

NTSYSAPI BOOLEAN NTAPI RtlTimeToSecondsSince1970(PLARGE_INTEGER Time, PULONG ElapsedSeconds);
