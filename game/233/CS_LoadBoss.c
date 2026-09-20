#include <common.h>

void CS_LoadBossCallback(struct LoadQueueSlot *lqs)
{
	s32 *loadInProgress = &sdata->load_inProgress;
	void *model = lqs->ptrDestination;

	*loadInProgress = 0;
	CS_BOSS_HEAD_MODEL = model;
}

void CS_LoadBoss(const struct BossCutsceneData *bcd)
{
	s32 i;
	s32 fileIndex;
	s32 otherHubMempack = LOAD_HUB_MEMPACK_PAIR_INDEX_SUM - GAME_TRACKER->activeMempackIndex;
	const s32 *files = bcd->fileIDs;

	CDSYS_XAPauseRequest();
	for (i = CS_BOSS_MODEL_COUNT - 1; i >= 0; i--)
		CS_BOSS_MODELS[i] = NULL;

	// The boss borrows the inactive hub's memory pack.
	GAME_TRACKER->levID_in_each_mempack[otherHubMempack] = LOAD_NO_LEVEL_IN_MEMPACK;
	MEMPACK_SwapPacks(otherHubMempack);
	MEMPACK_ClearLowMem();
	CS_LOAD_IN_PROGRESS = 1;

	fileIndex = files[0];
	if (fileIndex != 0)
	{
		LOAD_AppendQueue(CS_BIGFILE_HEADER, LT_VRAM, fileIndex + (otherHubMempack - 1), NULL, NULL);
	}

	files++;
	for (i = CS_BOSS_MODEL_COUNT - 1; i > CS_BOSS_MODEL_HEAD; i--)
	{
		fileIndex = files[i];
		if (fileIndex != 0)
		{
			LOAD_AppendQueue(CS_BIGFILE_HEADER, LT_DRAM, fileIndex + (otherHubMempack - 1), &CS_BOSS_MODELS[i], LOAD_QUEUE_CALLBACK_SET_POINTER);
		}
	}

	// The head is queued last; its callback releases the loading state.
	LOAD_AppendQueue(CS_BIGFILE_HEADER, LT_DRAM, files[i] + (otherHubMempack - 1), NULL, CS_LoadBossCallback);
}
