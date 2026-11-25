#ifndef CTR_EXTENDED_LAP_TIMES_H
#define CTR_EXTENDED_LAP_TIMES_H

enum { EXTENDED_LAP_HISTORY_CAPACITY = 7 };

// Long races display no lap rows and only need the single best lap.
union ExtendedLapHistory
{
	int completed[EXTENDED_LAP_HISTORY_CAPACITY];
	struct { int time, index; } best;
};

static inline void ExtendedLapTimes_Record(union ExtendedLapHistory *history, int lap, int raceLaps, int lapTime)
{
	if (lap < 0 || lap >= raceLaps)
	{
		return;
	}
	if (raceLaps <= EXTENDED_LAP_HISTORY_CAPACITY)
	{
		history->completed[lap] = lapTime;
	}
	else if (lap == 0 || lapTime < history->best.time)
	{
		history->best.time = lapTime;
		history->best.index = lap;
	}
}

#endif
