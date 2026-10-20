#include <shared/ctr_test.h>
#include <common.h>

struct sData sdata_static;
NativeConfig g_config;
static struct GameTracker s_tracker;
static bool s_teleported;
static int s_noticeDraws;
static const char *s_noticeText;
static s16 s_noticeX;
static s16 s_noticeY;
static s16 s_noticeFont;
static u16 s_noticeFlags;
static u32 *s_noticeOT;

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
	(void)font;
	return (s32)strlen(text) * 8;
}

void DecalFont_DrawLineOT(char *text, s16 x, s16 y, s16 font, s16 flags, u32 *ot)
{
	s_noticeDraws++;
	s_noticeText = text;
	s_noticeX = x;
	s_noticeY = y;
	s_noticeFont = font;
	s_noticeFlags = (u16)flags;
	s_noticeOT = ot;
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
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.noticeRemainingMS, 3000, "Random activation also starts the visual announcement");
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
		CTR_TEST_CHECK_EQ(sdata->chaosRuntime.noticeRemainingMS, 3000, "each elapsed Random gap produces a fresh activation");
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
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.noticeRemainingMS, 2680, "pause freezes announcement time together with effect timers");
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
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.noticeRemainingMS, 3000, "a later activation restarts the announcement lifetime");
	Tick(320);
	DrawNotice();
	CTR_TEST_CHECK(strcmp(s_noticeText, "No Hopping") == 0, "the newest activation replaces the displayed name");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 2, "replacing the announcement preserves both running effects");
	const struct ChaosRuntime beforeReject = sdata->chaosRuntime;
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 1000);
	CTR_TEST_CHECK(memcmp(&beforeReject, &sdata->chaosRuntime, sizeof(beforeReject)) == 0, "a rejected activation cannot replace the current announcement");
	Tick(2500);
	DrawNotice();
	CTR_TEST_CHECK(s_noticeX < SCREEN_WIDTH / 2, "the name slides left during its exit");
	Tick(180);
	DrawNotice();
	CTR_TEST_CHECK_EQ(s_noticeDraws, 0, "the notice disappears after three seconds of gameplay");
	CTR_TEST_CHECK(Chaos_IsActive(CHAOS_EFFECT_NO_HOPPING), "notification expiry does not expire the effect itself");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.noticeRemainingMS, 0, "race reset also clears pending announcement state");
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
	return CtrTest_Summary();
}
