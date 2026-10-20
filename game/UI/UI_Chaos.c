#include <common.h>

enum
{
	UI_CHAOS_NOTICE_SLIDE_MS = 320,
	UI_CHAOS_NOTICE_MULTIPLAYER_Y = 100,
	UI_CHAOS_BANNER_ROW_HEIGHT = 20,
	UI_CHAOS_COMPACT_BANNER_ROW_HEIGHT = 9,
	UI_CHAOS_LIST_RIGHT = SCREEN_WIDTH - 12,
	UI_CHAOS_LIST_TOP = 64,
	UI_CHAOS_LIST_MULTIPLAYER_TOP = 32,
	UI_CHAOS_MAX_DOCKED = 4,
	UI_CHAOS_LIST_ROW_HEIGHT = 6,
	UI_CHAOS_LIST_SCALE = FP(0.5),
	UI_CHAOS_ROW_GAP = 2,
	UI_CHAOS_SCREEN_MARGIN = 8,
	UI_CHAOS_FINAL_LAP_HEIGHT = 17,
};

static int UI_Chaos_FreeRowY(const struct GameTracker *gGT, int y, int height)
{
	// Final Lap retains its viewport position; the shared table reserves its rows.
	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		if (sdata->finalLapTextTimer[i] > 0)
		{
			const struct PushBuffer *pb = &gGT->pushBuffer[i];
			const int lapY = pb->rect.y + pb->rect.h / 4;
			if (y < lapY + UI_CHAOS_FINAL_LAP_HEIGHT && y + height > lapY)
			{
				y = lapY + UI_CHAOS_FINAL_LAP_HEIGHT + UI_CHAOS_ROW_GAP;
			}
		}
	}
	return y;
}

static s16 UI_Chaos_LayoutNotices(const struct GameTracker *gGT, const int *latest, int *rowY, int dockTop, int dockBottom)
{
	const struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	const bool stackDown = gGT->numPlyrCurrGame == 1;
	const int soloHeight = gGT->pushBuffer[0].rect.h > 0 ? gGT->pushBuffer[0].rect.h : SCREEN_HEIGHT;
	int y = gGT->numPlyrCurrGame > 1 ? UI_CHAOS_NOTICE_MULTIPLAYER_Y : soloHeight / 4;
	for (int i = runtime->numActive - 1; i >= 0; i--)
	{
		const struct ChaosActivation *entry = &runtime->active[i];
		if (ChaosEffect_Def(entry->effect) == NULL || latest[entry->effect] != i ||
		    entry->noticeRemainingMS <= CHAOS_ACTIVATION_DOCK_MS)
		{
			continue;
		}
		for (int player = gGT->numPlyrCurrGame - 1; player >= 0; player--)
		{
			const struct PushBuffer *pb = &gGT->pushBuffer[player];
			const int lapY = pb->rect.y + pb->rect.h / 4;
			if (sdata->finalLapTextTimer[player] > 0 && y < lapY + UI_CHAOS_FINAL_LAP_HEIGHT &&
			    y + UI_CHAOS_BANNER_ROW_HEIGHT > lapY)
			{
				y = stackDown ? lapY + UI_CHAOS_FINAL_LAP_HEIGHT + UI_CHAOS_ROW_GAP
				              : lapY - UI_CHAOS_BANNER_ROW_HEIGHT - UI_CHAOS_ROW_GAP;
			}
		}
		if (y < UI_CHAOS_SCREEN_MARGIN || y + UI_CHAOS_BANNER_ROW_HEIGHT > SCREEN_HEIGHT - UI_CHAOS_SCREEN_MARGIN)
		{
			// A crowded announcement stack uses compact rows below the docked list.
			y = dockBottom > dockTop ? dockBottom + UI_CHAOS_ROW_GAP : UI_CHAOS_SCREEN_MARGIN;
			for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
			{
				if (latest[effect] >= 0 && runtime->active[latest[effect]].noticeRemainingMS > CHAOS_ACTIVATION_DOCK_MS)
				{
					y = UI_Chaos_FreeRowY(gGT, y, UI_CHAOS_COMPACT_BANNER_ROW_HEIGHT);
					rowY[effect] = y;
					y += UI_CHAOS_COMPACT_BANNER_ROW_HEIGHT;
				}
			}
			return FONT_SMALL;
		}
		rowY[entry->effect] = y;
		y += stackDown ? UI_CHAOS_BANNER_ROW_HEIGHT : -UI_CHAOS_BANNER_ROW_HEIGHT;
	}
	return FONT_BIG;
}

static void UI_Chaos_DrawNotice(struct GameTracker *gGT, const struct ChaosActivation *entry,
                               int bannerY, s16 bannerFont)
{
	const ChaosEffectDef *effect = ChaosEffect_Def(entry->effect);
	const int elapsedMS = CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS - entry->noticeRemainingMS;
	const int centerX = SCREEN_WIDTH / 2;
	int startX = centerX;
	int endX = centerX;
	int slideMS = 0;
	int slideDurationMS = UI_CHAOS_NOTICE_SLIDE_MS;
	s16 font = bannerFont;
	s32 scale = FP(1.0);
	u32 *ot = gGT->backBuffer->otMem.uiOT;
	u16 flags = JUSTIFY_CENTER | ORANGE;
	if (elapsedMS < UI_CHAOS_NOTICE_SLIDE_MS)
	{
		startX = SCREEN_WIDTH + DecalFont_GetLineWidth((char *)effect->label, font) / 2;
		slideMS = elapsedMS;
	}
	else if (entry->noticeRemainingMS <= CHAOS_ACTIVATION_DOCK_MS)
	{
		font = FONT_SMALL;
		scale = UI_CHAOS_LIST_SCALE;
		ot += UI_HUD_ORDERING_SLOT_BEHIND_HUD;
		flags = JUSTIFY_RIGHT | ORANGE;
		startX = centerX + DecalFont_GetLineWidth((char *)effect->label, font) * scale / FP(1.0) / 2;
		endX = UI_CHAOS_LIST_RIGHT;
		slideMS = CHAOS_ACTIVATION_DOCK_MS - entry->noticeRemainingMS;
		slideDurationMS = CHAOS_ACTIVATION_DOCK_MS;
	}
	SVec2 pos;
	UI_Lerp2D_Linear(CTR_VECTOR_DATA(&pos), (s16)startX, (s16)bannerY, (s16)endX, (s16)bannerY,
	                 slideMS, (s16)slideDurationMS);
	DecalFont_DrawLineScaledOT((char *)effect->label, pos.x, pos.y, font, (s16)flags, scale,
	                           ot);
}

void UI_Chaos_DrawActivation(struct GameTracker *gGT)
{
	if (!Chaos_IsRunning(gGT) || (gGT->hudFlags & HUD_FLAG_RACE_HUD) == 0 || gGT->numPlyrCurrGame < 1 ||
	    MainGameEnd_AllHumansFinished(gGT))
	{
		return;
	}
	const struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	int latest[CHAOS_EFFECT_COUNT];
	int rowY[CHAOS_EFFECT_COUNT];
	for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
	{
		latest[effect] = -1;
	}
	for (int i = 0; i < runtime->numActive; i++)
	{
		if (ChaosEffect_Def(runtime->active[i].effect) != NULL)
		{
			latest[runtime->active[i].effect] = i;
		}
	}
	int docked[UI_CHAOS_MAX_DOCKED];
	int numDocked = 0;
	const int dockTop = gGT->numPlyrCurrGame > 1 ? UI_CHAOS_LIST_MULTIPLAYER_TOP : UI_CHAOS_LIST_TOP;
	int dockBottom = dockTop;
	for (int i = runtime->numActive - 1; i >= 0 && numDocked < UI_CHAOS_MAX_DOCKED; i--)
	{
		const struct ChaosActivation *entry = &runtime->active[i];
		if (ChaosEffect_Def(entry->effect) != NULL && latest[entry->effect] == i &&
		    entry->noticeRemainingMS <= CHAOS_ACTIVATION_DOCK_MS)
		{
			rowY[entry->effect] = dockBottom;
			docked[numDocked++] = i;
			dockBottom += UI_CHAOS_LIST_ROW_HEIGHT;
		}
	}
	const s16 bannerFont = UI_Chaos_LayoutNotices(gGT, latest, rowY, dockTop, dockBottom);
	for (int row = 0; row < numDocked; row++)
	{
		const struct ChaosActivation *entry = &runtime->active[docked[row]];
		if (entry->noticeRemainingMS > 0)
		{
			UI_Chaos_DrawNotice(gGT, entry, rowY[entry->effect], FONT_SMALL);
		}
		else
		{
			DecalFont_DrawLineScaledOT((char *)g_chaosEffectDefs[entry->effect].label, UI_CHAOS_LIST_RIGHT, (s16)rowY[entry->effect],
			                           FONT_SMALL, JUSTIFY_RIGHT | ORANGE, UI_CHAOS_LIST_SCALE,
			                           &gGT->backBuffer->otMem.uiOT[UI_HUD_ORDERING_SLOT_BEHIND_HUD]);
		}
	}
	for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
	{
		if (latest[effect] >= 0 && runtime->active[latest[effect]].noticeRemainingMS > CHAOS_ACTIVATION_DOCK_MS)
		{
			const struct ChaosActivation *entry = &runtime->active[latest[effect]];
			UI_Chaos_DrawNotice(gGT, entry, rowY[effect], bannerFont);
		}
	}
}
