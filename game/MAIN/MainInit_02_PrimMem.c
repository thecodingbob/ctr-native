#include <common.h>

static int MainInit_GetPrimMemSize(struct GameTracker *gGT)
{
	int levelID;

	// adv garage
	if (gGT->levelID == ADVENTURE_GARAGE)
		return 0x1b800;

	// main menu
	if ((gGT->gameMode1 & MAIN_MENU) != 0)
		return 0x17c00;

	levelID = gGT->levelID;

	switch (gGT->numPlyrCurrGame)
	{
	case 0:
		return 0x25800;

	case 1:
		if ((gGT->gameMode1 & ADVENTURE_ARENA) != 0)
			return 0x1c000;

		if ((u32)(levelID - INTRO_RACE_TODAY) < 9)
			return 0x1e000;

		if (levelID < GEM_STONE_VALLEY)
			return data.primMem_SizePerLEV_1P[levelID] << 10;

		return 0x17c00;

	case 2:
		if (levelID < GEM_STONE_VALLEY)
			return data.primMem_SizePerLEV_2P[levelID] << 10;

		return 0x1e000;

	case 3:
	case 4:
		if (levelID < GEM_STONE_VALLEY)
			return data.primMem_SizePerLEV_4P[levelID] << 10;

		return 0x25800;

	default:
		return 0;
	}
}

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8003b0f0-0x8003b2d4.
void MainInit_PrimMem(struct GameTracker *gGT)
{
	int size = MainInit_GetPrimMemSize(gGT);

	if (size == 0)
		return;

	MainDB_PrimMem(&gGT->db[0].primMem, size);
	MainDB_PrimMem(&gGT->db[1].primMem, size);
}
