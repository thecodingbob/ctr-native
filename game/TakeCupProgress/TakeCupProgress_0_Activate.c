#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004b230-0x8004b258.
void TakeCupProgress_Activate(s16 stringIndex)
{
	// string index to print
	sdata->stringIndexSaveCupProgress = stringIndex;

	// Draw the menu
	// for "Save Game", "yes / no"
	RECTMENU_Show(&data.menuSaveGame);
}
