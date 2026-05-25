#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b1f0-0x8002b1fc
int howl_ModeGet(void)
{
	return sdata->boolStereoEnabled;
}
