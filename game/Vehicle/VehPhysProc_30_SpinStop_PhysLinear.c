#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800642f4-0x80064320
void VehPhysProc_SpinStop_PhysLinear(struct Thread *t, struct Driver *d)
{
	DECOMP_VehPhysProc_Driving_PhysLinear(t, d);

	d->baseSpeed = 0;
	d->fireSpeed = 0;
}

void DECOMP_VehPhysProc_SpinStop_PhysLinear(struct Thread *t, struct Driver *d)
{
	VehPhysProc_SpinStop_PhysLinear(t, d);
}
