#include <common.h>

void LOAD_OvrLOD(u32 numPlyrCurrGame)
{
	// change {1-4} -> {0-3}
	u32 overlayIndex = numPlyrCurrGame - 1;

	struct GameTracker *gGT = sdata->gGT;

	// if new LOD overlay needs to load
	if ((u32)gGT->overlayIndex_LOD != overlayIndex)
	{

		// save ID, and reload next overlay (sector read invalidation)
		gGT->overlayIndex_LOD = overlayIndex;
		gGT->overlayIndex_Threads = OVERLAY_INDEX_NONE;
	}
	return;
}

void LOAD_OvrEndRace(u32 overlayIndex)
{
	struct GameTracker *gGT = sdata->gGT;

	// if new EndOfRace overlay needs to load
	if ((u32)gGT->overlayIndex_EndOfRace != overlayIndex)
	{

		gGT->overlayIndex_EndOfRace = overlayIndex;
		gGT->overlayIndex_LOD = OVERLAY_INDEX_NONE;
	}
	return;
}

static void LOAD_NativeResetThreadsOverlay(enum OverlayIndex overlayIndex)
{
	switch (overlayIndex)
	{
	case OVERLAY_INDEX_NONE:
		break;
	case OVERLAY_INDEX_MAIN_MENU:
		OVR230_InitData();
		break;
	case OVERLAY_INDEX_RACING_OR_BATTLE:
		OVR231_InitData();
		break;
	case OVERLAY_INDEX_ADV_HUB:
		OVR232_InitData();
		break;
	case OVERLAY_INDEX_PODIUMS:
		OVR233_InitData();
		break;
	}
}

void LOAD_OvrThreads(u32 overlayIndex)
{
	struct GameTracker *gGT = sdata->gGT;

	// if new Threads overlay needs to load
	if ((u32)gGT->overlayIndex_Threads != overlayIndex)
	{
		// NOTE(aalhendi): Native overlays are already linked, so reset the
		// overlay-owned data that retail would refresh by streaming into OVR_Region3.
		gGT->overlayIndex_Threads = OVERLAY_INDEX_NONE;
		LOAD_NativeResetThreadsOverlay((enum OverlayIndex)overlayIndex);
		((void (*)())data.overlayCallbackFuncs[overlayIndex])();
	}
}

int LOAD_GetAdvPackIndex(void)
{
	int levelID = sdata->gGT->levelID;

	if ((levelID != GEM_STONE_VALLEY) && (levelID != GLACIER_PARK))
	{
		return 1;
	}

	return 2;
}
