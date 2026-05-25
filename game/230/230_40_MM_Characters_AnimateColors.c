#include <common.h>
void MM_Characters_AnimateColors(u8 *colorData, s16 playerID, s16 flag)
{
	u8 colorAdjustmentValue;
	u32 trigApproximationIndex;
	u32 trigApprox;

	// access int RGBA as a char array,
	// for editing components of color
	u8 *ptrColor;
	ptrColor = (u8 *)data.ptrColor[playerID + PLAYER_BLUE];

	trigApprox = 0;

	// if player has not selected character yet
	// see MM_Characters_MenuProc
	if (flag == 0)
	{
		trigApproximationIndex = sdata->frameCounter * 0x100 + playerID * 0x400;

		// approximate trigonometry
		trigApprox = *(u32 *)&data.trigApprox[trigApproximationIndex & 0x3ff];

		if ((trigApproximationIndex & 0x400) == 0)
		{
			trigApprox = trigApprox << 0x10;
		}
		trigApprox = trigApprox >> 0x10;

		if ((trigApproximationIndex & 0x800) != 0)
			trigApprox = -(int)trigApprox;
	}

	colorAdjustmentValue = 0;
	if (0xc00 < (int)trigApprox)
	{
		colorAdjustmentValue = ((trigApprox << 7) >> 0xc);
	}

	colorData[0] = ptrColor[0] | colorAdjustmentValue;
	colorData[1] = ptrColor[1] | colorAdjustmentValue;
	colorData[2] = ptrColor[2] | colorAdjustmentValue;
	colorData[3] = 0;

	return;
}
