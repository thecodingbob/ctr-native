#include <common.h>

enum
{
	UI_POS_SUFFIX_BIG_NUM_Z_BASE = 0x100,
	UI_POS_SUFFIX_SPLIT_PLAYER_COUNT = 3,
	UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET = 0x10,
	UI_POS_SUFFIX_3P4P_BIG_NUM_Y_OFFSET = 4,
	// The 3D number extends left of the 2D icon's layout anchor.
	UI_POS_SUFFIX_3P4P_MODEL_LEFT_OVERHANG = 6,
	UI_POS_SUFFIX_ORDINAL_VISIBLE_WIDTH = 6,
	UI_POS_SUFFIX_FIRST_DIGIT_ICON = 0x19,
	UI_POS_SUFFIX_DIGIT_WIDTH = 16,
	UI_POS_SUFFIX_DIGIT_Y_OFFSET = 5,
};

static int UI_PosSuffixStringIndex(int rank)
{
	return (unsigned int)rank < len(data.stringIndexSuffix) ? data.stringIndexSuffix[rank] : LNG_TH;
}

static void UI_PosSuffixDrawDigit(int digit, int x, int y, s16 flags)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Icon *icon = digit == 0 ? NULL : gGT->ptrIcons[UI_POS_SUFFIX_FIRST_DIGIT_ICON + (digit == 9 ? 6 : digit) - 1];
	if (icon == NULL)
	{
		// Use the existing font until a matching zero icon is available.
		char text[] = {(char)('0' + digit), '\0'};
		DecalFont_DrawLine(text, x, y - UI_POS_SUFFIX_DIGIT_Y_OFFSET, FONT_BIG, flags | JUSTIFY_CENTER);
		return;
	}
	const unsigned int colorIndex = flags & 0xfff;
	const u32 *colors = data.ptrColor[colorIndex < NUM_COLORS ? colorIndex : WHITE];
	DecalHUD_Arrow2D(icon, x, y, &gGT->backBuffer->primMem, gGT->pushBuffer_UI.ptrOT,
	                 ColorCode_Load(&colors[0]), ColorCode_Load(&colors[1]), ColorCode_Load(&colors[2]), ColorCode_Load(&colors[3]),
	                 0, FP(0.75), digit == 9 ? ANG_PI : 0);
}

void UI_DrawExpandedPosition(s16 suffixX, s16 suffixY, struct Driver *driver, s16 font, s16 flags)
{
	const int position = driver->driverRank + 1;
	if (driver->instBigNum != NULL)
	{
		driver->instBigNum->flags |= HIDE_MODEL;
	}
	if (position <= RACE_FIELD_FULL_GRID_RACERS || position > RACE_FIELD_CONFIG_MAX_RACERS)
	{
		return;
	}
	const int digitWidth = Widescreen_ScaleWidth(UI_POS_SUFFIX_DIGIT_WIDTH);
	const int x = suffixX - digitWidth / 2;
	const int y = suffixY + UI_POS_SUFFIX_DIGIT_Y_OFFSET;
	if (position >= 10)
	{
		UI_PosSuffixDrawDigit(position / 10, x - digitWidth, y, flags);
	}
	UI_PosSuffixDrawDigit(position % 10, x, y, flags);
	DecalFont_DrawLine(sdata->lngStrings[LNG_TH], suffixX, suffixY, font, flags);
}

static int UI_PosSuffixRank(struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	if ((gGT->gameMode1 & BATTLE_MODE) == 0)
	{
		return d->driverRank;
	}
	return gGT->battleSetup.finishedRankOfEachTeam[d->BattleHUD.teamID];
}

int UI_PosSuffixOffset(s16 posX, struct Driver *d)
{
	struct GameTracker *gGT = sdata->gGT;
	if (gGT->numPlyrCurrGame >= UI_POS_SUFFIX_SPLIT_PLAYER_COUNT && (d->driverID & 1) != 0)
	{
		struct UiElement2D *leftHud = data.hudStructPtr[gGT->numPlyrCurrGame - 1] +
		                              (d->driverID - 1) * UI_HUD_SLOT_COUNT;
		char *suffix = sdata->lngStrings[UI_PosSuffixStringIndex(UI_PosSuffixRank(d))];
		int leftRankX = leftHud[UI_HUD_SLOT_RANK].x;
		int leftNumberX = leftRankX + Widescreen_ScaleWidth(leftHud[UI_HUD_SLOT_BIG1].x - leftRankX);
		struct Driver *leftDriver = gGT->drivers[d->driverID - 1];
		if (leftDriver != NULL && leftDriver->instBigNum != NULL)
		{
			leftNumberX -= Widescreen_ScaleWidth(UI_POS_SUFFIX_3P4P_MODEL_LEFT_OVERHANG);
		}
		int rightEdge = SCREEN_WIDTH - leftNumberX;
		int suffixWidth = suffix[0] == '&' && suffix[1] == '\0'
		                      ? Widescreen_ScaleWidth(UI_POS_SUFFIX_ORDINAL_VISIBLE_WIDTH)
		                      : DecalFont_GetLineWidth(suffix, FONT_BIG);
		return rightEdge - suffixWidth - posX;
	}
	return 0;
}

int UI_PosSuffixNumberX(s16 suffixX, s16 numberX, struct Driver *d)
{
	if (sdata->gGT->numPlyrCurrGame < UI_POS_SUFFIX_SPLIT_PLAYER_COUNT)
	{
		return numberX;
	}
	return suffixX + UI_PosSuffixOffset(suffixX, d) + Widescreen_ScaleWidth(numberX - suffixX);
}

void UI_DrawPosSuffix(s16 posX, s16 posY, struct Driver *d, s16 flags)
{
	s16 bigNumX;
	struct GameTracker *gGT = sdata->gGT;
	int currRank = UI_PosSuffixRank(d);
	posX += UI_PosSuffixOffset(posX, d);
	if ((gGT->gameMode1 & BATTLE_MODE) == 0 && (unsigned int)currRank >= RACE_FIELD_FULL_GRID_RACERS)
	{
		UI_DrawExpandedPosition(posX, posY, d, FONT_BIG, flags);
		return;
	}

	// Draw the suffix of your current position
	DecalFont_DrawLine(sdata->lngStrings[UI_PosSuffixStringIndex(currRank)], posX, posY, FONT_BIG, flags);

	// setting posZ changes which number draws
	if (d->instBigNum != 0)
	{
		s16 bigNumDepth = d->driverRank + UI_POS_SUFFIX_BIG_NUM_Z_BASE;

		d->instBigNum->matrix.t[2] = bigNumDepth;
		if (gGT->numPlyrCurrGame >= UI_POS_SUFFIX_SPLIT_PLAYER_COUNT)
		{
			// The number keeps the same offset from the suffix on both sides.
			bigNumX = posX - Widescreen_ScaleWidth(UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET);
			d->instBigNum->matrix.t[0] = UI_ConvertX_2(bigNumX, bigNumDepth);
			d->instBigNum->matrix.t[1] = UI_ConvertY_2(posY + UI_POS_SUFFIX_3P4P_BIG_NUM_Y_OFFSET, bigNumDepth);
		}
	}
}
