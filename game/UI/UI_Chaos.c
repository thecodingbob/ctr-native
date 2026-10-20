#include <common.h>

enum
{
	UI_CHAOS_NOTICE_SLIDE_MS = 320,
	UI_CHAOS_NOTICE_MULTIPLAYER_Y = 100,
};

void UI_Chaos_DrawActivation(struct GameTracker *gGT)
{
	const struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_IsRunning(gGT) || runtime->noticeRemainingMS <= 0 ||
	    (gGT->hudFlags & HUD_FLAG_RACE_HUD) == 0 || gGT->numPlyrCurrGame < 1)
	{
		return;
	}
	const ChaosEffectDef *effect = ChaosEffect_Def(runtime->noticeEffect);
	if (effect == NULL)
	{
		return;
	}
	const int elapsedMS = CHAOS_ACTIVATION_NOTICE_MS - runtime->noticeRemainingMS;
	const int halfWidth = DecalFont_GetLineWidth((char *)effect->label, FONT_BIG) / 2;
	const int centerX = SCREEN_WIDTH / 2;
	const int y = gGT->numPlyrCurrGame > 1 ? UI_CHAOS_NOTICE_MULTIPLAYER_Y : SCREEN_HEIGHT / 4;
	int startX = centerX;
	int endX = centerX;
	int slideMS = 0;
	if (elapsedMS < UI_CHAOS_NOTICE_SLIDE_MS)
	{
		startX = SCREEN_WIDTH + halfWidth;
		slideMS = elapsedMS;
	}
	else if (runtime->noticeRemainingMS < UI_CHAOS_NOTICE_SLIDE_MS)
	{
		endX = -halfWidth;
		slideMS = UI_CHAOS_NOTICE_SLIDE_MS - runtime->noticeRemainingMS;
	}
	SVec2 pos;
	UI_Lerp2D_Linear(CTR_VECTOR_DATA(&pos), (s16)startX, (s16)y, (s16)endX, (s16)y, slideMS, UI_CHAOS_NOTICE_SLIDE_MS);
	// Full-screen UI OT keeps the 2P announcement shared, like the continue prompt.
	DecalFont_DrawLineOT((char *)effect->label, pos.x, pos.y, FONT_BIG, JUSTIFY_CENTER | ORANGE,
	                     gGT->backBuffer->otMem.uiOT);
}
