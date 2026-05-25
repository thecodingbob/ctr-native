#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80069370-0x800693c8.
void VehTurbo_ThDestroy(struct Thread *t)
{
	struct Turbo *turboObj;
	turboObj = t->object;

	struct Driver *d = turboObj->driver;
	d->actionsFlagSet &= 0xfffffdff;

	INSTANCE_Death(t->inst);
	INSTANCE_Death(turboObj->inst);
}
