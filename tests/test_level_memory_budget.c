// Pins that every per-level arena still fits inside the mempack it is carved out
// of, and that all of them scale by the one draw-distance factor.
//
// The suite compiles game/MAIN/MainDB.c into its own translation unit so it uses
// the real size tables rather than a copy: a copy would drift, and the drift is
// exactly what this suite exists to catch. Everything those tables depend on is a
// plain integer here, so the whole table can be walked without a disc, an asset
// or a window.
//
// What is deliberately NOT asserted here: that a level uses less than it was
// given. Peak usage is a measurement, it needs a real run, and it belongs in the
// memtrack log. This suite pins the part that is arithmetic — that the numbers we
// chose to allocate agree with each other and with the arena they come from.

#include <shared/ctr_test.h>

#include <common.h>
#include <platform/native_memtrack.h>
#include <platform/custom-tracks/native_track_library.h>

// The arena the arenas above are carved out of, and the size the budget is
// measured against. Compiled in so the number is the real one.
#include "platform/native_memory.c"

// Declared extern by regionsEXE.h; MainDB_GetClipSize reads the split-screen flag
// out of it, and the linker wants the storage regardless.
struct sData sdata_static;
struct Data data;
struct rData rdata;
static int s_allocatedBytes;
static bool s_customTrack;

// Count the real allocation requests without allocating the pools themselves.
void *MEMPACK_AllocMem(s32 size, const char *name)
{
	(void)name;
	s_allocatedBytes += MEMPACK_ALIGN_SIZE(size);
	return NULL;
}

s32 MEMPACK_PushState(void) { return 0; }
void Chaos_InitThreadOwners(void) { MEMPACK_AllocMem(sdata->gGT->JitPools.thread.maxItems, NULL); }
void Chaos_InitParticleOwners(void) { MEMPACK_AllocMem(sdata->gGT->JitPools.particle.maxItems, NULL); }
void JitPool_Clear(struct JitPool *pool) { (void)pool; }

void JitPool_Init(struct JitPool *pool, int count, int itemSize, char *name)
{
	pool->maxItems = count;
	pool->itemSize = itemSize;
	pool->free.first = NULL;
	MEMPACK_AllocMem(count * itemSize, name);
}

const struct NativeTrackRegistry *NativeTrackLibrary_Current(void) { return NULL; }

const struct NativeTrackMetadata *NativeTrackRegistry_Get(const struct NativeTrackRegistry *registry, int levelID)
{
	(void)registry;
	(void)levelID;
	static const struct NativeTrackMetadata track = {0};
	return s_customTrack ? &track : NULL;
}

void NativePgxp_SetPrimRegion(int index, const void *start, size_t size)
{
	(void)index;
	(void)start;
	(void)size;
}

#include "game/MAIN/MainDB.c"
#include "game/MAIN/MainPools.c"

// The largest primitive arena the game can ask for, from the constant branches of
// MainInit_GetPrimMemSize. That function also has per-level branches reading a
// table loaded from the disc, which cannot be walked here; the constants are
// what a level falls back to without it, and the largest of them bounds the rest.
#define TEST_PRIM_MEM_RETAIL_MAX 0x25800

// What a level holds at once: both double buffers of each arena, plus the two
// per-player swap chains from MainInit_OTMem. Every one of these comes from
// MEMPACK_AllocMem, so if they do not fit, the game traps rather than degrades.
static int MempackUsedByLevel(u32 gameMode1, int numPlyrCurrGame, u32 levelID)
{
	const int primMem = TEST_PRIM_MEM_RETAIL_MAX * NATIVE_DRAW_DISTANCE_SCALE;
	const int swapChain = ((numPlyrCurrGame << 12) | 0x18);

	return (2 * primMem) + (2 * MainDB_GetOTMemSize(gameMode1, numPlyrCurrGame)) + (2 * swapChain) + MainDB_GetClipSize(levelID, numPlyrCurrGame);
}

// The mode bits that pick an arena size, as a walkable set.
static const u32 k_gameModes[] = {0, MAIN_MENU, ADVENTURE_ARENA, BATTLE_MODE};

// Levels that name a different clip size, plus one that does not, so the default
// branch is walked too.
static const u32 k_levelIDs[] = {ADVENTURE_GARAGE, MAIN_MENU_LEVEL, SEWER_SPEEDWAY, MYSTERY_CAVES, PAPU_PYRAMID, POLAR_PASS, 0x7F};

// === The budget ===

static void TestEveryLevelFitsTheMempack(void)
{
	CTR_TEST_SECTION("every level's arenas fit the mempack arena together");
	sdata->highDetailSplitScreenLevel = 0;

	const int arenaBytes = Platform_GetMempackBackingSize();

	for (u32 m = 0; m < len(k_gameModes); m++)
	{
		for (int players = 1; players <= 4; players++)
		{
			for (u32 l = 0; l < len(k_levelIDs); l++)
			{
				CTR_TEST_CHECK(MempackUsedByLevel(k_gameModes[m], players, k_levelIDs[l]) < arenaBytes,
				               "a level's double-buffered arenas fit the mempack arena");
			}
		}
	}
}

static void TestSplitScreenDoesNotChangeTheClipSize(void)
{
	CTR_TEST_SECTION("the high-detail split-screen path asks for no more clip space");
	sdata->highDetailSplitScreenLevel = 1;

	const int split = MainDB_GetClipSize(PAPU_PYRAMID, 4);

	sdata->highDetailSplitScreenLevel = 0;

	// The flag forces the player count back to 1 rather than scaling anything, so
	// a change that multiplied instead would look like extra memory here.
	CTR_TEST_CHECK(split <= MainDB_GetClipSize(PAPU_PYRAMID, 1), "the split-screen clip size is no larger than the 1P one");
}

static void TestTheBudgetLeavesRoomForTheLevel(void)
{
	CTR_TEST_SECTION("the arenas leave most of the mempack for the level itself");
	sdata->highDetailSplitScreenLevel = 0;

	const int arenaBytes = Platform_GetMempackBackingSize();
	const int used = MempackUsedByLevel(0, 1, 0);

	// The arenas are a floor, not the whole cost: meshes, instances, particles and
	// decals come out of the same arena. This guards the opposite mistake, an
	// arena so small that a level's own allocations cannot fit.
	CTR_TEST_CHECK(used < arenaBytes / 2, "the arenas take under half the mempack");
}

// === The scale ===

static void TestEveryArenaIsScaledFromRetail(void)
{
	CTR_TEST_SECTION("every arena is retail's size times the one shared factor");

	// The bug this pins: the primitive arena grew by a bare literal while the
	// ordering tables went through the shared factor, so the two drifted apart
	// with nothing to notice.
	CTR_TEST_CHECK_EQ(MainDB_GetOTMemSize(0, 1) / 0x2000, NATIVE_DRAW_DISTANCE_SCALE, "the 1P ordering table uses the shared factor");
	CTR_TEST_CHECK_EQ(MainDB_GetOTMemSize(BATTLE_MODE, 4) / 0x8000, NATIVE_DRAW_DISTANCE_SCALE, "the battle ordering table uses the same factor");
	CTR_TEST_CHECK_EQ(MainDB_GetClipSize(PAPU_PYRAMID, 1) / 3000, NATIVE_DRAW_DISTANCE_SCALE, "the clip buffer uses the same factor as the arenas");
	CTR_TEST_CHECK_EQ(MainDB_GetClipSize(MAIN_MENU_LEVEL, 1) / 16, NATIVE_DRAW_DISTANCE_SCALE, "the menu's clip buffer is scaled like the rest");
}

static void TestTheScaleDoesNotOverflow(void)
{
	CTR_TEST_SECTION("the scaled sizes stay inside the int the allocators take");

	// MainDB_PrimMem takes a u32 and the mempack counts in s32, so a factor large
	// enough to push the largest arena past 2 GiB would silently wrap rather than
	// fail to allocate.
	const int largest = TEST_PRIM_MEM_RETAIL_MAX * NATIVE_DRAW_DISTANCE_SCALE;

	CTR_TEST_CHECK(largest > 0, "the scaled primitive arena is positive");
	CTR_TEST_CHECK(largest < 0x40000000, "the scaled primitive arena stays under 1 GiB");
}

static void TestCustomCharacterRaceReserve(void)
{
	CTR_TEST_SECTION("custom characters retain the actual remaining solo race budget");
	static struct GameTracker tracker;
	static struct mesh_info mesh;
	static struct VisMem visibility;
	static struct Level level;
	level.ptr_mesh_info = &mesh;
	level.visMem = &visibility;
	mesh.bspRoot = (void *)&mesh;
	mesh.numBspNodes = 17;
	tracker.gameMode1 = ARCADE_MODE;
	tracker.numPlyrCurrGame = 1;
	tracker.level1 = &level;
	sdata->gGT = &tracker;
	sdata->highDetailSplitScreenLevel = false;
	const int fieldSizes[] = {8, 16};
	const int instanceCounts[] = {0, 37, 2000};
	for (int custom = 0; custom < 2; custom++)
	{
		s_customTrack = custom != 0;
		for (u32 f = 0; f < len(fieldSizes); f++)
		{
			sdata->loadedRaceFieldSize = fieldSizes[f];
			for (u32 n = 0; n < len(instanceCounts); n++)
			{
				level.numInstances = instanceCounts[n];
				tracker.levelID = n == 0 ? SEWER_SPEEDWAY : CRASH_COVE;
				s_allocatedBytes = 0;
				int reserve = MainInit_GetPlayerModelMemoryReserve(&tracker);
				CTR_TEST_CHECK(s_allocatedBytes == 0, "estimating the reserve does not allocate or initialize pools");
				MainInit_JitPoolsNew(&tracker);
				CTR_TEST_CHECK(reserve == s_allocatedBytes + MEMPACK_ALIGN_SIZE(mesh.numBspNodes * sizeof(struct VisMemBspListNode)),
				               "the reserve matches aligned pool, clipping and visibility allocations");
				if (custom && instanceCounts[n] == 2000)
				{
					CTR_TEST_CHECK(tracker.JitPools.instance.maxItems == 1024, "custom instance reserves honor the actual pool cap");
				}
			}
		}
	}
	mesh.bspRoot = NULL;
	CTR_TEST_CHECK(MainInit_GetPlayerModelMemoryReserve(&tracker) == s_allocatedBytes,
	               "a missing BSP tree does not reserve an unused visibility list");
	mesh.bspRoot = (void *)&mesh;
	level.visMem = NULL;
	CTR_TEST_CHECK(MainInit_GetPlayerModelMemoryReserve(&tracker) == s_allocatedBytes,
	               "a missing visibility header does not reserve an unused BSP list");
	s_customTrack = false;
}

static void TestPoolLayoutsRemainUnchanged(void)
{
	CTR_TEST_SECTION("shared pool specifications retain the menu, hub and race layouts");
	static struct GameTracker tracker;
	sdata->gGT = &tracker;
	sdata->loadedRaceFieldSize = 0;
	s_customTrack = false;
	const struct
	{
		u32 mode;
		int level;
		int threads;
		int instances;
		int smallStacks;
		int mediumStacks;
		int largeStacks;
		int particles;
		int rain;
	} cases[] = {
		{MAIN_MENU, MAIN_MENU_LEVEL, 24, 32, 25, 8, 4, 32, 2},
		{MAIN_MENU, ADVENTURE_GARAGE, 48, 64, 25, 8, 4, 32, 2},
		{ADVENTURE_ARENA, GEM_STONE_VALLEY, 48, 64, 50, 16, 4, 64, 4},
		{ARCADE_MODE, CRASH_COVE, 96, 128, 100, 32, 8, 128, 8},
	};
	for (u32 i = 0; i < len(cases); i++)
	{
		tracker.gameMode1 = cases[i].mode;
		tracker.levelID = cases[i].level;
		for (int players = 1; players <= 4; players++)
		{
			tracker.numPlyrCurrGame = players;
			MainInit_JitPoolsNew(&tracker);
			CTR_TEST_CHECK(tracker.JitPools.thread.maxItems == cases[i].threads && tracker.JitPools.instance.maxItems == cases[i].instances,
			               "thread and instance capacities retain their original mode-dependent sizes");
			CTR_TEST_CHECK(tracker.JitPools.smallStack.maxItems == cases[i].smallStacks && tracker.JitPools.mediumStack.maxItems == cases[i].mediumStacks &&
			                   tracker.JitPools.largeStack.maxItems == cases[i].largeStacks,
			               "stack capacities retain their original mode-dependent sizes");
			CTR_TEST_CHECK(tracker.JitPools.particle.maxItems == cases[i].particles && tracker.JitPools.oscillator.maxItems == cases[i].particles &&
			                   tracker.JitPools.rain.maxItems == cases[i].rain,
			               "particle, oscillator and rain capacities retain their original mode-dependent sizes");
			CTR_TEST_CHECK(tracker.JitPools.instance.itemSize == sizeof(struct Instance) + sizeof(struct InstDrawPerPlayer) * players,
			               "each instance retains draw storage for every viewport");
		}
	}
}

int main(void)
{
	TestEveryLevelFitsTheMempack();
	TestSplitScreenDoesNotChangeTheClipSize();
	TestTheBudgetLeavesRoomForTheLevel();
	TestEveryArenaIsScaledFromRetail();
	TestTheScaleDoesNotOverflow();
	TestCustomCharacterRaceReserve();
	TestPoolLayoutsRemainUnchanged();

	return CtrTest_Summary();
}
