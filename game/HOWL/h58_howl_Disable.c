#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002ac94-0x8002acb8
int howl_Disable(void)
{
	if (sdata->boolAudioEnabled == 0)
		return 0;

	sdata->boolAudioEnabled = 0;
	return 1;
}
