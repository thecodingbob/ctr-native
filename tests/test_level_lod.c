#include <shared/ctr_test.h>
#include "game/RenderLevel/RenderLevelLod.h"

// common.h's resident sdata pointer needs storage even though LOD setup is pure.
struct sData sdata_static;

struct TestLodValues
{
	int depthScale;
	int bsp;
	int texture0;
	int texture1;
	int topNear;
	int recursiveNear;
};

// Independent retail fixtures: deriving these through NativeDrawDistance would
// let a broken helper agree with both the implementation and its expectations.
static const struct TestLodValues s_special = {0x1e00, 1600, 1600, 1280, 640, 320};
static const struct TestLodValues s_twoPlayer = {0x19c0, 0, 4096, 2048, 1536, 768};
static const struct TestLodValues s_splitGround = {0x780, 0, 1600, 1280, 640, 320};

static void CheckTextureThresholds(const struct MainRenderLevelGeometryScratch *scratch, const struct TestLodValues *retail, bool extended)
{
	const int factor = extended ? NATIVE_DRAW_DISTANCE_SCALE : 1;
	CTR_TEST_CHECK_EQ(scratch->depthScale, retail->depthScale, "depth projection is independent of extended draw distance");
	CTR_TEST_CHECK_EQ(scratch->textureLodDepthThreshold0, retail->texture0 * factor, "the first texture LOD uses the shared factor exactly once");
	CTR_TEST_CHECK_EQ(scratch->textureLodDepthThreshold1, retail->texture1 * factor, "the second texture LOD uses the shared factor exactly once");
	CTR_TEST_CHECK_EQ(scratch->topLevelNearDepthThreshold, retail->topNear * factor, "top-level subdivision uses the shared factor exactly once");
	// Regression: scaling the three values above alone leaves mosaic textures at
	// retail distance, because their UVs are loaded only by deepest recursion.
	CTR_TEST_CHECK_EQ(scratch->recursiveNearDepthThreshold, retail->recursiveNear * factor,
	                  "recursive mosaic subdivision uses the shared factor exactly once");
	CTR_TEST_CHECK(scratch->textureLodDepthThreshold0 >= scratch->textureLodDepthThreshold1 &&
	                   scratch->textureLodDepthThreshold1 >= scratch->topLevelNearDepthThreshold &&
	                   scratch->topLevelNearDepthThreshold >= scratch->recursiveNearDepthThreshold,
	               "texture and subdivision thresholds retain their near-to-far ordering");
}

static void Check1P(const struct MainRenderLevelGeometryScratch *scratch, const struct TestLodValues *retail, bool extended)
{
	const int factor = extended ? NATIVE_DRAW_DISTANCE_SCALE : 1;
	CheckTextureThresholds(scratch, retail, extended);
	CTR_TEST_CHECK_EQ(scratch->bspLodDistanceThreshold, retail->bsp * factor, "the 1P BSP threshold uses the same distance factor as textures");
	CTR_TEST_CHECK_EQ(scratch->fullDynamicFadeDepthStart, retail->bsp * factor + 320,
	                  "full-dynamic fading follows the scaled BSP threshold with the original offset");
	CTR_TEST_CHECK_EQ(RenderLevelLod_BspThreshold(scratch, 1, extended), retail->bsp * factor,
	                  "the 1P BSP walk consumes the initialized threshold without scaling it twice");
}

static void TestRaceAndHighDetailScratch(void)
{
	static const struct
	{
		int projection;
		struct TestLodValues retail;
	} cases[] = {
	    {128, {4160, 3328, 3072, 1536, 896, 448}},
	    {256, {8320, 6656, 6144, 3072, 1792, 896}},
	    // Odd projection catches scaling before the retail fixed-point rounding.
	    {257, {8352, 6682, 6168, 3084, 1799, 899}},
	    {320, {10400, 8320, 7680, 3840, 2240, 1120}},
	};
	struct MainRenderLevelGeometryScratch scratch;
	memset(&scratch, 0xa5, sizeof(scratch));
	CTR_TEST_SECTION("race / high-detail split-screen scratch, multiple camera projections");
	for (u32 i = 0; i < len(cases); i++)
	{
		printf("  projection %d\n", cases[i].projection);
		// Reuse scratch on purpose: off -> on -> off must not accumulate a factor
		// or leave stale extended thresholds after editing the in-game option.
		for (int pass = 0; pass < 3; pass++)
		{
			bool extended = pass == 1;
			RenderLevelLod_Init1P(&scratch, CRASH_COVE, 0, cases[i].projection, extended);
			Check1P(&scratch, &cases[i].retail, extended);
		}
	}
}

static void TestSpecialScenes(void)
{
	static const struct
	{
		const char *name;
		int level;
		u32 mode;
		bool special;
	} scenes[] = {
	    {"garage", ADVENTURE_GARAGE, 0, true},
	    {"garage cutscene", ADVENTURE_GARAGE, GAME_CUTSCENE, true},
	    {"cutscene", CRASH_COVE, GAME_CUTSCENE, true},
	    {"Crash intro exception", INTRO_CRASH, GAME_CUTSCENE, false},
	    {"intro level without cutscene flag", INTRO_CRASH, 0, false},
	};
	const struct TestLodValues race = {8352, 6682, 6168, 3084, 1799, 899};
	struct MainRenderLevelGeometryScratch scratch;
	memset(&scratch, 0xa5, sizeof(scratch));
	CTR_TEST_SECTION("garage, cutscenes and the Crash intro exception");
	for (u32 i = 0; i < len(scenes); i++)
	{
		printf("  %s\n", scenes[i].name);
		for (int pass = 0; pass < 3; pass++)
		{
			bool extended = pass == 1;
			RenderLevelLod_Init1P(&scratch, scenes[i].level, scenes[i].mode, 257, extended);
			Check1P(&scratch, scenes[i].special ? &s_special : &race, extended);
		}
	}
}

static void TestSplitScreenScratch(void)
{
	struct MainRenderLevelGeometryScratch scratch;
	CTR_TEST_SECTION("2P helper thresholds and shared 3P/4P ground thresholds");
	for (int players = 2; players <= 4; players++)
	{
		printf("  %d players\n", players);
		memset(&scratch, 0xa5, sizeof(scratch));
		// These initializers update only texture/subdivision fields. BSP walking
		// uses its own split-screen cutoff, not a stale 1P scratch value.
		scratch.bspLodDistanceThreshold = 123;
		scratch.fullDynamicFadeDepthStart = 456;
		for (int pass = 0; pass < 3; pass++)
		{
			bool extended = pass == 1;
			if (players == 2)
			{
				RenderLevelLod_Init2P(&scratch, extended);
			}
			else
			{
				RenderLevelLod_InitSplitGround(&scratch, extended);
			}
			CheckTextureThresholds(&scratch, players == 2 ? &s_twoPlayer : &s_splitGround, extended);
			CTR_TEST_CHECK_EQ(RenderLevelLod_BspThreshold(&scratch, (u8)players, extended),
			                  5440 * (extended ? NATIVE_DRAW_DISTANCE_SCALE : 1),
			                  "the split-screen BSP cutoff scales independently of 1P scratch");
			CTR_TEST_CHECK_EQ(scratch.bspLodDistanceThreshold, 123, "split-screen texture setup preserves the BSP scratch field");
			CTR_TEST_CHECK_EQ(scratch.fullDynamicFadeDepthStart, 456, "split-screen texture setup preserves the fade scratch field");
		}
	}
}

int main(void)
{
	TestRaceAndHighDetailScratch();
	TestSpecialScenes();
	TestSplitScreenScratch();
	return CtrTest_Summary();
}
