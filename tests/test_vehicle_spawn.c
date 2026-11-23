#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
struct rData rdata;
NativeConfig g_config;
u8 *gCTRNativeScratchpadBase;

#include <game/Vehicle/VehStartGrid.c>
#include <game/Vehicle/VehBirth.c>

static bool s_testCollisionSearch;
static bool s_collisionMiss;
static int s_probeMaxX = INT16_MAX;
static int s_probeCount;
static struct QuadBlock s_probeGround = {.quadFlags = QUADBLOCK_FLAG_GROUND};
static u32 s_searchFlags;
static struct Thread *s_birthThread;
static struct Instance s_birthInstance;

// Allocation, terrain and motion are doubled.
struct Terrain *VehAfterColl_GetTerrain(u32 terrainType)
{
	static struct Terrain terrain;
	CTR_TEST_CHECK_EQ(terrainType, TERRAIN_NONE, "spawn initializes neutral terrain");
	return &terrain;
}

void COLL_SearchBSP_CallbackQUADBLK(const SVec3 *top, const SVec3 *bottom, struct ScratchpadStruct *sps, s32 radius)
{
	(void)radius;
	if (!s_testCollisionSearch)
	{
		CtrTest_Bail("appearance initialization must not search level geometry");
	}
	s_searchFlags = sps->Union.QuadBlockColl.searchFlags;
	s_probeCount++;
	if (s_collisionMiss || bottom->x > s_probeMaxX)
	{
		sps->Union.QuadBlockColl.hitPos = *top;
		sps->boolDidTouchQuadblock = false;
		return;
	}
	sps->Union.QuadBlockColl.hitPos = *bottom;
	sps->boolDidTouchQuadblock = true;
	sps->hit.ptrQuadblock = &s_probeGround;
	sps->hit.plane.normal.y = FP_ONE;
}
u32 VehFrameInst_GetNumAnimFrames(struct Instance *inst, int animIndex)
{
	(void)inst; (void)animIndex;
	if (!s_testCollisionSearch)
	{
		CtrTest_Bail("appearance initialization must not read model animations");
	}
	return 1;
}
int VehFrameInst_GetStartFrame(int animIndex, int numFrames)
{
	(void)animIndex; (void)numFrames;
	if (!s_testCollisionSearch)
	{
		CtrTest_Bail("appearance initialization must not read model animations");
	}
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
int Chaos_EngineClassForDriver(const struct Driver *driver, int engineClass)
{
	(void)driver;
	return engineClass;
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
	if (s_birthThread != NULL)
	{
		return &s_birthInstance;
	}
	CtrTest_Bail("appearance initialization must not allocate model instances");
	return NULL;
}
struct Thread *PROC_BirthWithObject(ThreadFlags flags, void *tick, const char *name, struct Thread *relative)
{
	(void)flags; (void)tick; (void)name; (void)relative;
	if (s_birthThread != NULL)
	{
		CTR_TEST_CHECK_EQ((u32)flags >> 16, sizeof(struct Driver), "birth reserves the full native driver payload");
		return s_birthThread;
	}
	CtrTest_Bail("appearance initialization must not allocate threads");
	return NULL;
}
struct Model *LOAD_GetExtraCharacterModelByName(char *name)
{
	(void)name;
	CtrTest_Bail("appearance initialization must not load character assets");
	return NULL;
}

static void TestPreviewBirthClearsClock(void)
{
	CTR_TEST_SECTION("Menu previews do not inherit Clock timers from recycled driver memory");
	struct GameTracker tracker = {.gameMode1 = MAIN_MENU, .numPlyrCurrGame = 4, .levelID = MAIN_MENU_LEVEL};
	struct IconGroup tires = {0};
	struct Model model = {0};
	struct Driver driver;
	struct Thread thread = {.object = &driver};
	const struct MetaDataCHAR original = data.MetaDataCharacters[0];
	sdata->gGT = &tracker;
	tracker.iconGroup[0] = &tires;
	data.driverModelExtras[0].model = &model;
	data.MetaDataCharacters[0].name_Debug = model.name;
	s_birthThread = &thread;
	memset(&driver, 0xa5, sizeof(driver));
	driver.clockReceive = 1429527521; // Stale value captured from preview slot 3 in the failing quick state.
	CTR_TEST_CHECK(VehBirth_Player(3) == &driver, "preview reuses the supplied pool slot");
	CTR_TEST_CHECK_EQ(driver.clockReceive, 0, "the native Clock timer is cleared along with the retail timers");
	CTR_TEST_CHECK(driver.clockSend == 0 && driver.clockFlash == 0, "a newborn preview cannot emit Clock feedback from stale state");
	CTR_TEST_CHECK_EQ(driver.maskIsAku, -1, "birth preserves the unset mask sentinel");
	s_birthThread = NULL;
	data.driverModelExtras[0].model = NULL;
	data.MetaDataCharacters[0] = original;
	sdata->gGT = NULL;
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
		memset(&driver, 0xa5, sizeof(driver));
		driver.kartScale = 0.5f;
		VehBirth_TireSprites(&thread);
		CTR_TEST_CHECK_EQ(driver.kartScale, 1, "spawn replaces stale pool scale with normal scale");
		CTR_TEST_CHECK_EQ(driver.wheelSize, 0xccc, "spawn retains the retail wheel size");
		CTR_TEST_CHECK(driver.wheelSprites == ICONGROUP_GETICONS(&tires), "spawn uses the supplied tire sprites");
	}
	sdata->gGT = NULL;
}

static void TestCharacterWheels(void)
{
	CTR_TEST_SECTION("Character changes restore normal wheels or hide Oxide's wheels");
	struct Driver driver = {.kartScale = 2};
	const int characters[] = {CRASH_BANDICOOT, NITROS_OXIDE, NEO_CORTEX};
	const int sizes[] = {0xccc, 0, 0xccc};
	for (unsigned int i = 0; i < len(characters); i++)
	{
		VehBirth_SetWheelSizeForCharacter(&driver, characters[i]);
		CTR_TEST_CHECK_EQ(driver.wheelSize, sizes[i], "each character change replaces the previous wheel size");
		CTR_TEST_CHECK_EQ(driver.kartScale, 2, "character wheel selection preserves Chaos body scaling");
	}
}

static void TestOxideBirthWheels(void)
{
	CTR_TEST_SECTION("Oxide birth retains the menu exception until character selection");
	struct GameTracker tracker = {0};
	struct IconGroup tires = {0};
	struct Driver driver = {0};
	struct Thread thread = {.object = &driver};
	const int levels[] = {MAIN_MENU_LEVEL, CRASH_COVE};
	const int sizes[] = {0xccc, 0};
	sdata->gGT = &tracker;
	tracker.iconGroup[0] = &tires;
	GAME_CHARACTER_IDS[0] = NITROS_OXIDE;
	for (unsigned int i = 0; i < len(levels); i++)
	{
		tracker.levelID = levels[i];
		VehBirth_TireSprites(&thread);
		CTR_TEST_CHECK_EQ(driver.wheelSize, sizes[i], "only race birth hides Oxide's wheels immediately");
		VehBirth_SetWheelSizeForCharacter(&driver, NITROS_OXIDE);
		CTR_TEST_CHECK_EQ(driver.wheelSize, 0, "explicit character selection hides Oxide's wheels in either scene");
	}
	GAME_CHARACTER_IDS[0] = CRASH_BANDICOOT;
	sdata->gGT = NULL;
}

static void TestSpawnCollisionLod(void)
{
	CTR_TEST_SECTION("Spawn collision detail follows the loaded level, not just player count");
	struct mesh_info mesh = {0};
	struct Level level = {.ptr_mesh_info = &mesh};
	struct GameTracker tracker = {.level1 = &level};
	struct Instance instance = {0};
	struct Driver driver = {.instSelf = &instance};
	_Alignas(4) u8 scratchpad[0x1000] = {0};
	sdata->gGT = &tracker;
	gCTRNativeScratchpadBase = scratchpad;
	s_testCollisionSearch = true;
	for (int players = 1; players <= 4; players++)
	{
		tracker.numPlyrCurrGame = players;
		for (int highDetail = 0; highDetail <= 1; highDetail++)
		{
			sdata->highDetailSplitScreenLevel = highDetail;
			VehBirth_TeleportSelf(&driver, 0, 0);
			CTR_TEST_CHECK_EQ(s_searchFlags, players < 3 || highDetail ? COLL_SEARCH_HIGH_LOD : 0,
			                  "high-detail split-screen spawns use high-detail collision while retail 3/4P retains low detail");
			CTR_TEST_CHECK_EQ(driver.AxisAngle2_normalVec.y, FP_ONE, "spawn keeps the collision normal for kart orientation");
		}
	}
	s_testCollisionSearch = false;
	sdata->highDetailSplitScreenLevel = false;
	sdata->gGT = NULL;
	gCTRNativeScratchpadBase = NULL;
}

static void TestGridTransforms(void)
{
	CTR_TEST_SECTION("Expanded grid transforms extend the LEV formation without changing it");
	struct Level level = {0};
	for (unsigned int i = 0; i < len(level.DriverSpawn); i++)
	{
		int row = i / 2;
		int lane = i % 2;
		level.DriverSpawn[i].pos = (SVec3){.x = (s16)(row * 100), .y = (s16)(row * 5), .z = (s16)(lane * 120)};
		level.DriverSpawn[i].rot = (SVec3){.x = 8, .y = (s16)(ANG_PI + i), .z = -8};
	}

	SVec3 pos, rot;
	for (unsigned int i = 0; i < RACE_FIELD_CONFIG_MAX_RACERS; i++)
	{
		unsigned int source = i % len(level.DriverSpawn);
		bool expanded = i >= len(level.DriverSpawn);
		CTR_TEST_CHECK(VehStartGrid_GetTransform(&level, i, &pos, &rot), "every slot in the complete formation has a transform");
		CTR_TEST_CHECK_EQ(pos.x, level.DriverSpawn[source].pos.x + (expanded ? 400 : 0), "the extra formation starts one full grid behind");
		CTR_TEST_CHECK_EQ(pos.y, level.DriverSpawn[source].pos.y + (expanded ? 20 : 0), "the extra formation follows the initial slope");
		CTR_TEST_CHECK_EQ(pos.z, level.DriverSpawn[source].pos.z, "the formation keeps each kart in its lane");
		CTR_TEST_CHECK(rot.x == level.DriverSpawn[source].rot.x && rot.y == level.DriverSpawn[source].rot.y &&
		                   rot.z == level.DriverSpawn[source].rot.z,
		               "each slot keeps the orientation of its LEV template");
	}
	CTR_TEST_CHECK(!VehStartGrid_GetTransform(&level, RACE_FIELD_CONFIG_MAX_RACERS, &pos, &rot), "indices beyond the new grid are rejected");

	struct mesh_info mesh = {0};
	level.ptr_mesh_info = &mesh;
	struct GameTracker tracker = {.level1 = &level, .numPlyrCurrGame = 1};
	struct Instance instance = {0};
	struct Driver driver = {.instSelf = &instance};
	_Alignas(4) u8 scratchpad[0x1000] = {0};
	sdata->gGT = &tracker;
	gCTRNativeScratchpadBase = scratchpad;
	s_testCollisionSearch = true;
	u8 savedSpawn = VEH_KART_SPAWN_ORDER[0];
	VEH_KART_SPAWN_ORDER[0] = 15;
	VehBirth_TeleportSelf(&driver, VEH_BIRTH_SPAWN_USE_LEVEL_POSITION, 0);
	CTR_TEST_CHECK_EQ(driver.posCurr.x, 700 << FRACTIONAL_BITS_8, "kart birth probes the extrapolated location instead of the LEV slot");
	CTR_TEST_CHECK_EQ(driver.rotCurr.y, ANG_MODULO_TWO_PI(ANG_PI + 7 + ANG_HALF_PI), "kart birth applies the usual yaw adjustment");
	CTR_TEST_CHECK((driver.actionsFlagSet & ACTION_BEHIND_START_LINE) != 0, "an expanded slot starts behind the line");
	s_collisionMiss = true;
	driver.posCurr.x = 123;
	VehBirth_TeleportSelf(&driver, VEH_BIRTH_SPAWN_USE_LEVEL_POSITION, 0);
	CTR_TEST_CHECK_EQ(driver.posCurr.x, 123, "an expanded slot without ground never applies the previous probe's hit");
	s_collisionMiss = false;
	VEH_KART_SPAWN_ORDER[0] = savedSpawn;
	s_testCollisionSearch = false;
	sdata->gGT = NULL;
	gCTRNativeScratchpadBase = NULL;

	level.DriverSpawn[7].pos.x = INT16_MAX;
	CTR_TEST_CHECK(!VehStartGrid_GetTransform(&level, 15, &pos, &rot), "extra positions cannot wrap in LEV coordinates");
	memset(&level, 0, sizeof(level));
	CTR_TEST_CHECK(!VehStartGrid_GetTransform(&level, 8, &pos, &rot), "missing rear spacing cannot duplicate the original grid");
}

static void TestGridPreflight(void)
{
	CTR_TEST_SECTION("Expanded grid preflight retains only a safe prefix");
	struct mesh_info mesh = {0};
	struct Level level = {.ptr_mesh_info = &mesh};
	struct GameTracker tracker = {.level1 = &level, .numPlyrCurrGame = 4};
	_Alignas(struct ScratchpadStruct) u8 scratchpad[0x1000] = {0};
	gCTRNativeScratchpadBase = scratchpad;
	for (unsigned int i = 0; i < len(level.DriverSpawn); i++)
	{
		level.DriverSpawn[i].pos.x = (s16)(200 * (i / 2));
		level.DriverSpawn[i].pos.z = (s16)(200 * (i % 2));
	}

	// Probing while disabled would bail, so the retail request also checks that
	// the normal path does not pay the cost of preflight.
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 8), 8, "the full retail grid needs no probing");
	s_testCollisionSearch = true;
	s_probeCount = 0;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 16, "a wide flat grid accepts all extra slots");
	CTR_TEST_CHECK_EQ(s_probeCount, 8 * 5, "preflight probes the center and four edges only for the eight extra slots");
	CTR_TEST_CHECK_EQ(s_searchFlags, 0, "preflight uses the same low-detail collision as split-screen birth");
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 40), 16, "preflight cannot return more than the supported slots");
	s_probeMaxX = 1199;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 12, "a missed center discards that slot and all following slots");
	s_probeMaxX = 1250;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 12, "ground under the center alone does not support the kart footprint");
	s_probeMaxX = INT16_MAX;
	s_probeGround.quadFlags |= QUADBLOCK_FLAG_KILL_PLANE;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 8, "kill planes cannot support additional karts");
	s_probeGround.quadFlags = QUADBLOCK_FLAG_GROUND;
	for (unsigned int i = 1; i < len(level.DriverSpawn); i += 2)
	{
		level.DriverSpawn[i].pos.z = 90;
	}
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 9, "additional karts cannot start inside each other's collision radii");
	// Keep the same average translation but move one retail position near slot 9.
	level.DriverSpawn[6].pos.x = 790;
	level.DriverSpawn[7].pos.x = 410;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 8, "an extra kart still cannot overlap an unprobed retail slot");
	level.ptr_mesh_info = NULL;
	CTR_TEST_CHECK_EQ(VehStartGrid_ValidateSize(&tracker, 16), 8, "missing collision geometry permits no expansion");
	s_testCollisionSearch = false;
	gCTRNativeScratchpadBase = NULL;
}

int main(void)
{
	TestSpawnScale();
	TestPreviewBirthClearsClock();
	TestCharacterWheels();
	TestOxideBirthWheels();
	TestSpawnCollisionLod();
	TestGridTransforms();
	TestGridPreflight();
	return CtrTest_Summary();
}
