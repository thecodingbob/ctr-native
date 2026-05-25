#include <common.h>

// all other Spinning functions should execute for one frame,
// and then VehPhysProc_SpinFirst_Update should happen after that,
// otherwise driver wont spin out if hitting armadillo while driver is motionless
void VehPhysProc_SpinFirst_InitSetUpdate(struct Thread *t, struct Driver *d)
{
	d->funcPtrs[0] = 0;
	d->funcPtrs[1] = VehPhysProc_SpinFirst_Update;
}
