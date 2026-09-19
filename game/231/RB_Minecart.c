#include "RB_Collision.h"

extern s16 minecartArr[50];

static inline void RB_Minecart_SetDirection(struct Minecart *minecartObj)
{
	// NOTE(aalhendi): Keep the narrowed Z difference live through the pitch calculation.
	s16 dirZ;
	minecartObj->dir.x = minecartObj->posStart.x - minecartObj->posEnd.x;
	minecartObj->dir.y = minecartObj->posStart.y - minecartObj->posEnd.y;
	dirZ = minecartObj->posStart.z - minecartObj->posEnd.z;
	minecartObj->dir.z = dirZ;
	minecartObj->rotDesired.x = ratan2(minecartObj->dir.y, SquareRoot0_stub((u32)(minecartObj->dir.x * minecartObj->dir.x) + (u32)(dirZ * dirZ)));
	minecartObj->rotDesired.y = ratan2(minecartObj->dir.x, minecartObj->dir.z) - 0x800;
}

void RB_Minecart_ThTick(struct Thread *t)
{
	struct Instance *minecartInst;
	struct Minecart *minecartObj;
	struct Instance *hitInst;

	minecartInst = t->inst;
	minecartObj = t->object;

	// if animation is not over
	if ((minecartInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(minecartInst, 0))
	{
		// increment frame
		minecartInst->animFrame = minecartInst->animFrame + 1;
	}

	// if animation is done
	else
	{
		// reset animation
		minecartInst->animFrame = 0;
	}

	if (GAME_TRACKER->level1->numSpawnType2 == 0)
	{
		return;
	}

	// between two points
	if (minecartObj->betweenPoints_currFrame < minecartObj->betweenPoints_numFrames)
	{
		minecartObj->betweenPoints_currFrame++;
	}

	// reached point
	else
	{
		minecartObj->betweenPoints_currFrame = 1;

		// if not at end of path
		if (minecartObj->posIndex + 1 < GAME_TRACKER->level1->ptrSpawnType2->numCoords)
		{
			minecartObj->posIndex++;
			minecartObj->posStart.x = minecartObj->posEnd.x;
			minecartObj->posStart.y = minecartObj->posEnd.y;
			minecartObj->posStart.z = minecartObj->posEnd.z;
		}

		// end of path, reset
		else
		{
			minecartObj->posIndex = 1;
			minecartInst->matrix.t[0] = minecartObj->posStart.x = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[0].x;
			minecartInst->matrix.t[1] = minecartObj->posStart.y = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[0].y;
			minecartInst->matrix.t[2] = minecartObj->posStart.z = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[0].z;
		}

		minecartObj->posEnd.x = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].x;
		minecartObj->posEnd.y = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].y;
		minecartObj->posEnd.z = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].z;
		RB_Minecart_SetDirection(minecartObj);

		if ((minecartObj->posIndex == 1) && (minecartInst->model->id == DYNAMIC_MINE_CART))
		{
			minecartObj->rotCurr.x = minecartObj->rotDesired.x;
			minecartObj->rotCurr.y = minecartObj->rotDesired.y;
			minecartObj->rotCurr.z = minecartObj->rotDesired.z;
		}
	}

	// per-path depth bias
	minecartInst->depthBiasNormal = minecartArr[minecartObj->posIndex];
	minecartInst->depthBiasSecondary = minecartArr[minecartObj->posIndex];

	minecartInst->matrix.t[0] = minecartObj->posStart.x - (minecartObj->betweenPoints_currFrame * minecartObj->dir.x) / minecartObj->betweenPoints_numFrames;
	minecartInst->matrix.t[1] = minecartObj->posStart.y - (minecartObj->betweenPoints_currFrame * minecartObj->dir.y) / minecartObj->betweenPoints_numFrames;
	minecartInst->matrix.t[2] = minecartObj->posStart.z - (minecartObj->betweenPoints_currFrame * minecartObj->dir.z) / minecartObj->betweenPoints_numFrames;

	minecartObj->rotCurr.y = RB_Hazard_InterpolateValue(minecartObj->rotCurr.y, minecartObj->rotDesired.y, minecartObj->rotSpeed);
	minecartObj->rotCurr.x = RB_Hazard_InterpolateValue(minecartObj->rotCurr.x, minecartObj->rotDesired.x, minecartObj->rotSpeed);

	ConvertRotToMatrix(&minecartInst->matrix, &minecartObj->rotCurr);

	PlaySound3D_Flags(&minecartObj->soundIDCount,
	                  0x72, // minecart sound
	                  minecartInst);

	hitInst = RB_FindDriverCollision(minecartInst, t);
	if (hitInst != NULL)
	{
		RB_Hazard_HurtDriver(hitInst->thread->object, minecartInst->model->id == DYNAMIC_SKUNK ? 1 : 3, 0, 0);
	}
}

void RB_Minecart_LInB(struct Instance *inst)
{
	struct Minecart *minecartObj;
	struct Thread *t;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Minecart), NONE, SMALL, STATIC),

	    RB_Minecart_ThTick, // behavior
	    "minecart",         // debug name
	    0                   // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	minecartObj = t->object;
	t->inst = inst;

	memset(minecartObj, 0, sizeof(struct Minecart));

	// stagger the three carts across the shared path
	if (inst->name[strlen(inst->name) - 1] == '0')
	{
		minecartObj->posIndex = 1;
	}
	else if (inst->name[strlen(inst->name) - 1] - '0' == 1)
	{
		minecartObj->posIndex = GAME_TRACKER->level1->ptrSpawnType2->numCoords / 3;
	}
	else
	{
		minecartObj->posIndex = (s32)((u32)GAME_TRACKER->level1->ptrSpawnType2->numCoords << 1) / 3;
	}

	inst->scale.x = 0x1000;
	inst->scale.y = 0x1000;
	inst->scale.z = 0x1000;
	minecartObj->betweenPoints_numFrames = 8;
	minecartObj->rotSpeed = 0x20;

	if (inst->model->id == DYNAMIC_SKUNK)
	{
		inst->scale.x = 0x2000;
		inst->scale.y = 0x2000;
		inst->scale.z = 0x2000;
		minecartObj->betweenPoints_numFrames = 4;
		minecartObj->rotSpeed = 0x18;
	}

	else if (inst->model->id == DYNAMIC_VONLABASS)
	{
		inst->scale.x = 0x800;
		inst->scale.y = 0x800;
		inst->scale.z = 0x800;
		minecartObj->betweenPoints_numFrames = 4;
		minecartObj->rotSpeed = 0x18;
	}

	minecartObj->betweenPoints_currFrame = 0;
	minecartObj->posStart.x = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex - 1].x;
	minecartObj->posStart.y = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex - 1].y;
	minecartObj->posStart.z = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex - 1].z;
	minecartObj->posEnd.x = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].x;
	minecartObj->posEnd.y = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].y;
	minecartObj->posEnd.z = GAME_TRACKER->level1->ptrSpawnType2->coords.positions[minecartObj->posIndex].z;
	RB_Minecart_SetDirection(minecartObj);
	minecartObj->rotDesired.z = 0;
	minecartObj->soundIDCount = 0;
}

s16 minecartArr[50] = {0xC,  0xC,  0xC,  0xC,  0xC,  0xC,  0x6,  0x6,  0xC,  0xC,  0x9,  0x9,  0xC,  0xC,  0xC, 0x18, 0x18,
                       0x18, 0x1A, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0x18, 0xC,  0xC, 0xC,  0xC,
                       0xC,  0xC,  0xC,  0xC,  0xC,  0x0,  0x0,  0x6,  0x6,  0x6,  0x18, 0x18, 0x18, 0x18, 0xC, 0xC};
