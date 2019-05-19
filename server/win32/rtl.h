#pragma once

#undef RtlFillMemory
WINBASEAPI VOID WINAPI RtlFillMemory(VOID UNALIGNED* Destination, SIZE_T Length, UCHAR Fill);

#undef RtlMoveMemory
WINBASEAPI VOID RtlMoveMemory(VOID UNALIGNED* Destination, const VOID UNALIGNED* Source, SIZE_T Length);
