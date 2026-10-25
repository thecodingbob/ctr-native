#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
struct rData rdata;
NativeConfig g_config;
static struct GameTracker s_tracker;
static struct Driver s_defaultDrivers[2];
struct OverlayRDATA_231 R231;
u8 *gCTRNativeScratchpadBase;
static struct Instance *s_spawnedOrb;
static struct Particle *s_particle;

// Orb regression checks exercise launch and teardown without world assets.
s32 MATH_VectorNormalize(SVec3 *vector)
{
	(void)vector;
	CtrTest_Bail("orb launch must not inspect target path geometry");
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
#include <game/Vehicle/VehPickupItem.c>
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
	return s_particle;
}

void PlaySound3D(u32 sound, struct Instance *inst)
{
	(void)sound;
	(void)inst;
}

// The real item dispatcher also links the other weapons; none may run here.
struct Instance *INSTANCE_BirthWithThread(s32 model, const char *name, s32 pool, s32 bucket,
                                        void *tick, s32 size, struct Thread *parent)
{
	(void)name; (void)pool; (void)bucket; (void)size; (void)parent;
	CTR_TEST_CHECK_EQ(model, WARPBALL_MODEL, "orb launch allocates the warpball model");
	s_spawnedOrb->thread->funcThTick = tick;
	return s_spawnedOrb;
}
void GAMEPAD_ShockFreq(struct Driver *driver, int frames, int value) { (void)driver; (void)frames; (void)value; }
void GAMEPAD_ShockForce1(struct Driver *driver, int frames, int value) { (void)driver; (void)frames; (void)value; }
void Voiceline_RequestPlay(u32 sound, u32 character, u32 flags) { (void)sound; (void)character; (void)flags; }
void PROC_DestroyInstance(struct Thread *thread) { (void)thread; }

s32 MixRNG_Scramble(void) { CtrTest_Bail("orb launch needs no retail RNG"); return 0; }
s32 RngDeadCoed(struct RngDeadCoedState *state) { (void)state; CtrTest_Bail("orb launch needs no potion RNG"); return 0; }
b32 LOAD_IsOpen_RacingOrBattle(void) { CtrTest_Bail("orb launch needs no mask overlay check"); return false; }
void RB_MaskWeapon_ThTick(struct Thread *thread) { (void)thread; CtrTest_Bail("orb launch must not tick a mask"); }
void RB_MovingExplosive_ThTick(struct Thread *thread) { (void)thread; CtrTest_Bail("orb launch must not tick a missile"); }
void RB_GenericMine_ThTick(struct Thread *thread) { (void)thread; CtrTest_Bail("orb launch must not tick a mine"); }
void RB_ShieldDark_ThTick_Grow(struct Thread *thread) { (void)thread; CtrTest_Bail("orb launch must not tick a shield"); }
void PROC_DestroyTracker(struct Thread *thread) { (void)thread; CtrTest_Bail("orb launch must not destroy a tracker"); }
void RB_MinePool_Add(struct MineWeapon *mine) { (void)mine; CtrTest_Bail("orb launch must not create a mine"); }
struct Instance *INSTANCE_Birth3D(struct Model *model, const char *name, struct Thread *thread)
{
	(void)model; (void)name; (void)thread;
	CtrTest_Bail("orb launch needs only its threaded instance");
	return NULL;
}
int OtherFX_Play(u32 sound, int flags) { (void)sound; (void)flags; CtrTest_Bail("orb launch uses spatial audio"); return 0; }
void OtherFX_Play_Echo(u32 sound, int flags, int echo) { (void)sound; (void)flags; (void)echo; CtrTest_Bail("orb launch uses spatial audio"); }
void VehFire_Increment(struct Driver *driver, int reserves, u32 type, int level)
{
	(void)driver; (void)reserves; (void)type; (void)level;
	CtrTest_Bail("orb launch must not grant turbo");
}
void VehPhysForce_RotAxisAngle(MATRIX *matrix, s16 *normal, s32 angle)
{
	(void)matrix; (void)normal; (void)angle;
	CtrTest_Bail("orb launch must not rotate a missile");
}
void CTR_MatrixToRot(SVECTOR *rot, MATRIX *matrix, u32 flags)
{
	(void)rot; (void)matrix; (void)flags;
	CtrTest_Bail("orb launch must not rotate a missile");
}
int RB_Hazard_ThCollide_Missile(struct Thread *thread, struct Thread *other, void *collide, struct ScratchpadStruct *sps)
{
	(void)thread; (void)other; (void)collide; (void)sps;
	CtrTest_Bail("orb launch must not collide a missile");
	return 0;
}
int RB_Hazard_ThCollide_Generic(struct Thread *thread, struct Thread *other, void *collide, struct ScratchpadStruct *sps)
{
	(void)thread; (void)other; (void)collide; (void)sps;
	CtrTest_Bail("orb launch must not collide a mine");
	return 0;
}
void Chaos_ApplyItemRules(struct Driver *driver) { (void)driver; CtrTest_Bail("direct launch does not process item input"); }
int VehPickState_NewState(struct Driver *victim, int damage, struct Driver *attacker, int reason)
{
	(void)victim; (void)damage; (void)attacker; (void)reason;
	CtrTest_Bail("orb launch must not process pending damage");
	return 0;
}
void CTC2(u32 value, s32 reg) { (void)value; (void)reg; CtrTest_Bail("orb launch needs no projection"); }
void MTC2(u32 value, s32 reg) { (void)value; (void)reg; CtrTest_Bail("orb launch needs no projection"); }
s32 doCOP2(s32 op) { (void)op; CtrTest_Bail("orb launch needs no projection"); return 0; }
s32 MFC2_S(s32 reg) { (void)reg; CtrTest_Bail("orb launch needs no projection"); return 0; }
u32 CFC2(s32 reg) { (void)reg; CtrTest_Bail("orb launch needs no projection"); return 0; }
void NativePgxp_LoadTransform(const void *key, const s16 *rotation, const s32 *translation, int bank)
{
	(void)key; (void)rotation; (void)translation; (void)bank;
	CtrTest_Bail("orb launch needs no projection");
}

static void SetupArena(void)
{
	static struct Instance kart;
	sdata->gGT = &s_tracker;
	s_tracker.numPlyrCurrGame = 2;
	for (int i = 0; i < 2; i++)
	{
		s_defaultDrivers[i].driverID = i;
		s_defaultDrivers[i].instSelf = &kart;
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
	instance.thread = &thread;
	s_spawnedOrb = &instance;
	s_tracker.level1 = &level;
	s_tracker.gameMode1 |= CRYSTAL_CHALLENGE | WARPBALL_HELD;
	s_defaultDrivers[1].posCurr.x = 100 << 8;
	VehPickupItem_ShootNow(&s_defaultDrivers[0], WEAPON_ID_WARPBALL, 0);
	CTR_TEST_CHECK(orb.ptrNodeCurr == NULL && orb.ptrNodeNext == NULL, "arena launch leaves race-node pointers empty");
	CTR_TEST_CHECK(orb.driverTarget == NULL, "a crystal orb never selects another driver as a target");
	CTR_TEST_CHECK(thread.funcThTick == RB_Warpball_FadeAway, "a crystal orb starts fading immediately on spawn");
	s_tracker.drivers[1] = NULL;
	VehPickupItem_ShootNow(&s_defaultDrivers[0], WEAPON_ID_WARPBALL, 0);
	CTR_TEST_CHECK(orb.driverTarget == NULL, "solo crystal launch needs no target or checkpoint path");
	CTR_TEST_CHECK(thread.funcThTick == RB_Warpball_FadeAway, "a solo crystal orb also fades immediately without waiting for impact");
	for (int i = 0; i < 6; i++)
	{
		RB_Warpball_FadeAway(&thread);
	}
	CTR_TEST_CHECK((thread.flags & THREAD_FLAG_DEAD) != 0, "an arena orb completes teardown even without an allocated particle");
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) == 0, "orb teardown releases inventory ownership");
	struct Particle particle = {.framesLeftInLife = 10};
	s_particle = &particle;
	thread.flags = 0;
	s_tracker.gameMode1 = BATTLE_MODE | WARPBALL_HELD;
	VehPickupItem_ShootNow(&s_defaultDrivers[0], WEAPON_ID_WARPBALL, 0);
	CTR_TEST_CHECK(thread.funcThTick == RB_Warpball_FadeAway, "battle arenas use the same immediate fade without a race path");
	CTR_TEST_CHECK_EQ(particle.framesLeftInLife, 0, "immediate teardown also stops an allocated orb particle");

	struct CheckpointNode nodes[2] = {
		{.distToFinish = 1000, .nextIndex_forward = 1, .nextIndex_left = 0xff},
		{.distToFinish = 500, .nextIndex_forward = 0, .nextIndex_left = 0xff},
	};
	level.ptr_restart_points = nodes;
	level.cnt_restart_points = 2;
	s_tracker.gameMode1 = 0;
	thread.funcThTick = RB_Warpball_ThTick;
	VehPickupItem_ShootNow(&s_defaultDrivers[0], WEAPON_ID_WARPBALL, 0);
	CTR_TEST_CHECK(orb.ptrNodeCurr == &nodes[1] && orb.ptrNodeNext == &nodes[0], "normal tracks retain checkpoint-based launch and lookahead");
	CTR_TEST_CHECK(thread.funcThTick == RB_Warpball_ThTick, "race tracks keep the ordinary orb update instead of immediate fading");
}

int main(void)
{
	TestArenaWarpball();
	return CtrTest_Summary();
}
