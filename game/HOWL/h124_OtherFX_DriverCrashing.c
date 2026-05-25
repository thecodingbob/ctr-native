#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002e760-0x8002e7bc
void OtherFX_DriverCrashing(u32 boolEcho, u32 volume)
{
	u32 crashID;

	if (volume < 0xdd)
	{
		crashID = 0xb;

		if (volume > 0xa0)
		{
			crashID = 0xc;
		}
	}
	else
	{
		crashID = 0xa;
	}

	OtherFX_Play_LowLevel(crashID, 0, (boolEcho << 0x18) | ((volume & 0xff) << 0x10) | 0x8080);
}
