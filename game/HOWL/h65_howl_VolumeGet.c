#include <common.h>

// real Naughty Dog name
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b0e0-0x8002b130
int howl_VolumeGet(int type)
{
	if (type == 1)
		return sdata->vol_Music;

	if (type == 0)
		return sdata->vol_FX;

	if (type == 2)
		return sdata->vol_Voice;

	return 0;
}
