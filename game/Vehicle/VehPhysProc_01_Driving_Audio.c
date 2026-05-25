#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80062a2c-0x80062a4c.
void VehPhysProc_Driving_Audio(struct Thread *t, struct Driver *d)
{
	EngineSound_Player(d);
}
