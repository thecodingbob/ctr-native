#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80058898-0x80058948.
void VehBirth_TeleportAll(struct GameTracker *gGT, u32 spawnFlags)
{
	struct Driver *d;

	for (int i = 0; i < 8; i++)
	{
		d = gGT->drivers[i];

		if (d == NULL)
			continue;

		if (d->instSelf->thread->modelIndex == DYNAMIC_ROBOT_CAR)
		{
			BOTS_GotoStartingLine(d);
		}

		else
		{
			VehBirth_TeleportSelf(d, spawnFlags | 1, 0);
		}
	}
}
