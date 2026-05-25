#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044058-0x80044088.
void RaceFlag_SetFullyOffScreen()
{
	sdata->RaceFlag_AnimationType = 0;
	sdata->RaceFlag_LoadingTextAnimFrame = -1;

	// flag is now fully off-screen
	sdata->RaceFlag_Position = 5000;

	// disable loading screen's checkered flag
	sdata->gGT->renderFlags &= ~(0x1000);
}
