#include <common.h>

void SelectProfile_QueueLoadHub_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

// This was needed by VehBirth_TeleportSelf,
// checking prevLev == MAIN_MENU_LEVEL, but
// now levID can be advHub and new code checks
// for prevLev < LAB_BASEMENT, so removing this
#if 0
  gGT->levelID = MAIN_MENU_LEVEL;
#endif

	data.characterIDs[0] = sdata->advProgress.characterID;
	MainRaceTrack_RequestLoad(gGT->currLEV);
	RECTMENU_Hide(menu);
	return;
}
