#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;
static char *s_suffixStrings[LNG_TH + 1] = {"th", "o", "&", [LNG_TH] = "th"};
static struct UiElement2D s_hud[UI_HUD_SLOT_COUNT * 4];
static s16 s_suffixX;
static s16 s_suffixY;
static int s_aspectFactor = 1000;
static int s_digitCount;
static struct Icon *s_lastDigit;
static u16 s_digitRotation;
static int s_digitX;
static int s_digitY;
static char s_firstFallback;

#include <game/UI/UI_Convert.c>
#include <game/UI/UI_PosSuffix.c>

s32 DecalFont_GetLineWidth(char *str, s16 fontType)
{
	(void)fontType;
	return (str[1] == '\0' ? 9 : 18) * s_aspectFactor / 1000;
}

int Widescreen_ScaleWidth(int width)
{
	return width * s_aspectFactor / 1000;
}

void DecalFont_DrawLine(char *str, s16 posX, s16 posY, s16 fontType, s16 flags)
{
	(void)fontType;
	if ((flags & JUSTIFY_CENTER) != 0)
	{
		s_firstFallback = str[0];
	}
	s_suffixX = posX;
	s_suffixY = posY;
}

void DecalHUD_Arrow2D(struct Icon *icon, s32 x, s32 y, struct PrimMem *primMem, u32 *ot, Color c0, Color c1, Color c2, Color c3,
                     u8 transparency, s16 scale, u16 rotation)
{
	(void)primMem; (void)ot; (void)c0; (void)c1; (void)c2; (void)c3; (void)transparency; (void)scale;
	s_digitCount++;
	s_lastDigit = icon;
	s_digitRotation = rotation;
	s_digitX = x;
	s_digitY = y;
}

int main(void)
{
	sdata->gGT = &s_tracker;
	sdata->lngStrings = s_suffixStrings;
	data.stringIndexSuffix[0] = 0;
	data.stringIndexSuffix[1] = 1;
	data.stringIndexSuffix[2] = 2;
	data.hudStructPtr[0] = data.hud_1P_P1;
	data.hud_1P_P1[UI_HUD_SLOT_BIG1] = (struct UiElement2D){50, 191, 256, 5120};
	data.hudStructPtr[2] = s_hud;
	data.hudStructPtr[3] = s_hud;
	s_hud[UI_HUD_SLOT_BIG1].x = 10;
	s_hud[UI_HUD_SLOT_RANK].x = 38;
	s_hud[2 * UI_HUD_SLOT_COUNT + UI_HUD_SLOT_BIG1].x = 10;
	s_hud[2 * UI_HUD_SLOT_COUNT + UI_HUD_SLOT_RANK].x = 38;

	CTR_TEST_SECTION("position indicator margins mirror in split-screen modes");

	// 4P layouts: left RANK x is 38, right RANK x is 462, both at y 77.
	const s16 leftX = 38;
	const s16 rightX = 462;
	const s16 y = 77;
	const int depth = UI_POS_SUFFIX_BIG_NUM_Z_BASE;

	for (int players = 1; players <= 4; players++)
	{
		s_tracker.numPlyrCurrGame = players;
		for (int id = 0; id < players; id++)
		{
			struct Driver driver = {0};
			struct Instance bigNum = {0};
			driver.driverID = id;
			driver.driverRank = 0;
			driver.instBigNum = &bigNum;
			bigNum.matrix.t[0] = 0x7fff;

			s16 posX = players >= 3 && (id & 1) != 0 ? rightX : leftX;
			UI_DrawPosSuffix(posX, y, &driver, 0);

			CTR_TEST_CHECK(bigNum.matrix.t[2] == depth, "the rank number keeps its depth");
			CTR_TEST_CHECK_EQ(s_suffixY, y, "the suffix keeps its vertical position");
			if (players >= 3 && (id & 1) != 0)
			{
				CTR_TEST_CHECK_EQ(s_suffixX + 18, SCREEN_WIDTH - 10,
				                  "the two-letter suffix ends at the mirrored left margin");
				CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(s_suffixX - UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET, depth),
				                  "right-side rank numbers keep the suffix on their right");
				// A two-letter suffix ("st", "nd", "rd", "th") is about 18
				// layout units wide and must not be clipped by the viewport.
				CTR_TEST_CHECK(s_suffixX + 18 < 512, "a two-letter suffix stays inside the right viewport");
			}
			else
			{
				CTR_TEST_CHECK_EQ(s_suffixX, posX, "1P/2P and left-side suffixes keep their layout X");
				if (players < 3)
				{
					const int expectedX = players == 1 ? UI_ConvertX_2(50, depth) : 0x7fff;
					CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], expectedX, "1P uses its number anchor and 2P keeps the existing placement");
				}
				else
				{
					CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(leftX - UI_POS_SUFFIX_3P4P_BIG_NUM_X_OFFSET, depth),
					                  "left-side rank numbers keep the inward offset");
				}
			}
		}
	}

	CTR_TEST_SECTION("suffix width and battle rank determine the right margin");
	struct Driver driver = {0};
	driver.driverID = 1;
	s_tracker.numPlyrCurrGame = 4;
	s_tracker.gameMode1 = BATTLE_MODE;
	s_tracker.battleSetup.finishedRankOfEachTeam[0] = 1;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK_EQ(s_suffixX + 9, SCREEN_WIDTH - 10, "a one-character battle suffix ends at the same mirrored margin");
	CTR_TEST_CHECK_EQ(UI_PosSuffixOffset(rightX, &driver), s_suffixX - rightX, "the 2D number receives the same shift as the suffix");

	CTR_TEST_SECTION("number spacing follows widescreen compression");
	s_aspectFactor = 500;
	s_tracker.gameMode1 = 0;
	struct Instance bigNum = {0};
	struct Driver leftDriver = {0};
	struct Instance leftBigNum = {0};
	s_tracker.drivers[0] = &leftDriver;
	driver.instBigNum = &bigNum;
	for (int id = 0; id < 2; id++)
	{
		driver.driverID = id;
		s16 suffixX = id == 0 ? leftX : rightX;
		s16 numberX = id == 0 ? 10 : 434;
		UI_DrawPosSuffix(suffixX, y, &driver, 0);
		CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(s_suffixX - 8, depth),
		                  "the 3D number keeps a compressed distance from the suffix");
		CTR_TEST_CHECK_EQ(UI_PosSuffixNumberX(suffixX, numberX, &driver), s_suffixX - 14,
		                  "the 2D number keeps a compressed distance from the suffix");
		if (id == 1)
		{
			CTR_TEST_CHECK_EQ(s_suffixX + 9, SCREEN_WIDTH - 24, "the compressed suffix keeps its mirrored margin");
		}
	}
	leftDriver.instBigNum = &leftBigNum;
	driver.driverID = 1;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK_EQ(s_suffixX + 9, SCREEN_WIDTH - 21,
	                  "the right margin accounts for the left 3D number's scaled overhang");

	CTR_TEST_SECTION("the right-hand number and suffix move together when the suffix width changes");
	s_aspectFactor = 1000;
	s_tracker.gameMode1 = BATTLE_MODE;
	s_tracker.battleSetup.finishedRankOfEachTeam[0] = 0;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	int twoLetterSuffixX = s_suffixX;
	int twoLetterModelX = bigNum.matrix.t[0];
	int twoLetterIconX = UI_PosSuffixNumberX(rightX, 434, &driver);
	s_tracker.battleSetup.finishedRankOfEachTeam[0] = 1;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK_EQ(s_suffixX - twoLetterSuffixX, 9, "the shorter suffix moves right by the difference in width");
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[0] - twoLetterModelX, 9, "the 3D number moves with the suffix");
	CTR_TEST_CHECK_EQ(UI_PosSuffixNumberX(rightX, 434, &driver) - twoLetterIconX, 9, "the 2D number moves with the suffix");
	CTR_TEST_CHECK_EQ(s_suffixX + 9, twoLetterSuffixX + 18, "the right edge of the block stays anchored");
	for (int factor = 500; factor <= 1000; factor += 500)
	{
		s_aspectFactor = factor;
		s_tracker.battleSetup.finishedRankOfEachTeam[0] = 2;
		UI_DrawPosSuffix(rightX, y, &driver, 0);
		int edge = SCREEN_WIDTH - (leftX + Widescreen_ScaleWidth(10 - leftX) - Widescreen_ScaleWidth(UI_POS_SUFFIX_3P4P_MODEL_LEFT_OVERHANG));
		CTR_TEST_CHECK_EQ(s_suffixX + Widescreen_ScaleWidth(UI_POS_SUFFIX_ORDINAL_VISIBLE_WIDTH), edge,
		                  "the ordinal indicator uses its scaled visible width at the mirrored margin");
	}

	CTR_TEST_SECTION("expanded positions reuse the retail six and have room for two digits");
	static struct DB buffer;
	static struct Icon one, six;
	static u32 colors[4];
	s_tracker.backBuffer = &buffer;
	s_tracker.ptrIcons[UI_POS_SUFFIX_FIRST_DIGIT_ICON] = &one;
	s_tracker.ptrIcons[UI_POS_SUFFIX_FIRST_DIGIT_ICON + 5] = &six;
	data.ptrColor[0] = colors;
	s_tracker.gameMode1 = 0;
	driver.driverRank = 8;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK(s_lastDigit == &six && s_digitRotation == ANG_PI, "ninth draws the retail six rotated by half a turn");
	CTR_TEST_CHECK((bigNum.flags & HIDE_MODEL) != 0, "the retail model is not drawn over the fallback");
	driver.driverRank = 9;
	s_digitCount = 0;
	UI_DrawPosSuffix(leftX, y, &driver, 0);
	CTR_TEST_CHECK(s_digitCount == 1 && s_lastDigit == &one && s_firstFallback == '0', "tenth draws both one and zero");
	CTR_TEST_CHECK_EQ(s_digitX, s_suffixX - 24, "the tens digit leaves room before the suffix");
	driver.driverRank = 15;
	s_digitCount = 0;
	UI_DrawPosSuffix(leftX, y, &driver, 0);
	CTR_TEST_CHECK(s_digitCount == 2 && s_lastDigit == &six && s_digitRotation == 0, "sixteenth draws two upright retail digits");

	CTR_TEST_SECTION("one-player packs can draw ninth with the retail model alone");
	s_tracker.numPlyrCurrGame = 1;
	driver.driverID = 0;
	const int soloSuffixX = 70;
	const int soloSuffixY = 171;
	const int soloNumberY = data.hud_1P_P1[UI_HUD_SLOT_BIG1].y;
	s_tracker.ptrIcons[UI_POS_SUFFIX_FIRST_DIGIT_ICON + 5] = NULL;
	bigNum.flags = 0;
	bigNum.matrix.m[0][0] = bigNum.matrix.m[1][1] = bigNum.matrix.m[2][2] = FP(1.0);
	driver.driverRank = 7;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[1], UI_ConvertY_2(soloNumberY, 0x107), "eighth stays on the retail number anchor");
	driver.driverRank = 8;
	s_digitCount = 0;
	s_firstFallback = 0;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[2], 0x105, "the model uses the sixth-place geometry even in ninth");
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[1], UI_ConvertY_2(soloNumberY, 0x105), "ninth stays on the number anchor instead of jumping toward the suffix");
	CTR_TEST_CHECK_EQ(s_suffixY, soloSuffixY, "ninth does not move the suffix either");
	CTR_TEST_CHECK(bigNum.matrix.m[0][0] == -FP(1.0) && bigNum.matrix.m[1][1] == -FP(1.0) && bigNum.matrix.m[2][2] == FP(1.0),
	               "repeated draws keep the model rotated by half a turn in the screen plane");
	CTR_TEST_CHECK(s_digitCount == 0 && s_firstFallback == 0 && (bigNum.flags & HIDE_MODEL) == 0,
	               "the model renders without a duplicate or font fallback");
	driver.driverRank = 5;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK(bigNum.matrix.m[0][0] == FP(1.0) && bigNum.matrix.m[1][1] == FP(1.0), "the ordinary sixth-place model is upright again");
	driver.driverRank = 9;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK_EQ(s_digitY, soloNumberY, "tenth uses the same vertical anchor as the solo model");
	driver.driverRank = 8;
	driver.actionsFlagSet = ACTION_RACE_FINISHED;
	UI_DrawExpandedPosition(leftX, y, &driver, FONT_SMALL, 0);
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[1], UI_ConvertY_2(y + UI_POS_SUFFIX_3P4P_BIG_NUM_Y_OFFSET, 0x105),
	                  "the finish card still positions the model relative to its own suffix");

	CTR_TEST_SECTION("two-digit positions share the original rank model");
	struct Instance tens = {.flags = HIDE_MODEL};
	struct Thread child = {.inst = &tens};
	struct Thread parent = {.childThread = &child};
	bigNum.thread = &parent;
	bigNum.scale.x = 0x1400;
	bigNum.flags = 0;
	driver.actionsFlagSet = 0;
	s_firstFallback = 0;
	s_digitCount = 0;
	for (int position = 11; position <= 16; position++)
	{
		driver.driverRank = position - 1;
		UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
		CTR_TEST_CHECK_EQ(tens.matrix.t[2], 0x100, "the tens digit always selects the retail one");
		CTR_TEST_CHECK_EQ(bigNum.matrix.t[2], 0x100 + position - 11, "the units digit selects the matching retail digit");
		CTR_TEST_CHECK_EQ(bigNum.matrix.t[1], UI_ConvertY_2(soloNumberY, bigNum.matrix.t[2]), "units keep the original solo height");
		CTR_TEST_CHECK_EQ(tens.matrix.t[1], UI_ConvertY_2(soloNumberY, tens.matrix.t[2]), "both digits share the same height");
		CTR_TEST_CHECK_EQ(tens.matrix.t[0], UI_ConvertX_2(50, 0x100), "the leading digit keeps the safe solo anchor in 4:3");
		CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(100, bigNum.matrix.t[2]), "units are a full model advance to the right in 4:3");
		CTR_TEST_CHECK(tens.scale.x == bigNum.scale.x && (tens.flags & HIDE_MODEL) == 0, "both model digits are drawn at the same scale");
	}
	CTR_TEST_CHECK(s_firstFallback == 0 && s_digitCount == 0, "11 through 16 do not draw any font digits or icons when the models are available");
	driver.driverRank = 9;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK((bigNum.flags & HIDE_MODEL) != 0 && (tens.flags & HIDE_MODEL) == 0 && s_firstFallback == '0',
	               "tenth pairs the retail one with the temporary zero glyph");
	// The HUD thread refreshes visibility before drawing a new frame.
	bigNum.flags = 0;
	tens.flags |= HIDE_MODEL;
	driver.driverRank = 8;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK((tens.flags & HIDE_MODEL) != 0 && bigNum.matrix.m[0][0] < 0,
	               "returning to ninth hides the tens model and rotates the six again");
	driver.driverRank = 15;
	s_aspectFactor = 750;
	UI_DrawPosSuffix(soloSuffixX, soloSuffixY, &driver, 0);
	CTR_TEST_CHECK_EQ(tens.matrix.t[0], UI_ConvertX_2(50, 0x100), "the leading digit stays at the solo anchor in widescreen");
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(87, 0x105), "solo widescreen scales the advance rather than crowding the digits");
	CTR_TEST_CHECK(s_suffixX > 87 + 37 / 2, "the suffix clears the units digit");
	s_tracker.numPlyrCurrGame = 4;
	bigNum.scale.x = 0xe00;
	bigNum.matrix.m[0][0] = FP(0.75);
	UI_DrawPosSuffix(leftX, y, &driver, 0);
	CTR_TEST_CHECK_EQ(tens.matrix.t[0], UI_ConvertX_2(26, 0x100), "the left split-screen indicator grows inward");
	CTR_TEST_CHECK_EQ(bigNum.matrix.t[0], UI_ConvertX_2(45, 0x105), "split-screen scales the advance to the smaller models");
	CTR_TEST_CHECK(tens.matrix.m[0][0] > 0 && bigNum.matrix.m[0][0] > 0,
	               "returning to a two-digit rank leaves both models upright");
	driver.driverID = 1;
	const int rightSuffix = rightX + UI_PosSuffixOffset(rightX, &driver);
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK_EQ(s_suffixX, rightSuffix, "the right split-screen indicator keeps the suffix inside the viewport");
	CTR_TEST_CHECK_EQ(tens.matrix.t[0], UI_ConvertX_2(rightSuffix - 12 - 19, 0x100), "the right split-screen indicator grows to the left");

	CTR_TEST_SECTION("the original hub meshes replace the temporary zero and rotated six");
	static struct Model retailDigits, zero, nine;
	s_tracker.modelPtr[STATIC_BIG0] = &zero;
	s_tracker.modelPtr[STATIC_BIG1] = &retailDigits;
	s_tracker.modelPtr[STATIC_BIG9] = &nine;
	tens.model = &retailDigits;
	s_firstFallback = 0;
	driver.driverRank = 9;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK(bigNum.model == &zero && tens.model == &retailDigits && s_firstFallback == 0,
	               "tenth uses the original one and zero without a font glyph");
	CTR_TEST_CHECK((bigNum.flags & HIDE_MODEL) == 0 && bigNum.matrix.t[2] == 0x100,
	               "the standalone zero is visible and uses its first mesh");
	driver.driverRank = 8;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK(bigNum.model == &nine && bigNum.matrix.m[0][0] > 0 && bigNum.matrix.t[2] == 0x100,
	               "ninth uses the original nine without rotating the six");
	driver.driverRank = 15;
	UI_DrawPosSuffix(rightX, y, &driver, 0);
	CTR_TEST_CHECK(bigNum.model == &retailDigits && bigNum.matrix.t[2] == 0x105, "sixteenth returns to the shared 1-8 model");
	CTR_TEST_CHECK_EQ(UI_PosSuffix_ReverseWarmRamp(0x000000ff), 0x0000ffff, "the rotated model turns its upper red into yellow");
	CTR_TEST_CHECK_EQ(UI_PosSuffix_ReverseWarmRamp(0x0000ffff), 0x000000ff, "the rotated model turns its lower yellow into red");
	CTR_TEST_CHECK_EQ(UI_PosSuffix_ReverseWarmRamp(0x30000000), 0x30000000, "the black outline and packet flags remain unchanged");
	return CtrTest_Summary();
}
