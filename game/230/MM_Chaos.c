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

	// Row 0 is "Chaos", row 1 is "Regular". The settings that go with the answer
	// are the next step of this work; this only records the switch for the
	// session, and the row the cursor is on is what the box goes on showing.
	sdata->chaosArmed = (row == 0);

	MM_NewLoad_Advance(menu);
}
