#include <common.h>

void VehPhysProc_SpinLast_PhysLinear(struct Thread *t, struct Driver *d)
{
	VehPhysProc_Driving_PhysLinear(t, d);

	// baseSpeed and fireSpeed
	// set both "shorts" in one "int"
	*(int *)&d->baseSpeed = 0;

	d->actionsFlagSet |= 0x4008;
}
