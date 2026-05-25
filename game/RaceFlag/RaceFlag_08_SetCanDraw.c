#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044088-0x80044094.
void RaceFlag_SetCanDraw(s16 param_1)
{
	sdata->RaceFlag_CanDraw = param_1;
}
