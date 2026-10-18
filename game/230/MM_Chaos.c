#include <common.h>

// The Chaos prompt is one step of the new-file chain in MM_MenuFlow.c, between
// the Adventure new/load menu and the racer-count screen. Attaching it, advancing
// past it and backing out of it are all done there, with the other steps.

void MM_MenuProc_Chaos(struct RectMenu *menu)
{
	if (menu->funcState != RECTMENU_FUNC_STATE_INPUT)
	{
		return;
	}

	const s16 row = menu->rowSelected;

	// Triangle: back to the screen before this one.
	if (row == -1)
	{
		MM_NewLoad_Retreat(menu);
		return;
	}

	if ((row < 0) || (row > 1))
	{
		return;
	}

	// Row 0 is "Chaos", row 1 is "Regular". Either answer describes a file that is about
	// to be created, so whatever a file made earlier in the session left in the live
	// settings starts over here.
	sdata->chaosArmed = (row == 0);
	GAMEPROG_ResetChaosSettings(&sdata->chaosSettings);
	sdata->chaosSettings.armed = sdata->chaosArmed;

	MM_NewLoad_Advance(menu);
}
