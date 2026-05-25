#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043f8c-0x80043fb0.
void RaceFlag_SetDrawOrder(int drawOrder)
{
	sdata->RaceFlag_DrawOrder = (drawOrder != 0) ? 1 : -1;
}
