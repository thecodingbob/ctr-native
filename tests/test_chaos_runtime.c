#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
NativeConfig g_config;
static struct GameTracker s_tracker;
static bool s_teleported;
static bool s_allHumansFinished;
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

#include <game/231/RB_Chaos.c>
#include <game/MAIN/MainGameStart.c>
#include <game/Vehicle/VehPhysProc_Input.c>
#include <game/UI/UI_Lerp2D.c>
#include <game/UI/UI_Chaos.c>

// Only the unused angular helper needs this dependency; the banner uses real linear interpolation.
int MATH_Sin(u32 angle)
{
	(void)angle;
	CtrTest_Bail("the Chaos banner must not use angular HUD interpolation");
	return 0;
}

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
void howl_StopAudio(b32 backup, b32 music, b32 fx)
{
	(void)backup;
	(void)music;
	(void)fx;
}

static void SetupRace(int trigger)
{
	s_allHumansFinished = false;
	memset(&sdata_static, 0, sizeof(sdata_static));
	memset(&s_tracker, 0, sizeof(s_tracker));
	memset(&g_config, 0, sizeof(g_config));
	sdata->gGT = &s_tracker;
	sdata->Loading.stage = LOAD_IDLE;
	s_tracker.gameMode1 = ADVENTURE_MODE;
	s_tracker.overlayIndex_Threads = OVERLAY_INDEX_RACING_OR_BATTLE;
	s_tracker.numPlyrCurrGame = 2;
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
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "Interval does not wait for the earlier effect to expire");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 25024, "the original timer keeps its own age");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].remainingMS, 30000, "repeated selection appends a distinct timer");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "the second boundary returns to the original cadence");
}

static void TestIndependentExpiryAndOrder(void)
{
	CTR_TEST_SECTION("expiry preserves activation order and duplicate lifetimes");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 200), "the first selected effect can activate");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 100), "an unrelated later effect can overlap");
	Tick(50);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 300), "a repeated activation gets another entry");
	Tick(50);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "only the entry reaching its exact expiry is removed");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 100, "compaction keeps the oldest surviving timer");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[1].remainingMS, 250, "compaction preserves the later duplicate and its timer");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SUPER_SPEED), "an expired effect no longer reports active");
	Tick(100);
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_LOW_GRAVITY), "expiry of one duplicate does not stop the remaining activation");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.active[0].remainingMS, 150, "the surviving duplicate is not refreshed or shortened");
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
	CTR_TEST_SECTION("capacity rejects new entries without eviction, refresh or RNG consumption");
	SetupRace(CHAOS_TRIGGER_INTERVAL);
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NONE, 100), "the none sentinel cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_COUNT, 100), "out-of-range effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 100), "disabled catalogue effects cannot activate");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 0), "zero duration cannot create an entry");
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, -1), "negative duration cannot create an entry");
	for (int i = 0; i < CHAOS_MAX_ACTIVATIONS; i++)
	{
		CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 5000 + i), "each available slot accepts an independent activation");
	}
	const struct ChaosRuntime full = sdata->chaosRuntime;
	CTR_TEST_CHECK(!Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 9999), "a full queue rejects a new request");
	CTR_TEST_CHECK(memcmp(&full, &sdata->chaosRuntime, sizeof(full)) == 0, "overflow does not evict or refresh existing activations");
	sdata->chaosRuntime.intervalRemainingMS = 1;
	const u32 rng = sdata->chaosRng;
	Tick(1);
	CTR_TEST_CHECK_EQ(sdata->chaosRng, rng, "an Interval event rejected at capacity does not consume a random choice");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.intervalRemainingMS, 5000, "a rejected Interval event still consumes its scheduled gap");
	Tick(4999);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, CHAOS_MAX_ACTIVATIONS - 1, "expiring an entry makes room without disturbing the others");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 1000), "a freed slot can accept a later activation");
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
		Tick(gap);
		CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "each elapsed Random gap produces a fresh activation");
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
	CTR_TEST_CHECK_EQ(LatestNoticeRemaining(), CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS, "a later activation starts its own announcement lifetime");
	Tick(320);
	DrawNotice();
	CTR_TEST_CHECK(strcmp(s_noticeText, "No Hopping") == 0, "the newest activation replaces the displayed name");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "replacing the announcement preserves both running effects");
	const struct ChaosRuntime beforeReject = sdata->chaosRuntime;
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 1000);
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
		Chaos_Activate((ChaosEffectId)i, 30000);
	}
	s_tracker.numPlyrCurrGame = 2;
	s_tracker.pushBuffer[0].rect = (RECT){0, 0, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	s_tracker.pushBuffer[1].rect = (RECT){0, SCREEN_HEIGHT / 2, SCREEN_WIDTH, SCREEN_HEIGHT / 2};
	sdata->finalLapTextTimer[0] = 30;
	sdata->finalLapTextTimer[1] = 30;
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, CHAOS_EFFECT_COUNT, "a full set of simultaneous announcements remains visible");
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
	CTR_TEST_CHECK(s_noticeY + UI_CHAOS_LIST_ROW_HEIGHT <= SCREEN_HEIGHT, "the full catalogue fits within the screen height");
	Chaos_Reset();
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 100);
	Tick(100);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "a short-lived effect cannot leave a banner or table row after expiry");
}

static void TestHudVisibilityAndDockLimit(void)
{
	CTR_TEST_SECTION("1P and 2P show only the four latest docked effects and hide the HUD when racing ends");
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
	for (int effect = 0; effect < 5; effect++)
	{
		Chaos_Activate((ChaosEffectId)effect, 30000);
	}
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 30000);
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 4, "2P limits the docked list to four distinct effects");
	CTR_TEST_CHECK(strcmp(s_drawText[0], "Inverted Steering") == 0, "reactivation moves an existing effect to the newest row");
	CTR_TEST_CHECK(strcmp(s_drawText[1], "Heavy Karts") == 0 && strcmp(s_drawText[3], "Wobble Steering") == 0,
	               "the docked list keeps the latest activations rather than the first catalogue entries");
	CTR_TEST_CHECK_EQ(s_drawY[0], UI_CHAOS_LIST_MULTIPLAYER_TOP, "the 2P docked list starts near the top edge");
	const int firstPlayerLapCounterY = 71;
	CTR_TEST_CHECK(s_drawY[3] + UI_CHAOS_LIST_ROW_HEIGHT < firstPlayerLapCounterY,
	               "all four 2P docked rows stay above the lap counter");
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 30000);
	Tick(UI_CHAOS_NOTICE_SLIDE_MS);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 5, "a central announcement can coexist with four docked effects");
	CTR_TEST_CHECK_EQ(s_noticeX, SCREEN_WIDTH / 2, "the new 2P announcement is horizontally centered");
	CTR_TEST_CHECK_EQ(s_noticeY, UI_CHAOS_NOTICE_MULTIPLAYER_Y, "four docked effects cannot push a new 2P announcement toward the bottom");
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
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 7, "the display limit does not remove any running activation");
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
	for (int effect = 0; effect < UI_CHAOS_MAX_DOCKED; effect++)
	{
		Chaos_Activate((ChaosEffectId)effect, 30000);
	}
	Tick(CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS);
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 30000);
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
	                  "four docked rows do not shift the shared 2P center");
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

int main(void)
{
	TestIntervalCadence();
	TestIndependentExpiryAndOrder();
	TestGatesAndPause();
	TestCapacityAndValidation();
	TestResetAndInvalidSettings();
	TestResidentRestore();
	TestRandomTrigger();
	TestSteeringAndDriverScope();
	TestHopInputAndCloudOwnership();
	TestDriverApplication();
	TestActivationNotice();
	TestActiveEffectList();
	TestHudVisibilityAndDockLimit();
	TestHudLayeringAndSoloStack();
	TestBannerBaselineWithDockedEffects();
	TestNotificationRows();
	return CtrTest_Summary();
}
