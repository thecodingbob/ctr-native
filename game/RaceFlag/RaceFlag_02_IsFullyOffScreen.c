#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80043f28-0x80043f44.
int RaceFlag_IsFullyOffScreen(void)
{
	// return false, "not true", if flag is < 5000, partially on-screen
	// return true, "not false", if flag is >= 5000, fully off-screen
	return ((((u16)sdata->RaceFlag_Position + 4999U) & 0xffff) < 9999) ^ 1;
}
