#include <common.h>

void AH_WarpPad_MenuProc(struct RectMenu *menu)
{
	struct GameTracker *gGT = sdata->gGT;

	if (menu->rowSelected == 0)
	{
		gGT->gameMode2 |= TOKEN_RACE;
	}

	else
	{
		gGT->gameMode1 |= RELIC_RACE;
	}

	// optimizes to JMP
	RECTMENU_Hide(menu);
	return;
}
