#include <common.h>

void VehPhysProc_PowerSlide_Finalize(struct Thread *t, struct Driver *d)
{
	d->timeUntilDriftSpinout = d->unk46b << 5;
	d->previousFrameMultDrift = d->multDrift;
}
