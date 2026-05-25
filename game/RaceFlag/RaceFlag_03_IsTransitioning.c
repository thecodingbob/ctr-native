#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043f44-0x80043f8c.
int RaceFlag_IsTransitioning()
{
	int pos = sdata->RaceFlag_Position;

	return
	    // if checkered flag is not fully on-screen and not fully off-screen
	    (pos != 0) && (pos != -5000) && (pos != 5000) &&

	    // is allowed to render
	    ((sdata->gGT->renderFlags & 0x1000) != 0);
}
