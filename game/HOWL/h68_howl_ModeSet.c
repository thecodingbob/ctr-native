#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002b1fc-0x8002b208
void howl_ModeSet(int newMode)
{
	sdata->boolStereoEnabled = newMode;
}
