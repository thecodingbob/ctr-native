#include <common.h>

void RB_Follower_ProcessBucket(struct Thread *t)
{
	s32 i;
	s32 driverID;
	struct Follower *fObj;
	struct Instance *inst;

	for (/**/; t != 0; t = t->siblingThread)
	{
		// skip dead threads
		if ((t->flags & THREAD_FLAG_DEAD) != 0)
		{
			continue;
		}

		fObj = t->object;
		inst = t->inst;
		driverID = fObj->driver->driverID;

		// make Follower invisible to all other players
		for (i = 0; i < GAME_TRACKER->numPlyrNextGame; i++)
		{
			if (driverID != i)
			{
				inst->idpp[i].instFlags &= ~DRAW_SUCCESSFUL;
			}
		}

		// make Mine invisible to this player
		fObj->mineTh->inst->idpp[driverID].instFlags &= ~DRAW_SUCCESSFUL;
	}
}

void RB_Follower_ThTick(struct Thread *t)
{
	s32 kartState;
	struct Driver *d;
	struct Follower *fObj;
	struct Instance *inst;
	struct Thread *mine;

	fObj = t->object;
	d = fObj->driver;
	mine = fObj->mineTh;
	kartState = d->kartState;

	fObj->frameCount = CTR_MipsSubLo(fObj->frameCount, 1);

	if ((fObj->frameCount <= 0) || ((kartState != KS_NORMAL) && (kartState != KS_DRIFTING)) ||

	    // NOTE(aalhendi): The pool can reuse a destroyed mine's thread; check its generation.
	    (mine->timesDestroyed != fObj->backupTimesDestroyed) ||

	    (d->speedApprox < 0))
	{
		t->flags |= THREAD_FLAG_DEAD;
		return;
	}

	inst = t->inst;
	if (inst->scale.x < 0x800)
	{
		inst->scale.x = (u16)inst->scale.x << 1;
		inst->scale.y = (u16)inst->scale.y << 1;
		inst->scale.z = (u16)inst->scale.z << 1;
	}

	// midpoint between real mine position, and driver position
	inst->matrix.t[0] = (fObj->realPos.x + (d->posCurr.x >> 8)) >> 1;
	inst->matrix.t[1] = (fObj->realPos.y + (d->posCurr.y >> 8)) >> 1;
	inst->matrix.t[2] = (fObj->realPos.z + (d->posCurr.z >> 8)) >> 1;
}

void RB_Follower_Init(struct Driver *d, struct Thread *mineTh)
{
	struct Thread *t;
	struct Instance *followerInst;
	struct Follower *fObj;
	struct Instance *mineInst;

	// disable for slow speed
	if (d->speedApprox <= 0x1e00)
	{
		return;
	}

	// disable for AI
	if ((d->actionsFlagSet & ACTION_BOT) != 0)
	{
		return;
	}

	// disable for reverse camera
	if (((GAME_TRACKER->cameraDC[d->driverID].flags) & CAMERA_FLAG_REVERSE) != 0)
	{
		return;
	}

	// create a thread and an Instance
	followerInst = INSTANCE_BirthWithThread(mineTh->modelIndex, "follower", SMALL, FOLLOWER, RB_Follower_ThTick, sizeof(struct Follower), 0);

	if (followerInst == NULL)
	{
		return;
	}

	// grow the afterimage from one quarter of the normal scale
	followerInst->scale.x = 0x200;
	followerInst->scale.y = 0x200;
	followerInst->scale.z = 0x200;

	mineInst = mineTh->inst;

	memcpy(&followerInst->matrix, &mineInst->matrix, sizeof(followerInst->matrix));

	t = followerInst->thread;
	t->funcThDestroy = PROC_DestroyInstance;

	fObj = t->object;
	fObj->frameCount = 7;
	fObj->driver = d;
	fObj->mineTh = mineTh;
	fObj->backupTimesDestroyed = mineTh->timesDestroyed;

	// keep the original mine position as the other end of the afterimage
	fObj->realPos.x = mineInst->matrix.t[0];
	fObj->realPos.y = mineInst->matrix.t[1];
	fObj->realPos.z = mineInst->matrix.t[2];
}
