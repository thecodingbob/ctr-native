#include <common.h>

// param1 - soundID
// param2:
// 0 - play with no duplicates (dont recycle old)
// 1 - play with no duplicates (recycle old)
// 2 - play with duplicates
int OtherFX_Play(u32 soundID, int flags)
{
	// ff8080:
	// 0x00 - no echo
	// 0xff - volume
	// 0x80 - distortion (none)
	// 0x80 - LR (center of left and right)
	return DECOMP_OtherFX_Play_LowLevel(soundID & 0xfff, flags & 0xff, 0xff8080);
}

int DECOMP_OtherFX_Play(u32 soundID, int flags)
{
	return OtherFX_Play(soundID, flags);
}
