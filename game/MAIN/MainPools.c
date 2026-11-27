#include <common.h>
#include <race_field.h>
#include <platform/custom-tracks/native_track_library.h>

enum { MAIN_INIT_POOL_COUNT = 8 };

struct MainInitPoolSpec
{
	struct JitPool *pool;
	int count;
	int itemSize;
	char *name;
};

static void MainInit_GetPoolSpecs(struct GameTracker *gGT, struct MainInitPoolSpec *specs)
{
	u32 gameMode = gGT->gameMode1;
	int poolScale = 0x800;
	int renderBucketSize = 0x800;
	if ((gameMode & ADVENTURE_ARENA) == 0)
	{
		poolScale = (gameMode & MAIN_MENU) != 0 ? 0x400 : 0x1000;
		renderBucketSize = poolScale;
		if ((gameMode & MAIN_MENU) != 0 && gGT->levelID == ADVENTURE_GARAGE)
		{
			renderBucketSize = 0x800;
		}
	}

	const int extraRacers = RaceField_ExtraPoolSlots(sdata->loadedRaceFieldSize);
	const int extraRankDigits = sdata->loadedRaceFieldSize >= 10 ? gGT->numPlyrCurrGame : 0;
	// Each extra kart can use an instance for itself and another for its wake.
	int instanceCount = (renderBucketSize >> 5) + 2 * extraRacers + extraRankDigits;
	if (gGT->level1 != NULL && NativeTrackRegistry_Get(NativeTrackLibrary_Current(), gGT->levelID) != NULL)
	{
		enum { CUSTOM_TRACK_MAX_INSTANCE_POOL = 1024 };
		u32 available = CUSTOM_TRACK_MAX_INSTANCE_POOL - instanceCount;
		instanceCount += (int)(gGT->level1->numInstances < available ? gGT->level1->numInstances : available);
	}
	int numDriver = (gameMode & MAIN_MENU) != 0 ? 4 : poolScale >> 9;
	int numParticle = poolScale >> 5;
	const struct MainInitPoolSpec layout[MAIN_INIT_POOL_COUNT] = {
		{&gGT->JitPools.thread, ((renderBucketSize * 3) >> 7) + extraRacers + extraRankDigits, sizeof(struct Thread), rdata.s_ThreadPool},
		{&gGT->JitPools.instance, instanceCount, sizeof(struct Instance) + sizeof(struct InstDrawPerPlayer) * gGT->numPlyrCurrGame, rdata.s_InstancePool},
		{&gGT->JitPools.smallStack, ((poolScale * 0x19) >> 10) + extraRankDigits, THREAD_SMALL_STACK_SIZE, rdata.s_SmallStackPool},
		{&gGT->JitPools.mediumStack, poolScale >> 7, THREAD_MEDIUM_STACK_SIZE, rdata.s_MediumStackPool},
		{&gGT->JitPools.largeStack, numDriver + extraRacers, THREAD_LARGE_STACK_SIZE, rdata.s_LargeStackPool},
		{&gGT->JitPools.particle, numParticle, sizeof(struct Particle), rdata.s_ParticlePool},
		{&gGT->JitPools.oscillator, numParticle, 0x18, rdata.s_OscillatorPool},
		{&gGT->JitPools.rain, poolScale >> 9, sizeof(struct RainLocal), rdata.s_RainPool},
	};
	memcpy(specs, layout, sizeof(layout));
}

// Called after level assets are loaded, before pools and viewport lists are created.
int MainInit_GetPlayerModelMemoryReserve(struct GameTracker *gGT)
{
	struct MainInitPoolSpec specs[MAIN_INIT_POOL_COUNT];
	MainInit_GetPoolSpecs(gGT, specs);
	int bytes = 0;
	for (int i = 0; i < MAIN_INIT_POOL_COUNT; i++)
	{
		bytes += MEMPACK_ALIGN_SIZE(specs[i].count * specs[i].itemSize);
		if (specs[i].pool == &gGT->JitPools.thread || specs[i].pool == &gGT->JitPools.particle)
		{
			// Chaos attribution allocates one owner byte for each of these slots.
			bytes += MEMPACK_ALIGN_SIZE(specs[i].count);
		}
	}
	int numPlayers = gGT->numPlyrCurrGame;
	bytes += numPlayers * MEMPACK_ALIGN_SIZE(MainDB_GetClipSize(gGT->levelID, numPlayers) << 2);
	if (gGT->level1 != NULL && gGT->level1->ptr_mesh_info != NULL &&
	    gGT->level1->visMem != NULL)
	{
		struct Level *level = gGT->level1;
		struct mesh_info *mesh = level->ptr_mesh_info;
		if (mesh->bspRoot != NULL && ((gGT->gameMode1 & ADVENTURE_ARENA) == 0 || numPlayers > 2))
		{
			bytes += numPlayers * MEMPACK_ALIGN_SIZE(mesh->numBspNodes * sizeof(struct VisMemBspListNode));
		}
		if (sdata->highDetailSplitScreenLevel && numPlayers >= 2)
		{
			bytes += numPlayers * (MEMPACK_ALIGN_SIZE(((mesh->numBspNodes + 0x1f) >> 5) << 2) +
			                      MEMPACK_ALIGN_SIZE(((mesh->numQuadBlock + 0x1f) >> 5) << 2) +
			                      MEMPACK_ALIGN_SIZE(((level->numWaterVertices + 0x1f) >> 5) << 2) +
			                      MEMPACK_ALIGN_SIZE(((level->numSCVert + 0x1f) >> 5) << 2));
		}
	}
	return bytes;
}

void MainInit_JitPoolsReset(struct GameTracker *gGT)
{
	JitPool_Clear(&gGT->JitPools.thread);
	JitPool_Clear(&gGT->JitPools.instance);
	JitPool_Clear(&gGT->JitPools.smallStack);
	JitPool_Clear(&gGT->JitPools.mediumStack);
	JitPool_Clear(&gGT->JitPools.largeStack);
	JitPool_Clear(&gGT->JitPools.particle);
	JitPool_Clear(&gGT->JitPools.oscillator);
	JitPool_Clear(&gGT->JitPools.rain);
}

void MainInit_JitPoolsNew(struct GameTracker *gGT)
{
	struct MainInitPoolSpec specs[MAIN_INIT_POOL_COUNT];
	MainInit_GetPoolSpecs(gGT, specs);
	MEMPACK_PushState();
	for (int i = 0; i < MAIN_INIT_POOL_COUNT; i++)
	{
		JitPool_Init(specs[i].pool, specs[i].count, specs[i].itemSize, specs[i].name);
		if (specs[i].pool == &gGT->JitPools.thread)
		{
			Chaos_InitThreadOwners();
		}
		if (specs[i].pool == &gGT->JitPools.particle)
		{
			Chaos_InitParticleOwners();
		}
	}

	// NOTE(aalhendi): Native reuses static RDATA scratch for existing PC memory headroom.
	gGT->ptrRenderBucketInstance = (void *)((u32)&rdata.s_STATIC_GNORMALZ[0] + 148);
	for (int i = 0; i < 3; i++)
	{
		struct JitPool *pool = (struct JitPool *)((char *)&gGT->JitPools.smallStack + sizeof(struct JitPool) * i);
		int *pointer = (int *)pool->free.first;
		while (pointer != NULL)
		{
			*(int **)(pointer + 2) = pointer + 2;
			pointer = (int *)*pointer;
		}
	}
	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		data.PtrClipBuffer[i] = MEMPACK_AllocMem(MainDB_GetClipSize(gGT->levelID, gGT->numPlyrCurrGame) << 2, NULL);
	}
}
