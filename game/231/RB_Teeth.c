#include <common.h>

void RB_Teeth_LInB(struct Instance *inst)
{
	inst->depthBiasNormal += 2;

	// Relic races leave the shortcut open without requiring a weapon.
	if ((GAME_TRACKER->gameMode1 & RELIC_RACE) != 0)
	{
		inst->flags |= HIDE_MODEL;
		GAME_DOOR_ACCESS_FLAGS |= 1;
	}
}

void RB_Teeth_BSP_Callback(struct ScratchpadStruct *sps, void *hitObject)
{
	struct Thread *weaponThread = hitObject;
	s16 model;
	struct Thread *teethTh;
	struct Instance *weaponInst;
	struct Instance *teethInst;

	model = weaponThread->modelIndex;

	switch (model)
	{
	case DYNAMIC_PLAYER:
	case PU_EXPLOSIVE_CRATE:
	case DYNAMIC_POISON:
	case STATIC_CRATE_TNT:
	{
		break;
	}
	default:
	{
		return;
	}
	}

	teethTh = sps->Union.ThBuckColl.thread;

	weaponInst = weaponThread->inst;

	teethInst = teethTh->inst;

	if ((weaponInst != NULL) && (teethInst != NULL))
	{
		s32 doorSideDistance = CTR_MipsAddLo(CTR_MipsMulLo(CTR_MipsSubLo(sps->Input1.pos.x, weaponInst->matrix.t[0]), teethInst->matrix.m[0][2]),
		                                     CTR_MipsMulLo(CTR_MipsSubLo(sps->Input1.pos.z, weaponInst->matrix.t[2]), teethInst->matrix.m[2][2]));

		// Distance along the door normal, retaining the CPU's word wrapping.
		if (doorSideDistance < 0)
		{
			doorSideDistance = CTR_MipsNegLo(doorSideDistance);
		}

		if ((doorSideDistance >> 12) > 0x100)
		{
			return;
		}
	}

	((struct Teeth *)sps->Union.ThBuckColl.thread->object)->direction = TEETH_DIRECTION_OPENING;
}

void RB_Teeth_ThTick(struct Thread *t)
{
	struct ScratchpadStruct *sps;
	u16 z;
	struct Teeth *teeth;
	struct Instance *inst;

	teeth = t->object;
	inst = t->inst;

	if (teeth->direction != TEETH_DIRECTION_IDLE)
	{
		s32 numAnimFrames;

		inst->animFrame = (u32)inst->animFrame + (u32)teeth->direction;
		numAnimFrames = VehFrameInst_GetNumAnimFrames(inst, 0);
		if (inst->animFrame >= numAnimFrames)
		{
			inst->animFrame = (u32)numAnimFrames - 1;
			teeth->direction = TEETH_DIRECTION_IDLE;
			teeth->timeOpen = 0x780;
		}
		else if (inst->animFrame < 0)
		{
			inst->animFrame = 0;
			teeth->direction = TEETH_DIRECTION_IDLE;
			teeth->timeOpen = 0;
			GAME_DOOR_ACCESS_FLAGS &= ~1u;
		}
	}
	else
	{
		s32 remainingOpenTime;

		if (teeth->timeOpen == 0)
		{
			goto checkDirection;
		}

		remainingOpenTime = CTR_MipsSubLo(teeth->timeOpen, GAME_TRACKER->elapsedTimeMS);
		teeth->timeOpen = remainingOpenTime;
		if (remainingOpenTime > 0)
		{
			goto checkObstruction;
		}

		PlaySound3D(0x75, inst);
		teeth->timeOpen = 0;
		teeth->direction = TEETH_DIRECTION_CLOSING;
	}

	if (teeth->timeOpen == 0)
	{
	checkDirection:
		if (teeth->direction >= TEETH_DIRECTION_IDLE)
		{
			goto updateVisibility;
		}
	}

checkObstruction:
	// Players and mines keep the door from closing through them.
	sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
	sps->Input1.pos.x = inst->matrix.t[0];
	sps->Input1.pos.y = inst->matrix.t[1];
	z = inst->matrix.t[2];
	sps->Input1.hitRadius = 0x300;
	sps->Input1.hitRadiusSquared = 0x90000;
	sps->Union.ThBuckColl.thread = t;
	sps->Union.ThBuckColl.funcCallback = RB_Teeth_BSP_Callback;
	sps->Input1.modelID = STATIC_TEETH;
	sps->Input1.pos.z = z;

	PROC_CollideHitboxWithBucket(GAME_TRACKER->threadBuckets[PLAYER].thread, sps, 0);
	PROC_CollideHitboxWithBucket(GAME_TRACKER->threadBuckets[MINE].thread, sps, 0);

updateVisibility:
	// Only the fully open door is hidden.
	if (teeth->timeOpen != 0)
	{
		inst->flags |= HIDE_MODEL;
	}
	else
	{
		inst->flags &= ~HIDE_MODEL;
	}
}

int RB_Teeth_LInC(struct Instance *teethInst, struct Thread *t, struct ScratchpadStruct *sps)
{
	struct Thread *teethTh;
	struct Teeth *teeth;
	s32 distance;
	struct ScratchpadStruct *input;

	// The relic-race shortcut ignores collision, even before a thread exists.
	if (GAME_TRACKER->gameMode1 & RELIC_RACE)
	{
		goto ignoreCollision;
	}
	teethTh = teethInst->thread;
	input = sps;
	if (teethTh == NULL)
	{
		teethInst->thread = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Teeth), NONE, SMALL, STATIC), RB_Teeth_ThTick, "teeth", NULL);
		teethTh = teethInst->thread;
		if (teethTh == NULL)
		{
			goto ignoreCollision;
		}
		teeth = teethTh->object;
		teethTh->inst = teethInst;
		teeth->direction = TEETH_DIRECTION_IDLE;
		teeth->timeOpen = 0;
	}
	teeth = teethTh->object;
	if (input->Input1.modelID == DYNAMIC_PLAYER)
	{
		if (((struct Driver *)t->object)->actionsFlagSet & ACTION_MASK_WEAPON)
		{
			RB_Teeth_OpenDoor(teethInst);
		}

		// NOTE(aalhendi): Retail rechecks the collision input after opening the door.
		if (input->Input1.modelID == DYNAMIC_PLAYER)
		{
			return 2;
		}
	}
	if (teeth->timeOpen != 0)
	{
	ignoreCollision:
		return 2;
	}
	distance = CTR_MipsAddLo(CTR_MipsMulLo(CTR_MipsSubLo(input->Input1.pos.x, teethInst->matrix.t[0]), teethInst->matrix.m[0][2]),
	                         CTR_MipsMulLo(CTR_MipsSubLo(input->Input1.pos.z, teethInst->matrix.t[2]), teethInst->matrix.m[2][2]));
	if (distance < 0)
	{
		distance = CTR_MipsNegLo(distance);
	}
	if ((distance >> 12) >= 0x81)
	{
		return 2;
	}
	return 1;
}

void RB_Teeth_OpenDoor(struct Instance *inst)
{
	struct Teeth *teeth;
	struct Thread *teethTh = inst->thread;
	if (teethTh == NULL)
	{
		inst->thread = PROC_BirthWithObject(SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Teeth), NONE, SMALL, STATIC), RB_Teeth_ThTick, "teeth", NULL);
		teethTh = inst->thread;
		if (teethTh == NULL)
		{
			return;
		}
		teeth = teethTh->object;
		teethTh->inst = inst;
		teeth->timeOpen = 0;
	}
	PlaySound3D(0x75, inst);
	teeth = teethTh->object;
	teeth->direction = TEETH_DIRECTION_OPENING;
	// Collision access is enabled immediately; the opening animation follows.
	GAME_DOOR_ACCESS_FLAGS |= 1;
}
