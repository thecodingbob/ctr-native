#include <common.h>

#ifndef CUP_PROGRESS_STRING
#define CUP_PROGRESS_STRING sdata->stringIndexSaveCupProgress
#define CUP_PROGRESS_SAVE   sdata->boolSaveCupProgress
#endif

void TakeCupProgress_Activate(s16 stringIndex)
{
	// string index to print
	CUP_PROGRESS_STRING = stringIndex;

	// Draw the menu
	// for "Save Game", "yes / no"
	RECTMENU_Show(&data.menuSaveGame);
}


void TakeCupProgress_MenuProc(struct RectMenu *menu)
{
	if (menu->funcState == RECTMENU_FUNC_STATE_UPDATE)
	{
		s16 stringIndex = CUP_PROGRESS_STRING;
		if (stringIndex != 0)
		{
			DecalFont_DrawMultiLine(GAME_LANGUAGE_STRINGS[stringIndex], 0x100, 0x3c, 0x1cc, FONT_BIG, (s16)JUSTIFY_CENTER);
		}
		return;
	}

	switch (menu->rowSelected)
	{
	case -1:
	case 1:
	{
		RECTMENU_Hide(menu);
		break;
	}
	case 0:
	{
		CUP_PROGRESS_SAVE = 1;
		SelectProfile_ToggleMode(SELECT_PROFILE_MODE_SLOT_SAVE);
		sdata->ptrDesiredMenu = &data.menuWarning2;
		break;
	}
	}
}
