#include <common.h>

// param1 pointer to array of two shorts (x,y)
void UI_Lerp2D_HUD(s16 *ptrPos, s16 startX, s16 startY, s16 endX, s16 endY, int curFrame, s16 endFrame)
{
	int endFrameInt;
	int newPosX;

	newPosX = curFrame * ((int)startX - (int)endX);
	endFrameInt = (int)endFrame;

	// newPosY
	curFrame = curFrame * ((int)startY - (int)endY);

	*ptrPos = endX + (s16)(newPosX / endFrameInt);
	ptrPos[1] = endY + (s16)(curFrame / endFrameInt);
	return;
}
