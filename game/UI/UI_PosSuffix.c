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
	UI_POS_SUFFIX_SIX_MODEL_RANK = 5,
	UI_POS_SUFFIX_DIGIT_WIDTH = 16,
	UI_POS_SUFFIX_DIGIT_Y_OFFSET = 5,
	// BigNum's widest digits need this advance at unit model and HUD scale.
	UI_POS_SUFFIX_MODEL_DIGIT_ADVANCE = 40,
	UI_POS_SUFFIX_MODEL_SUFFIX_GAP = 3,
};

static int UI_PosSuffixStringIndex(int rank)
{
	return (unsigned int)rank < len(data.stringIndexSuffix) ? data.stringIndexSuffix[rank] : LNG_TH;
}

static struct Icon *UI_PosSuffixDigitIcon(int digit)
{
	return sdata->gGT->ptrIcons[UI_POS_SUFFIX_FIRST_DIGIT_ICON + digit - 1];
}

// Reverse the red-to-yellow ramp for the rotated six without changing the
// dark outline or the shared model data.
u32 UI_PosSuffix_ReverseWarmRamp(u32 color)
{
	const u32 red = color & 0xff;
	const u32 green = (color >> 8) & 0xff;
	const u32 blue = (color >> 16) & 0xff;
	if (green >= blue && red >= green)
	{
		return (color & ~0xff00u) | ((red - green + blue) << 8);
	}
	return color;
}

static void UI_PosSuffixOrientModel(struct Instance *inst, bool upsideDown)
{
	// The BigNum model has an upright, axis-aligned HUD basis. Rotate around
	// screen Z without touching its depth, and make repeated draws idempotent.
	if ((inst->matrix.m[0][0] < 0) != upsideDown)
	{
		for (int col = 0; col < 3; col++)
		{
			inst->matrix.m[0][col] = -inst->matrix.m[0][col];
			inst->matrix.m[1][col] = -inst->matrix.m[1][col];
		}
	}
}

static void UI_PosSuffixDrawDigit(int digit, int x, int y, s16 flags)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Icon *icon = digit == 0 ? NULL : UI_PosSuffixDigitIcon(digit == 9 ? 6 : digit);
	if (icon == NULL && digit == 9)
	{
		struct IconGroup *fontGroup = gGT->iconGroup[data.font_IconGroupID[FONT_BIG]];
		const int six = data.font_characterIconID['6' - '!'];
		if (fontGroup != NULL && six < fontGroup->numIcons)
		{
			struct Icon **icons = ICONGROUP_GETICONS(fontGroup);
			icon = icons[six];
		}
	}
	if (icon == NULL)
	{
		// Zero has no retail rank model or icon.
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

static void UI_PosSuffixPlaceModel(struct Instance *model, int rank, int x, int y)
{
	const int depth = UI_POS_SUFFIX_BIG_NUM_Z_BASE + rank;
	model->matrix.t[0] = UI_ConvertX_2(x, depth);
	model->matrix.t[1] = UI_ConvertY_2(y, depth);
	model->matrix.t[2] = depth;
}

static void UI_DrawExpandedPositionAt(s16 suffixX, s16 suffixY, struct Driver *driver, s16 font, s16 flags,
                                      const struct UiElement2D *soloNumber, bool rightAligned)
{
	const int position = driver->driverRank + 1;
	if (position <= RACE_FIELD_FULL_GRID_RACERS || position > RACE_FIELD_CONFIG_MAX_RACERS)
	{
		return;
	}
	struct Instance *model = driver->instBigNum;
	struct Thread *child = model != NULL && model->thread != NULL ? model->thread->childThread : NULL;
	struct Instance *tens = child != NULL ? child->inst : NULL;
	const int modelX = soloNumber != NULL ? soloNumber->x : suffixX - Widescreen_ScaleWidth(UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET);
	const int modelY = soloNumber != NULL ? soloNumber->y : suffixY + UI_POS_SUFFIX_3P4P_BIG_NUM_Y_OFFSET;
	// The 1P pack provides the 3D rank model but not the 2D rank icons.
	if (position == 9 && model != NULL && UI_PosSuffixDigitIcon(6) == NULL)
	{
		UI_PosSuffixOrientModel(model, true);
		UI_PosSuffixPlaceModel(model, UI_POS_SUFFIX_SIX_MODEL_RANK, modelX, modelY);
	}
	else if (position >= 10 && tens != NULL && (model->flags & HIDE_MODEL) == 0)
	{
		UI_PosSuffixOrientModel(model, false);
		const int step = Widescreen_ScaleWidth(FP_Mult(FP_Mult(UI_POS_SUFFIX_MODEL_DIGIT_ADVANCE, model->scale.x), model->matrix.m[0][0]));
		// Grow away from the viewport edge: keep the leading digit on the
		// left side, or keep the suffix fixed on the right side.
		const int suffixOffset = suffixX - modelX;
		const int suffixClearance = step / 2 + Widescreen_ScaleWidth(UI_POS_SUFFIX_MODEL_SUFFIX_GAP);
		const int gap = suffixOffset > suffixClearance ? suffixOffset : suffixClearance;
		const int unitsX = rightAligned ? suffixX - gap : modelX + step;
		suffixX = unitsX + gap;
		tens->matrix = model->matrix;
		tens->scale = model->scale;
		UI_PosSuffixPlaceModel(tens, 0, unitsX - step, modelY);
		tens->flags &= ~HIDE_MODEL;
		if (position == 10)
		{
			model->flags |= HIDE_MODEL;
			UI_PosSuffixDrawDigit(0, unitsX, modelY, flags);
		}
		else
		{
			UI_PosSuffixPlaceModel(model, position % 10 - 1, unitsX, modelY);
		}
	}
	else
	{
		if (model != NULL)
		{
			model->flags |= HIDE_MODEL;
		}
		const int step = Widescreen_ScaleWidth(UI_POS_SUFFIX_DIGIT_WIDTH);
		const int x = suffixX - step / 2;
		const int y = soloNumber != NULL ? soloNumber->y : suffixY + UI_POS_SUFFIX_DIGIT_Y_OFFSET;
		if (position >= 10)
		{
			UI_PosSuffixDrawDigit(position / 10, x - step, y, flags);
		}
		UI_PosSuffixDrawDigit(position % 10, x, y, flags);
	}
	DecalFont_DrawLine(sdata->lngStrings[LNG_TH], suffixX, suffixY, font, flags);
}

void UI_DrawExpandedPosition(s16 suffixX, s16 suffixY, struct Driver *driver, s16 font, s16 flags)
{
	UI_DrawExpandedPositionAt(suffixX, suffixY, driver, font, flags, NULL, false);
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
	// The solo race number uses its own anchor, well below the suffix.
	// Finish cards supply animated coordinates and must keep using those.
	const struct UiElement2D *soloNumber = NULL;
	if (gGT->numPlyrCurrGame == 1 && (gGT->gameMode1 & BATTLE_MODE) == 0 && (d->actionsFlagSet & ACTION_RACE_FINISHED) == 0)
	{
		soloNumber = &data.hudStructPtr[0][UI_HUD_SLOT_BIG1];
	}
	posX += UI_PosSuffixOffset(posX, d);
	if ((gGT->gameMode1 & BATTLE_MODE) == 0 && (unsigned int)currRank >= RACE_FIELD_FULL_GRID_RACERS)
	{
		const bool rightAligned = gGT->numPlyrCurrGame >= UI_POS_SUFFIX_SPLIT_PLAYER_COUNT && (d->driverID & 1) != 0;
		UI_DrawExpandedPositionAt(posX, posY, d, FONT_BIG, flags, soloNumber, rightAligned);
		return;
	}

	// Draw the suffix of your current position
	DecalFont_DrawLine(sdata->lngStrings[UI_PosSuffixStringIndex(currRank)], posX, posY, FONT_BIG, flags);

	// setting posZ changes which number draws
	if (d->instBigNum != 0)
	{
		s16 bigNumDepth = d->driverRank + UI_POS_SUFFIX_BIG_NUM_Z_BASE;

		UI_PosSuffixOrientModel(d->instBigNum, false);
		d->instBigNum->matrix.t[2] = bigNumDepth;
		if (soloNumber != NULL)
		{
			d->instBigNum->matrix.t[0] = UI_ConvertX_2(soloNumber->x, bigNumDepth);
			d->instBigNum->matrix.t[1] = UI_ConvertY_2(soloNumber->y, bigNumDepth);
		}
		else if (gGT->numPlyrCurrGame >= UI_POS_SUFFIX_SPLIT_PLAYER_COUNT)
		{
			// The number keeps the same offset from the suffix on both sides.
			bigNumX = posX - Widescreen_ScaleWidth(UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET);
			d->instBigNum->matrix.t[0] = UI_ConvertX_2(bigNumX, bigNumDepth);
			d->instBigNum->matrix.t[1] = UI_ConvertY_2(posY + UI_POS_SUFFIX_3P4P_BIG_NUM_Y_OFFSET, bigNumDepth);
		}
	}
}
