#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;
static struct DB s_buffer;
static struct Driver s_drivers[8];
static int s_labels;
static int s_portraits;

#include <game/UI/UI_Rank.c>

void DecalFont_DrawLine(char *text, s16 x, s16 y, s16 font, s16 flags)
{
	(void)font;
	(void)flags;
	CTR_TEST_CHECK_EQ(text[0], '1' + s_labels, "rank labels are consecutive");
	CTR_TEST_CHECK(x == 0x34 && y == 0x38 + s_labels * 0x1b, "labels retain retail placement");
	s_labels++;
}

struct Icon *LOAD_GetDriverIcon(int driverID)
{
	CTR_TEST_CHECK_EQ(driverID, s_portraits, "rank portraits resolve the actual driver's identity");
	return NULL;
}

void UI_DrawDriverIcon(struct Icon *icon, s16 x, s16 y, struct PrimMem *prim, u32 *ot, char transparency, s16 scale, u32 color)
{
	(void)icon; (void)prim; (void)ot; (void)transparency; (void)color;
	CTR_TEST_CHECK(x == 0x14 && y == 0x39 + s_portraits * 0x1b && scale == FP(1), "portraits retain retail placement and size");
	s_portraits++;
}

// This fixture draws stationary solo standings, not transitions or multiplayer progress.
void UI_Lerp2D_Angular(SVec2 *pos, s16 from, s16 to, s16 frame)
{
	(void)pos; (void)from; (void)to; (void)frame;
	CtrTest_Bail("fixture should not animate a rank transition");
}

void DecalHUD_DrawPolyGT4(struct Icon *icon, s32 x, s32 y, struct PrimMem *prim, u32 *ot,
                          Color c0, Color c1, Color c2, Color c3, u8 transparency, s16 scale)
{
	(void)icon; (void)x; (void)y; (void)prim; (void)ot; (void)c0; (void)c1; (void)c2; (void)c3; (void)transparency; (void)scale;
	CtrTest_Bail("fixture should not draw multiplayer progress");
}

void DecalHUD_DrawWeapon(struct Icon *icon, s32 x, s32 y, struct PrimMem *prim, u32 *ot, u8 transparency, s16 scale, s16 rotation)
{
	(void)icon; (void)x; (void)y; (void)prim; (void)ot; (void)transparency; (void)scale; (void)rotation;
	CtrTest_Bail("fixture should not draw a Warpball");
}

s32 MATH_VectorNormalize(SVec3 *v) { (void)v; return 0; }
void MTC2(u32 value, s32 reg) { (void)value; (void)reg; }
void CTC2(u32 value, s32 reg) { (void)value; (void)reg; }
u32 CFC2(s32 reg) { (void)reg; return 0; }
s32 MFC2_S(s32 reg) { (void)reg; return 0; }
int doCOP2(int op) { (void)op; return 0; }

static void CheckField(int racers, u32 mode, int expectedRows)
{
	s_tracker.gameMode1 = mode;
	s_tracker.numBotsNextGame = racers - 1;
	for (int i = 0; i < (int)len(s_drivers); i++)
	{
		s_drivers[i].driverRank = i;
		s_drivers[i].driverID = i;
		data.rankIconsCurr[i] = i;
		s_tracker.drivers[i] = i < racers ? &s_drivers[i] : NULL;
	}
	s_labels = 0;
	s_portraits = 0;
	UI_DrawRankedDrivers();
	CTR_TEST_CHECK_EQ(s_labels, expectedRows, "only occupied visible positions get a label");
	CTR_TEST_CHECK_EQ(s_portraits, expectedRows, "the displayed ranks have matching portraits");
}

int main(void)
{
	sdata->gGT = &s_tracker;
	s_tracker.numPlyrCurrGame = 1;
	s_tracker.backBuffer = &s_buffer;

	CTR_TEST_SECTION("small solo fields leave no orphaned rank labels");
	CheckField(2, 0, 2);
	CheckField(3, 0, 3);
	CTR_TEST_SECTION("full fields and boss races keep their retail rows");
	CheckField(4, 0, 4);
	CheckField(8, 0, 4);
	CheckField(2, ADVENTURE_BOSS, 2);
	return CtrTest_Summary();
}
