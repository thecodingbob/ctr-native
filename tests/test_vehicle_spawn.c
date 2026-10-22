#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
struct rData rdata;
NativeConfig g_config;
u8 *gCTRNativeScratchpadBase;

#include <game/Vehicle/VehBirth.c>

// Only appearance initialization runs; allocation, terrain and motion are doubled.
struct Terrain *VehAfterColl_GetTerrain(u32 terrainType)
{
	static struct Terrain terrain;
	CTR_TEST_CHECK_EQ(terrainType, TERRAIN_NONE, "spawn initializes neutral terrain");
	return &terrain;
}

void COLL_SearchBSP_CallbackQUADBLK(const SVec3 *top, const SVec3 *bottom, struct ScratchpadStruct *sps, s32 radius)
{
	(void)top; (void)bottom; (void)sps; (void)radius;
	CtrTest_Bail("appearance initialization must not search level geometry");
}
u32 VehFrameInst_GetNumAnimFrames(struct Instance *inst, int animIndex)
{
	(void)inst; (void)animIndex;
	CtrTest_Bail("appearance initialization must not read model animations");
	return 0;
}
int VehFrameInst_GetStartFrame(int animIndex, int numFrames)
{
	(void)animIndex; (void)numFrames;
	CtrTest_Bail("appearance initialization must not read model animations");
	return 0;
}
s16 *AH_WarpPad_GetSpawnPosRot(s16 *posData)
{
	(void)posData;
	CtrTest_Bail("appearance initialization must not access warp pads");
	return NULL;
}
void CAM_StartOfRace(struct CameraDC *camera)
{
	(void)camera;
	CtrTest_Bail("appearance initialization must not initialize cameras");
}
void VehPhysProc_Driving_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread; (void)driver;
	CtrTest_Bail("appearance initialization must not run driving physics");
}
void VehStuckProc_RevEngine_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread; (void)driver;
	CtrTest_Bail("appearance initialization must not run driving physics");
}
int MATH_Cos(u32 angle)
{
	(void)angle;
	CtrTest_Bail("appearance initialization must not calculate spawn positions");
	return 0;
}
int MATH_Sin(u32 angle)
{
	(void)angle;
	CtrTest_Bail("appearance initialization must not calculate spawn positions");
	return 0;
}
void BOTS_GotoStartingLine(struct Driver *driver)
{
	(void)driver;
	CtrTest_Bail("appearance initialization must not reposition bots");
}
b32 EngineAudio_InitOnce(u32 soundID, u32 flags)
{
	(void)soundID; (void)flags;
	CtrTest_Bail("appearance initialization must not start audio");
	return false;
}
struct Instance *INSTANCE_Birth3D(struct Model *model, const char *name, struct Thread *thread)
{
	(void)model; (void)name; (void)thread;
	CtrTest_Bail("appearance initialization must not allocate model instances");
	return NULL;
}
struct Thread *PROC_BirthWithObject(ThreadFlags flags, void *tick, const char *name, struct Thread *relative)
{
	(void)flags; (void)tick; (void)name; (void)relative;
	CtrTest_Bail("appearance initialization must not allocate threads");
	return NULL;
}
struct Model *LOAD_GetExtraCharacterModelByName(char *name)
{
	(void)name;
	CtrTest_Bail("appearance initialization must not load character assets");
	return NULL;
}

static void TestSpawnScale(void)
{
	CTR_TEST_SECTION("Reused driver memory starts at normal scale in every mode");
	struct GameTracker tracker = {0};
	struct IconGroup tires = {0};
	struct Driver driver;
	struct Thread thread = {.object = &driver};
	const u32 modes[] = {ADVENTURE_MODE | ADVENTURE_ARENA, ARCADE_MODE};
	sdata->gGT = &tracker;
	tracker.iconGroup[0] = &tires;
	for (unsigned int i = 0; i < len(modes); i++)
	{
		tracker.gameMode1 = modes[i];
		// Birth clears only the retail prefix; the native tail may contain pool data.
		memset(&driver, 0xa5, sizeof(driver));
		memset(&driver, 0, DRIVER_NTSC_RETAIL_SIZE);
		CTR_TEST_CHECK_EQ(driver.kartScale, 0xa5, "retail initialization leaves a nonzero native-tail scale");
		VehBirth_TireSprites(&thread);
		CTR_TEST_CHECK_EQ(driver.kartScale, 1, "spawn replaces stale pool scale with normal scale");
		CTR_TEST_CHECK_EQ(driver.wheelSize, 0xccc, "spawn retains the retail wheel size");
		CTR_TEST_CHECK(driver.wheelSprites == ICONGROUP_GETICONS(&tires), "spawn uses the supplied tire sprites");
	}
	sdata->gGT = NULL;
}

int main(void)
{
	TestSpawnScale();
	return CtrTest_Summary();
}
