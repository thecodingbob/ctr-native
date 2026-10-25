#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
struct Data data;
NativeConfig g_config;
static struct GameTracker s_tracker;
static struct Driver s_defaultDrivers[2];
static bool s_teleported;
static bool s_allHumansFinished;
static int s_activationSounds;
static int s_clockSounds;
static int s_squishSounds;
static int s_crateExplosions;
static int s_crateSounds;
static struct Instance *s_lastExplodingCrate;
static u32 s_lastSound;
static u32 s_lastSoundFlags;
static int s_noticeDraws;
static const char *s_noticeText;
static s16 s_noticeX;
static s16 s_noticeY;
static s16 s_noticeFont;
static u16 s_noticeFlags;
static u32 *s_noticeOT;
static const char *s_drawText[CHAOS_EFFECT_COUNT];
static s16 s_drawX[CHAOS_EFFECT_COUNT];
static s16 s_drawY[CHAOS_EFFECT_COUNT];
static s16 s_drawFont[CHAOS_EFFECT_COUNT];
static s32 s_drawScale[CHAOS_EFFECT_COUNT];
static u32 *s_drawOT[CHAOS_EFFECT_COUNT];
static int s_maskGrants;
static int s_shieldGrants;
static int s_itemSounds;
static u32 s_itemSound;
static int s_juicedSounds;
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

void VehStuckProc_RIP_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Wumpa crediting must not eliminate a driver");
}

void MainGameEnd_Initialize(void)
{
	CtrTest_Bail("Wumpa crediting must not end the race");
}

s32 MixRNG_Scramble(void)
{
	CtrTest_Bail("Chaos item selection must use its dedicated RNG");
	return 0;
}

b32 VehPickupItem_ApplyMaskMode(struct Driver *driver)
{
	(void)driver;
	return true;
}

struct MaskHeadWeapon *VehPickupItem_MaskUseWeapon(struct Driver *driver, b32 playSound)
{
	CTR_TEST_CHECK_EQ(driver->maskIsAku, -1, "direct mask activation ignores the held item's mask choice");
	CTR_TEST_CHECK(playSound, "direct mask activation requests the normal power-up sound");
	s_maskGrants++;
	return NULL;
}

void VehPickupItem_ShootNow(struct Driver *driver, s32 weapon, s32 flags)
{
	(void)driver;
	CTR_TEST_CHECK_EQ(flags, 0, "Chaos power-ups use the normal activation flags");
	if (weapon == HELD_ITEM_MASK)
	{
		s_maskGrants++;
	}
	else if (weapon == HELD_ITEM_SHIELD)
	{
		s_shieldGrants++;
	}
	else
	{
		CtrTest_Bail("Chaos direct grants must dispatch a mask or shield");
	}
}

// VehPickupItem.c precedes Chaos in the game unity build.
enum { SOUND_CLOCK = 0x44 };

#include <game/Vehicle/VehPhysCrash.c>
#include <game/Vehicle/VehPhysForce.c>
#include <game/Vehicle/VehPickState.c>
#include <game/Vehicle/VehPhysGeneral.c>
// Keep the damage-path guard double while exercising real Wumpa crediting.
#define RB_Player_KillPlayer RB_Player_KillPlayer_Unused
#include <game/231/RB_Player.c>
#undef RB_Player_KillPlayer
#include <game/231/RB_Warpball.c>
#include <game/231/RB_Chaos.c>

static const char *s_externalCommand;
static int s_externalReads;
static int s_externalErrors;
static char s_externalCatalogue[NATIVE_CHAOS_CATALOGUE_SIZE];
static int s_cataloguePublications;

int NativeChaosIPC_ReadCommand(char *command, size_t capacity)
{
	s_externalReads++;
	if (s_externalCommand == NULL)
	{
		return NATIVE_CHAOS_NO_COMMAND;
	}
	snprintf(command, capacity, "%s", s_externalCommand);
	s_externalCommand = NULL;
	return NATIVE_CHAOS_COMMAND;
}

void NativeChaosIPC_WriteEffectsCatalogue(const char *catalogue)
{
	s_cataloguePublications++;
	snprintf(s_externalCatalogue, sizeof(s_externalCatalogue), "%s", catalogue);
}

void Platform_LogError(const char *format, ...)
{
	(void)format;
	s_externalErrors++;
}
#include <game/MAIN/MainGameStart.c>
#include <game/Vehicle/VehPhysProc_Input.c>
#include <game/Vehicle/VehFire.c>
#include <game/Vehicle/VehAfterColl.c>
#include <game/UI/UI_Lerp2D.c>
#include <game/UI/UI_Chaos.c>
#include <game/MATH/MATH_0_Sin.c>

s32 DecalFont_GetLineWidth(char *text, s16 font)
{
	return (s32)strlen(text) * (font == FONT_SMALL ? 4 : 8);
}

void DecalFont_DrawLineOT(char *text, s16 x, s16 y, s16 font, s16 flags, u32 *ot)
{
	if (s_noticeDraws >= CHAOS_EFFECT_COUNT)
	{
		CtrTest_Bail("the Chaos HUD must draw at most one label per catalogue effect");
	}
	s_drawText[s_noticeDraws] = text;
	s_drawX[s_noticeDraws] = x;
	s_drawY[s_noticeDraws] = y;
	s_drawFont[s_noticeDraws] = font;
	s_drawOT[s_noticeDraws] = ot;
	s_drawScale[s_noticeDraws] = FP(1.0);
	s_noticeDraws++;
	s_noticeText = text;
	s_noticeX = x;
	s_noticeY = y;
	s_noticeFont = font;
	s_noticeFlags = (u16)flags;
	s_noticeOT = ot;
}

void DecalFont_DrawLineScaledOT(char *text, s16 x, s16 y, s16 font, s16 flags, s32 scale, u32 *ot)
{
	DecalFont_DrawLineOT(text, x, y, font, flags, ot);
	s_drawScale[s_noticeDraws - 1] = scale;
}

static s32 LatestNoticeRemaining(void)
{
	const struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	return runtime->numActive > 0 ? runtime->active[runtime->numActive - 1].noticeRemainingMS : 0;
}

b32 MainGameEnd_AllHumansFinished(struct GameTracker *gGT)
{
	(void)gGT;
	return s_allHumansFinished;
}

void VehBirth_TeleportAll(struct GameTracker *gGT, u32 spawnFlags)
{
	(void)gGT;
	(void)spawnFlags;
	s_teleported = true;
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "race setup runs after vehicle initialization");
}

void Music_Stop(void) {}
void RB_Burst_InitVisual(struct Instance *inst)
{
	s_crateExplosions++;
	s_lastExplodingCrate = inst;
}
void PlaySound3D(u32 soundID, struct Instance *inst)
{
	if (soundID == 0x4f)
	{
		return;
	}
	CTR_TEST_CHECK_EQ(soundID, HOWL_SFX_TNT_EXPLOSION, "explosive crates use explosion audio");
	CTR_TEST_CHECK(inst == s_lastExplodingCrate, "explosion audio originates at the touched crate");
	s_crateSounds++;
}
void VehPhysProc_SpinFirst_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
}
void VehStuckProc_PlantEaten_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Clock Curse must select spin damage");
}
void VehStuckProc_Tumble_Init(struct Thread *thread, struct Driver *driver)
{
	(void)thread;
	(void)driver;
	CtrTest_Bail("Clock Curse must select spin damage");
}
void GAMEPAD_ShockFreq(struct Driver *driver, int frame, int value)
{
	(void)driver;
	(void)frame;
	(void)value;
}
void OtherFX_Play_Echo(u32 soundID, int flags, int echoFlag)
{
	(void)flags;
	(void)echoFlag;
	CTR_TEST_CHECK_EQ(soundID, VEH_PICK_SOUND_SQUISH, "squish damage uses the retail audio");
	s_squishSounds++;
}
void RB_Player_KillPlayer(struct Driver *attacker, struct Driver *victim)
{
	(void)attacker;
	(void)victim;
	CtrTest_Bail("Clock Curse must not attribute damage to an attacker");
}
void CTC2(u32 value, s32 reg)
{
	(void)value;
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
}
void MTC2(u32 value, s32 reg)
{
	(void)value;
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
}
s32 MFC2_S(s32 reg)
{
	(void)reg;
	CtrTest_Bail("clock damage without an attacker needs no projection");
	return 0;
}
s32 doCOP2(s32 op)
{
	(void)op;
	CtrTest_Bail("clock damage without an attacker needs no projection");
	return 0;
}
int OtherFX_Play(u32 soundID, int flags)
{
	if (soundID == 0x5e || soundID == 0x41)
	{
		CTR_TEST_CHECK_EQ(flags, soundID == 0x41, "item acquisition retains the normal Wumpa-dependent sound flags");
		s_itemSounds++;
		s_itemSound = soundID;
		if (soundID == 0x41)
		{
			s_juicedSounds++;
		}
		return 0;
	}
	CTR_TEST_CHECK_EQ(soundID, SOUND_CLOCK, "Clock Curse plays the retail clock sound");
	s_clockSounds++;
	return 0;
}
int OtherFX_Play_LowLevel(u32 soundID, u8 antiSpam, u32 flags)
{
	(void)antiSpam;
	s_activationSounds++;
	s_lastSound = soundID;
	s_lastSoundFlags = flags;
	return 0;
}

void howl_StopAudio(b32 backup, b32 music, b32 fx)
{
	(void)backup;
	(void)music;
	(void)fx;
}

void GhostTape_WriteBoosts(int addReserve, u32 type, int speedCap)
{
	(void)addReserve;
	(void)type;
	(void)speedCap;
}

void Voiceline_RequestPlay(u32 voiceID, u32 characterID, u32 characterID2)
{
	(void)voiceID;
	(void)characterID;
	(void)characterID2;
}

void GAMEPAD_ShockForce1(struct Driver *driver, int frame, int value)
{
	(void)driver;
	(void)frame;
	(void)value;
}

struct Instance *INSTANCE_BirthWithThread(s32 modelID, const char *name, s32 poolType,
                                        s32 bucket, void *tick, s32 objSize, struct Thread *parent)
{
	(void)modelID;
	(void)name;
	(void)poolType;
	(void)bucket;
	(void)tick;
	(void)objSize;
	(void)parent;
	return NULL;
}

struct Instance *INSTANCE_Birth3D(struct Model *model, const char *name, struct Thread *thread)
{
	(void)model;
	(void)name;
	(void)thread;
	return NULL;
}

void VehTurbo_ThTick(struct Thread *thread)
{
	(void)thread;
}

void VehTurbo_ThDestroy(struct Thread *thread)
{
	(void)thread;
}

// These full-physics paths are outside the asset-free scale/contact checks.
u32 VehCalc_FastSqrt(u32 value, u32 shift)
{
	(void)value; (void)shift;
	CtrTest_Bail("scale/contact checks must not calculate velocity");
	return 0;
}
s32 MATH_FastSqrt(u32 value, s32 shift)
{
	(void)value; (void)shift;
	CtrTest_Bail("scale/contact checks must not search collision buckets");
	return 0;
}
int ratan2(int y, int x)
{
	(void)y; (void)x;
	CtrTest_Bail("scale/contact checks must not calculate angles");
	return 0;
}
void ConvertRotToMatrix(MATRIX *matrix, const SVec3 *rotation)
{
	(void)matrix; (void)rotation;
	CtrTest_Bail("scale/contact checks must not animate matrices");
}
void MatrixRotate(void *dst, MATRIX *src, MATRIX *rotation)
{
	(void)dst; (void)src; (void)rotation;
	CtrTest_Bail("scale/contact checks must not animate matrices");
}
void OtherFX_DriverCrashing(u32 echo, u32 volume)
{
	(void)echo; (void)volume;
	CtrTest_Bail("squash checks must not use the ordinary crash sound");
}
void BOTS_CollideWithOtherAI(struct Driver *first, struct Driver *second)
{
	(void)first; (void)second;
	CtrTest_Bail("squash checks must not run bot navigation");
}
void GAMEPAD_JogCon1(struct Driver *driver, u8 value, u16 timeMS)
{
	(void)driver; (void)value; (void)timeMS;
	CtrTest_Bail("scale/contact checks must not run ordinary crash rumble");
}
int VehCalc_MapToRange(int value, int oldMin, int oldMax, int newMin, int newMax)
{
	(void)value; (void)oldMin; (void)oldMax; (void)newMin; (void)newMax;
	CtrTest_Bail("scale/contact checks must not run full vehicle physics");
	return 0;
}
int VehCalc_InterpBySpeed(int value, int speed, int desired)
{
	(void)value; (void)speed; (void)desired;
	CtrTest_Bail("scale/contact checks must not run full vehicle physics");
	return 0;
}
void PROC_CollidePointWithBucket(struct Thread *thread, struct BucketSearchParams *search)
{
	(void)thread; (void)search;
	CtrTest_Bail("scale/contact checks must not search collision buckets");
}
u32 MFC2(int reg)
{
	(void)reg;
	CtrTest_Bail("scale/contact checks must not project geometry");
	return 0;
}
struct Particle *Particle_Init(u32 flags, struct IconGroup *icons, struct ParticleEmitter *emitter)
{
	(void)flags; (void)icons; (void)emitter;
	CtrTest_Bail("scale/contact checks must not spawn particles");
	return NULL;
}

static void SetupRace(int trigger)
{
	// Stand-in retail storage: repeated entries must not weight uniform gifts.
	memset(data.RNG_itemSetRace2, HELD_ITEM_TURBO, sizeof(data.RNG_itemSetRace2));
	int poolIndex = 0;
	for (int item = HELD_ITEM_TURBO; item <= HELD_ITEM_MISSILE_3X; item++)
	{
		if (item != HELD_ITEM_SPRING)
		{
			data.RNG_itemSetRace2[poolIndex++] = (u8)item;
		}
	}
	s_allHumansFinished = false;
	s_activationSounds = 0;
	s_clockSounds = 0;
	s_squishSounds = 0;
	s_crateExplosions = 0;
	s_crateSounds = 0;
	s_lastExplodingCrate = NULL;
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(s_defaultDrivers, 0, sizeof(s_defaultDrivers));
	memset(&g_config, 0, sizeof(g_config));
	sdata->gGT = &s_tracker;
	sdata->Loading.stage = LOAD_IDLE;
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	s_tracker.numPlyrCurrGame = 2;
	for (int i = 0; i < s_tracker.numPlyrCurrGame; i++)
	{
		s_defaultDrivers[i].driverID = i;
		s_tracker.drivers[i] = &s_defaultDrivers[i];
	}
	sdata->chaosSettings = (struct ChaosSettings){
		.armed = true, .trigger = (u8)trigger, .durationSeconds = 30,
		.triggerIntervalSeconds = 5, .enabledCount = 1,
	};
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	Chaos_BeginRace(&s_tracker);
}

static void Tick(int elapsedMS)
{
	s_tracker.elapsedTimeMS = elapsedMS;
	Chaos_Update(&s_tracker);
}

static void TestIntervalCadence(void)
{
	CTR_TEST_SECTION("Interval starts after GO, carries frame overshoot and gives new entries full duration");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	s_teleported = false;
	Chaos_Reset();
	MainGameStart_Initialize(&s_tracker, true);
	CTR_TEST_CHECK(s_teleported && sdata->chaosRuntime.raceInitialized, "the actual race-start callback initializes Chaos after teleporting");
	CTR_TEST_CHECK(!g_config.chaosMode, "the creation prompt is disabled in this fixture");
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the countdown cannot activate an effect");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "the countdown does not consume trigger time");
	s_tracker.trafficLightsTimer = 0;
	Tick(4992);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "Interval waits for the whole first gap");
	Tick(32);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "an armed file activates despite the disabled creation prompt");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_LOW_GRAVITY, "Interval chooses from selected effects only");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "a new activation starts with its full duration");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 4976, "frame overshoot is retained rather than drifting the cadence");
	Tick(4976);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "Interval replaces an already active effect");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "repeated selection resets the duration");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].elapsedMS, 0, "repeated selection resets the elapsed time");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "the second boundary returns to the original cadence");
}

static void TestIndependentExpiryAndOrder(void)
{
	CTR_TEST_SECTION("reactivation replaces the old entry and preserves unrelated timers");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 200), "the first selected effect can activate");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 100), "an unrelated later effect can overlap");
	Tick(50);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 300), "a repeated activation replaces the old entry");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "reactivation does not add a duplicate slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_SUPER_SPEED, "unrelated effects keep their relative order");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 50, "reactivation leaves unrelated timers unchanged");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].effect, CHAOS_EFFECT_LOW_GRAVITY, "reactivation becomes the newest entry");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].elapsedMS, 0, "reactivation resets its phase");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].noticeRemainingMS, CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "reactivation restarts its notification");
	Tick(50);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "only the entry reaching its exact expiry is removed");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 250, "compaction preserves the replacement timer");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SUPER_SPEED), "an expired effect no longer reports active");
	Tick(100);
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "the superseded expiry cannot stop the replacement");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 150, "replacement continues from its new activation time");
	Tick(150);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the last timer expires exactly at zero");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "no active status remains after final expiry");

	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 100);
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 300);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 200);
	Tick(100);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_SUPER_SPEED, "removing the oldest entry preserves the next effect's order");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].effect, CHAOS_EFFECT_LOW_GRAVITY, "the newest effect remains last after compaction");
}

static void CheckSuspended(void)
{
	const struct ChaosRuntime before = sdata->chaosRuntime;
	const u32 rng = sdata->chaosRng;
	Tick(1000);
	CTR_TEST_CHECK(memcmp(&before, &sdata->chaosRuntime, sizeof(before)) == 0, "ineligible updates preserve timers and cadence");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "ineligible updates do not draw Chaos RNG");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "ineligible frames reject activation requests");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "effect hooks see no active modifier outside eligible gameplay");
}

static void TestGatesAndPause(void)
{
	CTR_TEST_SECTION("pause, menus, transitions and adjacent modes cannot advance or activate Chaos");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	const int blocked[] = {PAUSE_1, DEBUG_MENU, ADVENTURE_ARENA, MAIN_MENU, GAME_CUTSCENE, LOADING, END_OF_RACE};
	for (int i = 0; i < (int)len(blocked); i++)
	{
		s_tracker.gameMode1 = ADVENTURE_MODE | blocked[i];
		CheckSuspended();
	}
	s_tracker.gameMode1 = ARCADE_MODE;
	CheckSuspended();
	s_tracker.gameMode1 = BATTLE_MODE;
	CheckSuspended();
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_ADV_HUB;
	CheckSuspended();
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	sdata->ptrActiveMenu = &sdata->chaosMenu.menu;
	CheckSuspended();
	sdata->ptrActiveMenu = NULL;
	sdata->Loading.stage = LOAD_REQUESTED;
	CheckSuspended();
	sdata->Loading.stage = LOAD_IDLE;
	sdata->chaosSettings.armed = false;
	CheckSuspended();
	sdata->chaosSettings.armed = true;
	Tick(1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 9000, "resuming consumes only the next gameplay delta");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 4000, "the trigger resumes its remaining gap");
	const int variants[] = {RELIC_RACE, CRYSTAL_CHALLENGE, ADVENTURE_CUP, (int)ADVENTURE_BOSS};
	for (int i = 0; i < (int)len(variants); i++)
	{
		s_tracker.gameMode1 = ADVENTURE_MODE | variants[i];
		CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "every Adventure race type can consume active effects");
	}
}

static void TestCapacityAndValidation(void)
{
	CTR_TEST_SECTION("capacity rejects new effects but permits reactivation and scheduled refresh");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NONE, 100), "the none sentinel cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_COUNT, 100), "out-of-range effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 100), "disabled catalogue effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 0), "zero duration cannot create an entry");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, -1), "negative duration cannot create an entry");
	const ChaosEffectId effects[] = {CHAOS_EFFECT_LOW_GRAVITY, CHAOS_EFFECT_NO_HOPPING,
	    CHAOS_EFFECT_NO_ITEMS, CHAOS_EFFECT_WUMPA_DRAIN};
	for (int i = 0; i < CHAOS_MAX_ACTIVATIONS; i++)
	{
		sdata->chaosSettings.effects[effects[i]] = true;
		CTR_TEST_CHECK(Chaos_Activate(effects[i], 5000 + i), "each available slot accepts a distinct effect");
	}
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = true;
	const struct ChaosRuntime full = sdata->chaosRuntime;
	CTR_TEST_CHECK_EQ(full.numActive, 4, "four distinct effects fill the array");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 9999), "a full queue rejects a fifth distinct effect");
	CTR_TEST_CHECK(memcmp(&full, &sdata->chaosRuntime, sizeof(full)) == 0, "overflow preserves existing activations");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 0), "invalid reactivation cannot remove the running effect");
	CTR_TEST_CHECK(memcmp(&full, &sdata->chaosRuntime, sizeof(full)) == 0, "invalid reactivation preserves all timers");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 9999), "reactivation succeeds at full capacity");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "reactivation keeps the array at capacity without duplicates");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[3].remainingMS, 9999, "reactivation replaces the old duration");
	for (int i = 0; i < 3; i++)
	{
		CTR_TEST_CHECK(memcmp(&full.active[i + 1], &sdata->chaosRuntime.active[i], sizeof(full.active[0])) == 0,
		    "reactivation preserves every unrelated entry");
	}
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	sdata->chaosSettings.enabledCount = 1;
	sdata->chaosRuntime.intervalRemainingMS = 1;
	const u32 rng = sdata->chaosRng;
	Tick(1);
	CTR_TEST_CHECK(sdata->chaosRng != rng, "a scheduled event chooses an effect even at capacity");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[3].remainingMS, 30000, "Interval can refresh an effect at capacity");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "a scheduled refresh retains the configured gap");
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = false;
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = true;
	sdata->chaosRuntime.intervalRemainingMS = 1;
	const int sounds = s_activationSounds;
	const u32 beforeRejectedChoice = sdata->chaosRng;
	Tick(1);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "a scheduled fifth distinct effect cannot evict running effects");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_INVERTED_STEERING), "capacity also rejects a distinct scheduled effect");
	CTR_TEST_CHECK_EQ(s_activationSounds, sounds, "a rejected scheduled activation cannot announce");
	CTR_TEST_CHECK(sdata->chaosRng != beforeRejectedChoice, "a rejected scheduled event still consumes its effect choice");
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = false;
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	Tick(5003);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "expired unrelated effects leave only the refreshed effect");
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = true;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 1000), "a freed slot can accept a new effect");
}

static void TestResetAndInvalidSettings(void)
{
	CTR_TEST_SECTION("race reset preserves file settings and handles empty or invalid timed settings");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	const struct ChaosSettings settings = sdata->chaosSettings;
	const u32 rng = sdata->chaosRng;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "retry cannot inherit running activations");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "retry starts a fresh first interval");
	CTR_TEST_CHECK(memcmp(&settings, &sdata->chaosSettings, sizeof(settings)) == 0, "race setup does not modify creation settings");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "race reset does not reseed the dedicated generator");
	Chaos_Reset();
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "a race must initialize before accepting activations");
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "teardown cannot restart the engine through an update");
	sdata->chaosSettings.armed = false;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "a Regular file never initializes Chaos");
	sdata->chaosSettings.armed = true;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK(!sdata->chaosRuntime.raceInitialized, "entering a hub does not initialize a Chaos race");

	SetupRace(CHAOS_TRIGGER_INTERVAL);
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.enabledCount = 0;
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "an empty armed file produces no activation");
	CTR_TEST_CHECK(sdata->chaosSettings.armed, "an empty selection never reclassifies the file as Regular");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, 0, "empty selection consumes no random choice");
	sdata->chaosSettings.triggerIntervalSeconds = 0;
	Chaos_BeginRace(&s_tracker);
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "a zero saved interval cannot spin a scheduling loop");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	sdata->chaosSettings.durationSeconds = 0;
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "zero saved duration cannot create expired entries");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, 0, "invalid duration does not draw RNG");
	for (int trigger = 0; trigger < CHAOS_TRIGGER_COUNT; trigger++)
	{
		if (trigger == CHAOS_TRIGGER_INTERVAL || trigger == CHAOS_TRIGGER_RANDOM)
		{
			continue;
		}
		SetupRace(trigger);
		Tick(5000);
		CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "contact and external triggers do not fall back to timed scheduling");
	}
}

static void TestPickupTrigger(void)
{
	CTR_TEST_SECTION("Pickup activates selected effects for eligible human contacts only");
	SetupRace(CHAOS_TRIGGER_PICKUP);
	struct Driver human = {.driverID = 0};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	CTR_TEST_CHECK(Chaos_OnPickup(&human), "a human pickup activates a selected effect");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].effect, CHAOS_EFFECT_LOW_GRAVITY, "Pickup draws only from enabled effects");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "Pickup uses the configured duration");
	Tick(250);
	CTR_TEST_CHECK(Chaos_OnPickup(&partner), "the co-op partner can trigger Pickup");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "another pickup refreshes the same effect without duplicates");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].elapsedMS, 0, "a new pickup restarts the effect phase");
	const u32 rng = sdata->chaosRng;
	CTR_TEST_CHECK(!Chaos_OnPickup(&bot), "bot pickups cannot activate effects");
	CTR_TEST_CHECK(!Chaos_OnPickup(NULL), "missing drivers cannot activate effects");
	human.actionsFlagSet = ACTION_RACE_FINISHED;
	CTR_TEST_CHECK(!Chaos_OnPickup(&human), "finished humans cannot activate effects");
	human.actionsFlagSet = 0;
	const int blocked[] = {PAUSE_1, ADVENTURE_ARENA, LOADING, END_OF_RACE, MAIN_MENU};
	for (unsigned int i = 0; i < len(blocked); i++)
	{
		s_tracker.gameMode1 = ADVENTURE_MODE | blocked[i];
		CTR_TEST_CHECK(!Chaos_OnPickup(&human), "race gates block pickup activation");
	}
	s_tracker.gameMode1 = ARCADE_MODE;
	CTR_TEST_CHECK(!Chaos_OnPickup(&human), "Arcade retains ordinary pickup behavior");
	s_tracker.gameMode1 = ADVENTURE_MODE;
	for (int trigger = 0; trigger < CHAOS_TRIGGER_COUNT; trigger++)
	{
		if (trigger != CHAOS_TRIGGER_PICKUP)
		{
			sdata->chaosSettings.trigger = trigger;
			CTR_TEST_CHECK(!Chaos_OnPickup(&human), "other trigger modes ignore pickup events");
		}
	}
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_PICKUP;
	sdata->chaosSettings.durationSeconds = 0;
	CTR_TEST_CHECK(!Chaos_OnPickup(&human), "zero duration cannot consume a pickup activation");
	sdata->chaosSettings.durationSeconds = 30;
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = false;
	sdata->chaosSettings.enabledCount = 0;
	CTR_TEST_CHECK(!Chaos_OnPickup(&human), "empty selections cannot activate from pickups");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "ineligible contacts and invalid settings consume no RNG");
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	sdata->chaosSettings.enabledCount = 1;
	const ChaosEffectId fillers[] = {CHAOS_EFFECT_NO_HOPPING, CHAOS_EFFECT_NO_ITEMS, CHAOS_EFFECT_NO_RESERVES};
	for (unsigned int i = 0; i < len(fillers); i++)
	{
		sdata->chaosSettings.effects[fillers[i]] = true;
		Chaos_Activate(fillers[i], 10000);
		sdata->chaosSettings.effects[fillers[i]] = false;
	}
	CTR_TEST_CHECK(Chaos_OnPickup(&human), "Pickup can refresh a selected effect at capacity");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "Pickup replacement does not exceed capacity");
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = false;
	sdata->chaosSettings.effects[CHAOS_EFFECT_WUMPA_DRAIN] = true;
	const struct ChaosRuntime full = sdata->chaosRuntime;
	CTR_TEST_CHECK(!Chaos_OnPickup(&human), "Pickup rejects a fifth unrelated effect");
	CTR_TEST_CHECK(memcmp(&full, &sdata->chaosRuntime, sizeof(full)) == 0, "a rejected pickup preserves all running effects");

	CTR_TEST_SECTION("Turbo pads activate once per continuous traversal and preserve their latch on pause/restore");
	SetupRace(CHAOS_TRIGGER_PICKUP);
	struct Instance inst = {0};
	struct Thread thread = {.inst = &inst, .flags = THREAD_FLAG_DISABLE_COLLISION};
	human = (struct Driver){.driverID = 0, .instSelf = &inst};
	thread.object = &human;
	inst.thread = &thread;
	human.stepFlagSet = COLL_STEP_TRIGGER_TURBO_PAD;
	VehPhysForce_CollideDrivers(&thread, &human);
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "normal turbo pad physics dispatches a pickup on entry");
	Tick(100);
	VehPhysForce_CollideDrivers(&thread, &human);
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "remaining on a turbo pad cannot retrigger");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].elapsedMS, 100, "sustained pad contact cannot reset effect timers");
	human.stepFlagSet = COLL_STEP_TRIGGER_SUPER_TURBO_PAD;
	VehPhysForce_CollideDrivers(&thread, &human);
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "moving between connected normal and super pads is one traversal");
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Chaos_OnTurboPadContact(&human, false);
	s_tracker.gameMode1 &= ~PAUSE_1;
	Chaos_OnTurboPadContact(&human, true);
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "pause cannot clear the contact latch");
	human.stepFlagSet = 0;
	VehPhysForce_CollideDrivers(&thread, &human);
	human.stepFlagSet = COLL_STEP_TRIGGER_SUPER_TURBO_PAD;
	VehPhysForce_CollideDrivers(&thread, &human);
	CTR_TEST_CHECK_EQ(s_activationSounds, 2, "leaving then entering a super pad permits a fresh activation");
	sdata->chaosRuntime = saved;
	Chaos_OnTurboPadContact(&human, true);
	CTR_TEST_CHECK_EQ(s_activationSounds, 2, "resident restoration preserves continuous contact");
	Chaos_OnTurboPadContact(&partner, true);
	CTR_TEST_CHECK_EQ(s_activationSounds, 3, "the co-op partner has an independent pad latch");
	Chaos_OnTurboPadContact(&bot, true);
	CTR_TEST_CHECK_EQ(s_activationSounds, 3, "bot pad contact cannot activate effects");
	Chaos_Reset();
	Chaos_BeginRace(&s_tracker);
	Chaos_OnTurboPadContact(&human, true);
	CTR_TEST_CHECK_EQ(s_activationSounds, 4, "race reset clears pad contact latches");
}

static void TestResidentRestore(void)
{
	CTR_TEST_SECTION("restoring resident state resumes timers, activation order and RNG choices");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	Tick(5000);
	Tick(1234);
	static struct sData saved;
	saved = sdata_static;
	const int deltas[] = {3766, 5000, 5000, 5000, 5000, 5000};
	for (int i = 0; i < (int)len(deltas); i++)
	{
		Tick(deltas[i]);
	}
	const struct ChaosRuntime expected = sdata->chaosRuntime;
	const u32 expectedRng = sdata->chaosRng;
	Chaos_Reset();
	sdata_static = saved;
	for (int i = 0; i < (int)len(deltas); i++)
	{
		Tick(deltas[i]);
	}
	CTR_TEST_CHECK(memcmp(&expected, &sdata->chaosRuntime, sizeof(expected)) == 0, "resident restore reproduces timer expiry, queue order and trigger cadence");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, expectedRng, "resident restore reproduces subsequent effect choices");
	CTR_TEST_CHECK_EQ(sdata->randomNumber, saved.randomNumber, "Chaos scheduling leaves retail RNG untouched");
}

static void EnableInputEffects(void)
{
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.effects[CHAOS_EFFECT_INVERTED_STEERING] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_HOPPING] = true;
	sdata->chaosSettings.enabledCount = 2;
}

static void TestRandomTrigger(void)
{
	CTR_TEST_SECTION("Random schedules selected gameplay effects with bounded, reproducible gaps");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_RANDOM;
	Chaos_BeginRace(&s_tracker);
	const int firstGap = sdata->chaosRuntime.intervalRemainingMS;
	CTR_TEST_CHECK(firstGap >= 1500 && firstGap <= 5000, "the first Random gap is between 30 and 100 percent of the configured interval");
	Tick(firstGap - 1);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "Random waits for its initial gap instead of activating immediately");
	Tick(1);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "Random activates at its first deadline");
	const ChaosEffectId first = sdata->chaosRuntime.active[0].effect;
	struct Driver driver = {.driverID = 0};
	CTR_TEST_CHECK(Chaos_IsActiveForDriver(first, &driver), "a Random activation reaches the gameplay effect query");
	CTR_TEST_CHECK(first == CHAOS_EFFECT_INVERTED_STEERING || first == CHAOS_EFFECT_NO_HOPPING,
	               "Random chooses only the effects selected for this file");
	CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "Random activation also starts the visual announcement");
	const struct ChaosRuntime beforePause = sdata->chaosRuntime;
	const u32 pausedRng = sdata->chaosRng;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(5000);
	CTR_TEST_CHECK(memcmp(&beforePause, &sdata->chaosRuntime, sizeof(beforePause)) == 0, "pause preserves the Random deadline and active timers");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, pausedRng, "pause does not redraw the Random gap");
	s_tracker.gameMode1 &= ~PAUSE_1;
	bool varied = false;
	for (int i = 0; i < 32; i++)
	{
		const int gap = sdata->chaosRuntime.intervalRemainingMS;
		CTR_TEST_CHECK(gap >= 1500 && gap <= 5000, "every new Random gap stays within the configured bounds");
		varied |= gap != firstGap;
		int surviving = 0;
		for (int entry = 0; entry < sdata->chaosRuntime.numActive; entry++)
		{
			if (sdata->chaosRuntime.active[entry].remainingMS > gap)
			{
				surviving++;
			}
		}
		const int soundsBefore = s_activationSounds;
		const struct ChaosRuntime before = sdata->chaosRuntime;
		Tick(gap);
		const ChaosEffectId selected = sdata->chaosRuntime.active[sdata->chaosRuntime.numActive - 1].effect;
		bool refreshed = false;
		for (int entry = 0; entry < before.numActive; entry++)
		{
			refreshed |= before.active[entry].effect == selected && before.active[entry].remainingMS > gap;
		}
		CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, surviving + (refreshed ? 0 : 1), "Random replaces a matching survivor or adds a distinct effect");
		CTR_TEST_CHECK_EQ(s_activationSounds, soundsBefore + 1, "every accepted Random activation announces once");
		CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "a Random refresh starts a fresh announcement");
		for (int entry = 0; entry < sdata->chaosRuntime.numActive; entry++)
		{
			for (int other = entry + 1; other < sdata->chaosRuntime.numActive; other++)
			{
				CTR_TEST_CHECK(sdata->chaosRuntime.active[entry].effect != sdata->chaosRuntime.active[other].effect,
				    "Random scheduling never creates duplicate effect IDs");
			}
		}
	}
	CTR_TEST_CHECK(varied, "Random does not silently use a fixed Interval cadence");
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	const u32 savedRng = sdata->chaosRng;
	Tick(saved.intervalRemainingMS);
	const struct ChaosRuntime expected = sdata->chaosRuntime;
	const u32 expectedRng = sdata->chaosRng;
	sdata->chaosRuntime = saved;
	sdata->chaosRng = savedRng;
	Tick(saved.intervalRemainingMS);
	CTR_TEST_CHECK(memcmp(&expected, &sdata->chaosRuntime, sizeof(expected)) == 0, "restoring Random state reproduces the next effect and gap");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, expectedRng, "Random uses the captured dedicated RNG stream");
	CTR_TEST_CHECK_EQ(sdata->randomNumber, 0, "Random scheduling leaves retail RNG untouched");

	sdata->chaosSettings.triggerIntervalSeconds = 120;
	Chaos_BeginRace(&s_tracker);
	bool longGap = false;
	for (int i = 0; i < 32; i++)
	{
		const int gap = sdata->chaosRuntime.intervalRemainingMS;
		CTR_TEST_CHECK(gap >= 36000 && gap <= 120000, "long configured intervals retain the full Random range");
		longGap |= gap > 90000;
		Tick(gap);
	}
	CTR_TEST_CHECK(longGap, "15-bit RNG samples are scaled rather than truncating long gaps with a modulus");
}

static int PreparedSteering(struct Driver *driver, const struct GamepadBuffer *pad)
{
	int steering = pad->stickLX;
	Chaos_ApplyDriver(driver, pad, &steering, driver->actionsFlagSet);
	return steering;
}

static u32 PreparedJump(struct Driver *driver, u32 tapped, u32 held, int rainCloudEffect, u32 actions)
{
	const struct GamepadBuffer pad = {.buttonsTapped = tapped, .buttonsHeldCurrFrame = held};
	driver->actionsFlagSet = actions;
	actions = VehPhysProc_JumpInput(driver, tapped, held, rainCloudEffect, actions);
	int steering = pad.stickLX;
	return Chaos_ApplyDriver(driver, &pad, &steering, actions);
}

static void TestSteeringAndDriverScope(void)
{
	CTR_TEST_SECTION("steering is mirrored once around the active center for unfinished humans only");
	EnableInputEffects();
	struct Driver human = {.driverID = 0};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct GamepadBuffer pad = {.stickLX = 0};
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 0, "inactive steering preserves the axis");
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 1000);
	const int axes[] = {0, 1, 64, 127, 128, 129, 192, 255};
	for (int i = 0; i < (int)len(axes); i++)
	{
		pad.stickLX = (s16)axes[i];
		CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 256 - axes[i], "analog and resolved D-pad extremes mirror without byte wrapping");
		CTR_TEST_CHECK_EQ(pad.stickLX, axes[i], "steering never overwrites the shared gamepad axis");
	}
	pad.stickLX = 64;
	CTR_TEST_CHECK_EQ(PreparedSteering(&partner, &pad), 192, "the second human receives the same effect");
	CTR_TEST_CHECK_EQ(PreparedSteering(&bot, &pad), 64, "the first bot above the human slots is unaffected");
	human.actionsFlagSet = ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 64, "a finished human is unaffected while the partner continues");
	human.actionsFlagSet = 0;
	struct RacingWheelData wheel = {.gamepadCenter = 100};
	pad.rwd = &wheel;
	pad.stickLX = 60;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 140, "calibrated wheel steering mirrors around its own center");
	pad.stickLX = 100;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 100, "the wheel's neutral position stays neutral");
	pad.stickLX = 60;
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 140, "duplicate activations do not invert twice and cancel");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 140, "the later activation keeps steering inverted after the first expires");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 60, "steering returns to the live input after final expiry");
	CTR_TEST_CHECK(!Chaos_IsActiveForDriver(CHAOS_EFFECT_NONE, &human), "invalid effect IDs cannot apply to a driver");
	CTR_TEST_CHECK(!Chaos_IsActiveForDriver(CHAOS_EFFECT_INVERTED_STEERING, NULL), "a missing driver cannot receive an effect");
}

static void EnableWobble(void)
{
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	sdata->chaosSettings.effects[CHAOS_EFFECT_WOBBLE_STEERING] = true;
	sdata->chaosSettings.enabledCount++;
	// Cardinal samples exercise the game's real sine lookup and quadrant handling.
	data.trigApprox[0] = (struct TrigTable){ .sin = 0, .cos = FP(1.0) };
}

static void TestWobbleSteering(void)
{
	CTR_TEST_SECTION("Wobble oscillates in game time with calibrated limits and human-only scope");
	EnableWobble();
	struct Driver human = {.driverID = 0};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct GamepadBuffer pad = {.stickLX = 128};
	Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 6000);
	const u32 rng = sdata->chaosRng;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "Wobble begins at the live steering input");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 224, "a quarter period produces the positive steering peak beyond the retail dead zone");
	CTR_TEST_CHECK_EQ(PreparedSteering(&partner, &pad), 224, "the co-op partner receives the same phase");
	CTR_TEST_CHECK_EQ(PreparedSteering(&bot, &pad), 128, "Wobble does not change a bot's steering");
	human.actionsFlagSet = ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "Wobble leaves finished humans alone");
	human.actionsFlagSet = 0;
	CTR_TEST_CHECK_EQ(pad.stickLX, 128, "Wobble does not overwrite shared controller input");
	pad.stickLX = 255;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 256, "the positive peak clamps to the signed controller limit without wrapping");
	pad.stickLX = 128;
	struct RacingWheelData wheel = {.gamepadCenter = 100, .range = 64};
	pad.rwd = &wheel;
	pad.stickLX = 100;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 148, "wheel oscillation scales to the calibrated range");
	pad.stickLX = 160;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 164, "wheel steering clamps around its calibrated center");
	wheel.range = 0;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 160, "a wheel without a usable range preserves its live input");
	pad.rwd = NULL;
	pad.stickLX = 128;
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(1000);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes the oscillator together with effect timers");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Tick(1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 32, "three quarters of a period produces the opposite steering peak");
	pad.stickLX = 0;
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 0, "the negative peak clamps without reversing a full steering input");
	pad.stickLX = 128;
	sdata->chaosRuntime = saved;
	Tick(1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 32, "restoring resident runtime reproduces the oscillator phase");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "a full two-second period returns to the live input");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "Wobble phase consumes no random choices");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "reset leaves no synthetic steering behind");

	CTR_TEST_SECTION("steering reactivation resets phase and replaces the previous lifetime");
	EnableWobble();
	Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 6000);
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "a new Wobble activation uses its own phase instead of adding both waves");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 224, "the latest Wobble supplies one oscillation amplitude");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 128, "expiry of reactivated Wobble cannot reveal the replaced oscillator");
	pad.stickLX = 64;
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 192, "later Inverted Steering overrides the older Wobble");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_WOBBLE_STEERING), "Inverted Steering immediately deactivates conflicting Wobble entries");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 64, "expiry of Inverted Steering returns to live input without reviving Wobble");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "replaced effects do not remain dormant in the activation queue");
	EnableWobble();
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 4000);
	Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_INVERTED_STEERING), "Wobble immediately deactivates conflicting Inverted Steering");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 160, "later Wobble overrides Inverted Steering using live input");
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 64, "expiry of Wobble returns to live input without reviving Inverted Steering");
	Tick(3000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&human, &pad), 64, "final expiry restores ordinary input without a stored snapshot");

	CTR_TEST_SECTION("conflict replacement frees capacity and preserves unrelated effects");
	EnableWobble();
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 6000);
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 6000);
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_ITEMS] = true;
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 5000);
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 5000);
	Tick(500);
	const struct ChaosActivation firstHop = sdata->chaosRuntime.active[0];
	const struct ChaosActivation gravity = sdata->chaosRuntime.active[2];
	const struct ChaosActivation items = sdata->chaosRuntime.active[3];
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 0), "invalid activation cannot cancel conflicting running effects");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "a rejected conflict replacement preserves all running activations");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 1000), "a conflicting replacement can activate even when all four slots were occupied");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "replacement removes the conflicting effect before adding the new activation");
	CTR_TEST_CHECK(memcmp(&firstHop, &sdata->chaosRuntime.active[0], sizeof(firstHop)) == 0,
	    "replacement preserves the first unrelated activation and its timers");
	CTR_TEST_CHECK(memcmp(&gravity, &sdata->chaosRuntime.active[1], sizeof(gravity)) == 0,
	    "replacement preserves unrelated gravity and its ordering");
	CTR_TEST_CHECK(memcmp(&items, &sdata->chaosRuntime.active[2], sizeof(items)) == 0,
	    "replacement preserves unrelated item restrictions");
}

static void TestGravityEffects(void)
{
	CTR_TEST_SECTION("gravity effects derive from live driver constants without accumulating or overwriting them");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_HEAVY_KARTS] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_HOPPING] = true;
	sdata->chaosSettings.enabledCount += 2;
	struct Driver human = {.driverID = 0, .const_Gravity = 1000, .const_JumpForce = 800};
	struct Driver partner = {.driverID = 1, .const_Gravity = 1500};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .const_Gravity = 2000};
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1000, "inactive Chaos preserves normal gravity");
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 2000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 250, "Low Gravity applies one quarter of the human driver's gravity");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 250, "repeated physics reads do not repeatedly divide gravity");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&partner), 375, "the co-op partner uses their own gravity baseline");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&bot), 2000, "gravity effects leave bots unchanged");
	CTR_TEST_CHECK_EQ(human.const_Gravity, 1000, "Chaos does not overwrite the stored gravity constant");
	CTR_TEST_CHECK_EQ(human.const_JumpForce, 800, "Low Gravity preserves the jump impulse");
	human.const_Gravity = 1200;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 300, "legitimate constant changes immediately become the new baseline");
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1500);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 300, "reactivating Low Gravity does not compound");
	Tick(500);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 300, "reactivated gravity retains its replacement lifetime");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "final expiry immediately uses the current baseline rather than a stale snapshot");
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 3000);
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 3000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 3600, "reactivating Heavy Karts does not compound gravity");
	CTR_TEST_CHECK_EQ(human.const_JumpForce, 800, "Heavy Karts changes gravity without disabling the jump impulse");
	human.const_Gravity = 32767;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 98301, "effective gravity remains a full integer rather than overflowing the stored halfword");
	human.const_Gravity = 1200;
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(5000);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes gravity effect lifetimes");
	s_tracker.gameMode1 &= ~PAUSE_1;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 3600, "unpausing retains the gravity modifier");
	human.actionsFlagSet = ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "finished humans immediately use normal gravity");
	human.actionsFlagSet = 0;
	Tick(3000);
	sdata->chaosRuntime = saved;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 3600, "resident restoration needs no separate baseline repair");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "reset leaves no modified driver constants behind");

	CTR_TEST_SECTION("opposite gravity effects deactivate each other while preserving jumping and unrelated effects");
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 10000);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 3000);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 2000);
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "Heavy Karts replaces Low Gravity but preserves unrelated effects");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "replaced Low Gravity entries are permanently deactivated");
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_NO_HOPPING), "gravity replacement leaves No Hopping active");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 3600, "the latest gravity effect takes effect immediately");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "expiry of Heavy Karts cannot revive replaced Low Gravity");
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 3000);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_HEAVY_KARTS), "Low Gravity also permanently deactivates Heavy Karts");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 300, "Low Gravity can replace Heavy Karts without multiplying modified values");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "expiry of Low Gravity cannot revive replaced Heavy Karts");
	Chaos_Reset();
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 1000);
	CTR_TEST_CHECK((PreparedJump(&human, BTN_L1, BTN_L1, RAIN_CLOUD_EFFECT_NONE, 0) & ACTION_JUMP_BUTTON_HELD) != 0,
	    "Heavy Karts still allows manual hops");
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 10, "Heavy Karts retains the retail hop buffer");
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 1200, "Adventure hubs cannot receive gravity modifiers");
}

static void TestSpeedEffects(void)
{
	CTR_TEST_SECTION("speed effects derive targets and acceleration from live values without overwriting kart constants");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SPEED_CAPPED] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount += 2;
	struct Driver human = {
		.driverID = 0, .baseSpeed = 12000, .const_Speed_ClassStat = 12000,
		.const_Accel_ClassStat = 100, .const_Accel_Reserves = 200,
		.reserves = 3000, .turbo_outsideTimer = 1000, .fireSpeedCap = 5000,
	};
	struct Driver partner = {.driverID = 1, .baseSpeed = 14000, .const_Speed_ClassStat = 14000};
	struct Driver bot = {.driverID = 2, .baseSpeed = 12000, .const_Speed_ClassStat = 12000, .actionsFlagSet = ACTION_BOT};
	const struct Driver original = human;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 15000), 15000, "inactive Chaos preserves the resolved speed target");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "inactive Chaos preserves acceleration");
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 2000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 6000, "Speed Capped limits the target to half the unboosted class speed");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 20000), 6000, "boosted targets cannot bypass the class-based ceiling");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 4000), 4000, "a slower target from damage or partial speed remains below the ceiling");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 0), 0, "Speed Capped does not introduce motion at rest");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, -3000), -3000, "Speed Capped leaves reverse targets unchanged");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&partner, 18000), 7000, "the co-op partner retains their own class-based ceiling");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 18000), 18000, "Speed Capped does not affect bots");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "Speed Capped preserves normal acceleration");
	CTR_TEST_CHECK(memcmp(&human, &original, sizeof(human)) == 0, "speed queries do not overwrite constants, motion or boost fuel");
	human.const_Speed_ClassStat = 14000;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 20000), 7000, "legitimate class changes immediately update the ceiling");
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 1500);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 20000), 7000, "reactivating the cap does not halve the ceiling again");
	Tick(500);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 20000), 7000, "the replacement cap remains active until its own expiry");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 20000), 20000, "final cap expiry returns the current retail target");

	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 2000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 24000, "Super Speed doubles the resolved forward target before the retail hard limit");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 4000), 8000, "Super Speed scales the current damaged target rather than restoring an old one");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 0), 0, "Super Speed preserves a zero target");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, -3000), -3000, "Super Speed leaves reverse targets unchanged");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 200, "Super Speed doubles ordinary forward acceleration");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 200), 400, "Super Speed also doubles reserve acceleration");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 500), 1000, "Super Speed also scales turbo-pad acceleration");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 0), 0, "Super Speed does not create acceleration from zero");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, -100), -100, "Super Speed preserves negative acceleration");
	human.baseSpeed = -3000;
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "Super Speed preserves reverse acceleration");
	human.baseSpeed = 12000;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&partner, 14000), 28000, "Super Speed applies to the co-op partner before the retail clamp");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&partner, 100), 200, "the co-op partner also gets increased acceleration");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 12000), 12000, "Super Speed preserves bot targets");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&bot, 100), 100, "Super Speed preserves bot acceleration");
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 1500);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 24000, "reactivating Super Speed does not compound speed");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 200, "reactivating Super Speed does not compound acceleration");
	Tick(500);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 24000, "the refreshed speed effect uses its replacement lifetime");
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(5000);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes speed effect lifetimes");
	s_tracker.gameMode1 &= ~PAUSE_1;
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "finished humans immediately use normal targets");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "finished humans immediately use normal acceleration");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "final Super Speed expiry restores the current retail target");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "final Super Speed expiry restores retail acceleration");
	sdata->chaosRuntime = saved;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 24000, "resident restoration restores speed rules without baseline repair");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "reset restores normal speed rules");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "reset restores normal acceleration rules");

	CTR_TEST_SECTION("opposite speed effects permanently replace each other and preserve unrelated activations");
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 3000);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 3000);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 3000);
	const struct ChaosActivation gravity = sdata->chaosRuntime.active[0];
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 1000), "Super Speed can replace a refreshed cap");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "replacement removes the cap but keeps the unrelated gravity effect");
	CTR_TEST_CHECK(memcmp(&gravity, &sdata->chaosRuntime.active[0], sizeof(gravity)) == 0, "replacement preserves unrelated timers and order");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SPEED_CAPPED), "replaced caps are deactivated immediately");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "Super Speed expiry cannot revive replaced caps");
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 3000);
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 3000);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SUPER_SPEED), "Speed Capped removes all Super Speed copies");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 7000, "the replacement cap uses the live unmodified class speed");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "the replacement cap removes increased acceleration immediately");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "cap expiry cannot revive replaced Super Speed");
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 1000);
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "Adventure hubs preserve ordinary speed targets");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 100), 100, "Adventure hubs preserve ordinary acceleration");
}

static void TestDefectiveBoosts(void)
{
	CTR_TEST_SECTION("Defective Boosts inverts fire speed while retaining ordinary reserve grants");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_DEFECTIVE_BOOSTS] = true;
	sdata->chaosSettings.enabledCount++;
	struct Thread thread = {.modelIndex = DYNAMIC_PLAYER};
	struct Instance inst = {.thread = &thread};
	const struct Driver initial = {
		.driverID = 0, .instSelf = &inst, .kartState = KS_NORMAL,
		.const_SingleTurboSpeed = 1000, .const_SacredFireSpeed = 2000,
	};
	const u32 types[] = {
		0, POWER_SLIDE_HANG_TIME,
		TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD,
		TURBO_ITEM | FREEZE_RESERVES_ON_TURBO_PAD,
		TURBO_PAD | SUPER_ENGINE,
	};
	struct Driver retail[len(types)];
	for (size_t i = 0; i < len(types); i++)
	{
		retail[i] = initial;
		VehFire_Increment(&retail[i], 1000, types[i], 128);
	}
	struct Driver human = initial;
	human.reserves = 1000;
	human.fireSpeedCap = 3000;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 15000), 15000, "inactive Defective Boosts retains the ordinary fire bonus");
	Chaos_Activate(CHAOS_EFFECT_DEFECTIVE_BOOSTS, 5000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 15000), 9000, "the existing three-thousand fire bonus becomes a three-thousand penalty");
	for (size_t i = 0; i < len(types); i++)
	{
		human = initial;
		VehFire_Increment(&human, 1000, types[i], 128);
		CTR_TEST_CHECK_EQ(human.reserves, retail[i].reserves, "start, drift, hang-time, pad, item and Super Engine grants retain retail reserves");
		CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, retail[i].turbo_outsideTimer, "pad and item outside-reserve bookkeeping remains retail");
		CTR_TEST_CHECK_EQ(human.fireSpeedCap, retail[i].fireSpeedCap, "the stored fire level stays positive and unchanged");
		CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000 + human.fireSpeedCap), 12000 - human.fireSpeedCap,
		                  "each boost source subtracts its live bonus rather than adding it");
	}
	human = initial;
	VehFire_Increment(&human, 1000, POWER_SLIDE_HANG_TIME, 128);
	VehFire_Increment(&human, 1000, POWER_SLIDE_HANG_TIME, 256);
	CTR_TEST_CHECK_EQ(human.reserves, 2000, "repeated boosts accumulate reserves instead of clearing fuel");
	CTR_TEST_CHECK_EQ(human.fireSpeedCap, 2000, "a stronger boost raises the stored fire level normally");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 10000, "stronger fire produces a larger speed penalty");
	human.reserves = 1;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 10000, "the last remaining reserve keeps the penalty active");
	human.reserves = 0;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 12000), 12000, "empty reserves remove the penalty even if the stored fire level remains");
	human.reserves = 1000;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 3000), 0, "an oversized penalty stops at zero without creating reverse acceleration");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, -3000), -3000, "reverse targets remain unchanged");
	CTR_TEST_CHECK_EQ(Chaos_AccelerationForDriver(&human, 200), 200, "reserve acceleration stays retail and approaches the reduced speed target");
	struct Driver partner = human;
	partner.driverID = 1;
	struct Driver bot = human;
	bot.driverID = 2;
	bot.actionsFlagSet = ACTION_BOT;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&partner, 14000), 10000, "the co-op partner also receives defective boosts");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 14000), 14000, "bot boost bonuses remain ordinary");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 14000, "finished humans retain ordinary boost bonuses");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 14000, "Adventure hubs retain ordinary boost bonuses");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	Chaos_Activate(CHAOS_EFFECT_DEFECTIVE_BOOSTS, 5000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 10000, "duplicate activations invert the bonus only once");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 10000, "the replacement activation retains the penalty");
	Tick(4000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 14000), 14000, "final expiry restores the bonus from current reserves");
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "effect expiry never restores or discards fuel");
}

static void TestNoReserves(void)
{
	CTR_TEST_SECTION("No Reserves clears fuel at driver application while boost grants remain retail");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_RESERVES] = true;
	sdata->chaosSettings.enabledCount++;
	struct Thread thread = {.modelIndex = DYNAMIC_PLAYER};
	struct Instance inst = {.thread = &thread};
	struct Driver human = {
		.driverID = 0, .instSelf = &inst, .reserves = 2000, .turbo_outsideTimer = 1000,
		.kartState = KS_NORMAL, .baseSpeed = 1000, .const_SingleTurboSpeed = 100,
		.const_SacredFireSpeed = 200, .rainCloudEffect = RAIN_CLOUD_EFFECT_NONE,
	};
	struct GamepadBuffer pad = {.stickLX = 128};
	int steering = pad.stickLX;
	Chaos_ApplyReserveRules(&human);
	CTR_TEST_CHECK_EQ(human.reserves, 2000, "inactive Chaos leaves accumulated reserves alone");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 1000, "inactive Chaos leaves the outside turbo timer alone");
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 2000);
	const u32 actions = ACTION_JUMP_BUTTON_HELD | ACTION_MASK_WEAPON;
	CTR_TEST_CHECK_EQ(Chaos_ApplyDriver(&human, &pad, &steering, actions), actions,
	                  "No Reserves preserves manual hops and mask input flags");
	CTR_TEST_CHECK_EQ(human.reserves, 0, "the existing driver hook clears accumulated reserves before physics consumers");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 0, "the existing driver hook also clears outside turbo bookkeeping");
	CTR_TEST_CHECK_EQ(human.baseSpeed, 1000, "No Reserves does not overwrite base speed");
	CTR_TEST_CHECK_EQ(steering, 128, "No Reserves does not change steering");
	const u32 types[] = {
		POWER_SLIDE_HANG_TIME,
		TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD,
		TURBO_ITEM | FREEZE_RESERVES_ON_TURBO_PAD,
		TURBO_PAD | SUPER_ENGINE,
	};
	for (int i = 0; i < (int)(sizeof(types) / sizeof(types[0])); i++)
	{
		human.reserves = 500;
		human.turbo_outsideTimer = 100;
		VehFire_Increment(&human, 1000, types[i], 100);
		CTR_TEST_CHECK(human.reserves > 0, "retail boost grants can supply fuel until the next driver application");
		Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
		CTR_TEST_CHECK_EQ(human.reserves, 0, "the next driver application clears newly granted reserves");
		CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 0, "the next driver application clears outside turbo time too");
		CTR_TEST_CHECK((human.actionsFlagSet & ACTION_NEW_BOOST) != 0, "retail boost event flags remain available for presentation");
	}
	struct Driver partner = {.driverID = 1, .reserves = 500, .turbo_outsideTimer = 200};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .reserves = 700, .turbo_outsideTimer = 300};
	Chaos_ApplyReserveRules(&partner);
	Chaos_ApplyReserveRules(&bot);
	Chaos_ApplyReserveRules(NULL);
	CTR_TEST_CHECK_EQ(partner.reserves, 0, "No Reserves also applies to the co-op partner");
	CTR_TEST_CHECK_EQ(partner.turbo_outsideTimer, 0, "the partner's two fuel fields are cleared together");
	CTR_TEST_CHECK_EQ(bot.reserves, 700, "No Reserves preserves bot reserves");
	CTR_TEST_CHECK_EQ(bot.turbo_outsideTimer, 300, "No Reserves preserves bot outside turbo time");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	VehFire_Increment(&human, 1000, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 100);
	Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "finished humans can receive ordinary retail boosts");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 1000, "finished humans retain outside turbo time");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	human.reserves = 0;
	human.turbo_outsideTimer = 0;
	s_tracker.gameMode1 |= PAUSE_1;
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	Tick(5000);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes No Reserves lifetimes");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Tick(1000);
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 1000);
	Tick(500);
	VehFire_Increment(&human, 1000, POWER_SLIDE_HANG_TIME, 100);
	Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "the refreshed restriction still clears reserves before its own expiry");
	Tick(500);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "final expiry does not restore reserves erased earlier");
	VehFire_Increment(&human, 1000, POWER_SLIDE_HANG_TIME, 100);
	Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "new reserves can accumulate normally after final expiry");
	sdata->chaosRuntime = saved;
	Chaos_ApplyReserveRules(&human);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "resident restoration restores the restriction without a separate driver snapshot");
	Chaos_Reset();
	VehFire_Increment(&human, 1000, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 100);
	Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "reset immediately restores normal boost grants");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 1000, "reset also restores ordinary outside turbo bookkeeping");
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 1000);
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_ApplyReserveRules(&human);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "Adventure hubs do not receive reserve restrictions");
}

static void TestNoItems(void)
{
	CTR_TEST_SECTION("No Items discards inventory and pending requests before roulette and weapon processing");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_NO_ITEMS] = true;
	sdata->chaosSettings.enabledCount++;
	struct Instance shield = {0};
	struct Instance bomb = {0};
	struct Thread cloud = {0};
	struct Driver human = {
		.driverID = 0, .heldItemID = HELD_ITEM_BOMB_3X, .numHeldItems = 3,
		.noItemTimer = 20, .itemRollTimer = 10,
		.actionsFlagSet = ACTION_WEAPON_FIRE_REQUEST | ACTION_MASK_WEAPON | ACTION_JUMP_BUTTON_HELD,
		.instBubbleHold = &shield, .instBombThrow = &bomb, .thCloud = &cloud,
		.superEngineTimer = 1000, .invisibleTimer = 2000, .invincibleTimer = 3000,
		.numWumpas = 10, .reserves = 1000, .turbo_outsideTimer = 500,
	};
	struct Driver partner = {.driverID = 1, .heldItemID = HELD_ITEM_NONE};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	const struct Driver original = human;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK(memcmp(&human, &original, sizeof(human)) == 0, "inactive No Items leaves inventory and pending requests intact");
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 2000);
	const u8 items[] = {
		HELD_ITEM_TURBO, HELD_ITEM_BOMB_1X, HELD_ITEM_MISSILE_1X, HELD_ITEM_TNT,
		HELD_ITEM_POTION, HELD_ITEM_SPRING, HELD_ITEM_SHIELD, HELD_ITEM_MASK,
		HELD_ITEM_CLOCK, HELD_ITEM_BOMB_3X, HELD_ITEM_MISSILE_3X,
		HELD_ITEM_INVISIBILITY, HELD_ITEM_SUPER_ENGINE, HELD_ITEM_NONE, HELD_ITEM_ROULETTE,
	};
	for (size_t i = 0; i < len(items); i++)
	{
		human.heldItemID = items[i];
		human.numHeldItems = 3;
		human.itemRollTimer = 10;
		human.noItemTimer = 20;
		human.actionsFlagSet |= ACTION_WEAPON_FIRE_REQUEST;
		Chaos_ApplyItemRules(&human);
		CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_NONE, "every held item or roulette sentinel is discarded");
		CTR_TEST_CHECK_EQ(human.numHeldItems, 0, "discarding inventory clears the stack count");
		CTR_TEST_CHECK_EQ(human.itemRollTimer, 0, "discarding inventory cancels pending roulette time");
		CTR_TEST_CHECK_EQ(human.noItemTimer, 0, "discarding inventory cancels the old weapon cooldown");
		CTR_TEST_CHECK_EQ(human.actionsFlagSet, ACTION_MASK_WEAPON | ACTION_JUMP_BUTTON_HELD,
		                  "pending weapon requests are removed without changing mask or jump flags");
	}
	CTR_TEST_CHECK(human.instBubbleHold == &shield && human.instBombThrow == &bomb,
	               "already deployed shields and remote bombs remain owned by the driver");
	CTR_TEST_CHECK(human.thCloud == &cloud, "No Items leaves retail rain-cloud ownership intact");
	CTR_TEST_CHECK_EQ(human.superEngineTimer, 1000, "already activated Super Engine remains active");
	CTR_TEST_CHECK_EQ(human.invisibleTimer, 2000, "already activated invisibility remains active");
	CTR_TEST_CHECK_EQ(human.invincibleTimer, 3000, "No Items preserves existing invincibility");
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "No Items does not remove collected wumpa");
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "No Items does not remove existing boost reserves");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 500, "No Items does not alter outside turbo time");
	human.heldItemID = HELD_ITEM_ROULETTE;
	human.itemRollTimer = 0;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_NONE, "a roulette already at its final frame is cancelled before award selection");

	CTR_TEST_SECTION("No Items clears the warpball flag and also empties the co-op partner's inventory");
	human.heldItemID = HELD_ITEM_WARPBALL;
	human.actionsFlagSet |= ACTION_WEAPON_FIRE_REQUEST;
	s_tracker.gameMode1 |= WARPBALL_HELD;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) == 0, "discarding an unlaunched warpball releases its reservation");
	CTR_TEST_CHECK((human.actionsFlagSet & ACTION_WEAPON_FIRE_REQUEST) == 0, "discarding a requested warpball prevents its pending launch");
	s_tracker.gameMode1 |= WARPBALL_HELD;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "empty inventory preserves an already launched warpball's reservation");
	human.heldItemID = HELD_ITEM_TURBO;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "discarding a different item preserves another driver's warpball reservation");
	partner.heldItemID = HELD_ITEM_ROULETTE;
	partner.itemRollTimer = 30;
	Chaos_ApplyItemRules(&partner);
	CTR_TEST_CHECK_EQ(partner.heldItemID, HELD_ITEM_NONE, "No Items also empties the co-op partner's slot");
	CTR_TEST_CHECK_EQ(partner.itemRollTimer, 0, "No Items also cancels the co-op partner's roulette");

	CTR_TEST_SECTION("No Items respects scope, gates, independent lifetimes and reset without restoring discarded inventory");
	struct Driver bot = {.driverID = 2, .heldItemID = HELD_ITEM_MASK, .actionsFlagSet = ACTION_BOT | ACTION_WEAPON_FIRE_REQUEST};
	const struct Driver originalBot = bot;
	Chaos_ApplyItemRules(&bot);
	Chaos_ApplyItemRules(NULL);
	CTR_TEST_CHECK(memcmp(&bot, &originalBot, sizeof(bot)) == 0, "No Items leaves bot inventory and requests unchanged");
	human.heldItemID = HELD_ITEM_TURBO;
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_TURBO, "finished humans keep their inventory");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_TURBO, "Adventure hubs keep ordinary item behavior");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	s_tracker.gameMode1 |= PAUSE_1;
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	Tick(5000);
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_TURBO, "paused item processing cannot discard inventory");
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes No Items lifetimes");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Tick(1000);
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 1000);
	Tick(500);
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_NONE, "the refreshed restriction discards items until its own expiry");
	Tick(500);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_NONE, "final expiry does not restore discarded items");
	human.heldItemID = HELD_ITEM_ROULETTE;
	human.itemRollTimer = 90;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_ROULETTE, "new rewards survive after final expiry");
	CTR_TEST_CHECK_EQ(human.itemRollTimer, 90, "roulette can run normally after final expiry");
	sdata->chaosRuntime = saved;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_NONE, "resident restoration restores the restriction without an inventory snapshot");
	Chaos_Reset();
	human.heldItemID = HELD_ITEM_POTION;
	Chaos_ApplyItemRules(&human);
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_POTION, "race reset immediately restores ordinary inventory behavior");
}

static void TickAndApplyDrivers(int elapsedMS)
{
	Tick(elapsedMS);
	const struct GamepadBuffer pad = {.stickLX = 128};
	for (size_t i = 0; i < len(s_tracker.drivers); i++)
	{
		struct Driver *driver = s_tracker.drivers[i];
		if (driver != NULL)
		{
			// Retail driving physics consumes received-clock time before the Chaos hook.
			if ((s_tracker.gameMode1 & PAUSE_ALL) == 0 && driver->clockReceive > 0)
			{
				driver->clockReceive = driver->clockReceive > elapsedMS ? driver->clockReceive - elapsedMS : 0;
			}
			int steering = pad.stickLX;
			driver->actionsFlagSet = Chaos_ApplyDriver(driver, &pad, &steering, driver->actionsFlagSet);
		}
	}
}

static void TestWumpaDrain(void)
{
	CTR_TEST_SECTION("Wumpa Drain loses one credited fruit per game second for unfinished humans only");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_WUMPA_DRAIN] = true;
	sdata->chaosSettings.enabledCount++;
	struct Driver human = {.driverID = 0, .numWumpas = 10, .numTimesWumpa = 20, .reserves = 1000};
	struct Driver partner = {.driverID = 1, .numWumpas = 7};
	struct Driver bot = {.driverID = 2, .numWumpas = 10, .actionsFlagSet = ACTION_BOT};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	s_tracker.drivers[2] = &bot;
	human.PickupWumpaHUD.numCollected = 3;
	const u32 rng = sdata->chaosRng;
	TickAndApplyDrivers(2000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "inactive Wumpa Drain leaves fruit intact");
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 5000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "activation waits for the first full second");
	TickAndApplyDrivers(999);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "partial seconds do not drain fruit early");
	TickAndApplyDrivers(1);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "the first complete second drains one fruit");
	CTR_TEST_CHECK_EQ(partner.numWumpas, 6, "the co-op partner uses the same pulse cadence");
	CTR_TEST_CHECK_EQ(bot.numWumpas, 10, "Wumpa Drain preserves bot fruit");
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "frames after a pulse do not repeat already counted seconds");
	TickAndApplyDrivers(2468);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "a longer frame drains only one fruit even when it crosses several seconds");
	CTR_TEST_CHECK_EQ(partner.numWumpas, 5, "the partner also loses at most one fruit per activation per frame");
	CTR_TEST_CHECK_EQ(human.PickupWumpaHUD.numCollected, 3, "fruit still in the pickup animation remains pending");
	CTR_TEST_CHECK_EQ(human.numTimesWumpa, 20, "draining fruit does not rewrite collection statistics");
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "draining fruit leaves boost reserves alone");
	human.numWumpas++;
	TickAndApplyDrivers(500);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "the next pulse uses the current count including newly credited fruit");
	TickAndApplyDrivers(1000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "the expiry frame applies no additional drain");
	CTR_TEST_CHECK_EQ(partner.numWumpas, 4, "an activation removed at exact expiry cannot apply another pulse");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the drain activation expires normally");
	TickAndApplyDrivers(2000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "expired Wumpa Drain cannot remove more fruit");
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 500);
	TickAndApplyDrivers(2000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "a sub-second activation cannot drain a whole second from a longer frame");
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 1500);
	TickAndApplyDrivers(3000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "a frame extending past expiry cannot apply a removed activation");
	human.numWumpas = 1;
	partner.numWumpas = 0;
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 4000);
	TickAndApplyDrivers(3000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 0, "draining the last fruit stops at zero");
	CTR_TEST_CHECK_EQ(partner.numWumpas, 0, "an empty inventory cannot underflow");
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "drain timing consumes no Chaos RNG samples");

	CTR_TEST_SECTION("reactivating drain resets its pulse clock and replaces its expiry");
	Chaos_Reset();
	Chaos_BeginRace(&s_tracker);
	human.numWumpas = 10;
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 2500);
	TickAndApplyDrivers(750);
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 2500);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "reactivated drain occupies a single slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].elapsedMS, 0, "reactivated drain resets its pulse clock");
	TickAndApplyDrivers(250);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "reactivation cancels the old pending pulse");
	TickAndApplyDrivers(500);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "the replacement waits a full second before draining");
	TickAndApplyDrivers(250);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "the replacement produces its first pulse at one second");
	TickAndApplyDrivers(250);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "the superseded clock cannot produce a second pulse");
	TickAndApplyDrivers(500);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "the old deadline cannot expire the replacement");
	TickAndApplyDrivers(250);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "the replacement produces one second pulse");
	TickAndApplyDrivers(500);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "replacement expiry applies no extra pulse");
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 2000);
	TickAndApplyDrivers(750);
	CTR_TEST_CHECK_EQ(human.numWumpas, 8, "a new activation after expiry starts with a fresh second");
	TickAndApplyDrivers(250);
	CTR_TEST_CHECK_EQ(human.numWumpas, 7, "the fresh activation drains only after its own full second");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "reset removes the activation-owned drain clock");
	CTR_TEST_CHECK_EQ(human.numWumpas, 7, "reset does not restore already drained fruit");

	CTR_TEST_SECTION("drain cadence freezes with race gates and survives resident restoration");
	Chaos_BeginRace(&s_tracker);
	human.numWumpas = 10;
	partner.numWumpas = 10;
	partner.actionsFlagSet = ACTION_RACE_FINISHED;
	Chaos_Activate(CHAOS_EFFECT_WUMPA_DRAIN, 5000);
	TickAndApplyDrivers(500);
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	TickAndApplyDrivers(5000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "pause prevents drain pulses");
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes the partial cadence and independent timers");
	s_tracker.gameMode1 &= ~PAUSE_1;
	sdata->ptrActiveMenu = &sdata->chaosMenu.menu;
	TickAndApplyDrivers(5000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "menus prevent drain pulses");
	sdata->ptrActiveMenu = NULL;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	TickAndApplyDrivers(5000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "Adventure hubs prevent drain pulses");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	TickAndApplyDrivers(500);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "unpausing resumes the remaining half-second");
	CTR_TEST_CHECK_EQ(partner.numWumpas, 10, "finished humans are excluded from drain pulses");
	sdata->chaosRuntime = saved;
	human.numWumpas = 10;
	TickAndApplyDrivers(499);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "resident restoration resumes the saved partial cadence");
	TickAndApplyDrivers(1);
	CTR_TEST_CHECK_EQ(human.numWumpas, 9, "the restored cadence fires at the original boundary");

	CTR_TEST_SECTION("timed scheduling gives a new drain its full first second");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = false;
	sdata->chaosSettings.effects[CHAOS_EFFECT_WUMPA_DRAIN] = true;
	sdata->chaosSettings.triggerIntervalSeconds = 1;
	sdata->chaosSettings.durationSeconds = 2;
	Chaos_BeginRace(&s_tracker);
	human.numWumpas = 10;
	s_tracker.drivers[0] = &human;
	TickAndApplyDrivers(1000);
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_WUMPA_DRAIN), "Interval can schedule Wumpa Drain");
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "the scheduling frame does not retroactively consume the new drain's first second");
	TickAndApplyDrivers(1000);
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "scheduled reactivation resets the drain clock before driver application");
}

static void TestAnxiousMask(void)
{
	CTR_TEST_SECTION("Anxious Mask requests rescue on grounded off-road contact before checkpoint progress");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_ANXIOUS_MASK] = true;
	sdata->chaosSettings.enabledCount++;
	struct CheckpointNode restartPoints[2] = {0};
	struct Level level = {.ptr_restart_points = restartPoints, .cnt_restart_points = len(restartPoints)};
	s_tracker.level1 = &level;
	data.MetaDataTerrain[TERRAIN_ASPHALT].speedMultiplier = 256;
	data.MetaDataTerrain[TERRAIN_ICE].speedMultiplier = 256;
	data.MetaDataTerrain[TERRAIN_GRASS].speedMultiplier = 140;
	struct QuadBlock road = {.terrain_type = TERRAIN_ASPHALT, .checkpointIndex = 0};
	struct QuadBlock offRoad = {.terrain_type = TERRAIN_GRASS, .checkpointIndex = 1};
	struct Driver human = {
		.driverID = 0, .kartState = KS_NORMAL, .currBlockTouching = &offRoad, .lastValid = &road,
		.collisionFlags = DRIVER_COLL_FLAG_GROUNDED, .currentTerrain = TERRAIN_ICE,
		.reserves = 1000, .numWumpas = 10, .actionsFlagSet = ACTION_MASK_WEAPON,
	};
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "inactive Anxious Mask leaves off-road contact alone");
	Chaos_Activate(CHAOS_EFFECT_ANXIOUS_MASK, 10000);
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK((human.collisionFlags & DRIVER_COLL_FLAG_MASK_GRAB_REQUEST) != 0, "grounded grass contact requests rescue even when a cloud overrides current terrain");
	CTR_TEST_CHECK(human.lastValid == &road, "the request preserves the last valid road checkpoint");
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "the request leaves reserve loss to retail rescue initialization");
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "the request leaves fruit loss to retail rescue initialization");
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED | DRIVER_COLL_FLAG_MASK_GRAB_REQUEST, "an existing rescue request remains unchanged");
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK((human.collisionFlags & DRIVER_COLL_FLAG_MASK_GRAB_REQUEST) != 0, "a new eligible off-road contact can request rescue immediately");
	struct Driver partner = human;
	partner.driverID = 1;
	partner.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	Chaos_ApplyRescueRules(&partner);
	CTR_TEST_CHECK((partner.collisionFlags & DRIVER_COLL_FLAG_MASK_GRAB_REQUEST) != 0, "the co-op partner can independently request rescue");
	Chaos_Activate(CHAOS_EFFECT_ANXIOUS_MASK, 1000);
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED | DRIVER_COLL_FLAG_MASK_GRAB_REQUEST, "duplicate activations leave an existing rescue request unchanged");
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(5000);
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes Anxious Mask activation timers");
	s_tracker.gameMode1 &= ~PAUSE_1;
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;

	CTR_TEST_SECTION("Anxious Mask uses contact terrain and leaves rescue eligibility to retail");
	const u8 roadTypes[] = {TERRAIN_ASPHALT, TERRAIN_ICE};
	for (size_t i = 0; i < len(roadTypes); i++)
	{
		road.terrain_type = roadTypes[i];
		human.currBlockTouching = &road;
		Chaos_ApplyRescueRules(&human);
		CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "ordinary road and ice do not trigger rescue");
	}
	road.terrain_type = TERRAIN_ASPHALT;
	human.currBlockTouching = &offRoad;
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	const u8 states[] = {KS_MASK_GRABBED, KS_BLASTED, KS_SPINNING, KS_ENGINE_REVVING, KS_WARP_PAD};
	for (size_t i = 0; i < len(states); i++)
	{
		human.kartState = states[i];
		human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
		Chaos_ApplyRescueRules(&human);
		CTR_TEST_CHECK((human.collisionFlags & DRIVER_COLL_FLAG_MASK_GRAB_REQUEST) != 0, "kart state does not prevent an off-road rescue request");
	}
	human.kartState = KS_DRIFTING;
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "finished humans do not receive anxious rescues");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	human.driverID = 2;
	human.actionsFlagSet |= ACTION_BOT;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "bots do not receive anxious rescues");
	human.driverID = 0;
	human.actionsFlagSet &= ~ACTION_BOT;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "Adventure hubs retain ordinary off-road driving");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	sdata->chaosRuntime = saved;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK((human.collisionFlags & DRIVER_COLL_FLAG_MASK_GRAB_REQUEST) != 0, "restored Anxious Mask immediately permits drifting off-road rescue");
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	Tick(10000);
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "final activation expiry restores ordinary off-road driving");
	Chaos_Reset();
	human.collisionFlags = DRIVER_COLL_FLAG_GROUNDED;
	Chaos_ApplyRescueRules(&human);
	CTR_TEST_CHECK_EQ(human.collisionFlags, DRIVER_COLL_FLAG_GROUNDED, "reset disables anxious rescue requests");
}

static void TestExplosiveCrates(void)
{
	CTR_TEST_SECTION("Explosive Crates applies one blast per contact and bypasses protections");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_EXPLOSIVE_CRATES] = true;
	sdata->chaosSettings.enabledCount++;
	struct Model model = {.id = PU_RANDOM_CRATE};
	struct Instance crate = {.model = &model};
	struct Thread thread = {0};
	struct Instance inst = {.thread = &thread};
	struct Driver human = {.driverID = 0, .instSelf = &inst, .reserves = 1000, .heldItemID = HELD_ITEM_POTION};
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, 0, "inactive explosive crates do not create explosions");
	Chaos_Activate(CHAOS_EFFECT_EXPLOSIVE_CRATES, 5000);
	Chaos_Activate(CHAOS_EFFECT_EXPLOSIVE_CRATES, 1000);
	CTR_TEST_CHECK_EQ(s_crateExplosions, 0, "activation waits for a crate contact");
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, 1, "overlapping activations cause only one explosion per contact");
	CTR_TEST_CHECK_EQ(s_crateSounds, 1, "a contact plays one explosion sound");
	CTR_TEST_CHECK(s_lastExplodingCrate == &crate, "the visual explosion uses the contacted crate");
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehStuckProc_Tumble_Init, "crate contact selects retail blast damage rather than a spin");
	CTR_TEST_CHECK_EQ(human.reserves, 0, "the blast clears boost reserves through ordinary damage handling");
	CTR_TEST_CHECK_EQ(human.heldItemID, HELD_ITEM_POTION, "explosive crates do not discard the held item");

	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	human.actionsFlagSet |= ACTION_MASK_WEAPON;
	g_config.maskProtectsFromDamage = true;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehStuckProc_Tumble_Init, "crate blast damage bypasses a protective mask");
	CTR_TEST_CHECK((human.actionsFlagSet & ACTION_MASK_WEAPON) != 0, "the blast does not remove the mask");
	CTR_TEST_CHECK_EQ(s_crateExplosions, 2, "mask protection does not suppress the crate explosion");
	human.actionsFlagSet &= ~ACTION_MASK_WEAPON;
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	human.invincibleTimer = 100;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehStuckProc_Tumble_Init, "crate blast damage bypasses invincibility");
	CTR_TEST_CHECK_EQ(human.invincibleTimer, 100, "the blast preserves the invincibility timer");
	human.invincibleTimer = 0;
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	struct Shield shield = {0};
	struct Thread shieldThread = {.object = &shield};
	struct Instance bubble = {.thread = &shieldThread};
	human.instBubbleHold = &bubble;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehStuckProc_Tumble_Init, "crate blast damage bypasses a shield");
	CTR_TEST_CHECK_EQ(shield.flags, 0, "the blast does not request a shield pop");
	CTR_TEST_CHECK(human.instBubbleHold == &bubble, "the blast leaves the shield attached");

	CTR_TEST_SECTION("Explosive Crates respects scope, race gates and independent activation expiry");
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct Driver finished = {.driverID = 0, .actionsFlagSet = ACTION_RACE_FINISHED};
	const int explosions = s_crateExplosions;
	Chaos_ApplyCrateRules(&bot, &crate);
	Chaos_ApplyCrateRules(&finished, &crate);
	Chaos_ApplyCrateRules(NULL, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, explosions, "bots, finished humans and missing drivers cannot trigger explosions");
	s_tracker.gameMode1 |= PAUSE_1;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, explosions, "pause disables crate explosions");
	s_tracker.gameMode1 &= ~PAUSE_1;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, explosions, "Adventure hubs retain ordinary crates");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	Tick(1000);
	human.invincibleTimer = 0;
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, explosions, "the shorter replacement expires without reviving the old effect");
	Tick(4000);
	Chaos_ApplyCrateRules(&human, &crate);
	CTR_TEST_CHECK_EQ(s_crateExplosions, explosions, "the superseded expiry cannot trigger more explosions");
}

static void TestHardCrates(void)
{
	CTR_TEST_SECTION("Hard Crates routes unfinished human contacts to solid crate collision");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_HARD_CRATES] = true;
	sdata->chaosSettings.enabledCount++;
	struct Instance crate = {.scale.x = 0x1000};
	struct ScratchpadStruct sps = {0};
	sps.Input1.modelID = DYNAMIC_PLAYER;
	struct Driver human = {.driverID = 0};
	struct Thread collidingTh = {.object = &human};
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "inactive effect leaves crates breakable");
	Chaos_Activate(CHAOS_EFFECT_HARD_CRATES, 5000);
	CTR_TEST_CHECK(Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "active effect routes an unfinished human into solid collision");
	crate.scale.x = 0;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "invisible crates cannot block the kart");
	crate.scale.x = 0x1000;
	sps.Input1.modelID = DYNAMIC_BOMB;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "weapon hits keep the retail crate response");
	sps.Input1.modelID = DYNAMIC_PLAYER;
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct Driver finished = {.driverID = 0, .actionsFlagSet = ACTION_RACE_FINISHED};
	collidingTh.object = &bot;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "bots keep retail crates");
	collidingTh.object = &finished;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "finished humans keep retail crates");
	collidingTh.object = NULL;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "missing drivers keep retail crates");
	collidingTh.object = &human;
	s_tracker.gameMode1 |= PAUSE_1;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "pause disables solid crate collisions");
	s_tracker.gameMode1 &= ~PAUSE_1;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "Adventure hubs retain ordinary crates");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	Chaos_Activate(CHAOS_EFFECT_HARD_CRATES, 1000);
	Tick(1000);
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "the shorter replacement restores breakable crates at its expiry");
	Tick(4000);
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "final expiry restores ordinary crates");
	Chaos_Reset();
	CTR_TEST_CHECK(!Chaos_IsHardCrateForDriver(&crate, &collidingTh, &sps), "race reset leaves crates breakable");
}

static void TestGiantKarts(void)
{
	CTR_TEST_SECTION("Giant Karts doubles visual/contact size and raises forward speed by 30 percent");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_GIANT_KARTS] = true;
	sdata->chaosSettings.enabledCount++;
	struct Instance humanInst = {.scale = {0xccc, 0xccc, 0xccc}};
	struct Driver human = {.driverID = 0, .instSelf = &humanInst, .wheelSize = 0xccc};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .wheelSize = 0xccc};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	CTR_TEST_CHECK_EQ(human.kartScale, 0, "before activation the driver uses retail scale");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 1000), 1000, "inactive effect preserves ordinary jumps");
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 3000);
	CTR_TEST_CHECK_EQ(human.kartScale, 2, "activation changes the driver's scale target once");
	CTR_TEST_CHECK_EQ(partner.kartScale, 2, "co-op partner receives the same scale target");
	CTR_TEST_CHECK_EQ(humanInst.scale.x, 0xccc, "activation leaves the current scale for retail interpolation");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 13000, "forward target increases by 30 percent");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, -2000), -2000, "reverse speed stays retail");
	human.const_Speed_ClassStat = 20000;
	sdata->chaosSettings.effects[CHAOS_EFFECT_SPEED_CAPPED] = true;
	sdata->chaosSettings.enabledCount++;
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 10000, "Speed Capped still limits a giant's boosted forward speed");
	CTR_TEST_CHECK_EQ(bot.kartScale, 0, "bot keeps the retail scale");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 1000), 1300, "normal, forced and spring jumps gain 30 percent impulse");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&bot, 1000), 1000, "bots retain normal jump force");
	struct Thread botThread = {0};
	struct Instance botInst = {.thread = &botThread};
	bot.instSelf = &botInst;
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&human, &bot), "a successful squash skips the blocking kart collision");
	CTR_TEST_CHECK(bot.squishTimer > 0 && bot.funcPtrs[DRIVER_FUNC_INIT] == VehPhysProc_SpinFirst_Init,
	               "a giant squashes the smaller kart through retail damage");
	CTR_TEST_CHECK_EQ(human.forcedJumpType, FORCED_JUMP_HIGH, "the giant gets the Turbo-item hop over its victim");
	CTR_TEST_CHECK_EQ(s_squishSounds, 1, "squash plays its retail sound once");
	const int squishTimer = bot.squishTimer;
	human.forcedJumpType = FORCED_JUMP_NONE;
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&bot, &human), "a squished kart cannot block the giant on repeated contact");
	CTR_TEST_CHECK_EQ(bot.squishTimer, squishTimer, "continued contact does not restart an existing squash");
	CTR_TEST_CHECK_EQ(human.forcedJumpType, FORCED_JUMP_NONE, "continued contact does not queue repeated forced hops");
	bot.squishTimer = 0;
	bot.invincibleTimer = 100;
	bot.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&human, &bot), "a protected smaller kart keeps ordinary collision response");
	CTR_TEST_CHECK_EQ(bot.squishTimer, 0, "retail invincibility protects the smaller kart");
	bot.invincibleTimer = 0;
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&human, &partner), "equally giant humans keep ordinary kart collision");
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(1000);
	CTR_TEST_CHECK_EQ(human.kartScale, 2, "pause freezes the enlarged target");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 1000);
	CTR_TEST_CHECK_EQ(human.kartScale, 2, "duplicate activation does not compound the scale target");
	Tick(1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_GIANT_KARTS), "the replacement expiry deactivates Giant Karts without a surviving copy");
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "deactivating Giant Karts restores the scale target");
	Tick(2000);
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "final expiry restores the normal target");
	CTR_TEST_CHECK_EQ(partner.kartScale, 1, "final expiry restores the co-op partner's target");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 10000, "expiry restores speed");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 1000), 1000, "expiry restores jump force");
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 1000);
	CTR_TEST_CHECK_EQ(human.kartScale, 2, "a new activation changes the target again");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "race reset restores the normal target");
	CTR_TEST_CHECK_EQ(partner.kartScale, 1, "race reset also restores the partner's target");

	CTR_TEST_SECTION("Scale and squash use driver dimensions independently of Chaos");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&bot, 25), 25, "zero-initialized scale keeps retail collision size");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 0xccc), 0xccc, "explicit normal scale keeps the retail target");
	human.kartScale = 3;
	bot.kartScale = 2;
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 25), 75, "collision size follows arbitrary driver scale");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, human.wheelSize), 0x2664, "wheel size uses the same scale as the body");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 0), 0, "squish preserves zero vertical scale");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_GIANT_KARTS), "no Chaos activation is needed for dimension-based collision");
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&bot, &human), "a larger kart squashes a smaller enlarged kart without Chaos");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 25), 25, "finished drivers use retail dimensions");
	bot.squishTimer = 0;
	bot.kartScale = 1;
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&human, &bot), "a finished enlarged driver cannot squash an equally sized kart");
}

static void TestClockCurse(void)
{
	CTR_TEST_SECTION("Clock Curse sets the received clock once and bypasses protection");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_CLOCK_CURSE] = true;
	sdata->chaosSettings.enabledCount++;
	g_config.maskProtectsFromDamage = true;
	struct Shield shieldObject = {0};
	struct Thread shieldThread = {.object = &shieldObject};
	struct Instance shield = {.thread = &shieldThread};
	struct Thread humanThread = {0};
	struct Thread partnerThread = {0};
	struct Instance humanInst = {.thread = &humanThread};
	struct Instance partnerInst = {.thread = &partnerThread};
	struct Driver human = {
		.driverID = 0, .actionsFlagSet = ACTION_MASK_WEAPON | ACTION_JUMP_BUTTON_HELD,
		.invincibleTimer = 1000, .numWumpas = 10, .reserves = 2000, .baseSpeed = 12000,
		.instSelf = &humanInst,
	};
	struct Driver partner = {.driverID = 1, .instBubbleHold = &shield, .instSelf = &partnerInst};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .clockReceive = 3000};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	const struct GamepadBuffer pad = {.stickLX = 128};
	int steering = pad.stickLX;
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "inactive Clock Curse does not introduce a received clock");
	CTR_TEST_CHECK_EQ(VehPickState_NewState(&human, 1, NULL, 0), 0, "ordinary damage still respects mask and invincibility protection");
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 1000);
	CTR_TEST_CHECK_EQ(human.clockReceive, 1000, "activation immediately assigns the full clock duration");
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehPhysProc_SpinFirst_Init, "the initial clock spin bypasses mask and invincibility");
	CTR_TEST_CHECK(partner.funcPtrs[DRIVER_FUNC_INIT] == VehPhysProc_SpinFirst_Init, "the initial clock spin bypasses shields");
	CTR_TEST_CHECK_EQ(s_clockSounds, 1, "activation plays the clock sound once for all humans");
	CTR_TEST_CHECK_EQ(s_squishSounds, 0, "Clock Curse does not use the Giant Karts squish sound");
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == NULL, "continuous slowdown does not repeat the initial hit");
	CTR_TEST_CHECK_EQ(s_clockSounds, 1, "continuous slowdown does not repeat the clock sound");
	CTR_TEST_CHECK_EQ(human.clockReceive, 968, "the curse sets the received clock to the remaining Chaos duration");
	CTR_TEST_CHECK(partner.clockReceive > 32, "the curse also reaches a co-op partner protected by a shield");
	CTR_TEST_CHECK_EQ(human.actionsFlagSet, ACTION_MASK_WEAPON | ACTION_JUMP_BUTTON_HELD, "bypassing mask protection does not remove the mask or alter jump input");
	CTR_TEST_CHECK(partner.instBubbleHold == &shield, "bypassing a shield does not consume or detach it");
	CTR_TEST_CHECK_EQ(shieldObject.flags, 0, "the initial clock hit does not request a shield pop");
	CTR_TEST_CHECK_EQ(human.invincibleTimer, 1000, "the curse bypasses invincibility without changing its timer");
	CTR_TEST_CHECK_EQ(human.numWumpas, 10, "the retail spin setup leaves fruit loss to the spin state");
	CTR_TEST_CHECK_EQ(human.reserves, 0, "the initial clock hit clears reserves through retail damage handling");
	CTR_TEST_CHECK_EQ(human.baseSpeed, 12000, "Clock Curse leaves speed calculations to the retail clock consumers");
	CTR_TEST_CHECK_EQ(human.clockFlash, 0, "continuous application does not repeat the weapon's initial flash");
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 936, "the received clock follows the remaining effect time on subsequent frames");
	partner.clockReceive = 4000;
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(partner.clockReceive, 3968, "a later retail clock counts down without being overwritten by Chaos");
	human.clockReceive = 9000;
	TickAndApplyDrivers(1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_CLOCK_CURSE), "the curse expires on its independent Chaos timer");
	CTR_TEST_CHECK_EQ(human.clockReceive, 8000, "curse expiry preserves a longer clock received during the effect");
	CTR_TEST_CHECK_EQ(partner.clockReceive, 2968, "Chaos expiry leaves a later retail clock intact");

	CTR_TEST_SECTION("reactivated curses replace the clock and remove earlier timers");
	Chaos_BeginRace(&s_tracker);
	human.clockReceive = 0;
	partner.clockReceive = 0;
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 256);
	TickAndApplyDrivers(32);
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 64);
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 32, "the replacement assigns its own remaining duration");
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the shorter replacement curse leaves no old entry behind");
	for (int i = 0; i < 4; i++)
	{
		TickAndApplyDrivers(32);
		CTR_TEST_CHECK_EQ(human.clockReceive, 0, "the superseded activation cannot resume the overwritten clock");
	}
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_CLOCK_CURSE), "the last curse expires without a separate restoration step");
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "the received clock reaches zero with the final curse timer");
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 60000);
	CTR_TEST_CHECK_EQ(human.clockReceive, 60000, "a one-off activation stores the full sixty-second duration");
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 59968, "the received clock retains durations above the signed 16-bit range");
	TickAndApplyDrivers(32000);
	CTR_TEST_CHECK_EQ(human.clockReceive, 27968, "the full clock counts down across the old signed 16-bit boundary");
	TickAndApplyDrivers(27968);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "long curses also reach zero at final expiry");

	CTR_TEST_SECTION("Clock Curse respects race gates and driver scope without a separate baseline snapshot");
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 5000);
	s_tracker.elapsedTimeMS = 32;
	Chaos_ApplyDriver(&bot, &pad, &steering, bot.actionsFlagSet);
	CTR_TEST_CHECK_EQ(bot.clockReceive, 3000, "Clock Curse does not change bot clocks");
	human.clockReceive = 0;
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_ApplyDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "finished humans do not receive the curse");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	s_tracker.gameMode1 |= PAUSE_1;
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	TickAndApplyDrivers(5000);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "pause cannot reapply one-off clock damage");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 5000), "pause blocks a new one-off activation");
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosRuntime, sizeof(saved)) == 0, "pause freezes curse lifetimes");
	s_tracker.gameMode1 &= ~PAUSE_1;
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	TickAndApplyDrivers(5000);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "Adventure hubs do not receive Clock Curse");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 5000), "Adventure hubs block a new one-off activation");
	s_tracker.gameMode1 &= ~ADVENTURE_ARENA;
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "reopening the race gate does not repeat a one-off activation");
	sdata->chaosRuntime = saved;
	human.clockReceive = 0;
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, 0, "restoring only Chaos runtime does not replay a one-off activation");
	human.clockReceive = 4000;
	const int clockBeforeReset = human.clockReceive;
	Chaos_Reset();
	TickAndApplyDrivers(32);
	CTR_TEST_CHECK_EQ(human.clockReceive, clockBeforeReset - 32, "runtime reset leaves the existing received clock to its ordinary countdown");

	CTR_TEST_SECTION("initial Clock Curse damage and audio occur only on successful activations");
	Chaos_BeginRace(&s_tracker);
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	partner.actionsFlagSet |= ACTION_RACE_FINISHED;
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	partner.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	s_tracker.drivers[2] = &bot;
	Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 1000);
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == NULL && partner.funcPtrs[DRIVER_FUNC_INIT] == NULL, "the initial hit skips finished humans");
	CTR_TEST_CHECK(bot.funcPtrs[DRIVER_FUNC_INIT] == NULL, "the initial hit skips bots");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	for (int i = 1; i < CHAOS_MAX_ACTIVATIONS; i++)
	{
		human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
		Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 1000);
		CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehPhysProc_SpinFirst_Init, "each successful reactivation delivers a new initial hit");
	}
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	const int clockSounds = s_clockSounds;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 1000), "reactivation accepts another curse");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "repeated curses still occupy only one slot");
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == VehPhysProc_SpinFirst_Init, "reactivation repeats the one-off clock hit");
	CTR_TEST_CHECK_EQ(s_clockSounds, clockSounds + 1, "reactivation replays the clock sound once");
	human.funcPtrs[DRIVER_FUNC_INIT] = NULL;
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_CLOCK_CURSE, 0), "invalid reactivation rejects another curse");
	CTR_TEST_CHECK(human.funcPtrs[DRIVER_FUNC_INIT] == NULL, "rejected activations cannot inflict clock damage");
	CTR_TEST_CHECK_EQ(s_clockSounds, clockSounds + 1, "rejected activations cannot play clock audio");
	CTR_TEST_CHECK_EQ(VehPickState_NewState(&partner, 1, NULL, 0), 0, "ordinary damage still respects shield protection");
	CTR_TEST_CHECK((shieldObject.flags & SHIELD_FLAG_POP_ON_DAMAGE) != 0, "ordinary damage retains retail shield consumption");
}

static void TestHopInputAndCloudOwnership(void)
{
	CTR_TEST_SECTION("No Hopping clears pending manual jumps without overwriting retail cloud effects");
	EnableInputEffects();
	struct Driver driver = {.driverID = 0, .kartState = KS_NORMAL};
	const u32 unrelated = ACTION_TOUCH_GROUND | ACTION_MASK_WEAPON;
	u32 actions = PreparedJump(&driver, BTN_L1, BTN_L1, RAIN_CLOUD_EFFECT_NONE, unrelated);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "ordinary hop input still latches the shoulder button");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 10, "ordinary hop input still fills the retail buffer");
	CTR_TEST_CHECK_EQ(driver.buttonUsedToStartDrift, BTN_L1, "a single tapped shoulder selects its drift button");
	PreparedJump(&driver, BTN_L1 | BTN_R1, BTN_L1 | BTN_R1, RAIN_CLOUD_EFFECT_NONE, unrelated);
	CTR_TEST_CHECK_EQ(driver.buttonUsedToStartDrift, BTN_R1, "simultaneous shoulder taps preserve the retail R1 preference");
	driver.jump_TenBuffer = 4;
	actions = PreparedJump(&driver, 0, BTN_R1, RAIN_CLOUD_EFFECT_NONE, unrelated | ACTION_JUMP_BUTTON_HELD);
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 4, "holding an already-latched shoulder does not refresh the retail buffer");
	actions = PreparedJump(&driver, 0, 0, RAIN_CLOUD_EFFECT_NONE, actions);
	CTR_TEST_CHECK_EQ(actions, unrelated, "releasing the shoulders clears the ordinary jump latch");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "releasing the shoulders clears the ordinary jump buffer");
	driver.rainCloudEffect = RAIN_CLOUD_EFFECT_ITEM_ROLL;
	driver.jump_TenBuffer = 4;
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	actions = PreparedJump(&driver, BTN_R1, BTN_R1, driver.rainCloudEffect, unrelated | ACTION_JUMP_BUTTON_HELD);
	CTR_TEST_CHECK_EQ(actions, unrelated, "active No Hopping clears only the jump latch");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "a hop buffered before activation cannot fire later");
	CTR_TEST_CHECK_EQ(driver.rainCloudEffect, RAIN_CLOUD_EFFECT_ITEM_ROLL, "Chaos does not replace the real cloud's item effect");
	CTR_TEST_CHECK_EQ(PreparedJump(&driver, 0, BTN_L1, RAIN_CLOUD_EFFECT_NONE, unrelated), unrelated,
	                  "holding a shoulder cannot refill the buffer during No Hopping");
	Tick(500);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	Tick(500);
	CTR_TEST_CHECK_EQ(PreparedJump(&driver, BTN_R1, BTN_R1, RAIN_CLOUD_EFFECT_NONE, unrelated), unrelated,
	                  "the second independent activation continues blocking hops");
	Tick(500);
	actions = PreparedJump(&driver, BTN_R1, BTN_R1, driver.rainCloudEffect, unrelated);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "hop input resumes on final expiry even while a different cloud effect exists");
	driver.jump_TenBuffer = 0;
	actions = PreparedJump(&driver, BTN_R1, BTN_R1, RAIN_CLOUD_EFFECT_JUMP_LOCKOUT, unrelated);
	CTR_TEST_CHECK_EQ(actions, unrelated, "retail cloud lockout still blocks hops after Chaos expires");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "Chaos expiry does not clear the retail cloud's jump lockout");

	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	driver.driverID = 2;
	actions = PreparedJump(&driver, BTN_L1, BTN_L1, RAIN_CLOUD_EFFECT_NONE, unrelated);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "No Hopping does not suppress input for a bot slot");
	driver.driverID = 1;
	actions = PreparedJump(&driver, BTN_L1, BTN_L1, RAIN_CLOUD_EFFECT_NONE, unrelated);
	CTR_TEST_CHECK_EQ(actions, unrelated, "No Hopping applies to the second human too");
	driver.driverID = 0;
	Chaos_Reset();
	actions = PreparedJump(&driver, BTN_L1, BTN_L1, RAIN_CLOUD_EFFECT_NONE, unrelated);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "race reset leaves no synthetic jump lockout behind");
}

static void TestDriverApplication(void)
{
	CTR_TEST_SECTION("direct application removes resolved hops while preserving drift state and shared input");
	EnableInputEffects();
	struct Driver driver = {
		.driverID = 0, .kartState = KS_DRIFTING, .jump_TenBuffer = 4,
		.actionsFlagSet = ACTION_TOUCH_GROUND | ACTION_JUMP_BUTTON_HELD,
		.forcedJumpType = FORCED_JUMP_HIGH,
	};
	struct Driver partner = {.driverID = 1};
	const struct GamepadBuffer pad = {
		.stickLX = 32,
		.buttonsHeldCurrFrame = BTN_R1 | BTN_CROSS_one,
		.buttonsTapped = BTN_L1 | BTN_CIRCLE,
	};
	const struct GamepadBuffer original = pad;
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 1000);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	int steering = pad.stickLX;
	u32 actions = VehPhysProc_JumpInput(&driver, pad.buttonsTapped, pad.buttonsHeldCurrFrame,
	                                   RAIN_CLOUD_EFFECT_NONE, driver.actionsFlagSet);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "retail input resolves its hop latch before Chaos application");
	actions = Chaos_ApplyDriver(&driver, &pad, &steering, actions);
	CTR_TEST_CHECK_EQ(steering, 224, "steering and No Hopping are applied in the same pass");
	CTR_TEST_CHECK_EQ(driver.kartState, KS_DRIFTING, "an existing drift continues without a forced state transition");
	CTR_TEST_CHECK_EQ(actions, ACTION_TOUCH_GROUND, "application removes the resolved jump latch without clearing other actions");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "pending hops are removed before jump physics can consume them");
	CTR_TEST_CHECK_EQ(driver.forcedJumpType, FORCED_JUMP_HIGH, "forced jumps are not removed by application");
	CTR_TEST_CHECK(memcmp(&pad, &original, sizeof(pad)) == 0, "neither effect writes any shared gamepad field");
	CTR_TEST_CHECK((pad.buttonsHeldCurrFrame & BTN_R1) != 0, "drift still sees its held shoulder");
	CTR_TEST_CHECK((pad.buttonsTapped & BTN_L1) != 0, "drift still sees its boost tap");
	for (int frame = 0; frame < 3; frame++)
	{
		actions = VehPhysProc_JumpInput(&driver, 0, BTN_R1, RAIN_CLOUD_EFFECT_NONE, actions);
		actions = Chaos_ApplyDriver(&driver, &pad, &steering, actions);
		CTR_TEST_CHECK_EQ(actions, ACTION_TOUCH_GROUND, "held shoulders cannot retain a resolved hop on later frames");
		CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "held shoulders cannot retain a jump buffer on later frames");
	}
	const struct GamepadBuffer partnerPad = {.stickLX = 200, .buttonsHeldCurrFrame = BTN_SQUARE_one};
	CTR_TEST_CHECK_EQ(PreparedSteering(&partner, &partnerPad), 56, "each player resolves their own steering baseline");
	CTR_TEST_CHECK_EQ(steering, 224, "applying player two preserves player one's local steering");
	driver.kartState = KS_CRASHING;
	Chaos_ApplyDriver(&driver, &pad, &steering, actions);
	CTR_TEST_CHECK_EQ(driver.kartState, KS_CRASHING, "No Hopping does not replace damage states with normal driving");
	Tick(1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&driver, &pad), pad.stickLX, "expiry uses live steering without restoring a snapshot");
	actions = PreparedJump(&driver, BTN_R1, BTN_R1, RAIN_CLOUD_EFFECT_NONE, ACTION_TOUCH_GROUND);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "expiry lets retail hop input resume without restoring a snapshot");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 10, "expiry lets retail input fill the jump buffer again");
}

static void DrawNotice(void)
{
	s_noticeDraws = 0;
	UI_Chaos_DrawActivation(&s_tracker);
}

static void TestActivationNotice(void)
{
	CTR_TEST_SECTION("activation notices use Final Lap styling and a shared full-screen center in 2P");
	EnableInputEffects();
	static struct DB buffer;
	u32 uiOT[5];
	u32 viewportOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.pushBuffer[0].ptrOT = viewportOT;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	s_tracker.numPlyrCurrGame = 1;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "there is no message before an activation");
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 10000);
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "a successful activation plays one notification sound");
	CTR_TEST_CHECK_EQ(s_lastSound, 0x65u, "the activation sound is the transition swish");
	CTR_TEST_CHECK((s_lastSoundFlags & HOWL_SFX_GAIN_DOUBLE_FLAG) != 0, "Chaos requests double gain for its notification");
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 1, "a successful activation submits its notice");
	CTR_TEST_CHECK(strcmp(s_noticeText, "Inverted Steering") == 0, "the notice uses the catalogue effect name");
	CTR_TEST_CHECK(s_noticeX >= SCREEN_WIDTH, "the activation name begins its entrance off the right edge");
	Tick(320);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeX, SCREEN_WIDTH / 2, "the name settles horizontally centered");
	CTR_TEST_CHECK_EQ(s_noticeY, SCREEN_HEIGHT / 4, "solo placement follows the Final Lap quarter-height position");
	CTR_TEST_CHECK_EQ(s_noticeFont, FONT_BIG, "the activation name uses the Final Lap font");
	CTR_TEST_CHECK_EQ(s_noticeFlags, JUSTIFY_CENTER | ORANGE, "the activation name uses centered orange text");
	CTR_TEST_CHECK(s_noticeOT == uiOT, "the message uses the full-screen UI ordering table");
	const struct ChaosRuntime beforeDraw = sdata->chaosRuntime;
	DrawNotice();
	CTR_TEST_CHECK(memcmp(&beforeDraw, &sdata->chaosRuntime, sizeof(beforeDraw)) == 0, "drawing cannot advance notification or effect timers");
	CTR_TEST_CHECK_EQ(s_activationSounds, 1, "rendering an ongoing banner does not replay the activation sound");

	s_tracker.numPlyrCurrGame = 2;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	s_tracker.pushBuffer[1].rect = (RECT){0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 1, "2P gets one shared announcement rather than two viewport messages");
	CTR_TEST_CHECK_EQ(s_noticeX, 256, "the 2P announcement is horizontally centered on the whole screen");
	CTR_TEST_CHECK_EQ(s_noticeY, 100, "the 2P announcement matches the central continue-prompt baseline");
	CTR_TEST_CHECK(s_noticeOT == uiOT && s_noticeOT != viewportOT, "2P text is not clipped to either player's OT");

	s_tracker.gameMode1 |= PAUSE_1;
	Tick(1000);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "pause hides the activation announcement");
	CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), 3000, "pause freezes announcement time together with effect timers");
	s_tracker.gameMode1 &= ~PAUSE_1;
	s_tracker.hudFlags = 0;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "a disabled race HUD hides the announcement");
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	sdata->ptrActiveMenu = &sdata->chaosMenu.menu;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "menu screens do not receive the race announcement");
	sdata->ptrActiveMenu = NULL;
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 10000);
	CTR_TEST_CHECK_EQ(s_activationSounds, 2, "a second successful activation requests its own swish");
	CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "a later activation starts its own announcement lifetime");
	Tick(320);
	DrawNotice();
	CTR_TEST_CHECK(strcmp(s_noticeText, "No Hopping") == 0, "the newest activation replaces the displayed name");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "replacing the announcement preserves both running effects");
	const struct ChaosRuntime beforeReject = sdata->chaosRuntime;
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 1000);
	CTR_TEST_CHECK_EQ(s_activationSounds, 2, "a rejected activation does not request a sound");
	CTR_TEST_CHECK(memcmp(&beforeReject, &sdata->chaosRuntime, sizeof(beforeReject)) == 0, "a rejected activation cannot replace the current announcement");
	Tick(2840);
	DrawNotice();
	CTR_TEST_CHECK(s_drawX[0] > SCREEN_WIDTH / 2 && s_drawX[0] < UI_CHAOS_LIST_RIGHT, "the newest name moves toward the right-hand table after its initial three seconds");
	CTR_TEST_CHECK_EQ(s_drawFont[0], FONT_SMALL, "the docking name changes to the compact font");
	CTR_TEST_CHECK_EQ(s_drawY[0], UI_CHAOS_LIST_MULTIPLAYER_TOP, "the newest docking effect occupies the high first row");
	Tick(160);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "both running effects remain visible after the announcement finishes");
	CTR_TEST_CHECK_EQ(s_noticeX, UI_CHAOS_LIST_RIGHT, "the persistent names share a right-hand anchor");
	CTR_TEST_CHECK_EQ(s_noticeFlags, JUSTIFY_RIGHT | ORANGE, "persistent rows are right-aligned orange text");
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_NO_HOPPING), "notification expiry does not expire the effect itself");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), 0, "race reset also clears pending announcement state");
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "race reset removes the persistent list too");
}

static void TestActiveEffectList(void)
{
	CTR_TEST_SECTION("the borderless effect list deduplicates independent timers and removes expired rows");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 10000);
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 1, "a docked effect remains visible without a running notice timer");
	CTR_TEST_CHECK_EQ(s_noticeFont, FONT_SMALL, "docked effects use the compact font");
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 12000);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "a new banner does not hide older active effects");
	CTR_TEST_CHECK_EQ(s_drawFont[0], FONT_SMALL, "the earlier effect stays in its compact table row during a new banner");
	CTR_TEST_CHECK_EQ(s_noticeFont, FONT_BIG, "the newest activation still gets its large announcement");
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 15000);
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "overlapping copies of one effect share a single row");
	CTR_TEST_CHECK(strcmp(s_drawText[0], "Inverted Steering") == 0 && strcmp(s_drawText[1], "No Hopping") == 0,
	               "distinct rows follow activation order");
	CTR_TEST_CHECK_EQ(s_drawX[0], s_drawX[1], "all table rows use the same horizontal anchor");
	CTR_TEST_CHECK_EQ(s_drawY[1] - s_drawY[0], UI_CHAOS_LIST_ROW_HEIGHT, "table rows use a fixed vertical spacing");
	Tick(3360);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "expiry of the original copy preserves the later activation's row");
	Tick(5320);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 1, "a fully expired effect no longer occupies a row");
	CTR_TEST_CHECK(strcmp(s_noticeText, "Inverted Steering") == 0, "only the surviving effect remains listed");
	CTR_TEST_CHECK_EQ(s_noticeY, UI_CHAOS_LIST_MULTIPLAYER_TOP, "the surviving 2P rows compact without gaps");
	Tick(3000);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "the final expiry empties the table");

	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		sdata->chaosSettings.effects[i] = true;
	}
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		if (!ChaosEffect_IsInstant((ChaosEffectId)i))
		{
			Chaos_Activate((ChaosEffectId)i, 30000);
		}
	}
	s_tracker.numPlyrCurrGame = 2;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	s_tracker.pushBuffer[1].rect = (RECT){0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	sdata->finalLapTextTimer[0] = 30;
	sdata->finalLapTextTimer[1] = 30;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, CHAOS_MAX_ACTIVATIONS, "all four accepted simultaneous announcements remain visible");
	for (int i = 0; i < s_noticeDraws; i++)
	{
		CTR_TEST_CHECK(s_drawY[i] + UI_CHAOS_COMPACT_BANNER_ROW_HEIGHT <= SCREEN_HEIGHT,
		               "crowded announcements fit on screen even with both Final Lap messages");
	}
	sdata->finalLapTextTimer[0] = 0;
	sdata->finalLapTextTimer[1] = 0;
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, UI_CHAOS_MAX_DOCKED, "the docked table shows at most the four latest distinct effects");
	CTR_TEST_CHECK(s_noticeY + UI_CHAOS_LIST_ROW_HEIGHT <= SCREEN_HEIGHT, "all active docked effects fit within the screen height");
	Chaos_Reset();
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 100);
	Tick(100);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "a short-lived effect cannot leave a banner or table row after expiry");
}

static void TestHudVisibilityAndDockLimit(void)
{
	CTR_TEST_SECTION("1P and 2P show all four active effects and hide the HUD when racing ends");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
	{
		sdata->chaosSettings.effects[effect] = true;
	}
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	s_tracker.numPlyrCurrGame = 2;
	const ChaosEffectId effects[] = {
		CHAOS_EFFECT_INVERTED_STEERING, CHAOS_EFFECT_NO_HOPPING,
		CHAOS_EFFECT_LOW_GRAVITY, CHAOS_EFFECT_NO_ITEMS,
	};
	for (int effect = 0; effect < (int)len(effects); effect++)
	{
		Chaos_Activate(effects[effect], effect == 0 ? 5000 : 30000);
	}
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 5000), "an active effect can refresh even when all slots are occupied");
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 4, "2P limits the docked list to four distinct effects");
	CTR_TEST_CHECK(strcmp(s_drawText[0], "Inverted Steering") == 0, "reactivation moves the effect to the newest row");
	CTR_TEST_CHECK(strcmp(s_drawText[1], "No Items") == 0 && strcmp(s_drawText[3], "No Hopping") == 0,
	               "the docked list orders all four accepted effects by activation recency");
	CTR_TEST_CHECK_EQ(s_drawY[0], UI_CHAOS_LIST_MULTIPLAYER_TOP, "the 2P docked list starts near the top edge");
	const int firstPlayerLapCounterY = 71;
	CTR_TEST_CHECK(s_drawY[3] + UI_CHAOS_LIST_ROW_HEIGHT < firstPlayerLapCounterY,
	               "all four 2P docked rows stay above the lap counter");
	Tick(5000 - CHAOS_ACTIVATION_NOTICE_MS - CHAOS_ACTIVATION_DOCK_MS);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 30000), "expiry frees a slot for a new activation");
	Tick(UI_CHAOS_NOTICE_SLIDE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 4, "a central announcement can coexist with the three surviving docked effects");
	CTR_TEST_CHECK_EQ(s_noticeX, SCREEN_WIDTH / 2, "the new 2P announcement is horizontally centered");
	CTR_TEST_CHECK_EQ(s_noticeY, UI_CHAOS_NOTICE_MULTIPLAYER_Y, "docked effects cannot push a new 2P announcement toward the bottom");
	s_allHumansFinished = true;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "all humans finishing hides both central and docked messages before results initialization");
	s_allHumansFinished = false;
	s_tracker.gameMode1 |= END_OF_RACE;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "the results mode also hides central and docked messages");
	s_tracker.gameMode1 &= ~END_OF_RACE;
	s_tracker.numPlyrCurrGame = 1;
	Tick(CHAOS_ACTIVATION_NOTICE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 4, "1P uses the same four-effect docked limit");
	CTR_TEST_CHECK(strcmp(s_drawText[0], "Speed Capped") == 0, "the latest newly docked effect occupies the first row");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 4, "the HUD preserves all four running activations");
}

static void TestHudLayeringAndSoloStack(void)
{
	CTR_TEST_SECTION("solo banners stack downward and docked labels keep fixed rows behind lap statistics");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	s_tracker.numPlyrCurrGame = 1;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 20000);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 20000);
	Tick(UI_CHAOS_NOTICE_SLIDE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_drawY[1], SCREEN_HEIGHT / 4, "the newest solo banner stays on the normal row");
	CTR_TEST_CHECK_EQ(s_drawY[0], SCREEN_HEIGHT / 4 + UI_CHAOS_BANNER_ROW_HEIGHT,
	                  "the other solo banner goes below the normal row instead of over the item icon");
	Tick(CHAOS_ACTIVATION_NOTICE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "both effects finish docking");
	sdata->finalLapTextTimer[0] = 30;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_drawY[0], UI_CHAOS_LIST_TOP, "Final Lap does not move the first solo docked row");
	CTR_TEST_CHECK_EQ(s_drawY[1], UI_CHAOS_LIST_TOP + UI_CHAOS_LIST_ROW_HEIGHT,
	                  "Final Lap does not insert gaps into the docked list");
	s_tracker.numPlyrCurrGame = 2;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	s_tracker.pushBuffer[1].rect = (RECT){0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_drawY[0], 32, "the 2P dock preserves the user's adjusted vertical position even during Final Lap");
	for (int row = 0; row < s_noticeDraws; row++)
	{
		CTR_TEST_CHECK_EQ(s_drawScale[row], FP(0.5), "docked glyphs preserve the user's half-size scale");
		CTR_TEST_CHECK(s_drawOT[row] == &uiOT[UI_HUD_ORDERING_SLOT_BEHIND_HUD],
		               "docked effects use the lower UI layer behind finishing lap statistics");
	}
	sdata->chaosSettings.effects[CHAOS_EFFECT_WOBBLE_STEERING] = true;
	sdata->chaosSettings.enabledCount++;
	Chaos_Activate(CHAOS_EFFECT_WOBBLE_STEERING, 20000);
	Tick(UI_CHAOS_NOTICE_SLIDE_MS);
	DrawNotice();
	CTR_TEST_CHECK(s_noticeOT == uiOT, "central banners retain the normal foreground UI layer");
	Tick(2840);
	DrawNotice();
	CTR_TEST_CHECK(s_drawOT[0] == &uiOT[UI_HUD_ORDERING_SLOT_BEHIND_HUD],
	               "a label already moves behind lap statistics during its docking animation");
}

static void TestBannerBaselineWithDockedEffects(void)
{
	CTR_TEST_SECTION("docked rows do not displace the Final Lap baseline or the shared 2P center");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	for (int effect = 0; effect < CHAOS_EFFECT_COUNT; effect++)
	{
		sdata->chaosSettings.effects[effect] = true;
	}
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	s_tracker.numPlyrCurrGame = 1;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, 200};
	const ChaosEffectId docked[] = {
		CHAOS_EFFECT_INVERTED_STEERING, CHAOS_EFFECT_NO_HOPPING, CHAOS_EFFECT_LOW_GRAVITY,
	};
	for (int effect = 0; effect < (int)len(docked); effect++)
	{
		Chaos_Activate(docked[effect], 30000);
	}
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 30000);
	Tick(UI_CHAOS_NOTICE_SLIDE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeY, 50, "a solo banner uses exactly the viewport quarter-height used by Final Lap");
	CTR_TEST_CHECK_EQ(s_drawY[1] - s_drawY[0], 6, "half-size docked labels use the tighter six-pixel row spacing");
	CTR_TEST_CHECK_EQ(s_drawScale[0], FP(0.5), "tighter spacing preserves the user's docked font scale");
	sdata->finalLapTextTimer[0] = 30;
	DrawNotice();
	CTR_TEST_CHECK(s_noticeY >= 50 + UI_CHAOS_FINAL_LAP_HEIGHT,
	               "a visible Final Lap message reserves the normal solo banner row");
	sdata->finalLapTextTimer[0] = 0;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeY, 50, "the banner returns to the normal row as soon as Final Lap releases it");
	s_tracker.numPlyrCurrGame = 2;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeY, UI_CHAOS_NOTICE_MULTIPLAYER_Y,
	                  "three docked rows do not shift the shared 2P center");
	CTR_TEST_CHECK_EQ(s_drawY[0], 32, "tighter row spacing preserves the user's 2P dock position");
}

static void TestNotificationRows(void)
{
	CTR_TEST_SECTION("Final Lap, simultaneous activations and docked effects occupy disjoint rows");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	s_tracker.numPlyrCurrGame = 1;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT};
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 20000);
	Tick(1000);
	const s32 firstNoticeTime = LatestNoticeRemaining();
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 20000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].noticeRemainingMS, firstNoticeTime,
	                  "a second activation does not replace or restart the first announcement");
	Tick(320);
	sdata->finalLapTextTimer[0] = 1;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 2, "both recent activations remain visible at once");
	CTR_TEST_CHECK_EQ(s_drawFont[0], FONT_BIG, "the first activation retains its introductory banner");
	CTR_TEST_CHECK_EQ(s_drawFont[1], FONT_BIG, "the second activation also gets a large banner");
	CTR_TEST_CHECK(s_drawY[0] >= SCREEN_HEIGHT / 4 + UI_CHAOS_FINAL_LAP_HEIGHT,
	               "even the last rendered frame of Final Lap reserves its row");
	CTR_TEST_CHECK(s_drawY[0] >= s_drawY[1] + UI_CHAOS_BANNER_ROW_HEIGHT,
	               "concurrent activation banners cannot overlap vertically");
	sdata->finalLapTextTimer[0] = 0;
	Tick(2000);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_drawFont[0], FONT_SMALL, "the older activation docks while the newer one is still announcing");
	CTR_TEST_CHECK_EQ(s_drawScale[0], FP(0.5), "docked text preserves the user-selected half-size font");
	CTR_TEST_CHECK_EQ(s_drawFont[1], FONT_BIG, "docking an older activation does not shrink a newer banner");
	CTR_TEST_CHECK_EQ(s_drawY[1], SCREEN_HEIGHT / 4,
	                  "the solo banner keeps its normal baseline while older effects are docked");

	s_tracker.numPlyrCurrGame = 2;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	s_tracker.pushBuffer[1].rect = (RECT){0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	sdata->finalLapTextTimer[0] = 30;
	sdata->finalLapTextTimer[1] = 30;
	DrawNotice();
	const int bottomLapY = SCREEN_HEIGHT / 2 + SCREEN_HEIGHT / 8;
	for (int i = 0; i < s_noticeDraws; i++)
	{
		CTR_TEST_CHECK(s_drawY[i] + UI_CHAOS_FINAL_LAP_HEIGHT <= bottomLapY ||
		               s_drawY[i] >= bottomLapY + UI_CHAOS_FINAL_LAP_HEIGHT,
		               "the shared 2P table avoids player two's Final Lap row");
	}
	const struct ChaosRuntime beforeDraw = sdata->chaosRuntime;
	DrawNotice();
	CTR_TEST_CHECK(memcmp(&beforeDraw, &sdata->chaosRuntime, sizeof(beforeDraw)) == 0,
	               "table layout and drawing do not advance independent notice timers");
}

static void TestExternalCommands(void)
{
	CTR_TEST_SECTION("External waits from GO, consumes one command, and preserves pending requests across gates");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	const struct ChaosSettings saved = sdata->chaosSettings;
	s_externalCommand = "low-gravity";
	s_externalReads = 0;
	Tick(0);
	CTR_TEST_CHECK_EQ(s_externalReads, 0, "commands are not read before the initial interval");
	CTR_TEST_CHECK(strcmp(s_externalCatalogue, "low-gravity\n") == 0, "the catalogue contains only enabled stable slugs");
	s_tracker.trafficLightsTimer = 100;
	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "countdown does not spend the initial interval");
	s_tracker.trafficLightsTimer = 0;
	Tick(4999);
	CTR_TEST_CHECK_EQ(s_externalReads, 0, "the whole initial interval must elapse");
	Tick(1);
	CTR_TEST_CHECK(s_externalCommand == NULL, "activation consumes the pending command");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "external activation receives the full duration");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "successful activation restarts a full interval");
	s_externalCommand = "low-gravity";
	s_tracker.gameMode1 |= PAUSE_ALL;
	Tick(5000);
	CTR_TEST_CHECK(s_externalCommand != NULL, "pause preserves the next request");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "pause freezes the activation interval");
	s_tracker.gameMode1 &= ~PAUSE_ALL;
	Tick(5001);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "external reactivation preserves unique IDs");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 30000, "reactivation refreshes the saved duration");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "frame overshoot cannot shorten the next interval");

	Tick(5000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 0, "an empty slot leaves the provider ready");
	const char *invalid[] = {"", "missing", "giant-karts", "low-gravity 2", "effect low-gravity", "reset", "status"};
	s_externalErrors = 0;
	for (size_t i = 0; i < len(invalid); i++)
	{
		s_externalCommand = invalid[i];
		Tick(1);
		CTR_TEST_CHECK(s_externalCommand == NULL, "invalid and disabled requests are consumed");
		CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 0, "rejections do not restart the interval");
	}
	CTR_TEST_CHECK_EQ(s_externalErrors, len(invalid), "each rejected command is logged");
	s_externalCommand = "low-gravity";
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_ADV_HUB;
	Tick(1);
	CTR_TEST_CHECK(s_externalCommand != NULL, "hub transitions preserve pending commands");
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	Chaos_BeginRace(&s_tracker);
	Tick(0);
	CTR_TEST_CHECK(s_externalCommand != NULL, "a new race starts a fresh interval with the same pending command");
	Tick(5000);
	CTR_TEST_CHECK(s_externalCommand == NULL, "the pending command can run in the next race");
	CTR_TEST_CHECK(memcmp(&saved, &sdata->chaosSettings, sizeof(saved)) == 0, "provider commands never change creation settings");
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_INTERVAL;
	s_externalCommand = "low-gravity";
	Tick(1);
	CTR_TEST_CHECK(s_externalCommand != NULL, "other triggers leave the external slot untouched");
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = false;
	Chaos_BeginRace(&s_tracker);
	Tick(1);
	CTR_TEST_CHECK(s_externalCatalogue[0] == '\0', "empty selections replace the previous catalogue with an empty list");
	sdata->chaosSettings.armed = false;
	sdata->chaosSettings.effects[CHAOS_EFFECT_LOW_GRAVITY] = true;
	Chaos_BeginRace(&s_tracker);
	Tick(1);
	CTR_TEST_CHECK(s_externalCatalogue[0] == '\0', "leaving Chaos clears stale available effects");
	s_externalCommand = NULL;
}

static void TestCataloguePublicationLifecycle(void)
{
	CTR_TEST_SECTION("catalogue publication occurs only during initialization");
	s_cataloguePublications = 0;
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	CTR_TEST_CHECK_EQ(s_cataloguePublications, 1, "initialization publishes the active selection once");
	Tick(0);
	Tick(1);
	CTR_TEST_CHECK_EQ(s_cataloguePublications, 1, "frame updates never publish the catalogue");
	sdata->chaosSettings.effects[CHAOS_EFFECT_GIANT_KARTS] = true;
	Chaos_BeginRace(&s_tracker);
	Tick(0);
	CTR_TEST_CHECK_EQ(s_cataloguePublications, 2, "the next initialization publishes its selection exactly once");
	CTR_TEST_CHECK(strstr(s_externalCatalogue, "giant-karts\n") != NULL, "the new publication reflects the new profile");
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	Chaos_BeginRace(&s_tracker);
	CTR_TEST_CHECK(s_externalCatalogue[0] == '\0', "hubs publish no applicable race commands");
}

static void EnableGiftEffects(void)
{
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	s_externalCommand = NULL;
	s_maskGrants = 0;
	s_shieldGrants = 0;
	for (int id = 0; id < CHAOS_EFFECT_COUNT; id++)
	{
		sdata->chaosSettings.effects[id] = true;
	}
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
}

static void TestInstantGifts(void)
{
	CTR_TEST_SECTION("instant rewards bypass timed capacity and never dock");
	EnableGiftEffects();
	const ChaosEffectId timed[] = {
		CHAOS_EFFECT_LOW_GRAVITY, CHAOS_EFFECT_NO_HOPPING,
		CHAOS_EFFECT_NO_ITEMS, CHAOS_EFFECT_INVERTED_STEERING,
	};
	for (size_t i = 0; i < len(timed); i++)
	{
		CTR_TEST_CHECK(Chaos_Activate(timed[i], 10000), "the timed effect pool can fill independently");
	}
	s_defaultDrivers[1].actionsFlagSet = ACTION_RACE_FINISHED;
	s_defaultDrivers[1].numWumpas = 2;
	struct Driver bot = {.driverID = 2, .numWumpas = 3, .actionsFlagSet = ACTION_BOT};
	s_tracker.drivers[2] = &bot;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1), "a one-shot reward works with all timed slots occupied");
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].numWumpas, 0, "Juiced Up leaves crediting to the ordinary gradual HUD pickup");
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].PickupWumpaHUD.numCollected, DRIVER_WUMPA_JUICED_COUNT, "Juiced Up queues the missing Wumpas");
	const int chimesBefore = s_juicedSounds;
	while (s_defaultDrivers[0].PickupWumpaHUD.numCollected > 0)
	{
		s_defaultDrivers[0].PickupWumpaHUD.numCollected--;
		RB_Player_ModifyWumpa(&s_defaultDrivers[0], 1);
	}
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].numWumpas, DRIVER_WUMPA_JUICED_COUNT, "normal pickup crediting fills the human's Wumpas");
	CTR_TEST_CHECK_EQ(s_juicedSounds, chimesBefore + 1, "reaching ten through normal crediting plays the juiced-up chime once");
	CTR_TEST_CHECK_EQ(s_defaultDrivers[1].numWumpas, 2, "finished humans receive no gift");
	CTR_TEST_CHECK_EQ(bot.numWumpas, 3, "bots receive no human gift");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, CHAOS_MAX_ACTIVATIONS, "instant rewards consume no timed slot");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_JUICED_UP), "an instant reward is never an ongoing modifier");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 1, "the reward has its own notice");
	static struct DB buffer;
	u32 uiOT[5];
	buffer.otMem.uiOT = uiOT;
	s_tracker.backBuffer = &buffer;
	s_tracker.hudFlags = HUD_FLAG_RACE_HUD;
	const struct ChaosRuntime snapshot = sdata->chaosRuntime;
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(500);
	CTR_TEST_CHECK(memcmp(&snapshot, &sdata->chaosRuntime, sizeof(snapshot)) == 0, "pause also freezes instant notices");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Tick(CHAOS_ACTIVATION_NOTICE_MS - 100);
	DrawNotice();
	CTR_TEST_CHECK(strcmp(s_noticeText, "Juiced Up") == 0, "the instant notice survives the requested effect duration");
	CTR_TEST_CHECK_EQ(s_noticeFont, FONT_BIG, "the instant notice does not enter the compact docking animation");
	Tick(100);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, CHAOS_MAX_ACTIVATIONS, "only timed effects remain after the instant banner expires");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 0, "expired notices leave no docked entry");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1000), "Juiced Up remains eligible at ten Wumpas");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1000), "one-shot rewards can repeat immediately");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 1, "repeated gifts refresh rather than duplicate their banner");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 0), "zero-duration requests remain invalid for instant effects");
	const struct Driver before = s_defaultDrivers[0];
	Chaos_Activate(CHAOS_EFFECT_IT_IS_I, 1000);
	Chaos_Activate(CHAOS_EFFECT_PROVIDENCE, 1000);
	CTR_TEST_CHECK_EQ(s_maskGrants, 1, "It is I invokes the normal mask activation only for the unfinished human");
	CTR_TEST_CHECK_EQ(s_shieldGrants, 1, "Providence invokes the normal shield activation only for the unfinished human");
	CTR_TEST_CHECK(memcmp(&before, &s_defaultDrivers[0], sizeof(before)) == 0, "dispatch preserves held inventory and its mask choice");
	s_defaultDrivers[0].numWumpas = 0;
	Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1000);
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].PickupWumpaHUD.numCollected, DRIVER_WUMPA_JUICED_COUNT, "repeating an instant effect queues its reward again");
	Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1000);
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].PickupWumpaHUD.numCollected, DRIVER_WUMPA_JUICED_COUNT, "repeats during collection do not duplicate pending Wumpas");
	Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000);
	s_defaultDrivers[0].reserves = 1;
	s_defaultDrivers[0].fireSpeedCap = 1;
	Chaos_Activate(CHAOS_EFFECT_RESERVE_INJECTION, 1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, CHAOS_MAX_INSTANT_NOTICES, "a fifth distinct reward keeps notice storage bounded");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.instantNotices[0].effect, CHAOS_EFFECT_PROVIDENCE, "notice overflow drops the oldest banner");
	const struct ChaosRuntime notices = sdata->chaosRuntime;
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 0, "reset clears instant notices too");
	sdata->chaosRuntime = notices;
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, CHAOS_MAX_INSTANT_NOTICES, "resident restore retains instant announcements");
	s_tracker.gameMode1 |= CRYSTAL_CHALLENGE;
	s_defaultDrivers[0].numWumpas = 3;
	s_defaultDrivers[0].PickupWumpaHUD.numCollected = 2;
	Chaos_Activate(CHAOS_EFFECT_JUICED_UP, 1000);
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].numWumpas, DRIVER_WUMPA_JUICED_COUNT, "crystal events receive the Wumpa reward through normal crediting");
	CTR_TEST_CHECK_EQ(s_defaultDrivers[0].PickupWumpaHUD.numCollected, 2, "the Wumpa reward cannot add or consume queued crystals");
}

static void TestRandomItemGift(void)
{
	CTR_TEST_SECTION("Use This! shares item bookkeeping, uses Chaos RNG, and respects orb ownership");
	EnableGiftEffects();
	bool uniformSeen[HELD_ITEM_MISSILE_3X + 1] = {0};
	for (u32 sample = 0; sample < HELD_ITEM_MISSILE_3X; sample++)
	{
		VehPhysGeneral_GiveRandomItem(&s_defaultDrivers[0], sample);
		const u8 item = s_defaultDrivers[0].heldItemID;
		CTR_TEST_CHECK(!uniformSeen[item], "duplicate retail weights contribute only one outcome each to the uniform pool");
		uniformSeen[item] = true;
	}
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 10000);
	struct Driver *human = &s_defaultDrivers[0];
	s_itemSounds = 0;
	human->heldItemID = HELD_ITEM_ROULETTE;
	human->itemRollTimer = 100;
	human->noItemTimer = 20;
	human->actionsFlagSet |= ACTION_WEAPON_FIRE_REQUEST;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000), "the item gift replaces No Items before assignment");
	CTR_TEST_CHECK_EQ(s_itemSounds, s_tracker.numPlyrCurrGame, "each human item grant plays the usual acquisition sound");
	CTR_TEST_CHECK_EQ(s_itemSound, 0x5e, "unjuiced item grants use the ordinary crate-item sound");
	human->numWumpas = DRIVER_WUMPA_JUICED_COUNT;
	Chaos_GiveRandomItem(human);
	CTR_TEST_CHECK_EQ(s_itemSound, 0x41, "juiced item grants use the ordinary upgraded-item sound");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_ITEMS), "No Items cannot immediately erase the gift");
	CTR_TEST_CHECK_EQ(human->itemRollTimer, 0, "the previous roulette stops");
	CTR_TEST_CHECK_EQ(human->noItemTimer, 0, "the new item is immediately available");
	CTR_TEST_CHECK((human->actionsFlagSet & ACTION_WEAPON_FIRE_REQUEST) == 0, "a stale launch request cannot fire the new item");
	bool seen[HELD_ITEM_ROULETTE + 1] = {0};
	for (int sample = 0; sample < 300; sample++)
	{
		Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000);
		for (int player = 0; player < s_tracker.numPlyrCurrGame; player++)
		{
			const struct Driver *driver = s_tracker.drivers[player];
			CTR_TEST_CHECK(driver->heldItemID <= HELD_ITEM_MISSILE_3X && driver->heldItemID != HELD_ITEM_SPRING,
			               "race gifts exclude unused and battle-only items");
			seen[driver->heldItemID] = true;
			CTR_TEST_CHECK_EQ(driver->numHeldItems,
			                  driver->heldItemID == HELD_ITEM_BOMB_3X || driver->heldItemID == HELD_ITEM_MISSILE_3X ? HELD_ITEM_STACK_COUNT : 0,
			                  "multi-item rewards receive the standard stack count");
		}
	}
	CTR_TEST_CHECK(seen[HELD_ITEM_MASK] && seen[HELD_ITEM_CLOCK] && seen[HELD_ITEM_WARPBALL] &&
	               seen[HELD_ITEM_BOMB_3X] && seen[HELD_ITEM_MISSILE_3X], "the pool includes powerful and multi-item results");
	struct Thread orb = {.modelIndex = DYNAMIC_WARPBALL};
	s_tracker.threadBuckets[TRACKING].thread = &orb;
	s_tracker.gameMode1 |= WARPBALL_HELD;
	for (int sample = 0; sample < 50; sample++)
	{
		Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000);
		CTR_TEST_CHECK(human->heldItemID != HELD_ITEM_WARPBALL && s_defaultDrivers[1].heldItemID != HELD_ITEM_WARPBALL,
		               "an already deployed orb is excluded from both independent rolls");
	}
	human->heldItemID = HELD_ITEM_WARPBALL;
	VehPhysGeneral_ClearHeldItem(human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "discarding a held orb retains a deployed orb's flag");
	s_tracker.threadBuckets[TRACKING].thread = NULL;
	human->heldItemID = HELD_ITEM_WARPBALL;
	s_defaultDrivers[1].heldItemID = HELD_ITEM_WARPBALL;
	VehPhysGeneral_ClearHeldItem(human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "another holder retains the orb flag");
	VehPhysGeneral_ClearHeldItem(&s_defaultDrivers[1]);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) == 0, "discarding the last undeployed orb releases the flag");
	s_tracker.gameMode1 |= RELIC_RACE;
	Chaos_PublishEffects();
	CTR_TEST_CHECK(strstr(s_externalCatalogue, "use-this\n") != NULL, "relic races publish the item gift too");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000), "relic races accept the item gift");
	s_tracker.gameMode1 &= ~RELIC_RACE;
	s_tracker.gameMode1 |= CRYSTAL_CHALLENGE;
	Chaos_PublishEffects();
	CTR_TEST_CHECK(strstr(s_externalCatalogue, "use-this\n") != NULL, "crystal challenges publish the item gift too");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_USE_THIS, 1000), "crystal challenges accept the full race item pool");
	memset(sdata->chaosSettings.effects, 0, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.effects[CHAOS_EFFECT_USE_THIS] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_JUICED_UP] = true;
	sdata->chaosSettings.enabledCount = 2;
	for (int sample = 0; sample < 20; sample++)
	{
		const ChaosEffectId picked = Chaos_PickEligibleEffect();
		CTR_TEST_CHECK(picked == CHAOS_EFFECT_USE_THIS || picked == CHAOS_EFFECT_JUICED_UP, "crystal challenges can select either enabled reward");
	}
	s_defaultDrivers[0].actionsFlagSet |= ACTION_RACE_FINISHED;
	s_defaultDrivers[1].actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(Chaos_PickEligibleEffect(), CHAOS_EFFECT_NONE, "selection still filters effects without unfinished targets");
}

static void TestReserveGifts(void)
{
	CTR_TEST_SECTION("reserve gifts use normal fire grants, saturate fuel, and replace incompatible rules");
	EnableGiftEffects();
	struct Thread thread = {.modelIndex = DYNAMIC_PLAYER};
	struct Instance instance = {.thread = &thread};
	struct Driver *human = &s_defaultDrivers[0];
	human->instSelf = &instance;
	human->const_SingleTurboSpeed = 1000;
	human->const_SacredFireSpeed = 2000;
	s_defaultDrivers[1].reserves = 100;
	s_defaultDrivers[1].fireSpeedCap = 3000;
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 10000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_RESERVE_INJECTION, 1000), "Reserve Injection ends No Reserves and grants fuel");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_RESERVES), "No Reserves is removed before the grant");
	CTR_TEST_CHECK_EQ(human->reserves, CHAOS_INJECTION_RESERVES, "the empty kart receives ten seconds of reserves");
	CTR_TEST_CHECK_EQ(human->fireSpeedCap, 1500, "an empty kart gets the third drift boost's normal fire level");
	CTR_TEST_CHECK_EQ(s_defaultDrivers[1].fireSpeedCap, 3000, "an existing stronger fire level is preserved");
	VehFire_Increment(human, 1000, POWER_SLIDE_HANG_TIME, CHAOS_THIRD_DRIFT_FIRE_LEVEL);
	CTR_TEST_CHECK_EQ(human->reserves, CHAOS_INJECTION_RESERVES + 1000, "normal boost grants can still add to the ten-second reward");
	human->reserves = CHAOS_RESERVES_MAX;
	VehFire_Increment(human, 1000, POWER_SLIDE_HANG_TIME, CHAOS_THIRD_DRIFT_FIRE_LEVEL);
	CTR_TEST_CHECK_EQ(human->reserves, CHAOS_RESERVES_MAX, "earned reserves at the signed limit saturate instead of overflowing");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_RESERVE_INJECTION, 1000), "the gift remains eligible when reserves are already full");
	CTR_TEST_CHECK_EQ(human->reserves, CHAOS_RESERVES_MAX, "the gift cannot reduce a larger earned reserve balance");
	human->reserves = 2000;
	Chaos_Activate(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, 1000);
	const ScrubFlags originalFlags = data.MetaDataScrub[COLL_SCRUB_SOLID].flags;
	const struct Scrub protectedSurface = VehAfterColl_GetSurface(COLL_SCRUB_SOLID, human);
	CTR_TEST_CHECK((protectedSurface.flags & SCRUB_FLAG_KEEP_RESERVES) != 0, "protected wall contacts use the existing keep-reserves flag");
	CTR_TEST_CHECK_EQ(data.MetaDataScrub[COLL_SCRUB_SOLID].flags, originalFlags, "surface rules leave shared metadata untouched");
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 1968), 2000, "natural consumption cannot lower frozen reserves");
	VehFire_Increment(human, 500, POWER_SLIDE_HANG_TIME, CHAOS_THIRD_DRIFT_FIRE_LEVEL);
	CTR_TEST_CHECK_EQ(human->reserves, 2500, "boosts can still increase frozen reserves");
	VehPickState_ApplyDamage(human, VEH_PICK_DAMAGE_SPIN, NULL, 0, true);
	CTR_TEST_CHECK_EQ(human->reserves, 2500, "the actual damage path cannot clear frozen reserves");
	struct Driver bot = {.driverID = 2, .reserves = 2000, .actionsFlagSet = ACTION_BOT};
	const struct Scrub botSurface = VehAfterColl_GetSurface(COLL_SCRUB_SOLID, &bot);
	CTR_TEST_CHECK_EQ(botSurface.flags, originalFlags, "bot wall contacts retain their normal surface flags");
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(&bot, 0), 0, "bot fuel remains ordinary");
	const struct ChaosRuntime snapshot = sdata->chaosRuntime;
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 0), 0, "expiry restores reserve losses");
	const struct Scrub expiredSurface = VehAfterColl_GetSurface(COLL_SCRUB_SOLID, human);
	CTR_TEST_CHECK_EQ(expiredSurface.flags, originalFlags, "expiry restores ordinary wall-contact flags");
	sdata->chaosRuntime = snapshot;
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 0), 2500, "resident restore restores reserve protection");
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL), "No Reserves replaces reserve protection");
	Chaos_ApplyReserveRules(human);
	CTR_TEST_CHECK_EQ(human->reserves, 0, "the replacement rule clears fuel immediately when applied");
	Chaos_Activate(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_RESERVES), "reserve protection replaces No Reserves in the opposite order");
}

static void TestArenaWarpball(void)
{
	CTR_TEST_SECTION("orb launch and teardown do not require race checkpoints in crystal arenas");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
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
	TestCataloguePublicationLifecycle();
	TestExternalCommands();
	TestIntervalCadence();
	TestIndependentExpiryAndOrder();
	TestGatesAndPause();
	TestCapacityAndValidation();
	TestResetAndInvalidSettings();
	TestPickupTrigger();
	TestResidentRestore();
	TestRandomTrigger();
	TestSteeringAndDriverScope();
	TestWobbleSteering();
	TestGravityEffects();
	TestSpeedEffects();
	TestNoReserves();
	TestDefectiveBoosts();
	TestNoItems();
	TestWumpaDrain();
	TestClockCurse();
	TestExplosiveCrates();
	TestHardCrates();
	TestGiantKarts();
	TestAnxiousMask();
	TestHopInputAndCloudOwnership();
	TestDriverApplication();
	TestActivationNotice();
	TestActiveEffectList();
	TestHudVisibilityAndDockLimit();
	TestHudLayeringAndSoloStack();
	TestBannerBaselineWithDockedEffects();
	TestNotificationRows();
	TestInstantGifts();
	TestRandomItemGift();
	TestReserveGifts();
	TestArenaWarpball();
	return CtrTest_Summary();
}
