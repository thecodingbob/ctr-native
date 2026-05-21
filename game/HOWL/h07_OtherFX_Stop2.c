#include <common.h>

// all instances of soundID
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028844-0x80028880
void OtherFX_Stop2(int soundID_count)
{
	DECOMP_Smart_EnterCriticalSection();

	// all instances of soundID
	DECOMP_Channel_SearchFX_Destroy(1, soundID_count & 0xffff, 0xffff);

	DECOMP_Smart_ExitCriticalSection();
}

void DECOMP_OtherFX_Stop2(int soundID_count)
{
	OtherFX_Stop2(soundID_count);
}
