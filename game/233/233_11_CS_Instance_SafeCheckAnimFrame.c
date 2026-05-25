#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac638-0x800ac694
int CS_Instance_SafeCheckAnimFrame(struct Instance *inst, int animIndex, int LOD, int desiredFrame)
{
	// Default return value
	int animFrame = desiredFrame;

	if (inst == NULL)
		return animFrame;

	if (desiredFrame <= 0)
		return 0;

	int numFrames = CS_Instance_GetNumAnimFrames(inst, animIndex, LOD);

	// if negative
	if (numFrames < 1)
		return 0;

	// if more than 1 and out of bounds
	if (numFrames <= desiredFrame)
		animFrame = numFrames - 1;

	// Return adjusted animFrame
	return animFrame;
}
