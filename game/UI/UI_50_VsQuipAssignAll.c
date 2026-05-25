#include <common.h>

#define UI_QUIP_DATA_BASE_PSX 0x800864dcu
#define UI_QUIP_VS_META_OFF   0x170u
#define UI_QUIP_VS_META_END   0x518u
#define UI_QUIP_BAT_META_OFF  0x730u
#define UI_QUIP_BAT_META_END  0x850u

struct QuipMetaRaw
{
	u32 ptrQuipStrCurr;
	u32 ptrQuipStrNext;
	s16 conditionType;
	s16 flags;
	int threshold;
	int driverOffset;
	int dataSize;
};

_Static_assert(sizeof(struct QuipMetaRaw) == 0x18);

static u8 *UI_VsQuipData(void)
{
#if BUILD >= JpnTrial
	return data.data830;
#else
	return data.data850;
#endif
}

static struct QuipStr *UI_VsQuipPtrFromPsx(u32 psxAddr)
{
	return (struct QuipStr *)(void *)(UI_VsQuipData() + (psxAddr - UI_QUIP_DATA_BASE_PSX));
}

static struct QuipMeta UI_VsQuipMetaFromRaw(struct QuipMetaRaw *raw)
{
	struct QuipMeta meta;

	meta.ptrQuipStrCurr = UI_VsQuipPtrFromPsx(raw->ptrQuipStrCurr);
	meta.ptrQuipStrNext = UI_VsQuipPtrFromPsx(raw->ptrQuipStrNext);
	meta.conditionType = raw->conditionType;
	meta.flags = raw->flags;
	meta.threshold = raw->threshold;
	meta.driverOffset = raw->driverOffset;
	meta.dataSize = raw->dataSize;

	return meta;
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80054bfc-0x800550f4
void UI_VsQuipAssignAll(void)
{
	struct GameTracker *gGT = sdata->gGT;
	int characterID = 0;

	if (gGT->numPlyrCurrGame < 2)
		return;

	u8 *quipData = UI_VsQuipData();
	u32 metaStartOff = UI_QUIP_VS_META_OFF;
	u32 metaEndOff = UI_QUIP_VS_META_END;

	if ((gGT->gameMode1 & BATTLE_MODE) != 0)
	{
		metaStartOff = UI_QUIP_BAT_META_OFF;
		metaEndOff = UI_QUIP_BAT_META_END;
	}

	struct QuipMetaRaw *metaStart = (struct QuipMetaRaw *)(void *)(quipData + metaStartOff);
	struct QuipMetaRaw *metaEnd = (struct QuipMetaRaw *)(void *)(quipData + metaEndOff);

	for (struct QuipMetaRaw *raw = metaStart; raw < metaEnd; raw++)
	{
		struct QuipStr *curr = UI_VsQuipPtrFromPsx(raw->ptrQuipStrCurr);
		struct QuipStr *next = UI_VsQuipPtrFromPsx(raw->ptrQuipStrNext);

		while (curr < next)
		{
			curr->priority++;
			curr++;
		}
	}

	struct Driver *bestDriver = NULL;
	int bestScore = 0;
	int secondScore = 0;
	int scoreByDriverID[8];

	struct Thread *thread = gGT->threadBuckets[PLAYER].thread;

	while (thread != NULL)
	{
		struct Driver *driver = thread->object;

		driver->EndOfRaceComment_ptrQuip = NULL;

		int score;

		if ((gGT->gameMode1 & POINT_LIMIT) == 0)
		{
			score = driver->BattleHUD.numLives;
		}
		else
		{
			score = gGT->battleSetup.pointsPerTeam[driver->BattleHUD.teamID];
		}

		scoreByDriverID[driver->driverID] = score;

		if (score > bestScore)
		{
			secondScore = bestScore;
			bestDriver = driver;
			bestScore = score;
		}
		else if (score == bestScore)
		{
			secondScore = bestScore;
			bestDriver = NULL;
		}

		thread = thread->siblingThread;
	}

	for (struct QuipMetaRaw *raw = metaStart; raw < metaEnd; raw++)
	{
		struct QuipMeta meta = UI_VsQuipMetaFromRaw(raw);
		struct Driver *selectedDriver = NULL;
		int threshold = meta.threshold;
		u32 bestValue = (meta.conditionType == 5) ? 0x7fffffff : 0;

		if ((meta.flags & 1) != 0)
		{
			int numLaps = (s8)gGT->numLaps;

			if (numLaps < 0)
				numLaps = -numLaps;

			threshold *= numLaps;
		}

		thread = gGT->threadBuckets[PLAYER].thread;

		while (thread != NULL)
		{
			struct Driver *driver = thread->object;
			struct Driver *nextSelectedDriver = selectedDriver;
			int nextThreshold = threshold;
			u32 nextBestValue = bestValue;

			switch (meta.conditionType)
			{
			case 0:
			{
				int value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);

				nextSelectedDriver = driver;
				nextThreshold = value;

				if (value <= threshold)
				{
					nextSelectedDriver = selectedDriver;
					nextThreshold = threshold;

					if (value == threshold)
						nextSelectedDriver = NULL;
				}
				break;
			}

			case 1:
			{
				int value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);

				if (value >= 0)
				{
					nextSelectedDriver = driver;
					nextThreshold = value;

					if (threshold <= value)
					{
						nextSelectedDriver = selectedDriver;
						nextThreshold = threshold;

						if (value == threshold)
							nextSelectedDriver = NULL;
					}
				}
				break;
			}

			case 3:
			{
				for (int i = 0; i < 8; i++)
				{
					u32 value = (u8)driver->numTimesAttackedByPlayer[i];
					int delta = value - bestValue;

					if (threshold < delta)
					{
						characterID = i;
						selectedDriver = driver;
						bestValue = value;
					}
					else if (-threshold < delta)
					{
						selectedDriver = NULL;

						if ((int)bestValue < (int)value)
							bestValue = value;
					}
				}

				nextSelectedDriver = selectedDriver;
				nextBestValue = bestValue;
				break;
			}

			case 4:
			{
				u32 value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);
				int delta = value - bestValue;

				if (threshold < delta)
				{
					nextSelectedDriver = driver;
					nextBestValue = value;
				}
				else
				{
					nextSelectedDriver = selectedDriver;
					nextBestValue = bestValue;

					if (-threshold < delta)
					{
						nextSelectedDriver = NULL;

						if ((int)bestValue < (int)value)
							nextBestValue = value;
					}
				}
				break;
			}

			case 5:
			{
				u32 value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);
				int delta = bestValue - value;

				if ((int)value >= 0)
				{
					if (threshold < delta)
					{
						nextSelectedDriver = driver;
						nextBestValue = value;
					}
					else
					{
						nextSelectedDriver = selectedDriver;
						nextBestValue = bestValue;

						if (-threshold < delta)
						{
							nextSelectedDriver = NULL;

							if ((int)value < (int)bestValue)
								nextBestValue = value;
						}
					}
				}
				break;
			}

			case 6:
			{
				u32 value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);

				if (value == (u8)driver->numTimesAttacking)
					nextSelectedDriver = driver;
				break;
			}

			case 7:
				if (threshold == 0)
				{
					if (driver == bestDriver)
						nextSelectedDriver = bestDriver;
				}
				else if ((threshold == 1) && (secondScore != 0) && (scoreByDriverID[driver->driverID] == secondScore))
				{
					nextSelectedDriver = driver;
				}
				break;

			case 8:
			{
				int value = UI_VsQuipReadDriver(driver, meta.driverOffset, meta.dataSize);

				if (value == threshold)
					nextSelectedDriver = driver;
				break;
			}

			case 9:
				if (driver->EndOfRaceComment_ptrQuip == NULL)
					UI_VsQuipAssign(driver, &meta, bestDriver, 0);
				break;

			default:
				break;
			}

			if ((nextSelectedDriver != NULL) && ((meta.flags & 0xc) != 0))
				UI_VsQuipAssign(nextSelectedDriver, &meta, bestDriver, characterID);

			thread = thread->siblingThread;
			selectedDriver = nextSelectedDriver;
			threshold = nextThreshold;
			bestValue = nextBestValue;
		}

		UI_VsQuipAssign(selectedDriver, &meta, bestDriver, characterID);
	}
}
