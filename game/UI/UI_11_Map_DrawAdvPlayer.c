#include <common.h>

// Draw dot for Player on 2D Adv Map
void UI_Map_DrawAdvPlayer(int ptrMap, int *matrix, int unused1, int unused2, s16 param_5, s16 param_6)

{
	int *arrowColor;
	int posX;
	int posY;

	posX = *matrix;
	posY = matrix[2];

	UI_Map_GetIconPos((s16 *)ptrMap, &posX, &posY);

	arrowColor = &data.playerIconAdvMap.vertCol1[0];
	if ((sdata->gGT->timer & 2) != 0)
	{
		arrowColor = &data.playerIconAdvMap.vertCol2[0];
	}

	AH_Map_HubArrow(posX, posY, &data.playerIconAdvMap.unk_playerAdvMap[0], (char *)arrowColor, (int)param_6, (int)param_5);

	return;
}
