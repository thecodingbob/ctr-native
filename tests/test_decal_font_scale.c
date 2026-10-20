#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;
static int s_draws;
static s32 s_x[4];
static s32 s_y[4];
static s16 s_scale[4];
static u32 *s_ot;

#include <game/DecalFont.c>

void DecalHUD_DrawPolyGT4(struct Icon *icon, s32 x, s32 y, struct PrimMem *primMem, u32 *ot,
                         Color c0, Color c1, Color c2, Color c3, u8 transparency, s16 scale)
{
	(void)icon;
	(void)primMem;
	(void)c0;
	(void)c1;
	(void)c2;
	(void)c3;
	(void)transparency;
	if (s_draws >= (int)len(s_x))
	{
		CtrTest_Bail("unexpected glyph count");
	}
	s_x[s_draws] = x;
	s_y[s_draws] = y;
	s_scale[s_draws++] = scale;
	s_ot = ot;
}

int main(void)
{
	static struct DB buffer;
	static struct Icon icon;
	static struct { struct IconGroup group; struct Icon *icons[1]; } atlas;
	static u32 colors[4];
	u32 originalOT[5];
	u32 targetOT[5];
	sdata->gGT = &s_tracker;
	s_tracker.backBuffer = &buffer;
	s_tracker.pushBuffer_UI.ptrOT = originalOT;
	atlas.group.numIcons = 1;
	atlas.icons[0] = &icon;
	s_tracker.iconGroup[0] = &atlas.group;
	data.ptrColor[ORANGE] = colors;
	data.font_charPixWidth[FONT_SMALL] = 13;

	CTR_TEST_SECTION("scaled font geometry preserves right alignment, fractional advances and OT ownership");
	DecalFont_DrawLineScaledOT("AB", 100, 40, FONT_SMALL, JUSTIFY_RIGHT | ORANGE, FP(0.5), targetOT);
	CTR_TEST_CHECK_EQ(s_draws, 2, "scaled drawing emits both glyphs");
	CTR_TEST_CHECK_EQ(s_x[0], 87, "the right-aligned origin uses the scaled total width");
	CTR_TEST_CHECK_EQ(s_x[1], 93, "glyph positions scale the cumulative advance");
	CTR_TEST_CHECK_EQ(s_y[0], 40, "scaling preserves the requested baseline");
	CTR_TEST_CHECK_EQ(s_scale[0], FP(0.5), "the glyph geometry receives the compact scale");
	CTR_TEST_CHECK(s_ot == targetOT, "scaled glyphs use the requested ordering table");
	CTR_TEST_CHECK(s_tracker.pushBuffer_UI.ptrOT == originalOT, "scaled drawing restores the caller's ordering table");

	s_draws = 0;
	DecalFont_DrawLineScaledOT("AB", 100, 40, FONT_SMALL, JUSTIFY_CENTER | ORANGE, FP(0.5), targetOT);
	CTR_TEST_CHECK_EQ(s_x[0], 94, "center alignment uses half the scaled width");
	s_draws = 0;
	data.font_indentPixDimensions[FONT_SMALL * 2] = 2;
	data.font_indentPixDimensions[FONT_SMALL * 2 + 1] = 4;
	DecalFont_DrawLineScaledOT("\1A", 100, 40, FONT_SMALL, ORANGE, FP(0.5), targetOT);
	CTR_TEST_CHECK_EQ(s_x[0], 101, "accent horizontal offsets scale with their glyphs");
	CTR_TEST_CHECK_EQ(s_y[0], 42, "accent vertical offsets scale with their glyphs");
	CTR_TEST_CHECK_EQ(s_x[1], 100, "accent placeholders still have zero advance");

	CTR_TEST_SECTION("ordinary font calls retain full-size retail geometry");
	s_draws = 0;
	DecalFont_DrawLineOT("AB", 100, 40, FONT_SMALL, JUSTIFY_RIGHT | ORANGE, targetOT);
	CTR_TEST_CHECK_EQ(s_x[0], 74, "ordinary drawing retains the unscaled right-aligned origin");
	CTR_TEST_CHECK_EQ(s_x[1], 87, "ordinary drawing retains the full character advance");
	CTR_TEST_CHECK_EQ(s_scale[0], FP(1.0), "ordinary drawing retains full-size glyphs");
	CTR_TEST_CHECK(s_tracker.pushBuffer_UI.ptrOT == originalOT, "ordinary drawing still restores the ordering table");
	return CtrTest_Summary();
}
