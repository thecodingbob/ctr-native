#include <common.h>

#define TIMER_RCNT RCntCNT1

#ifndef TIMER_TOTAL_UNITS
#define TIMER_TOTAL_UNITS sdata->rcntTotalUnits
#endif

enum TimerConstants
{
	TIMER_RCNT_TARGET = 0xffff,
	TIMER_RCNT_MODE = 0x2000,
	TIMER_RCNT_LOW_RECHECK_THRESHOLD = 100,
	TIMER_MILLISECONDS_PER_SECOND = 1000,
	TIMER_RCNT_UNITS_PER_SECOND = 0x147e,
	TIMER_WRAP_MILLISECONDS = 0xc7e18,
};

void Timer_Init(void)
{
	EnterCriticalSection();
	StopRCnt(TIMER_RCNT);
	SetRCnt(TIMER_RCNT, TIMER_RCNT_TARGET, TIMER_RCNT_MODE);
	StartRCnt(TIMER_RCNT);
	ExitCriticalSection();
}

void Timer_Destroy(void)
{
	EnterCriticalSection();
	StopRCnt(TIMER_RCNT);
	ExitCriticalSection();
}

s32 Timer_GetTime_Total(void)
{
	u32 sysClock;
	s32 rcnt;

	sysClock = TIMER_TOTAL_UNITS;
	rcnt = GetRCnt(TIMER_RCNT);

	sysClock += rcnt;

	if (rcnt < TIMER_RCNT_LOW_RECHECK_THRESHOLD)
	{
		// VSync may have accumulated and reset the counter during GetRCnt.
		sysClock = TIMER_TOTAL_UNITS;
		sysClock += rcnt;
	}

	// NOTE(aalhendi): Retail wraps the 32-bit product before signed division.
	return (s32)(sysClock * TIMER_MILLISECONDS_PER_SECOND) / TIMER_RCNT_UNITS_PER_SECOND;
}

// Return elapsed time and optionally store the current timestamp for the next call.
s32 Timer_GetTime_Elapsed(s32 oldVal, s32 *retVal)
{
	s32 newVal = Timer_GetTime_Total();

	if (retVal != 0)
	{
		*retVal = newVal;
	}

	// Correct a wrap in the scaled root-counter value.
	if (newVal < oldVal)
	{
		newVal += TIMER_WRAP_MILLISECONDS;
	}

	return newVal - oldVal;
}
