#include <common.h>

void MM_Battle_DrawIcon_Character(struct Icon *icon, int posX, int posY, struct PrimMem *primMem, u_long *ot, char transparency, s16 scale)
{
	if (icon == 0)
		return;
	DecalHUD_DrawPolyFT4(icon, posX, posY, primMem, ot, transparency, scale);
}
