#include <common.h>

void LOAD_Callback_Overlay_Generic(struct LoadQueueSlot *lqs)
{
	(void)lqs;
	sdata->load_inProgress = 0;
}

void LOAD_Callback_Overlay_230(void)
{
	sdata->load_inProgress = 0;
	sdata->gGT->overlayIndex_Threads = OVERLAY_INDEX_MAIN_MENU;
}

void LOAD_Callback_Overlay_231(void)
{
	sdata->load_inProgress = 0;
	sdata->gGT->overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
}

void LOAD_Callback_Overlay_232(void)
{
	sdata->load_inProgress = 0;
	sdata->gGT->overlayIndex_Threads = OVERLAY_INDEX_ADV_HUB;
}

void LOAD_Callback_Overlay_233(void)
{
	sdata->load_inProgress = 0;
	sdata->gGT->overlayIndex_Threads = OVERLAY_INDEX_PODIUMS;
}

void LOAD_Callback_MaskHints3D(struct LoadQueueSlot *lqs)
{
	sdata->load_inProgress = 0;
	sdata->modelMaskHints3D = (struct Model *)lqs->ptrDestination;
}

void LOAD_Callback_Podiums(struct LoadQueueSlot *lqs)
{
	sdata->load_inProgress = 0;
	data.podiumModel_podiumStands = (struct Model *)lqs->ptrDestination;
}

void LOAD_Callback_LEV(struct LoadQueueSlot *lqs)
{
	if ((lqs->flags & LT_GETADDR) == 0)
	{
		sdata->load_inProgress = 0;
	}

	sdata->ptrLevelFile = (struct Level *)lqs->ptrDestination;
}

void LOAD_Callback_PatchMem(struct LoadQueueSlot *lqs)
{
	// CTR doesn't load one lev DRAM for AdvHub,
	// it loads one ReadFile for LEV in a sub-mempack,
	// it loads one ReadFile for PtrMap with AllocHighMem

	// that's why the patch map is handled here
	struct DramPointerMap *patchMap = lqs->ptrDestination;
	int patchNum = patchMap->numBytes >> DRAM_POINTER_MAP_WORD_SHIFT;

	sdata->load_inProgress = 0;

	LOAD_RunPtrMap((char *)sdata->ptrLevelFile, DRAM_GETOFFSETS(patchMap), patchNum);

	MEMPACK_SwapPacks(0);
	MEMPACK_ClearHighMem();
	MEMPACK_SwapPacks(sdata->gGT->activeMempackIndex);
}

void LOAD_Callback_DriverModels(struct LoadQueueSlot *lqs)
{
	sdata->load_inProgress = 0;
	sdata->ptrMPK = (int)lqs->ptrDestination;
}

void LOAD_HubCallback(struct LoadQueueSlot *lqs)
{
	sdata->load_inProgress = 0;
	LOAD_Callback_PatchMem(lqs);

	sdata->gGT->level2 = sdata->ptrLevelFile;
	MEMPACK_SwapPacks(sdata->gGT->activeMempackIndex);
}
