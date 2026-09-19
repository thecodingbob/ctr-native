#include "RB_Pickup.h"

SVec3 crystalLightDir = {0x94F, 0x94F, 0x94F};

static inline void RB_Crystal_RotateStep(struct Instance *crystalInst, struct Crystal *crystalObj)
{
	crystalObj->rot.y += 0x40;
	ConvertRotToMatrix(&crystalInst->matrix, &crystalObj->rot);
}

int RB_Crystal_ThCollide(struct Thread *crystalTh, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	SVec4 posWorld;
	union RBPickupScreen posScreen;
	struct Driver *driver;
	struct Instance *crystalInst;
	int modelID;
	(void)funcThCollide;


	modelID = sps->Input1.modelID;
	crystalInst = crystalTh->inst;

	// wumpa fruit or crystal can be grabbed
	// by player, or robotcar, and there's no
	// AIs in Crystal Challenge anyway
	if (
	    // player model
	    (modelID == DYNAMIC_PLAYER) ||

	    // bot model
	    (modelID == DYNAMIC_ROBOT_CAR))
	{
		// player gets pickup HUD feedback,
		// bots only erase the crystal/fruit
		driver = driverTh->object;
		if (modelID == DYNAMIC_PLAYER)
		{
			posWorld.x = (s16)crystalInst->matrix.t[0];
			posWorld.y = (s16)crystalInst->matrix.t[1];
			posWorld.z = (s16)crystalInst->matrix.t[2];
			RB_Pickup_SetCamera(driver);
			CTR_GteLoadPositionV0(&posWorld);
			gte_rtps();
			CTR_GteStorePositionXY(posScreen.coords);

			// lasts 5 frames, give start position, count numCollected
			driver->PickupWumpaHUD.startX = posScreen.coords[0] + GAME_TRACKER->pushBuffer[driver->driverID].rect.x;
			driver->PickupWumpaHUD.startY = posScreen.coords[1] + GAME_TRACKER->pushBuffer[driver->driverID].rect.y - 0x14;
			driver->PickupWumpaHUD.cooldown = 5;
			driver->PickupWumpaHUD.numCollected++;
		}

		crystalInst->scale.x = 0;
		crystalInst->scale.y = 0;
		crystalInst->scale.z = 0;
		crystalInst->thread = 0;

		// play sound
		PlaySound3D(0x43, crystalInst);
		crystalTh->flags |= THREAD_FLAG_DEAD;

		return 1;
	}
	return 0;
}

void RB_Crystal_ThTick(struct Thread *t)
{
	s32 heightOffset;
	struct Instance *crystalInst;
	struct Crystal *crystalObj;

	crystalInst = t->inst;
	crystalObj = t->object;

	// NOTE(aalhendi): Retail advances and rebuilds the rotation twice per tick.
	RB_Crystal_RotateStep(crystalInst, crystalObj);
	RB_Crystal_RotateStep(crystalInst, crystalObj);

	// sine curve for vertical bounce
	heightOffset = MATH_Sin(crystalObj->rot.y);
	heightOffset = ((s32)((u32)heightOffset << 4) >> 0xc) + 0x30;

	// set posY
	crystalInst->matrix.t[1] = crystalInst->instDef->pos.y + heightOffset;

	Vector_SpecLightSpin3D(crystalInst, &crystalObj->rot, &crystalLightDir);
}

int RB_Crystal_LInC(struct Instance *crystalInst, struct Thread *driverTh, struct ScratchpadStruct *sps)
{
	struct Thread *crystalTh;
	s32 result;

	crystalTh = crystalInst->thread;
	if (crystalTh == NULL)
	{
		crystalInst->thread = PROC_BirthWithObject(
		    // creation flags
		    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Crystal), NONE, SMALL, STATIC),

		    RB_Crystal_ThTick, // behavior
		    "crystal",         // debug name
		    0                  // thread relative
		);

		crystalTh = crystalInst->thread;
		if (crystalTh == NULL)
		{
			return 0;
		}

		crystalTh->inst = crystalInst;
		crystalTh->funcThCollide = (void *)RB_Crystal_ThCollide;
	}
	crystalTh = crystalInst->thread;

	if (crystalTh == NULL)
	{
		return 0;
	}

	if (crystalTh->funcThCollide == NULL)
	{
		return 0;
	}

	if (crystalInst->scale.x != 0)
	{
		result = ((ThreadScratchCollideFunc)crystalTh->funcThCollide)(crystalTh, driverTh, crystalTh->funcThCollide, sps);
	}
	else
	{
		result = 0;
	}
	return result;
}

void RB_Crystal_LInB(struct Instance *inst)
{
	struct Crystal *crystalObj;
	struct Thread *t;

	if (inst->thread == NULL)
	{
		t = PROC_BirthWithObject(
		    // creation flags
		    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Crystal), NONE, SMALL, STATIC),

		    RB_Crystal_ThTick, // behavior
		    "crystal",         // debug name
		    0                  // thread relative
		);

		inst->thread = t;
		if (t == 0)
		{
			return;
		}

		crystalObj = ((struct Crystal *)t->object);
		t->inst = inst;
		t->funcThCollide = (void *)RB_Crystal_ThCollide;

		// rotX, rotY, rotZ
		crystalObj->rot.x = 0;
		crystalObj->rot.y = 0;
		crystalObj->rot.z = 0;

		inst->colorRGBA = 0xd22fff0;

		inst->flags |= USE_SPECULAR_LIGHT;
	}

	RB_Default_LInB(inst);
}
