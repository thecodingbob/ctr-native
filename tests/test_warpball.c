#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
static struct GameTracker s_tracker;
static struct Driver s_defaultDrivers[2];
struct OverlayRDATA_231 R231;
u8 *gCTRNativeScratchpadBase;

// Orb regression checks exercise launch, targeting and teardown without world assets.
s32 MATH_VectorNormalize(SVec3 *vector)
{
	(void)vector;
	CtrTest_Bail("arena orb targeting must not inspect a race path");
	return 0;
}
void OtherFX_RecycleMute(u32 *sound) { *sound = 0; }
void ThTick_SetAndExec(struct Thread *thread, void (*tick)(struct Thread *))
{
	thread->funcThTick = tick;
	tick(thread);
}
s32 INSTANCE_GetNumAnimFrames(struct Instance *inst, s32 anim)
{
	(void)inst; (void)anim;
	return 1;
}
struct Thread *RB_GetThread_ClosestTracker(struct Driver *driver) { (void)driver; return NULL; }
int SquareRoot0_stub(int value) { (void)value; CtrTest_Bail("orb launch must not advance race segments"); return 0; }
void PlaySound3D_Flags(u32 *flags, u32 sound, struct Instance *inst) { (void)flags; (void)sound; (void)inst; }
void COLL_SearchBSP_CallbackQUADBLK(const SVec3 *top, const SVec3 *bottom, struct ScratchpadStruct *sps, s32 radius)
{
	(void)top; (void)bottom; (void)sps; (void)radius;
	CtrTest_Bail("orb launch must not run world collision");
}
void RB_MakeInstanceReflective(struct ScratchpadStruct *sps, struct Instance *inst) { (void)sps; (void)inst; }
struct Instance *RB_Hazard_CollideWithDrivers(struct Instance *inst, s16 safety, int radius, struct Instance *parent)
{
	(void)inst; (void)safety; (void)radius; (void)parent;
	CtrTest_Bail("orb launch must not test driver hits");
	return NULL;
}
int RB_Hazard_HurtDriver(struct Driver *victim, int damage, struct Driver *attacker, int reason)
{
	(void)victim; (void)damage; (void)attacker; (void)reason;
	CtrTest_Bail("orb launch must not hurt a driver");
	return 0;
}
s32 RB_Hazard_CollLevInst(struct ScratchpadStruct *sps, struct Thread *thread) { (void)sps; (void)thread; return 0; }
struct Instance *RB_Hazard_CollideWithBucket(struct Instance *inst, struct Thread *thread, struct Thread *bucket,
                                          s16 safety, int radius, struct Instance *parent)
{
	(void)inst; (void)thread; (void)bucket; (void)safety; (void)radius; (void)parent;
	return NULL;
}

#include <game/231/RB_Warpball.c>
#include <game/MATH/MATH_0_Sin.c>

int ratan2(int y, int x)
{
	(void)y; (void)x;
	CtrTest_Bail("orb launch must not calculate angles");
	return 0;
}

void ConvertRotToMatrix(MATRIX *matrix, const SVec3 *rotation)
{
	(void)matrix; (void)rotation;
	CtrTest_Bail("orb launch must not animate matrices");
}

struct Particle *Particle_Init(u32 flags, struct IconGroup *icons, struct ParticleEmitter *emitter)
{
	(void)flags; (void)icons; (void)emitter;
	CtrTest_Bail("orb launch must not spawn particles");
	return NULL;
}

void PlaySound3D(u32 sound, struct Instance *inst)
{
	(void)sound;
	(void)inst;
}

static void SetupArena(void)
{
	sdata->gGT = &s_tracker;
	s_tracker.numPlyrCurrGame = 2;
	for (int i = 0; i < 2; i++)
	{
		s_defaultDrivers[i].driverID = i;
		s_tracker.drivers[i] = &s_defaultDrivers[i];
	}
}

static void TestArenaWarpball(void)
{
	CTR_TEST_SECTION("orb launch and teardown do not require race checkpoints in crystal arenas");
	SetupArena();
	struct Level level = {0};
	struct Instance instance = {0};
	struct TrackerWeapon orb = {0};
	struct Thread thread = {.inst = &instance, .object = &orb};
	s_tracker.level1 = &level;
	s_tracker.gameMode1 |= CRYSTAL_CHALLENGE | WARPBALL_HELD;
	s_defaultDrivers[1].posCurr.x = 100 << 8;
	RB_Warpball_InitPath(&orb, &instance, &s_defaultDrivers[0]);
	CTR_TEST_CHECK(orb.ptrNodeCurr == NULL && orb.ptrNodeNext == NULL, "arena launch leaves race-node pointers empty");
	CTR_TEST_CHECK(orb.driverTarget == &s_defaultDrivers[1], "an arena orb can target a co-op partner even when both have rank zero");
	CTR_TEST_CHECK((orb.flags & TRACKER_FLAG_WARPBALL_TARGET_PATH) != 0, "arena targeting reuses the orb's direct homing movement");
	s_defaultDrivers[1].kartState = KS_MASK_GRABBED;
	CTR_TEST_CHECK(RB_Warpball_GetDriverTarget(&orb, &instance) == NULL, "arena targeting excludes rescued drivers");
	s_defaultDrivers[1].kartState = KS_NORMAL;
	orb.driversHit |= 1u << 1;
	CTR_TEST_CHECK(RB_Warpball_GetDriverTarget(&orb, &instance) == NULL, "a powered arena orb cannot retarget someone already hit");
	s_tracker.drivers[1] = NULL;
	RB_Warpball_InitPath(&orb, &instance, &s_defaultDrivers[0]);
	CTR_TEST_CHECK(orb.driverTarget == NULL, "solo crystal launch needs no target or checkpoint path");
	RB_Warpball_TurnAround(&thread);
	CTR_TEST_CHECK(thread.funcThTick == RB_Warpball_FadeAway, "an arena orb fades on impact without trying to reverse a race path");
	for (int i = 0; i < 6; i++)
	{
		RB_Warpball_FadeAway(&thread);
	}
	CTR_TEST_CHECK((thread.flags & THREAD_FLAG_DEAD) != 0, "an arena orb completes teardown even without an allocated particle");
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) == 0, "orb teardown releases inventory ownership");

	struct CheckpointNode nodes[2] = {
		{.distToFinish = 1000, .nextIndex_forward = 1, .nextIndex_left = 0xff},
		{.distToFinish = 500, .nextIndex_forward = 0, .nextIndex_left = 0xff},
	};
	level.ptr_restart_points = nodes;
	level.cnt_restart_points = 2;
	s_tracker.gameMode1 &= ~CRYSTAL_CHALLENGE;
	RB_Warpball_InitPath(&orb, &instance, &s_defaultDrivers[0]);
	CTR_TEST_CHECK(orb.ptrNodeCurr == &nodes[1] && orb.ptrNodeNext == &nodes[0], "normal tracks retain checkpoint-based launch and lookahead");
}

int main(void)
{
	TestArenaWarpball();
	return CtrTest_Summary();
}
