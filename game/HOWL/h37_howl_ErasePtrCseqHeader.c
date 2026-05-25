#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80029dc0-0x80029dcc
void howl_ErasePtrCseqHeader()
{
	// can not play a song anymore
	sdata->ptrCseqHeader = 0;
}
