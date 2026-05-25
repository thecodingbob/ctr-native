#include <common.h>

void VehPhysProc_PowerSlide_InitSetUpdate(struct Thread *t, struct Driver *d)
{
	// This is so the update function
	// is not called on the first frame,
	// just like spinning

	d->funcPtrs[0] = 0;
	d->funcPtrs[1] = VehPhysProc_PowerSlide_Update;
}
