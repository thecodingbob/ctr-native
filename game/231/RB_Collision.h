#ifndef RB_COLLISION_H
#define RB_COLLISION_H

#include <common.h>

// Narrow a movement component to a signed halfword, then shift by 0..31 bits.
static inline s16 RB_ShiftRightS16(s32 value, u32 shift)
{
	// NOTE(aalhendi): Keep narrowing separate so GCC 2.8.1 retains the signed load.
	s16 narrowed = value;
	narrowed >>= shift;
	return narrowed;
}

// Stop at the first hit, checking players before robots and mines.
static inline void RB_CheckHazardCollisions(struct Instance *inst, struct Thread *thread, s32 damage, u32 radius, s32 sound)
{
	struct Instance *hitInst;
	struct Driver *hitDriver;
	b32 wasNotSpinning;

	hitInst = LinkedCollide_Radius(inst, thread, GAME_TRACKER->threadBuckets[PLAYER].thread, radius);
	if (hitInst != NULL)
	{
		hitDriver = hitInst->thread->object;
		// Damage may change kartState; the sound depends on the previous state.
		wasNotSpinning = hitDriver->kartState != KS_SPINNING;
		if (RB_Hazard_HurtDriver(hitDriver, damage, 0, 0) && wasNotSpinning && sound != 0)
		{
			OtherFX_Play_Echo(sound, 1, (hitDriver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);
		}
		return;
	}

	hitInst = LinkedCollide_Radius(inst, thread, GAME_TRACKER->threadBuckets[ROBOT].thread, radius);
	if (hitInst != NULL)
	{
		hitDriver = hitInst->thread->object;
		RB_Hazard_HurtDriver(hitDriver, damage, 0, 0);
		return;
	}

	hitInst = LinkedCollide_Radius(inst, thread, GAME_TRACKER->threadBuckets[MINE].thread, radius);
	if (hitInst != NULL)
	{
		((ThreadScratchCollideFunc)hitInst->thread->funcThCollide)(hitInst->thread, thread, hitInst->thread->funcThCollide, NULL);
	}
}

static inline struct Instance *RB_FindDriverCollision(struct Instance *inst, struct Thread *thread)
{
	struct Instance *hitInst;

	hitInst = LinkedCollide_Radius(inst, thread, GAME_TRACKER->threadBuckets[PLAYER].thread, 0x10000);
	if (hitInst == NULL)
	{
		hitInst = LinkedCollide_Radius(inst, thread, GAME_TRACKER->threadBuckets[ROBOT].thread, 0x10000);
	}
	return hitInst;
}

#endif
