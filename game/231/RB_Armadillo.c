#include "RB_Collision.h"


int RB_Armadillo_ThCollide(struct Thread *armadilloThread, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	(void)armadilloThread;
	(void)driverTh;
	(void)funcThCollide;

	return sps->Input1.modelID == DYNAMIC_PLAYER;
}

void RB_Armadillo_ThTick_TurnAround(struct Thread *t)
{
	struct Instance *armInst;
	struct Armadillo *armObj;

	armObj = t->object;
	armInst = t->inst;

	do
	{
		if (armObj->rotCurr.y != armObj->rotDesired.y)
		{
			armObj->rotCurr.y = RB_Hazard_InterpolateValue(armObj->rotCurr.y, armObj->rotDesired.y, 0x100);
			ConvertRotToMatrix(&armInst->matrix, &armObj->rotCurr);
			armInst->animFrame++;
		}
		else if ((armInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(armInst, 0))
		{
			armInst->animFrame++;
		}
		else
		{
			// Reverse direction when the turn animation finishes.
			armObj->velX = -armObj->velX;
			armObj->numFramesSpinning = 0;
			armObj->velZ = -armObj->velZ;

			if (armObj->direction != 0)
			{
				PlaySound3D(0x70, armInst);
				armObj->direction = 0;
			}
			else
			{
				PlaySound3D(0x70, armInst);
				armObj->direction = 1;
			}

			armInst->animIndex = 1;
			armInst->animFrame = 0;
			ThTick_SetAndExec(t, RB_Armadillo_ThTick_Rolling);
		}

		RB_CheckHazardCollisions(armInst, t, 1, 0x2400, 0x71);
		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Armadillo_ThTick_Rolling(struct Thread *t)
{
	struct Instance *armInst;
	struct Armadillo *armObj;
	SVECTOR rot;

	armObj = t->object;
	armInst = t->inst;

	do
	{
		if (armObj->timeAtEdge != 0)
		{
			armObj->timeAtEdge--;
		}
		else if (armObj->timeRolling < 0x500)
		{
			// Advance the rolling phase by one 32 ms frame.
			armObj->timeRolling += 0x20;
			if (armObj->direction != 0)
			{
				armObj->distFromSpawn--;
			}
			else
			{
				armObj->distFromSpawn++;
			}

			armInst->matrix.t[0] += armObj->velX;
			armInst->matrix.t[2] += armObj->velZ;

			if ((armInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(armInst, 1))
			{
				armInst->animFrame++;
			}
			else
			{
				armInst->animFrame = 0;
			}

			RB_CheckHazardCollisions(armInst, t, 1, 0x2400, 0x71);
		}
		else
		{
			// Start the turn animation at the end of the rolling phase.
			CTR_MatrixToRot(&rot, &armInst->matrix, 0x11);
			armObj->rotCurr.x = rot.vy;
			armObj->rotCurr.y = rot.vx;
			armObj->rotCurr.z = rot.vz;
			armObj->timeRolling = 0;
			armInst->animIndex = 0;
			armInst->animFrame = 0;
			armObj->rotDesired.y = (armObj->rotCurr.y + 0x800) % 0x1000;
			ThTick_SetAndExec(t, RB_Armadillo_ThTick_TurnAround);
		}

		ThTick_FastRET(t);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

void RB_Armadillo_LInB(struct Instance *inst)
{
	struct Armadillo *armObj;
	SVECTOR rot;
	u8 *delays;
	void **pointers;
	struct Thread *t;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Armadillo), NONE, SMALL, STATIC), RB_Armadillo_ThTick_Rolling, "armadillo", NULL);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	// rolling animation
	inst->animIndex = 1;
	armObj = t->object;
	t->inst = inst;
	t->funcThCollide = (void *)RB_Armadillo_ThCollide;
	armObj->timeRolling = 0;
	armObj->numFramesSpinning = 0;

	CTR_MatrixToRot(&rot, &inst->matrix, 0x11);
	armObj->rotCurr.x = rot.vy;
	armObj->rotCurr.y = rot.vx;
	armObj->rotCurr.z = rot.vz;

	armObj->rotDesired.y = (armObj->rotCurr.y + 0x800) % 0x1000;

	armObj->spawnPosX = inst->matrix.t[0];
	armObj->spawnPosZ = inst->matrix.t[2];
	armObj->direction = 0;
	armObj->distFromSpawn = 0;

	armObj->velX = RB_ShiftRightS16(inst->matrix.m[0][2], 7);
	armObj->velZ = RB_ShiftRightS16(inst->matrix.m[2][2], 7);

	if (GAME_TRACKER->level1->ptrSpawnType1->count <= 0)
	{
		return;
	}

	// NOTE(aalhendi): The name's final digit selects a packed halfword delay to stagger instances.
	{
		s32 byteOffset = (inst->name[strlen(inst->name) - 1] - '0') * 2;
		pointers = ST1_GETPOINTERS(GAME_TRACKER->level1->ptrSpawnType1);
		delays = pointers[ST1_SPAWN];
		armObj->timeAtEdge = CTR_ReadU16AlignedLE(delays + byteOffset);
	}
}
