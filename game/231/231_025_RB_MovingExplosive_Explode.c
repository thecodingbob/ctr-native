#include <common.h>

void DECOMP_RB_MovingExplosive_Explode(struct Thread *t, struct Instance *inst, struct TrackerWeapon *tw)
{
	s16 soundId;
	struct Driver *d;

	// bomb
	if (inst->model->id == DYNAMIC_BOMB)
	{
		// bomb explode
		soundId = 0x49;
		tw->driverParent->instBombThrow = NULL;
	}

	// missile
	else
	{
		d = tw->driverTarget;
		if (d != NULL)
		{
			// remove 2D square-target being drawn on the player's screen
			d->actionsFlagSet &= 0xfbffffff;
		}
		// missile explode
		soundId = 0x4c;
	}

	// play explosion sound
	PlaySound3D(soundId, inst);

	// stop audio of rolling
	OtherFX_RecycleMute(&tw->audioPtr);

#ifndef REBUILD_PC
	DECOMP_RB_Burst_Init(inst);
#else
	// TODO(aalhendi): Retail calls RB_Burst_Init here.
	// Native still needs the burst/Teeth dependency chain wired.
#endif

	// This thread is now dead
	t->flags |= 0x800;
	return;
}
