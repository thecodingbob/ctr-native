#include <common.h>

// param_3:
// 0 - normal
// 1 - echo
// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80028494-0x800284d0
void OtherFX_Play_Echo(u32 soundID, int flags, int echoFlag)
{
	// ff8080:
	// 0x00 - no echo
	// 0xff - volume
	// 0x80 - distortion (none)
	// 0x80 - LR (center of left and right)
	int otherFlags = 0xff8080;

	if (echoFlag != 0)
		otherFlags |= 0x1000000;

	OtherFX_Play_LowLevel(soundID & 0xffff, flags & 0xff, otherFlags);
}
