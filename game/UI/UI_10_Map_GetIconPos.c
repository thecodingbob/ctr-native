#include <common.h>

// 488 / 760

// move to headers later
struct Map
{
	s16 worldEndX;
	s16 worldEndY;
	s16 worldStartX;
	s16 worldStartY;

	s16 iconSizeX;
	s16 iconSizeY;
	s16 iconStartX;
	s16 iconStartY;

	s16 mode;
};

void UI_Map_GetIconPos(s16 *m, int *posX, int *posY)

{
	s16 mode;
	int addX;
	int addY;
	int worldRangeX;
	int worldRangeY;

	struct Map *map = (struct Map *)m;

#if 0
  // trap() functions were removed from original,
  // we assume dividing by zero will never happen
#endif

	// rendering mode (forward, sideways, etc)
	mode = map->mode;

	worldRangeX = map->worldEndX - map->worldStartX;
	worldRangeY = map->worldEndY - map->worldStartY;

	if (mode == 0)
	{
		// 0 degrees
		addX = (*posX * map->iconSizeX) / worldRangeX;
		addY = (*posY * map->iconSizeY * 2) / worldRangeY;
	}

	else if (mode == 1)
	{
		// 90 degrees
		addX = -(*posY * map->iconSizeX) / worldRangeY;
		addY = (*posX * map->iconSizeY * 2) / worldRangeX;
	}

	else if (mode == 2)
	{
		// 180 degrees
		addX = -(*posX * map->iconSizeX) / worldRangeX;
		addY = -(*posY * map->iconSizeY * 2) / worldRangeY;
	}

	else
	{
		// 270 degrees
		addX = (*posY * map->iconSizeX) / worldRangeY;
		addY = -(*posX * map->iconSizeY * 2) / worldRangeX;
	}

	if (sdata->gGT->numPlyrCurrGame == 3)
	{
		addX -= 60;
		addY += 10;
	}

	*posX = map->iconStartX + addX;
	*posY = map->iconStartY + addY - 0x10;
	return;
}
