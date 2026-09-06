#include <common.h>

u8 MEMCARD_GetNextSwEvent(void)
{
	// IOE = IO End, meaning "finished without error"
	if (TestEvent(sdata->SwCARD_EvSpIOE))
	{
		return MC_RETURN_IOE;
	}
	if (TestEvent(sdata->SwCARD_EvSpERROR))
	{
		return MC_RETURN_TIMEOUT;
	}
	if (TestEvent(sdata->SwCARD_EvSpTIMOUT))
	{
		return MC_RETURN_NOCARD;
	}
	if (TestEvent(sdata->SwCARD_EvSpNEW))
	{
		return MC_RETURN_NEWCARD;
	}

	return MC_RETURN_PENDING;
}

u8 MEMCARD_GetNextHwEvent(void)
{
	// IOE = IO End, meaning "finished without error"
	if (TestEvent(sdata->HwCARD_EvSpIOE))
	{
		return MC_RETURN_IOE;
	}
	if (TestEvent(sdata->HwCARD_EvSpERROR))
	{
		return MC_RETURN_TIMEOUT;
	}
	if (TestEvent(sdata->HwCARD_EvSpTIMOUT))
	{
		return MC_RETURN_NOCARD;
	}
	if (TestEvent(sdata->HwCARD_EvSpNEW))
	{
		return MC_RETURN_NEWCARD;
	}

	return MC_RETURN_PENDING;
}

u8 MEMCARD_WaitForHwEvent(void)
{
	while (1)
	{
		// IOE = IO End, meaning "finished without error"
		if (TestEvent(sdata->HwCARD_EvSpIOE))
		{
			return MC_RETURN_IOE;
		}
		if (TestEvent(sdata->HwCARD_EvSpERROR))
		{
			return MC_RETURN_TIMEOUT;
		}
		if (TestEvent(sdata->HwCARD_EvSpTIMOUT))
		{
			return MC_RETURN_NOCARD;
		}
		if (TestEvent(sdata->HwCARD_EvSpNEW))
		{
			return MC_RETURN_NEWCARD;
		}

		// Not allowed to return PENDING, the goal is to
		// wait until the memcard is not PENDING anymore
		// return MC_RETURN_PENDING;
	}
}

void MEMCARD_SkipEvents(void)
{
	// Flush all "previous" Events until everything shows PENDING
	while (MEMCARD_GetNextSwEvent() != MC_RETURN_PENDING)
	{
		;
	}
	while (MEMCARD_GetNextHwEvent() != MC_RETURN_PENDING)
	{
		;
	}
}
