#include <common.h>

void AH_Map_HubArrow(int posX, int posY, s16 *vertPos, char *vertCol, int unk800, int angle)
{
	s16 local_30[6];
	s16 local_20[6];

	struct GameTracker *gGT = sdata->gGT;

	int sin = MATH_Sin(angle);
	int cos = MATH_Cos(angle);

	for (int i = 0; i < 3; i++)
	{
		local_30[i * 2 + 0] = posX + 6 +
		                      (s16)(((((vertPos[2 * i + 0] * cos) >> 0xc) + ((vertPos[2 * i + 1] * sin) >> 0xc)) * ((unk800 * 8) / 5)

		                                 ) >>
		                            0xc);

		local_30[i * 2 + 1] = posY + 4 +
		                      (s16)(((((vertPos[2 * i + 1] * cos) >> 0xc) - ((vertPos[2 * i + 0] * sin) >> 0xc)) * unk800

		                             ) >>
		                            0xc);
	}

	s16 *offset = &D232.primOffsetXY_HubArrow[0];

	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 3; j++)
		{
			local_20[j * 2 + 0] = local_30[j * 2 + 0] + offset[i * 2 + 0];

			local_20[j * 2 + 1] = local_30[j * 2 + 1] + offset[i * 2 + 1];
		}

		RECTMENU_DrawRwdTriangle(&local_20[0], vertCol, gGT->pushBuffer_UI.ptrOT, &gGT->backBuffer->primMem);

		vertCol = (char *)&D232.colorTri[0];
	}
}
