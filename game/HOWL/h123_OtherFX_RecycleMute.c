#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e724-0x8002e760
void OtherFX_RecycleMute(int *soundID_Count)
{
	if (*soundID_Count != 0)
	{
		OtherFX_Stop1(*soundID_Count);
		*soundID_Count = 0;
	}
}
