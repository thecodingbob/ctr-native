#include <common.h>

// NOTE(aalhendi): Retail retains the unused companion name beside the barrel name.
const struct
{
	char baron[8];
	char companion[12];
} rb_baronNames = {"baron", "drumbuddy"};

static inline const struct SpawnPosRot *RB_Baron_GetFrame(s32 pointIndex)
{
	const struct SpawnPosRot *base = GAME_TRACKER->level1->ptrSpawnType2_PosRot[0].coords.posRot;
	// NOTE(aalhendi): Keep the byte stride before the base in this 32-bit address calculation.
	return (const struct SpawnPosRot *)(pointIndex * sizeof(struct SpawnPosRot) + (u32)base);
}

void RB_Baron_ThTick(struct Thread *t)
{
	struct Instance *baronInst;
	struct Baron *baronObj;
	s32 numCoords;
	SVec3 rot;

	struct Driver *hitDriver;
	struct Instance *hitInst;

	baronInst = t->inst;
	baronObj = (struct Baron *)t->object;

	if ((baronInst->animFrame + 1) < INSTANCE_GetNumAnimFrames(baronInst, 0))
	{
		baronInst->animFrame++;
	}
	else
	{
		baronInst->animFrame = 0;
	}

	if (GAME_TRACKER->level1->numSpawnType2_PosRot == 0)
	{
		return;
	}

	numCoords = GAME_TRACKER->level1->ptrSpawnType2_PosRot[0].numCoords;
	baronObj->pointIndex = (baronObj->pointIndex + 1) % numCoords;

	// NOTE(aalhendi): Sample each component through the live path and point index;
	// caching a frame across the callbacks and stores changes retail's reloads.
	if (baronInst->model->id == DYNAMIC_DRUM)
	{
		if (baronObj->pointIndex == 0x10)
		{
			PlaySound3D(0xc, baronInst);
		}

		if (baronObj->pointIndex > 0x10)
		{
			PlaySound3D_Flags(&baronObj->soundID_flags, 0x74, baronInst);
		}
		else
		{
			OtherFX_RecycleMute(&baronObj->soundID_flags);
		}

		rot.x = RB_Baron_GetFrame(baronObj->pointIndex)->rot.x;
		rot.y = RB_Baron_GetFrame(baronObj->pointIndex)->rot.y;
		rot.z = RB_Baron_GetFrame(baronObj->pointIndex)->rot.z;
		ConvertRotToMatrix(&baronInst->matrix, &rot);

		baronInst->matrix.t[0] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.x;
		baronInst->matrix.t[1] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.y;
		baronInst->matrix.t[2] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.z;
		baronInst->matrix.t[0] += 0x111;
		baronInst->matrix.t[2] -= 0x110;
	}
	else
	{
		rot.x = RB_Baron_GetFrame(baronObj->pointIndex)->rot.x;
		rot.y = RB_Baron_GetFrame(baronObj->pointIndex)->rot.y;
		rot.z = RB_Baron_GetFrame(baronObj->pointIndex)->rot.z;
		rot.x = -rot.x;
		ConvertRotToMatrix(&baronInst->matrix, &rot);

		baronInst->matrix.t[0] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.x;
		baronInst->matrix.t[1] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.y;
		baronInst->matrix.t[2] = RB_Baron_GetFrame(baronObj->pointIndex)->pos.z;
	}

	if (baronObj->otherInst != 0)
	{
		// The companion follows the same path 120 samples ahead, offset diagonally.
		rot.x = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->rot.x;
		rot.y = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->rot.y;
		rot.z = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->rot.z;
		rot.x = -rot.x;
		ConvertRotToMatrix(&baronObj->otherInst->matrix, &rot);

		baronObj->otherInst->matrix.t[0] = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->pos.x;
		baronObj->otherInst->matrix.t[1] = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->pos.y;
		baronObj->otherInst->matrix.t[2] = RB_Baron_GetFrame((baronObj->pointIndex + 0x78) % numCoords)->pos.z;
		baronObj->otherInst->matrix.t[0] += 0x21f;
		baronObj->otherInst->matrix.t[2] -= 0x21f;
	}

	if (baronInst->model->id == DYNAMIC_DRUM)
	{
		hitInst = RB_Hazard_CollideWithDrivers(baronInst, 0, 0x19000, 0);
		if (hitInst != 0)
		{
			hitDriver = (struct Driver *)hitInst->thread->object;
			RB_Hazard_HurtDriver(hitDriver, 3, 0, 0);
		}
	}
}


void RB_Baron_LInB(struct Instance *inst)
{
	struct Baron *baronObj;
	struct Thread *t;
	s32 pointIndex;
	s32 nameLength;

	if (inst->thread != 0)
	{
		return;
	}

	t = PROC_BirthWithObject(
	    // creation flags
	    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Baron), NONE, SMALL, STATIC),

	    RB_Baron_ThTick,     // behavior
	    rb_baronNames.baron, // debug name
	    0                    // thread relative
	);

	inst->thread = t;
	if (t == 0)
	{
		return;
	}
	baronObj = t->object;
	t->inst = inst;

	nameLength = strlen(inst->name);
	if (inst->name[nameLength - 1] != '0')
	{
		pointIndex = 1;
	}
	else
	{
		pointIndex = GAME_TRACKER->level1->ptrSpawnType2->numCoords / 2;
	}

	baronObj->pointIndex = pointIndex;
	inst->scale.x = 0x1000;
	inst->scale.y = 0x1000;
	inst->scale.z = 0x1000;
	baronObj->unused1A = 4;
	baronObj->unused22 = 0x18;
	baronObj->unused06 = 0;
	baronObj->otherInst = 0;

	if (inst->model->id == DYNAMIC_VONLABASS)
	{
		inst->flags |= HIDE_MODEL;
	}

	baronObj->soundID_flags = 0;
}
