#include <common.h>

enum
{
	VEH_ITEM_THREE_MISSILES_MIN_PLAYERS = 3,
	VEH_ITEM_THREE_MISSILES_HELD_LIMIT = 2,
};

void VehPhysProc_PlayItemAcquireSound(const struct Driver *driver)
{
	const bool juiced = driver->numWumpas >= DRIVER_WUMPA_JUICED_COUNT;
	OtherFX_Play(juiced ? ITEMSET_ACQUIRED_JUICED_FX : ITEMSET_ACQUIRED_NORMAL_FX, juiced);
}

// Apply the same inventory bookkeeping to roulette and explicitly chosen items.
void VehPhysProc_AssignHeldItem(struct Driver *driver, u8 itemID)
{
	struct GameTracker *gGT = GAME_TRACKER;
	driver->heldItemID = itemID;
	if (driver->heldItemID == HELD_ITEM_WARPBALL)
	{
		if ((gGT->gameMode1 & WARPBALL_HELD) != 0)
		{
			driver->heldItemID = HELD_ITEM_MISSILE_3X;
		}
		else
		{
			gGT->gameMode1 |= WARPBALL_HELD;
		}
	}
	if (gGT->numPlyrCurrGame >= VEH_ITEM_THREE_MISSILES_MIN_PLAYERS &&
	    (gGT->gameMode1 & BATTLE_MODE) == 0 && driver->heldItemID == HELD_ITEM_MISSILE_3X)
	{
		if (gGT->numPlayersWith3Missiles < VEH_ITEM_THREE_MISSILES_HELD_LIMIT)
		{
			gGT->numPlayersWith3Missiles++;
		}
		else
		{
			driver->heldItemID = HELD_ITEM_MISSILE_1X;
		}
	}
	if ((u32)driver->heldItemID - HELD_ITEM_BOMB_3X < (HELD_ITEM_MISSILE_3X - HELD_ITEM_BOMB_3X + 1))
	{
		driver->numHeldItems = HELD_ITEM_STACK_COUNT;
	}
}

void VehPhysProc_RemoveHeldItem(struct Driver *driver)
{
	struct GameTracker *gGT = GAME_TRACKER;
	const bool hadOrb = driver->heldItemID == HELD_ITEM_WARPBALL;
	if (driver->heldItemID == HELD_ITEM_MISSILE_3X && (gGT->gameMode1 & BATTLE_MODE) == 0 &&
	    gGT->numPlyrCurrGame >= VEH_ITEM_THREE_MISSILES_MIN_PLAYERS && gGT->numPlayersWith3Missiles > 0)
	{
		gGT->numPlayersWith3Missiles--;
	}
	driver->heldItemID = HELD_ITEM_NONE;
	if (!hadOrb)
	{
		return;
	}
	// Discarding an inventory orb must preserve other holders and deployed orbs.
	for (int i = 0; i < (int)len(gGT->drivers); i++)
	{
		if (gGT->drivers[i] != NULL && gGT->drivers[i]->heldItemID == HELD_ITEM_WARPBALL)
		{
			return;
		}
	}
	for (const struct Thread *thread = gGT->threadBuckets[TRACKING].thread; thread != NULL; thread = thread->siblingThread)
	{
		if (thread->modelIndex == DYNAMIC_WARPBALL && (thread->flags & THREAD_FLAG_DEAD) == 0)
		{
			return;
		}
	}
	gGT->gameMode1 &= ~WARPBALL_HELD;
}
