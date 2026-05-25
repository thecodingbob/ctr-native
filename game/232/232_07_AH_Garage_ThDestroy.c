#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ae8a0-0x800ae8e0.
void AH_Garage_ThDestroy(struct Thread *t)
{
	struct BossGarageDoor *garage;
	garage = t->object;

	if (garage->garageTopInst != NULL)
	{
		INSTANCE_Death(garage->garageTopInst);
		garage->garageTopInst = NULL;
	}

	return;
}
