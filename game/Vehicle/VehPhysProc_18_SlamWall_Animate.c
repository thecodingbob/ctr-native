#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80063b2c-0x80063bd4
void VehPhysProc_SlamWall_Animate(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;

	inst->animFrame++;

	d->matrixIndex++;

	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);

	if (inst->animFrame < (numFrames - 1))
	{
		return;
	}

	numFrames = VehFrameInst_GetNumAnimFrames(inst, 0);
	if (numFrames > 0)
	{
		inst->animIndex = 0;
		inst->animFrame = VehFrameInst_GetStartFrame(0, numFrames);
		d->matrixArray = 0;
		d->matrixIndex = 0;
	}

	d->funcPtrs[0] = VehPhysProc_Driving_Init;
}
