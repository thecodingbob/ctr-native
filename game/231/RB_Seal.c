#include "RB_Collision.h"

int RB_Seal_ThCollide(struct Thread *sealThread, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	(void)sealThread;
	(void)driverTh;
	(void)funcThCollide;

	return sps->Input1.modelID == DYNAMIC_PLAYER;
}

void RB_Seal_ThTick_TurnAround(struct Thread *t)
{
	struct Instance *sealInst;
	struct Seal *sealObj;

	sealInst = t->inst;
	sealObj = (struct Seal *)t->object;

	// if animation is not over
	if ((sealInst->animFrame + 2) < INSTANCE_GetNumAnimFrames(sealInst, 0))
	{
		// increment frame
		sealInst->animFrame = sealInst->animFrame + 2;
	}

	// if animation is done
	else
	{
		// reset animation
		sealInst->animFrame = 0;

		// only play sound in TurnAround
		PlaySound3D(0x77, sealInst);
	}

	if (sealObj->rotCurr.y != sealObj->turnAroundRot.y)
	{
		// spin rotCurrY 180 degrees (turn around)
		sealObj->rotCurr.y = RB_Hazard_InterpolateValue(sealObj->rotCurr.y, sealObj->turnAroundRot.y, 0x80);

		// negate rotCurrX (slant)
		sealObj->rotCurr.x = RB_Hazard_InterpolateValue(sealObj->rotCurr.x, -sealObj->rotDesired.x, 0x14);

		// negate rotCurrZ (slant)
		sealObj->rotCurr.z = RB_Hazard_InterpolateValue(sealObj->rotCurr.z, -sealObj->rotDesired.z, 0x14);

		sealObj->numFramesSpinning++;

		ConvertRotToMatrix(&sealInst->matrix, &sealObj->rotCurr);
	}
	else
	{
		// Retain the new slant as the starting orientation for the next turn.
		sealObj->numFramesSpinning = 0;
		sealObj->rotDesired.x = sealObj->rotCurr.x;
		sealObj->rotDesired.y = sealObj->rotCurr.y;
		sealObj->rotDesired.z = sealObj->rotCurr.z;
		ConvertRotToMatrix(&sealInst->matrix, &sealObj->rotCurr);
		ThTick_SetAndExec(t, RB_Seal_ThTick_Move);
	}

	RB_CheckHazardCollisions(sealInst, t, 1, 0x4000, 0x78);
}

void RB_Seal_ThTick_Move(struct Thread *t)
{
	struct Instance *sealInst;
	struct Seal *sealObj;
	s32 turnRot;

	sealInst = t->inst;
	sealObj = (struct Seal *)t->object;

	// if animation is not over
	if ((sealInst->animFrame + 2) < INSTANCE_GetNumAnimFrames(sealInst, 0))
	{
		// increment frame
		sealInst->animFrame = sealInst->animFrame + 2;
	}

	// if animation is done
	else
	{
		// reset animation
		sealInst->animFrame = 0;

		// no sound here
	}

	// Interpolate the three position axes along the 45-frame path.
	sealInst->matrix.t[0] = sealObj->spawnPos.x - (sealObj->distFromSpawn * sealObj->vel.x) / 0x2d;
	sealInst->matrix.t[1] = sealObj->spawnPos.y - (sealObj->distFromSpawn * sealObj->vel.y) / 0x2d;
	sealInst->matrix.t[2] = sealObj->spawnPos.z - (sealObj->distFromSpawn * sealObj->vel.z) / 0x2d;

	// Moving away from spawn.
	if (sealObj->direction != 0)
	{
		if (sealObj->distFromSpawn < 0x2d)
		{
			sealObj->distFromSpawn++;
			goto CheckCollisions;
		}

		if (sealObj->distFromSpawn != 0x2d)
		{
			goto CheckCollisions;
		}

		turnRot = sealObj->rotCurr.y;
		sealObj->direction = 0;
	}

	// Moving towards spawn.
	else
	{
		if (sealObj->distFromSpawn > 0)
		{
			sealObj->distFromSpawn--;
			goto CheckCollisions;
		}

		if (sealObj->distFromSpawn != 0)
		{
			goto CheckCollisions;
		}

		turnRot = sealObj->rotCurr.y;
		sealObj->direction = 1;
	}

	// Turn through half a revolution at either endpoint.
	sealObj->turnAroundRot.y = (turnRot + 0x800) % 0x1000;
	ThTick_SetAndExec(t, RB_Seal_ThTick_TurnAround);

CheckCollisions:
	RB_CheckHazardCollisions(sealInst, t, 1, 0x4000, 0x78);
}

void RB_Seal_LInB(struct Instance *inst)
{
	struct Seal *sealObj;
	struct Thread *t;
	s32 sealID;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Seal), NONE, SMALL, STATIC),

	    RB_Seal_ThTick_Move, // behavior
	    "seal",              // debug name
	    0                    // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	sealObj = t->object;
	t->inst = inst;
	t->funcThCollide = (void *)RB_Seal_ThCollide;

	inst->scale.x = 0x2000;
	inst->scale.y = 0x2000;
	inst->scale.z = 0x2000;

	sealObj->distFromSpawn = 0;
	sealObj->direction = 1;
	sealID = inst->name[strlen(inst->name) - 1] - '0';
	sealObj->sealID = sealID;

	if (GAME_TRACKER->level1->numSpawnType2 != 0)
	{
		sealObj->spawnPos = GAME_TRACKER->level1->ptrSpawnType2[sealID].coords.positions[0];
		sealObj->endPos = GAME_TRACKER->level1->ptrSpawnType2[sealObj->sealID].coords.positions[1];
	}

	// distance between points
	sealObj->vel.x = sealObj->spawnPos.x - sealObj->endPos.x;
	sealObj->vel.y = sealObj->spawnPos.y - sealObj->endPos.y;
	sealObj->vel.z = sealObj->spawnPos.z - sealObj->endPos.z;

	// rotCurr
	sealObj->rotCurr.x = inst->instDef->rot.x;
	sealObj->rotCurr.z = inst->instDef->rot.z;
	sealObj->rotCurr.y = inst->instDef->rot.y;
	sealObj->rotDesired.x = sealObj->rotCurr.x;
	sealObj->rotDesired.y = sealObj->rotCurr.y;
	sealObj->rotDesired.z = sealObj->rotCurr.z;

	sealObj->numFramesSpinning = 0;

	ConvertRotToMatrix(&inst->matrix, &sealObj->rotCurr);

	// dont call RB_Default_LInB(inst),
	// we know seal is never over ice
}
