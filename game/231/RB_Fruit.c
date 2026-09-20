#include "RB_Pickup.h"

void RB_Fruit_ThTick(struct Thread *fruitTh)
{
	struct Instance *fruitInst;

	fruitInst = fruitTh->inst;
	do
	{
		fruitInst->thread = NULL;
		fruitTh->flags |= THREAD_FLAG_DEAD;
		ThTick_FastRET(fruitTh);
#ifdef CTR_NATIVE
		// NOTE(aalhendi): Native ticks return as callbacks; retail yields through FastRET.
		return;
#endif
	} while (1);
}

int RB_Fruit_ThCollide(struct Thread *fruitTh, struct Thread *driverTh, void *funcThCollide, struct ScratchpadStruct *sps)
{
	SVec4 posWorld;
	union RBPickupScreen posScreen;
	struct Driver *driver;
	struct Instance *fruitInst;
	struct Fruit *fruitObj;
	int modelID;
	(void)funcThCollide;
	fruitObj = fruitTh->object;
	modelID = sps->Input1.modelID;
	fruitInst = fruitTh->inst;

	// wumpa fruit can be grabbed by players and robotcars
	if ((modelID == DYNAMIC_PLAYER) || (modelID == DYNAMIC_ROBOT_CAR))
	{
		driver = driverTh->object;
		if (modelID == DYNAMIC_PLAYER)
		{
			posWorld.x = (s16)fruitInst->matrix.t[0];
			posWorld.y = (s16)fruitInst->matrix.t[1];
			posWorld.z = (s16)fruitInst->matrix.t[2];
			RB_Pickup_SetCamera(driver);
			CTR_GteLoadPositionV0(&posWorld);
			gte_rtps();
			CTR_GteStorePositionXY(posScreen.coords);

			driver->PickupWumpaHUD.startX = posScreen.coords[0] + GAME_TRACKER->pushBuffer[driver->driverID].rect.x;
			driver->PickupWumpaHUD.startY = posScreen.coords[1] + GAME_TRACKER->pushBuffer[driver->driverID].rect.y - 0x14;
			driver->PickupWumpaHUD.cooldown = 5;
			driver->PickupWumpaHUD.numCollected++;
		}

		fruitObj->driver = driver;

		fruitInst->scale.x = 0;
		fruitInst->scale.y = 0;
		fruitInst->scale.z = 0;
		fruitInst->thread = NULL;

		PlaySound3D(0x43, fruitInst);
		fruitTh->flags |= THREAD_FLAG_DEAD;

		return 1;
	}
	return 0;
}

void RB_Fruit_LInB(struct Instance *inst)
{
	RB_Default_LInB(inst);
	inst->animIndex = 0;
	inst->flags |= ANIM_LOOP;
}

int RB_Fruit_LInC(struct Instance *fruitInst, struct Thread *driverTh, struct ScratchpadStruct *sps)
{
	struct Thread *fruitTh;

	fruitTh = fruitInst->thread;
	if (fruitTh == NULL)
	{
		fruitInst->thread = PROC_BirthWithObject(
		    // creation flags
		    SIZE_RELATIVE_POOL_BUCKET(sizeof(struct Fruit), NONE, SMALL, STATIC),

		    RB_Fruit_ThTick, // behavior
		    "fruit",         // debug name
		    0                // thread relative
		);

		fruitTh = fruitInst->thread;
		if (fruitTh == NULL)
		{
			return 0;
		}

		fruitTh->inst = fruitInst;
		fruitTh->funcThCollide = (void *)RB_Fruit_ThCollide;
	}

	fruitTh = fruitInst->thread;
	if ((fruitTh == NULL) || (fruitTh->funcThCollide == NULL))
	{
		return 0;
	}

	if (fruitInst->scale.x == 0)
	{
		return 0;
	}

	return ((ThreadScratchCollideFunc)fruitTh->funcThCollide)(fruitTh, driverTh, fruitTh->funcThCollide, sps);
}
