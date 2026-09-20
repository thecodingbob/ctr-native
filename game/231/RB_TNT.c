#include <common.h>

// In air, after spamming L1 or R1,
// will explode on impact with ground
void RB_TNT_ThTick_ThrowOffHead(struct Thread *t)
{
	struct Instance *inst;
	struct MineWeapon *mw;

	inst = t->inst;

	//  object (tnt)
	mw = inst->thread->object;

	// do NOT use parenthesis
	inst->matrix.t[1] += (mw->velocity.y * GAME_TRACKER->elapsedTimeMS) >> 5;

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail reads through driverTarget blindly here. Boss-thrown TNT can have
	// no target, and native cannot mirror PS1 low-memory null reads.
	if ((mw->stopFallAtY == 0x3fff) && (mw->driverTarget != NULL))
#else
	if (mw->stopFallAtY == 0x3fff)
#endif
	{
		mw->stopFallAtY = mw->driverTarget->instSelf->matrix.t[1];
	}

	if (inst->matrix.t[1] <= mw->stopFallAtY)
	{
		// plays tnt explosion sound 3D
		PlaySound3D(0x3d, inst);

		RB_Blowup_Init(inst);

		// Set scale (x, y, z) to zero
		inst->scale.z = 0;
		inst->scale.y = 0;
		inst->scale.x = 0;

		// make invisible
		inst->flags |= HIDE_MODEL;

		// this thread is now dead
		t->flags |= THREAD_FLAG_DEAD;

#if defined(CTR_NATIVE)
		// NOTE(aalhendi): Retail writes through driverTarget blindly; boss-thrown TNT has no
		// driver-owned instTntRecv slot to clear.
		if (mw->driverTarget != NULL)
		{
#endif
			mw->driverTarget->instTntRecv = 0;
#ifdef CTR_NATIVE
		}
#endif
	}

	// decrease velocity (artificial gravity)
	mw->velocity.y -= ((GAME_TRACKER->elapsedTimeMS << 2) >> 5);
	if (mw->velocity.y < -0x60)
	{
		mw->velocity.y = -0x60;
	}
	ThTick_FastRET(t);
}

// NOTE(aalhendi): The final scale sample shares the first two character heights.
// One backing array preserves that overlap without an out-of-bounds native read.
static const s16 s_tntTables[0x5a * 2 + 0x10] = {
    2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2355,
    2148, 2539, 2248, 2462, 2293, 2280, 2273, 2061, 2220, 1878, 2148, 1802, 2070, 1815, 1998, 1850, 1945, 1898, 1925, 1951, 1934, 1999, 1957, 2034, 1986,
    2048, 2038, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048, 2048,
    2048, 2560, 2214, 2867, 2381, 2739, 2457, 2434, 2423, 2070, 2335, 1766, 2215, 1638, 2085, 1649, 1965, 1679, 1877, 1728, 1843, 1791, 1857, 1867, 1899,
    1954, 1964, 2048, 2048, 2150, 2150, 2048, 2048, 1945, 1945, 2048, 2048, 2150, 2150, 2048, 2048, 1945, 1945, 2048, 2048, 2150, 2150, 2123, 2123, 2074,
    2074, 2048, 2048, 2728, 2270, 3137, 2491, 2967, 2592, 2562, 2532, 2078, 2381, 1673, 2184, 1503, 1987, 1620, 1836, 1878, 1775, 2136, 2014, 2253, 2253,
    2052, 2052, 1851, 1851, 2067, 2067, 2283, 2283, 1943, 1943, 2331, 2331, 1926, 1926, 2344, 2344, 1899, 1899, 2379, 2379, 1904, 1904, 2327, 2327, 1926,
    1926, 2379, 2379, 3072, 3072, 32,   32,   64,   32,   32,   48,   32,   32,   48,   64,   32,   48,   56,   24,   32,   56,
};

// Restore the crate's size and launch it clear of either a player or an AI driver.
static inline void RB_TNT_Detach(struct Thread *t, struct Instance *inst, struct MineWeapon *mw)
{
	// set scale (x, y, z)
	inst->scale.z = 0x800;
	inst->scale.y = 0x800;
	inst->scale.x = 0x800;

	mw->driverTarget->instTntRecv = 0;

	mw->velocity.x = 0;
	mw->velocity.y = 0x30;
	mw->velocity.z = 0;
	mw->deltaPos.x = 0;
	mw->deltaPos.y = 0;
	mw->deltaPos.z = 0;

	// assign RB_TNT_ThTick_ThrowOffHead
	ThTick_SetAndExec(t, RB_TNT_ThTick_ThrowOffHead);
}

void RB_TNT_ThTick_SitOnHead(struct Thread *t)
{
	struct Instance *inst;
	struct MineWeapon *mw;
	u8 state;
	s16 numFrames;
	u16 scaleXZ;
	int rng;

	inst = t->inst;

	// object (tnt)
	mw = inst->thread->object;

	// CopyMatrix
	// To: TNT instance
	// From: obj->driverWhoHitMe->instance
	// Delta: TNT -> 0x1c (position relative to driver)
	LHMatrix_Parent(inst, mw->driverTarget->instSelf, &mw->deltaPos);

	// Get Kart State
	state = mw->driverTarget->kartState;

	if ((state == KS_CRASHING) || (state == KS_MASK_GRABBED) || (state == KS_SPINNING))
	{
		// Play explosion sound
		PlaySound3D(0x3d, inst);

		RB_Blowup_Init(inst);

		goto removeTnt;
	}
	if (state == KS_BLASTED)
	{
		PlaySound3D(0x3d, inst);
		RB_Explosion_InitGeneric(inst);

	removeTnt:

		// reset TNT-related pointers
		inst->scale.z = 0;
		inst->scale.y = 0;
		inst->scale.x = 0;

		// make invisible
		inst->flags |= HIDE_MODEL;

		// this thread is now dead
		t->flags |= THREAD_FLAG_DEAD;

		mw->driverTarget->instTntRecv = 0;

		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): FastRET exits the retail tick; native uses a callback return.
		return;
#endif
	}

	if ((mw->driverTarget->actionsFlagSet & ACTION_BOT) != 0)
	{
		rng = MixRNG_Scramble();
		if (rng != (rng / 0x10e) * 0x10e)
		{
			goto advanceTimer;
		}
		RB_TNT_Detach(t, inst, mw);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native transitions can return; do not resume the old tick.
		return;
#endif
	}
	else
	{
		// if player did not start jumping this frame
		if ((mw->driverTarget->actionsFlagSet & ACTION_JUMP_STARTED) == 0)
		{
			goto advanceTimer;
		}

		if (mw->jumpsRemaining != 0)
		{
			mw->jumpsRemaining += -1;
			goto advanceTimer;
		}
		mw->jumpsRemaining = 0;
		RB_TNT_Detach(t, inst, mw);
#ifdef CTR_NATIVE
		return;
#endif
	}

advanceTimer:

	// Get how many frames the TNT has
	// been on top of someone's head
	numFrames = mw->numFramesOnHead;

	// If there is time remaining until TNT blows up,
	// which takes 0x5a frames, 3 seconds
	if (numFrames < 0x5a)
	{
		// If frame is any of these 6 numbers
		if ((numFrames == 0x0) || (numFrames == 0x14) || (numFrames == 0x28) || (numFrames == 0x3c) || (numFrames == 0x46) || (numFrames == 0x50))
		{
			// Make a "honk" sound
			PlaySound3D(0x3e, inst);
		}

		// add to the frame counter
		mw->numFramesOnHead += 1;
	}

	// If time runs out
	else
	{
		// Blow up

		RB_Hazard_HurtDriver(mw->driverTarget, 2, mw->instParent->thread->object, 0);

		// icon damage timer, draw icon as red
		mw->driverTarget->damageColorTimer = 0x1e;

		// play 3D sound for TNT explosion
		PlaySound3D(0x3d, inst);

		RB_Blowup_Init(inst);

		// this thread is now dead
		t->flags |= THREAD_FLAG_DEAD;

		mw->driverTarget->instTntRecv = NULL;
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): FastRET exits before the expired timer indexes the scale table.
		return;
#endif
	}

	// set scale of TNT, given frame of animation
	scaleXZ = s_tntTables[mw->numFramesOnHead * 2 + 0];
	inst->scale.z = scaleXZ;
	inst->scale.x = scaleXZ;
	{
		const s16 *row = s_tntTables + mw->numFramesOnHead * 2;
		inst->scale.y = row[1];
	}
}

void RB_TNT_ThTick_ThrowOnHead(struct Thread *t)
{
	struct MineWeapon *mw;
	struct Instance *inst;
	const s16 *array;

	SVec3 rot;
	s16 distHead;

	// temporary rotation matrix
	MATRIX localMatrix;

	inst = t->inst;

	// object
	mw = inst->thread->object;

	// alter height of TNT as it flies onto a driver's head,
	// do NOT use parenthesis
	mw->deltaPos.y += (mw->velocity.y * GAME_TRACKER->elapsedTimeMS) >> 5;

	// if TNT is moving downward
	if (mw->velocity.y < 0)
	{
		array = &s_tntTables[0x5a * 2];

		distHead = array[GAME_CHARACTER_IDS[mw->driverTarget->driverID]];

		// if TNT landed on head
		if ((mw->deltaPos.y < distHead) && (mw->deltaPos.y = distHead, inst->scale.x == 0x800))
		{
			// Set TNT timer to 0, it blows up at 0x5a
			mw->numFramesOnHead = 0;

			// A negative odd RNG result gives retail a ninth jump.
			mw->jumpsRemaining = 8 - (MixRNG_Scramble() % 2);

			// play sound that you hit a TNT
			PlaySound3D(0x51, inst);

			// NOTE(aalhendi): Keep the signed decrement before storing the wrapped byte.
			inst->depthBiasNormal = (s8)(mw->driverTarget->instSelf->depthBiasNormal - 1);
			inst->depthBiasSecondary = (s8)(mw->driverTarget->instSelf->depthBiasSecondary - 1);

			// assign
			ThTick_SetAndExec(t, RB_TNT_ThTick_SitOnHead);
#ifdef CTR_NATIVE
			// NOTE(aalhendi): Native transitions return; retail resumes the new tick directly.
			return;
#endif
		}
	}

	// CopyMatrix
	LHMatrix_Parent(inst, mw->driverTarget->instSelf, &mw->deltaPos);

	// rotation
	rot.x = 0;
	rot.y = mw->tntSpinY;
	rot.z = 0;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix(&localMatrix, &rot);

	MatrixRotate(&inst->matrix, &inst->matrix, &localMatrix);

	// reduce time remaining until TNT lands on head
	mw->velocity.y -= ((GAME_TRACKER->elapsedTimeMS << 2) >> 5);

	// set a minimum value (-0x60)
	if (mw->velocity.y < -0x60)
	{
		mw->velocity.y = -0x60;
	}

	// rotation
	mw->tntSpinY += 0x100;

	// if scale is large
	if (inst->scale.x > 0x800)
	{
		// reduce scale
		inst->scale.x -= 0x100;
		inst->scale.y -= 0x100;
		inst->scale.z -= 0x100;
	}
	else
	{
		// set min scale
		inst->scale.x = 0x800;
		inst->scale.y = 0x800;
		inst->scale.z = 0x800;
	}
	return;
}
