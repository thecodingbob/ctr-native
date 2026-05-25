#include <common.h>

void UI_DrawNumCrystal(s16 posX, s16 posY, struct Driver *d)
{
	int numCrystalsOwned;
	int numCrystalsTotal;
	struct GameTracker *gGT;
	char string[8];

	DecalFont_DrawLine(&sdata->s_x[0], posX, posY + 4, FONT_SMALL, ORANGE);

	gGT = sdata->gGT;
	numCrystalsOwned = d->numCrystals;
	numCrystalsTotal = gGT->numCrystalsInLEV;

	sprintf(&string[0],
#ifndef REBUILD_PS1
	        &rdata.s_lapString[0],
#else
	        "%2.02d/%ld",
#endif
	        numCrystalsOwned, numCrystalsTotal);

	DecalFont_DrawLine(&string[0], posX + 0xD, posY, FONT_BIG, ORANGE);
}
