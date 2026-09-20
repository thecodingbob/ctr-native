#include <common.h>

#ifndef RB_MINE_POOL_TAKEN
#define RB_MINE_POOL_TAKEN D231.minePoolTaken
#define RB_MINE_POOL_FREE  D231.minePoolFree
#define RB_MINE_POOL_ITEMS D231.minePoolItem
#endif

static inline s32 RB_MinePool_Capacity(void)
{
	struct GameTracker *gGT = GAME_TRACKER;
	s32 count;

	if (gGT->gameMode1 & CRYSTAL_CHALLENGE)
	{
		// NOTE(aalhendi): Keep retail's 40-slot limit, including Nitro Court's
		// premature mine recycling; increasing it changes gameplay.
		count = 40;
	}
	else if ((gGT->gameMode1 & ADVENTURE_BOSS) && gGT->levelID == DRAGON_MINES)
	{
		count = 3;
	}
	else
	{
		// NOTE(aalhendi): Retail reloads resident state for the second boss check.
		struct GameTracker *currentGT = GAME_TRACKER_RELOAD();
		count = 10;
		if ((currentGT->gameMode1 & ADVENTURE_BOSS) && currentGT->levelID == ROO_TUBES)
		{
			count = 7;
		}
	}
	return count;
}

static inline void RB_MinePool_Fill(s16 numMines)
{
	s32 i;

	// add all mines
	i = 0;
	if (numMines > 0)
	{
		do
		{
			LIST_AddFront(&RB_MINE_POOL_FREE, &RB_MINE_POOL_ITEMS[i].item);
			i++;
		} while (i < numMines);
	}
}

void RB_MinePool_Init(void)
{
	LIST_Clear(&RB_MINE_POOL_TAKEN);
	LIST_Clear(&RB_MINE_POOL_FREE);
	RB_MinePool_Fill(RB_MinePool_Capacity());
}

void RB_MinePool_Remove(struct MineWeapon *mw)
{
	if (mw->weaponSlot231 == NULL)
	{
		return;
	}

	// remove from taken, add to free
	LIST_RemoveMember(&RB_MINE_POOL_TAKEN, (struct Item *)mw->weaponSlot231);
	LIST_AddFront(&RB_MINE_POOL_FREE, (struct Item *)mw->weaponSlot231);

	mw->boolDestroyed = 1;
	mw->weaponSlot231 = NULL;
}

void RB_MinePool_Add(struct MineWeapon *mw)
{
	struct WeaponSlot231 *ws;
	// if no more items on free list
	if (RB_MINE_POOL_FREE.count == 0)
	{
		// remove oldest mine
		RB_MinePool_Remove(((struct WeaponSlot231 *)RB_MINE_POOL_TAKEN.last)->mineWeapon);
	}

	// LIST_RemoveBack free list
	ws = (struct WeaponSlot231 *)LIST_RemoveBack(&RB_MINE_POOL_FREE);

	// link together
	ws->mineWeapon = mw;
	mw->weaponSlot231 = ws;

	// LIST_AddFront to taken list
	LIST_AddFront(&RB_MINE_POOL_TAKEN, (struct Item *)ws);
}
