#include <common.h>

void MEMCARD_SkipEvents(void)
{
	// Flush all "previous" Events until everything shows PENDING
	while (MEMCARD_GetNextSwEvent() != MC_RETURN_PENDING)
		;
	while (MEMCARD_GetNextHwEvent() != MC_RETURN_PENDING)
		;
}
