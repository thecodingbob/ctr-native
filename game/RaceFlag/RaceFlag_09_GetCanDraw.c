#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044094-0x800440a0.
int RaceFlag_GetCanDraw(void)
{
	return sdata->RaceFlag_CanDraw;
}
