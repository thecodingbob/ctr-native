#include <common.h>

enum
{
	UI_LERP2D_RANK_ICON_RADIUS = 0x14,
	UI_LERP2D_RANK_ICON_BASE_Y = 0x39,
	UI_LERP2D_RANK_ICON_SLOT_H = 0x1b,
	UI_LERP2D_ANGULAR_FRAME_SHIFT = 0xb,
	UI_LERP2D_FIXED_SHIFT = 0xc,
	UI_LERP2D_TRANSITION_FRAMES = 5,
};

void UI_Lerp2D_Angular(SVec2 *pos, s16 drawnPosition, s16 absolutePosition, s16 frameCounter)
{
	s32 drawnPositionInt = drawnPosition;
	s32 absolutePositionInt = absolutePosition;

	// NOTE(aalhendi): Keep each direction's sine sample and position update together for retail scheduling.
	if (absolutePositionInt < drawnPositionInt)
	{
		pos->x = UI_LERP2D_RANK_ICON_RADIUS +
		         (MATH_Sin((frameCounter * (1 << UI_LERP2D_ANGULAR_FRAME_SHIFT)) / UI_LERP2D_TRANSITION_FRAMES) * UI_LERP2D_RANK_ICON_RADIUS >>
		          UI_LERP2D_FIXED_SHIFT);
		pos->y = drawnPositionInt * UI_LERP2D_RANK_ICON_SLOT_H + UI_LERP2D_RANK_ICON_BASE_Y +
		         (frameCounter * (UI_LERP2D_RANK_ICON_SLOT_H * (absolutePositionInt - drawnPositionInt))) / UI_LERP2D_TRANSITION_FRAMES;
	}
	else
	{
		pos->x = UI_LERP2D_RANK_ICON_RADIUS -
		         (MATH_Sin((frameCounter * (1 << UI_LERP2D_ANGULAR_FRAME_SHIFT)) / UI_LERP2D_TRANSITION_FRAMES) * UI_LERP2D_RANK_ICON_RADIUS >>
		          UI_LERP2D_FIXED_SHIFT);
		pos->y = drawnPositionInt * UI_LERP2D_RANK_ICON_SLOT_H + UI_LERP2D_RANK_ICON_BASE_Y +
		         (frameCounter * (UI_LERP2D_RANK_ICON_SLOT_H * (absolutePositionInt - drawnPositionInt))) / UI_LERP2D_TRANSITION_FRAMES;
	}
}

void UI_Lerp2D_HUD(s16 *pos, s16 startX, s16 startY, s16 endX, s16 endY, s32 curFrame, s16 endFrame)
{
	// NOTE(aalhendi): Keep the divisor live and split Y sign extension around the endpoint's retail word load.
	register s32 duration CTR_PSX_REGISTER("$3") = endFrame;
	s32 deltaX = curFrame * ((s32)startX - (s32)endX) / duration;
	u32 startYHigh = (u32)startY << 16;
	s32 deltaY;
	CTR_PSX_OBSERVE_VALUE(endY);
	deltaY = curFrame * ((s16)(startYHigh >> 16) - (s32)endY) / duration;

	// NOTE(aalhendi): HUD timers count down, so zero is the destination rather than the starting point.
	pos[0] = endX + deltaX;
	pos[1] = endY + deltaY;
}

void UI_Lerp2D_Linear(s16 *pos, s16 startX, s16 startY, s16 endX, s16 endY, s32 curFrame, s16 endFrame)
{
	s32 endFrameInt = (s32)endFrame;

	// NOTE(aalhendi): This timer counts up and clamps at the destination after the transition.
	if (curFrame <= endFrameInt)
	{
		s32 newPosX = curFrame * ((s32)endX - (s32)startX);
		s32 newPosY = curFrame * ((s32)endY - (s32)startY);

		pos[0] = startX + (s16)(newPosX / endFrameInt);
		pos[1] = startY + (s16)(newPosY / endFrameInt);
		return;
	}

	pos[0] = endX;
	pos[1] = endY;
}
