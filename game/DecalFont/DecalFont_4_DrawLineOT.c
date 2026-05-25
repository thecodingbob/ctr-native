#include <common.h>

void DecalFont_DrawLineOT(char *str, int posX, int posY, s16 fontType, int flags, u_long *ot)
{
	struct GameTracker *gGT;
	u_long *backupOT;

	gGT = sdata->gGT;

	// backup
	backupOT = gGT->pushBuffer_UI.ptrOT;

	// alter
	gGT->pushBuffer_UI.ptrOT = ot;

	// draw
	DecalFont_DrawLine(str, posX, posY, fontType, flags);

	// reset
	gGT->pushBuffer_UI.ptrOT = backupOT;
}
