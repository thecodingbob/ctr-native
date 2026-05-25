#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80044290-0x800442a0.
void RaceFlag_ResetTextAnim(void)
{
	sdata->RaceFlag_LoadingTextAnimFrame = -1;
}
