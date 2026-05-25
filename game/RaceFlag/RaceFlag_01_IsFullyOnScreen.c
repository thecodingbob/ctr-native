#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043f1c-0x80043f28.
int RaceFlag_IsFullyOnScreen(void)
{
	// return true if flag is fully on screen
	// return false if flag is not fully on screen
	return (sdata->RaceFlag_Position == 0);
}
