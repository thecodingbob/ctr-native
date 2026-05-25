#include <common.h>

void VehPhysProc_PowerSlide_PhysLinear(struct Thread *thread, struct Driver *driver)
{
	VehPhysProc_Driving_PhysLinear(thread, driver);
	driver->actionsFlagSet |= 0x1800;
	driver->timeSpentDrifting += sdata->gGT->elapsedTimeMS;
}
