#include <shared/ctr_test.h>
#include <common.h>
#include <math.h>

NativeConfig g_config;
struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;

#include "platform/native_pgxp.c"
#include "platform/native_gte_core.c"
#include "platform/native_inline_c.c"
#include "game/Vehicle/VehCalc.c"
#include "game/UI/UI_RaceHud.c"

enum
{
	TEST_PRIM_COUNT = 64,
	TEST_TRACKER_DIVISOR = 0x32,
	TEST_ON_SCREEN_PIXELS = 160,
	TEST_FAR_PIXELS = 1300,
	TEST_NODE_COUNT = 2,
	TEST_WARPBALL_FAR_NODE_DIST = 0x1000,
};

static POLY_G3 s_prims[TEST_PRIM_COUNT];
static struct DB s_backBuffer;
static u32 s_ot;
static struct Icon s_lockIcon;
static struct Thread s_trackerThread;
static struct Instance s_trackerInst;
static struct Model s_trackerModel;
static struct TrackerWeapon s_trackerWeapon;
static struct CheckpointNode s_nodes[TEST_NODE_COUNT];
static struct Instance s_driverInst;
static int s_lockSquares;

struct Thread *RB_GetThread_ClosestTracker(struct Driver *d)
{
	(void)d;
	return s_trackerThread.object != NULL ? &s_trackerThread : NULL;
}

int OtherFX_Play(u32 id, int mode)
{
	(void)id;
	(void)mode;
	return 0;
}

void UI_TrackerBG(struct Icon *targetIcon, s16 centerX, s16 centerY, struct PrimMem *primMem, u32 *ot, char transparency, s16 angleX,
                  s16 angleY, int color, int anchorX)
{
	(void)centerX; (void)centerY; (void)primMem; (void)ot; (void)transparency; (void)angleX; (void)angleY; (void)color; (void)anchorX;
	if (targetIcon == &s_lockIcon)
	{
		s_lockSquares++;
	}
}

// Unused by the tracker, but defined in the same translation unit as its neighbours.
u32 NativeGpuLinks_FromHostPointerAt(const void *pointer, const char *file, int line, const char *function)
{
	(void)pointer; (void)file; (void)line; (void)function;
	return 1;
}

void DecalFont_DrawLine(char *str, s16 posX, s16 posY, s16 fontType, s16 flags)
{
	(void)str; (void)posX; (void)posY; (void)fontType; (void)flags;
}

void DecalHUD_DrawPolyFT4(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, u8 transparency, s16 scale)
{
	(void)icon; (void)posX; (void)posY; (void)primMem; (void)ot; (void)transparency; (void)scale;
}

int Widescreen_ScaleWidth(int width)
{
	return width;
}

static int DrawTracker(struct Driver *driver, u32 sqDistance, int modelID)
{
	s_backBuffer.primMem.cursor = s_prims;
	s_lockSquares = 0;
	s_trackerWeapon.distanceToTarget = sqDistance;
	s_trackerModel.id = modelID;
	UI_TrackerSelf(driver);
	return (int)((POLY_G3 *)s_backBuffer.primMem.cursor - s_prims);
}

static void ResetTracker(struct Driver *driver)
{
	memset(driver, 0, sizeof(*driver));
	driver->instSelf = &s_driverInst;
	memset(data.trackerTimer, 0, sizeof(data.trackerTimer));
	memset(data.trackerDist, 0, sizeof(data.trackerDist));
	memset(data.trackerType, 0, sizeof(data.trackerType));
}

int main(void)
{
	struct Driver driver;

	sdata->gGT = &s_tracker;
	s_tracker.backBuffer = &s_backBuffer;
	s_backBuffer.primMem.guardEnd = s_prims + TEST_PRIM_COUNT - 1;
	s_tracker.pushBuffer[0].ptrOT = &s_ot;
	s_tracker.ptrIcons[UI_TRACKER_LOCK_ICON] = &s_lockIcon;
	s_tracker.level1 = &(struct Level){.ptr_restart_points = s_nodes};
	s_trackerThread.object = &s_trackerWeapon;
	s_trackerThread.inst = &s_trackerInst;
	s_trackerInst.model = &s_trackerModel;
	s_trackerWeapon.ptrNodeCurr = &s_nodes[0];

	CTR_TEST_SECTION("the lock-on square is drawn at every tracker distance");
	// Mutation: returning early for a far tracker also dropped the square, so
	// it only showed once the missile or orb was nearly on top of the kart.
	const u32 distances[] = {0, 100, 2400, 8000, 20000, 60000, 0x7fffffff / TEST_TRACKER_DIVISOR};
	for (size_t i = 0; i < len(distances); i++)
	{
		ResetTracker(&driver);
		u32 pixels = distances[i];
		DrawTracker(&driver, pixels * TEST_TRACKER_DIVISOR * pixels * TEST_TRACKER_DIVISOR, DYNAMIC_ROCKET);
		CTR_TEST_CHECK_EQ(s_lockSquares, 1, "a chasing missile always shows the lock-on square");
	}

	CTR_TEST_SECTION("the side brackets appear as soon as they are on screen");
	// Upstream draws them at any distance; the fork once cut them off at 48 pixels (an `s16` cannot overflow: the root of a `u32` over 50 is at most 1310).
	const int pixelOffsets[] = {16, 48, 49, TEST_ON_SCREEN_PIXELS, TEST_FAR_PIXELS - 1};
	for (size_t i = 0; i < len(pixelOffsets); i++)
	{
		ResetTracker(&driver);
		u32 root = (u32)pixelOffsets[i] * TEST_TRACKER_DIVISOR;
		int prims = DrawTracker(&driver, root * root, DYNAMIC_ROCKET);
		CTR_TEST_CHECK_EQ(prims, UI_TRACKER_SIDE_COUNT * 2, "each side draws its border and fill triangles");
		CTR_TEST_CHECK_EQ(s_lockSquares, 1, "the square accompanies the brackets");
	}

	CTR_TEST_SECTION("the largest representable distance still draws both");
	ResetTracker(&driver);
	int prims = DrawTracker(&driver, 0xffffffffu, DYNAMIC_ROCKET);
	CTR_TEST_CHECK_EQ(prims, UI_TRACKER_SIDE_COUNT * 2, "the brackets are never cut off by distance");
	CTR_TEST_CHECK_EQ(s_lockSquares, 1, "the square is drawn too");

	CTR_TEST_SECTION("a distant orb shows the square without brackets");
	ResetTracker(&driver);
	s_nodes[0].distToFinish = TEST_WARPBALL_FAR_NODE_DIST;
	s_nodes[1].distToFinish = 0;
	s_trackerWeapon.ptrNodeCurr = &s_nodes[0];
	driver.checkpoint.currentIndex = 1;
	prims = DrawTracker(&driver, TEST_ON_SCREEN_PIXELS * TEST_TRACKER_DIVISOR * TEST_ON_SCREEN_PIXELS * TEST_TRACKER_DIVISOR,
	                    DYNAMIC_WARPBALL);
	CTR_TEST_CHECK_EQ(prims, 0, "an orb more than 16000 units behind draws no brackets");
	CTR_TEST_CHECK_EQ(s_lockSquares, 1, "the orb still shows the lock-on square");

	return CtrTest_Summary();
}
