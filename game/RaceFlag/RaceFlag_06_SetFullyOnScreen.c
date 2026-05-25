#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8004402c-0x80044058.
void RaceFlag_SetFullyOnScreen()
{
	sdata->RaceFlag_AnimationType = 0;
	sdata->RaceFlag_LoadingTextAnimFrame = -1;

	// flag is now fully on-screen
	sdata->RaceFlag_Position = 0;

	// enable loading screen's checkered flag
	sdata->gGT->renderFlags |= 0x1000;
}
