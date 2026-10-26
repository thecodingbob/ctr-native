#include <shared/chaos_fixture.h>
#include <game/Vehicle/VehPhysProc_Input.c>

static u32 PreparedDriver(struct Driver *driver, const struct GamepadBuffer *pad, int *steering, u32 actions)
{
	struct GamepadBuffer drivingPad = *pad;
	Chaos_ApplyInputRules(driver, &drivingPad);
	*steering = drivingPad.stickLX;
	return Chaos_ApplyDriver(driver, actions);
}

static int PreparedSteering(struct Driver *driver, const struct GamepadBuffer *pad)
{
	int steering = pad->stickLX;
	PreparedDriver(driver, pad, &steering, driver->actionsFlagSet);
	return steering;
}

static u32 PreparedJump(struct Driver *driver, u32 tapped, u32 held, int rainCloudEffect, u32 actions)
{
	struct GamepadBuffer pad = {.buttonsTapped = tapped, .buttonsHeldCurrFrame = held};
	driver->actionsFlagSet = actions;
	Chaos_ApplyInputRules(driver, &pad);
	actions = VehPhysProc_JumpInput(driver, pad.buttonsTapped, pad.buttonsHeldCurrFrame, rainCloudEffect, actions);
	return Chaos_ApplyDriver(driver, actions);
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

static void TestSabotage(void)
{
	CTR_TEST_SECTION("Sabotage filters local brake inputs and preserves controller state");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	sdata->chaosSettings.effects[CHAOS_EFFECT_SABOTAGE] = true;
	sdata->chaosSettings.enabledCount++;
	struct Driver driver = {.driverID = 0};
	const struct GamepadBuffer pad = {
		.buttonsHeldCurrFrame = BTN_SQUARE_one | BTN_CROSS_one,
		.buttonsTapped = BTN_SQUARE_one,
		.buttonsHeldPrevFrame = BTN_SQUARE_one,
		.stickLX = CHAOS_STICK_CENTER,
		.stickRY = 255,
		.stickLY = 255,
	};
	const struct GamepadBuffer original = pad;
	struct GamepadBuffer drivingPad = pad;
	Chaos_ApplyInputRules(&driver, &drivingPad);
	CTR_TEST_CHECK(memcmp(&drivingPad, &pad, sizeof(pad)) == 0, "inactive Sabotage preserves braking and reverse");
	Chaos_Activate(CHAOS_EFFECT_SABOTAGE, 1000);
	Chaos_ApplyInputRules(&driver, &drivingPad);
	CTR_TEST_CHECK((drivingPad.buttonsHeldCurrFrame & BTN_SQUARE_one) == 0 && drivingPad.stickRY == CHAOS_STICK_CENTER && drivingPad.stickLY == CHAOS_STICK_CENTER, "active Sabotage neutralizes digital and analog brake inputs");
	CTR_TEST_CHECK(memcmp(&pad, &original, sizeof(pad)) == 0, "filtering leaves shared controller input and button history intact");
	drivingPad.stickRY = drivingPad.stickLY = 0;
	Chaos_ApplyInputRules(&driver, &drivingPad);
	CTR_TEST_CHECK(drivingPad.stickRY == 0 && drivingPad.stickLY == 0, "forward analog input remains available");
	Tick(1000);
	drivingPad = pad;
	Chaos_ApplyInputRules(&driver, &drivingPad);
	CTR_TEST_CHECK(memcmp(&drivingPad, &pad, sizeof(pad)) == 0, "expiry restores ordinary braking and reverse");
}

static void TestHydraulics(void)
{
	CTR_TEST_SECTION("Hydraulics renews normal hop requests without bypassing physics or synthesizing shoulder input");
	EnableInputEffects();
	sdata->chaosSettings.trigger = CHAOS_TRIGGER_EXTERNAL;
	sdata->chaosSettings.effects[CHAOS_EFFECT_HYDRAULICS] = true;
	sdata->chaosSettings.enabledCount++;
	struct Driver human = {.driverID = 0, .kartState = KS_NORMAL, .const_JumpForce = 800};
	struct Driver partner = {.driverID = 1, .kartState = KS_NORMAL};
	struct Driver bot = {.driverID = 2, .kartState = KS_NORMAL, .actionsFlagSet = ACTION_BOT};
	PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_NONE, ACTION_TOUCH_GROUND);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 0, "inactive Hydraulics does not request a hop");
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 5000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_HYDRAULICS, 1000), "Hydraulics activates as a timed effect");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_HOPPING), "Hydraulics replaces No Hopping");
	const u8 states[] = {KS_NORMAL, KS_DRIFTING, KS_ANTIVSHIFT};
	for (size_t i = 0; i < len(states); i++)
	{
		human.kartState = states[i];
		const u32 actions = PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_NONE, ACTION_TOUCH_GROUND);
		CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 1, "normal driving, drifting and V-shift recovery receive a current-step hop request");
		CTR_TEST_CHECK_EQ(actions, ACTION_TOUCH_GROUND, "automatic hopping does not invent shoulder-held input or start a jump directly");
	}
	human.kartState = KS_NORMAL;
	human.jump_CooldownMS = 100;
	human.jump_CoyoteTimerMS = 0;
	human.jump_ForcedMS = 50;
	human.forcedJumpType = FORCED_JUMP_HIGH;
	PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_NONE, 0);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 1, "a request stays ready for the normal landing and cooldown checks later in the physics step");
	CTR_TEST_CHECK(human.jump_CooldownMS == 100 && human.jump_CoyoteTimerMS == 0 && human.jump_ForcedMS == 50,
	               "Hydraulics preserves normal jump timing and airborne restrictions");
	CTR_TEST_CHECK(human.const_JumpForce == 800 && human.forcedJumpType == FORCED_JUMP_HIGH,
	               "normal force and externally forced jumps are left to the existing physics");
	PreparedJump(&human, BTN_R1, BTN_R1, RAIN_CLOUD_EFFECT_NONE, 0);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, VEH_PHYS_PROC_JUMP_BUFFER_FRAMES, "a genuine shoulder press keeps its full normal buffer");
	human.rainCloudEffect = RAIN_CLOUD_EFFECT_JUMP_LOCKOUT;
	PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_JUMP_LOCKOUT, 0);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 0, "rain-cloud jump lockout prevents automatic hop requests");
	human.rainCloudEffect = RAIN_CLOUD_EFFECT_NONE;
	human.kartState = KS_SPINNING;
	PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_NONE, 0);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 0, "damage states do not receive automatic hop requests");
	PreparedJump(&bot, 0, 0, RAIN_CLOUD_EFFECT_NONE, ACTION_BOT);
	CTR_TEST_CHECK_EQ(bot.jump_TenBuffer, 0, "bots retain ordinary jump input");
	PreparedJump(&partner, 0, 0, RAIN_CLOUD_EFFECT_NONE, 0);
	CTR_TEST_CHECK_EQ(partner.jump_TenBuffer, 1, "the second human also receives automatic hop requests");
	partner.actionsFlagSet = ACTION_RACE_FINISHED;
	PreparedJump(&partner, 0, 0, RAIN_CLOUD_EFFECT_NONE, ACTION_RACE_FINISHED);
	CTR_TEST_CHECK_EQ(partner.jump_TenBuffer, 0, "finished humans stop receiving automatic hop requests");
	Tick(1000);
	human.kartState = KS_NORMAL;
	PreparedJump(&human, 0, 0, RAIN_CLOUD_EFFECT_NONE, 0);
	CTR_TEST_CHECK_EQ(human.jump_TenBuffer, 0, "expiry leaves no automatic input held or buffered");
	Chaos_Activate(CHAOS_EFFECT_HYDRAULICS, 5000);
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_HYDRAULICS), "No Hopping also replaces Hydraulics");
}

static void TestMyPlanetNeedsMe(void)
{
	CTR_TEST_SECTION("My Planet Needs Me launches once, including in midair, without a timed slot");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	memset(sdata->chaosSettings.effects, true, sizeof(sdata->chaosSettings.effects));
	sdata->chaosSettings.enabledCount = CHAOS_EFFECT_COUNT;
	data.trigApprox[0] = (struct TrigTable){.sin = 0, .cos = FP(1.0)};
	struct Driver human = {
		.driverID = 0, .speed = 3000, .const_JumpForce = 4596, .const_Gravity = 1000,
		.actionsFlagSet = ACTION_TOUCH_GROUND, .reserves = 500, .heldItemID = HELD_ITEM_POTION,
		.jump_CooldownMS = 100, .jump_CoyoteTimerMS = 0,
	};
	struct Driver partner = {
		.driverID = 1, .speed = 1000, .axisRotationY = -ANG_HALF_PI,
		.const_JumpForce = 900, .actionsFlagSet = ACTION_AIRBORNE,
	};
	struct Driver bot = human;
	bot.driverID = 2;
	bot.actionsFlagSet = ACTION_BOT;
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	s_tracker.drivers[2] = &bot;
	Chaos_Activate(CHAOS_EFFECT_NO_HOPPING, 5000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_MY_PLANET_NEEDS_ME, 1000), "the immediate launch activates without normal jump eligibility");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_HOPPING), "the launch replaces No Hopping");
	CTR_TEST_CHECK_EQ(s_launchConversions[0], 1, "activation commits one launch to the ground driver's canonical speed representation");
	CTR_TEST_CHECK_EQ(s_launchVelocity[0].y, 27576, "a grounded driver receives six times the real normal jump force upward");
	CTR_TEST_CHECK(s_launchVelocity[0].x == 0 && s_launchVelocity[0].z == 3000, "launch preserves horizontal momentum");
	CTR_TEST_CHECK_EQ(s_launchVelocity[1].y, 4400, "the airborne driver receives the impulse on top of existing falling momentum");
	CTR_TEST_CHECK_EQ(partner.velocity.y, 4400, "the working velocity also reflects the immediate impulse");
	CTR_TEST_CHECK_EQ(s_launchConversions[2], 0, "bots do not receive the launch");
	CTR_TEST_CHECK(human.reserves == 500 && human.heldItemID == HELD_ITEM_POTION, "launch preserves inventory and boost reserves");
	CTR_TEST_CHECK(human.jump_CooldownMS == 100 && human.jump_CoyoteTimerMS == 0, "the one-off impulse does not rewrite normal hop eligibility");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "the launch occupies no timed slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 1, "the launch creates an instant announcement");
	Tick(100);
	CTR_TEST_CHECK_EQ(s_launchConversions[0], 1, "later Chaos updates do not repeat the launch");
	Chaos_Activate(CHAOS_EFFECT_HYDRAULICS, 5000);
	Chaos_Activate(CHAOS_EFFECT_MY_PLANET_NEEDS_ME, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_HYDRAULICS), "the launch also replaces Hydraulics");
	CTR_TEST_CHECK_EQ(s_launchConversions[0], 2, "an explicit second activation applies one new impulse");
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 5000);
	Chaos_Activate(CHAOS_EFFECT_INVERTED_STEERING, 5000);
	Chaos_Activate(CHAOS_EFFECT_SPEED_CAPPED, 5000);
	Chaos_Activate(CHAOS_EFFECT_NO_ITEMS, 5000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_MY_PLANET_NEEDS_ME, 1000), "launch still works at full timed capacity");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, CHAOS_MAX_ACTIVATIONS, "launch preserves unrelated timed effects");
	CTR_TEST_CHECK_EQ(s_launchVelocity[0].y, 27576, "Low Gravity does not change the launch impulse");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 250, "Low Gravity continues to govern the flight");
	Chaos_Activate(CHAOS_EFFECT_HEAVY_KARTS, 5000);
	Chaos_Activate(CHAOS_EFFECT_MY_PLANET_NEEDS_ME, 1000);
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 3000, "Heavy Karts also remains effective after launch");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	const int launches = s_launchConversions[0];
	Chaos_Activate(CHAOS_EFFECT_MY_PLANET_NEEDS_ME, 1000);
	CTR_TEST_CHECK_EQ(s_launchConversions[0], launches, "finished humans receive no new launch");
}

static void TestLaunchSpeedLimit(void)
{
	CTR_TEST_SECTION("launches fit canonical speed without reversing at high speed or on repeated activation");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	for (int i = 0; i < ANG_HALF_PI; i++)
	{
		const double angle = i * atan2(1.0, 0.0) / ANG_HALF_PI;
		data.trigApprox[i] = (struct TrigTable){
		    .sin = (s16)lround(sin(angle) * FP(1.0)),
		    .cos = (s16)lround(cos(angle) * FP(1.0)),
		};
	}
	s_convertLaunchSpeed = true;
	const int speeds[] = {0, 3000, 25600, INT16_MAX, INT16_MIN};
	const int pitches[] = {0, ANG_HALF_PI / 2, -ANG_HALF_PI / 2};
	const int forces[] = {4596, 8000};
	for (int speed = 0; speed < (int)len(speeds); speed++)
	{
		for (int heading = 0; heading < ANG_TWO_PI; heading += ANG_HALF_PI / 2)
		{
			for (int pitch = 0; pitch < (int)len(pitches); pitch++)
			{
				for (int force = 0; force < (int)len(forces); force++)
				{
					struct Driver driver = {
					    .speed = speeds[speed], .axisRotationX = heading, .axisRotationY = pitches[pitch],
					    .const_JumpForce = forces[force],
					};
					driver.matrixMovingDir.m[0][0] = FP(1.0);
					driver.matrixMovingDir.m[1][1] = FP(1.0);
					driver.matrixMovingDir.m[2][2] = FP(1.0);
					for (int repeat = 0; repeat < 3; repeat++)
					{
						Vec3 before;
						VehPhysForce_ConvertSpeedToVec(&driver, &before);
						Chaos_LaunchDriver(&driver);
						const Vec3 launch = driver.velocity;
						const s64 limitSquared = (s64)INT16_MAX * INT16_MAX;
						const s64 launchSquared = (s64)launch.x * launch.x + (s64)launch.y * launch.y + (s64)launch.z * launch.z;
						CTR_TEST_CHECK(launchSquared <= limitSquared && driver.speed >= 0,
						    "the full launch vector remains representable as positive signed-16-bit speed");
						if ((s64)before.x * before.x + (s64)before.z * before.z <= limitSquared)
						{
							CTR_TEST_CHECK(launch.x == before.x && launch.z == before.z,
							    "the launch preserves representable horizontal momentum exactly");
						}
						Vec3 restored;
						VehPhysForce_ConvertSpeedToVec(&driver, &restored);
						CTR_TEST_CHECK((s64)restored.x * launch.x + (s64)restored.z * launch.z >= 0,
						    "reconstructing canonical motion never reverses horizontal direction");
						CTR_TEST_CHECK((s64)restored.y * launch.y >= 0,
						    "reconstructing canonical motion preserves the vertical direction");
					}
				}
			}
		}
	}
	s_convertLaunchSpeed = false;
}

static struct GamepadBuffer ShuffledPad(const struct Driver *driver, const struct GamepadBuffer *raw)
{
	struct GamepadBuffer pad = *raw;
	Chaos_ApplyInputRules(driver, &pad);
	return pad;
}

static void TestWhoBoundThis(void)
{
	CTR_TEST_SECTION("Who Bound This rolls independent stable nonidentity permutations for human driving");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_WHO_BOUND_THIS] = true;
	sdata->chaosSettings.enabledCount++;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_WHO_BOUND_THIS, 1000), "the control shuffle activates as a timed effect");
	for (int player = 0; player < 2; player++)
	{
		unsigned int seen = 0;
		bool changed = false;
		for (int action = 0; action < CHAOS_CONTROL_COUNT; action++)
		{
			const int destination = sdata->chaosRuntime.controls[player].destination[action];
			seen |= 1u << destination;
			changed |= destination != action;
		}
		CTR_TEST_CHECK_EQ(seen, (1u << CHAOS_CONTROL_COUNT) - 1, "each human receives every driving action exactly once");
		CTR_TEST_CHECK(changed, "a complete identity permutation is rerolled");
	}
	struct Driver human = {.driverID = 0};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	// Cross -> item, Square -> R1, Circle -> L1, L1 -> throttle, R1 -> brake.
	sdata->chaosRuntime.controls[0].destination[CHAOS_CONTROL_THROTTLE] = CHAOS_CONTROL_ITEM;
	sdata->chaosRuntime.controls[0].destination[CHAOS_CONTROL_BRAKE] = CHAOS_CONTROL_HOP_RIGHT;
	sdata->chaosRuntime.controls[0].destination[CHAOS_CONTROL_ITEM] = CHAOS_CONTROL_HOP_LEFT;
	sdata->chaosRuntime.controls[0].destination[CHAOS_CONTROL_HOP_LEFT] = CHAOS_CONTROL_THROTTLE;
	sdata->chaosRuntime.controls[0].destination[CHAOS_CONTROL_HOP_RIGHT] = CHAOS_CONTROL_BRAKE;
	sdata->chaosRuntime.controls[1].destination[CHAOS_CONTROL_THROTTLE] = CHAOS_CONTROL_BRAKE;
	sdata->chaosRuntime.controls[1].destination[CHAOS_CONTROL_BRAKE] = CHAOS_CONTROL_ITEM;
	sdata->chaosRuntime.controls[1].destination[CHAOS_CONTROL_ITEM] = CHAOS_CONTROL_THROTTLE;
	sdata->chaosRuntime.controls[1].destination[CHAOS_CONTROL_HOP_LEFT] = CHAOS_CONTROL_HOP_RIGHT;
	sdata->chaosRuntime.controls[1].destination[CHAOS_CONTROL_HOP_RIGHT] = CHAOS_CONTROL_HOP_LEFT;
	const u32 inputs[] = {BTN_CROSS_one, BTN_SQUARE_one, BTN_CIRCLE, BTN_L1, BTN_R1};
	const u32 outputs[] = {BTN_CIRCLE, BTN_R1, BTN_L1, BTN_CROSS_one, BTN_SQUARE_one};
	const u32 untouched = BTN_START | BTN_SELECT | BTN_LEFT | BTN_R2;
	for (size_t i = 0; i < len(inputs); i++)
	{
		const struct GamepadBuffer raw = {
			.stickLX = 42, .stickLY = CHAOS_STICK_CENTER, .stickRY = CHAOS_STICK_CENTER,
			.buttonsHeldCurrFrame = inputs[i] | untouched,
			.buttonsTapped = inputs[i] | BTN_START,
		};
		struct GamepadBuffer pad = ShuffledPad(&human, &raw);
		CTR_TEST_CHECK_EQ((u32)pad.buttonsHeldCurrFrame, outputs[i] | untouched, "only the five gameplay buttons are permuted");
		CTR_TEST_CHECK_EQ((u32)pad.buttonsTapped, outputs[i] | BTN_START, "tap edges follow the mapped action without changing pause");
		CTR_TEST_CHECK_EQ(pad.stickLX, 42, "left-stick steering is unchanged");
		struct GamepadBuffer released = raw;
		released.buttonsHeldPrevFrame = inputs[i];
		released.buttonsHeldCurrFrame = 0;
		released.buttonsTapped = 0;
		released.buttonsReleased = inputs[i];
		pad = ShuffledPad(&human, &released);
		CTR_TEST_CHECK(pad.buttonsReleased == (int)outputs[i] && pad.buttonsHeldPrevFrame == (int)outputs[i],
		               "release edges and previous held state use the same mapping");
	}
	const struct GamepadBuffer raw = {
		.stickLX = 42, .stickLY = CHAOS_STICK_CENTER, .stickRY = CHAOS_STICK_CENTER,
		.buttonsHeldCurrFrame = BTN_CROSS_one, .buttonsTapped = BTN_CROSS_one,
	};
	const struct GamepadBuffer original = raw;
	struct GamepadBuffer pad = ShuffledPad(&partner, &raw);
	CTR_TEST_CHECK_EQ(pad.buttonsHeldCurrFrame, BTN_SQUARE_one, "player two uses their own permutation");
	pad = ShuffledPad(&bot, &raw);
	CTR_TEST_CHECK(memcmp(&pad, &raw, sizeof(pad)) == 0, "bots retain ordinary controls");
	human.actionsFlagSet = ACTION_RACE_FINISHED;
	pad = ShuffledPad(&human, &raw);
	CTR_TEST_CHECK(memcmp(&pad, &raw, sizeof(pad)) == 0, "finished humans retain ordinary controls");
	human.actionsFlagSet = 0;
	s_tracker.gameMode1 |= PAUSE_1;
	pad = ShuffledPad(&human, &raw);
	CTR_TEST_CHECK(memcmp(&pad, &raw, sizeof(pad)) == 0, "paused menus receive unmodified input");
	s_tracker.gameMode1 &= ~PAUSE_1;
	const u32 rng = sdata->chaosRng;
	pad = ShuffledPad(&human, &raw);
	const struct GamepadBuffer again = ShuffledPad(&human, &raw);
	CTR_TEST_CHECK(memcmp(&pad, &again, sizeof(pad)) == 0 && sdata->chaosRng == rng,
	               "physics and drift consumers can resolve the same frame without reshuffling or double mapping");
	CTR_TEST_CHECK(memcmp(&raw, &original, sizeof(raw)) == 0, "the shared raw pad remains intact");
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	Chaos_Activate(CHAOS_EFFECT_WHO_BOUND_THIS, 1000);
	CTR_TEST_CHECK(sdata->chaosRng != rng, "reactivation rolls a fresh set of per-human mappings");
	sdata->chaosRuntime = saved;
	pad = ShuffledPad(&human, &raw);
	CTR_TEST_CHECK_EQ(pad.buttonsHeldCurrFrame, BTN_CIRCLE, "resident restoration restores the exact previous mapping");

	CTR_TEST_SECTION("analog throttle and reverse follow the shuffle with coherent press and release edges");
	s_gamepads.gamepad[0].stickRY = 0;
	Tick(32);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK(pad.buttonsHeldCurrFrame == BTN_CIRCLE && pad.buttonsTapped == BTN_CIRCLE,
	               "analog throttle fires its mapped action on entry");
	CTR_TEST_CHECK_EQ(pad.stickRY, CHAOS_STICK_CENTER, "the original analog throttle path is neutralized");
	Tick(32);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK(pad.buttonsHeldCurrFrame == BTN_CIRCLE && pad.buttonsTapped == 0, "holding an axis does not repeat item or hop taps");
	s_gamepads.gamepad[0].buttonsHeldCurrFrame = BTN_CROSS_one;
	Tick(32);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK_EQ(pad.buttonsTapped, 0, "pressing a digital source already held by its analog axis does not duplicate the action");
	s_gamepads.gamepad[0].buttonsHeldPrevFrame = BTN_CROSS_one;
	s_gamepads.gamepad[0].stickRY = CHAOS_STICK_CENTER;
	Tick(32);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK_EQ(pad.buttonsReleased, 0, "centering the axis does not release an action still held digitally");
	s_gamepads.gamepad[0].buttonsHeldCurrFrame = 0;
	s_gamepads.gamepad[0].buttonsHeldPrevFrame = 0;
	s_gamepads.gamepad[0].stickLY = 255;
	Tick(32);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK((pad.buttonsHeldCurrFrame & BTN_R1) != 0 && pad.stickLY == CHAOS_STICK_CENTER,
	               "left-stick reverse triggers the remapped brake action without an unmapped reverse bypass");
	Tick(1000);
	pad = ShuffledPad(&human, &s_gamepads.gamepad[0]);
	CTR_TEST_CHECK(memcmp(&pad, &s_gamepads.gamepad[0], sizeof(pad)) == 0, "expiry immediately returns the complete live pad");
}

static void TestTrySomethingNew(void)
{
	CTR_TEST_SECTION("Try Something New changes class constants until race end without changing character or race state");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_TRY_SOMETHING_NEW] = true;
	sdata->chaosSettings.enabledCount++;
	data.characterIDs[0] = CRASH_BANDICOOT;
	data.characterIDs[1] = COCO_BANDICOOT;
	data.MetaDataCharacters[CRASH_BANDICOOT].engineID = BALANCED;
	data.MetaDataCharacters[COCO_BANDICOOT].engineID = ACCEL;
	const struct MetaDataCHAR crashMetadata = data.MetaDataCharacters[CRASH_BANDICOOT];
	const struct MetaDataCHAR cocoMetadata = data.MetaDataCharacters[COCO_BANDICOOT];
	data.metaPhys[0] = (struct MetaPhys){.offset = SPEED_CLASS_STAT_OFFSET, .size = 2, .value = {10000, 12000, 14000, 8000}};
	data.metaPhys[1] = (struct MetaPhys){.offset = TURN_RATE_OFFSET, .size = 1, .value = {28, 26, 24, 48}};
	data.metaPhys[2] = (struct MetaPhys){.offset = GRAVITY_OFFSET, .size = 2, .value = {900, 900, 900, 900}};
	data.metaPhys[3] = (struct MetaPhys){.offset = JUMP_OFFSET, .size = 2, .value = {4596, 4596, 4596, 4596}};
	data.metaPhys[4] = (struct MetaPhys){.offset = TURBO_FULL_BAR_RESERVE_GAIN_OFFSET, .size = 1, .value = {10, 20, 30, 40}};
	g_config.speedMultiplier = 150;
	g_config.turnMultiplier = 100;
	g_config.gravityMultiplier = 200;
	g_config.jumpMultiplier = 50;
	g_config.reserveMultiplier = 100;
	struct Driver human = {
		.driverID = 0, .reserves = 500, .fireSpeedCap = 4096, .heldItemID = HELD_ITEM_POTION,
		.posCurr = {10, 20, 30}, .speed = 10000,
	};
	struct Driver partner = {.driverID = 1};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .const_Speed_ClassStat = 1234};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	s_tracker.drivers[2] = &bot;
	VehBirth_SetConsts(&human);
	VehBirth_SetConsts(&partner);
	CTR_TEST_CHECK_EQ(human.const_Speed_ClassStat, 15000, "ordinary vehicle birth retains character-based class and configured multipliers");
	Chaos_Activate(CHAOS_EFFECT_LOW_GRAVITY, 10000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_TRY_SOMETHING_NEW, 1000), "the class change activates as a one-off effect");
	int current = sdata->chaosRuntime.engineClass[0];
	CTR_TEST_CHECK_EQ(s_engineStopped[0], BALANCED * 4, "the old engine loop stops at the class change");
	CTR_TEST_CHECK_EQ(s_engineStarted[0], (u32)(current * 4), "the new class engine loop starts immediately");
	CTR_TEST_CHECK_EQ(Chaos_EngineClassForDriver(&human, BALANCED), current, "ongoing engine updates resolve the selected class");
	CTR_TEST_CHECK_EQ(s_engineStarted[1], (u32)(sdata->chaosRuntime.engineClass[1] * 4 + 1), "each human uses their own engine audio slot");
	struct Driver ghost = {.driverID = 0};
	CTR_TEST_CHECK_EQ(Chaos_EngineClassForDriver(&ghost, BALANCED), BALANCED, "a ghost sharing a driver ID retains its own engine class");
	CTR_TEST_CHECK(current != BALANCED && current < NUM_CLASSES, "the first class change excludes the character's current standard class");
	CTR_TEST_CHECK(sdata->chaosRuntime.engineClass[1] != ACCEL && sdata->chaosRuntime.engineClassChanged[1],
	               "the second human receives a class different from their own baseline");
	CTR_TEST_CHECK_EQ(human.const_Speed_ClassStat, data.metaPhys[0].value[current] * 150 / 100, "new class speed retains the configured multiplier");
	CTR_TEST_CHECK_EQ(human.const_TurnRate, data.metaPhys[1].value[current], "byte-sized handling stats switch with the class");
	CTR_TEST_CHECK_EQ(human.const_turboFullBarReserveGain, data.metaPhys[4].value[current], "reserve-gain constants switch through the shared table loader");
	CTR_TEST_CHECK(human.const_Gravity == 1800 && human.const_JumpForce == 2298, "class reload preserves configured gravity and jump multipliers");
	CTR_TEST_CHECK_EQ(Chaos_GravityForDriver(&human), 450, "existing continuous Chaos modifiers still use the new constants");
	CTR_TEST_CHECK(human.reserves == 500 && human.fireSpeedCap == 4096 && human.heldItemID == HELD_ITEM_POTION,
	               "class changes preserve current boost and inventory");
	CTR_TEST_CHECK(human.posCurr.x == 10 && human.posCurr.y == 20 && human.posCurr.z == 30 && human.speed == 10000,
	               "class changes do not reset position or momentum");
	CTR_TEST_CHECK_EQ(bot.const_Speed_ClassStat, 1234, "bots retain their existing class constants");
	CTR_TEST_CHECK(data.characterIDs[0] == CRASH_BANDICOOT && data.characterIDs[1] == COCO_BANDICOOT &&
	               memcmp(&crashMetadata, &data.MetaDataCharacters[CRASH_BANDICOOT], sizeof(crashMetadata)) == 0 &&
	               memcmp(&cocoMetadata, &data.MetaDataCharacters[COCO_BANDICOOT], sizeof(cocoMetadata)) == 0,
	               "characters and shared character metadata remain unchanged");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "the class change takes no timed slot and preserves Low Gravity");
	Tick(4000);
	CTR_TEST_CHECK_EQ(human.const_Speed_ClassStat, data.metaPhys[0].value[current] * 150 / 100, "notice expiry does not restore the old class");
	for (int i = 0; i < 20; i++)
	{
		Chaos_Activate(CHAOS_EFFECT_TRY_SOMETHING_NEW, 1000);
		const int next = sdata->chaosRuntime.engineClass[0];
		CTR_TEST_CHECK(next != current && next < NUM_CLASSES, "repeat activations exclude the live Chaos class rather than the character baseline");
		CTR_TEST_CHECK_EQ(s_engineStopped[0], (u32)(current * 4), "repeat changes stop the currently playing class loop");
		CTR_TEST_CHECK_EQ(s_engineStarted[0], (u32)(next * 4), "repeat changes start the newly selected engine loop");
		current = next;
	}
	const struct ChaosRuntime saved = sdata->chaosRuntime;
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(s_engineStopped[0], (u32)(current * 4), "teardown stops the changed engine loop");
	CTR_TEST_CHECK_EQ(s_engineStarted[0], BALANCED * 4, "teardown restores the character engine for post-race updates");
	CTR_TEST_CHECK_EQ(Chaos_EngineClassForDriver(&human, BALANCED), BALANCED, "teardown clears the audio class override");
	CTR_TEST_CHECK(!sdata->chaosRuntime.engineClassChanged[0], "race teardown clears class-selection bookkeeping");
	sdata->chaosRuntime = saved;
	const int partnerClass = sdata->chaosRuntime.engineClass[1];
	partner.actionsFlagSet = ACTION_RACE_FINISHED;
	const int partnerStarts = s_engineStarts[1];
	Chaos_Activate(CHAOS_EFFECT_TRY_SOMETHING_NEW, 1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.engineClass[1], partnerClass, "finished humans receive no further class changes");
	CTR_TEST_CHECK_EQ(s_engineStarts[1], partnerStarts, "finished humans receive no engine restart on later activations");
	Chaos_BeginRace(&s_tracker);
	VehBirth_SetConsts(&human);
	CTR_TEST_CHECK_EQ(human.const_Speed_ClassStat, 15000, "the next race starts with the character's ordinary class");
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

static void TestThisIsFire(void)
{
	CTR_TEST_SECTION("This Is Fire upgrades existing turbo without granting fuel or a synthetic boost");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_THIS_IS_FIRE] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_DEFECTIVE_BOOSTS] = true;
	sdata->chaosSettings.enabledCount += 2;
	struct Thread driverThread = {.modelIndex = DYNAMIC_PLAYER};
	struct Instance driverInst = {.thread = &driverThread};
	const struct Driver initial = {
		.driverID = 0, .instSelf = &driverInst, .kartState = KS_NORMAL,
		.const_SingleTurboSpeed = 1000, .const_SacredFireSpeed = 2000,
	};
	struct Driver human = initial;
	human.reserves = 500;
	human.turbo_outsideTimer = 100;
	human.fireSpeedCap = 1500;
	human.numTurbos = 3;
	struct Driver partner = initial;
	partner.driverID = 1;
	struct Driver bot = human;
	bot.driverID = 2;
	bot.actionsFlagSet = ACTION_BOT;
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	s_tracker.drivers[2] = &bot;
	struct Turbo turbo = {.driver = &human, .fireSize = 5};
	struct Thread turboThread = {.object = &turbo};
	s_tracker.threadBuckets[TURBO].thread = &turboThread;
	Chaos_Activate(CHAOS_EFFECT_DEFECTIVE_BOOSTS, 5000);
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_THIS_IS_FIRE, 1000), "This Is Fire activates as a timed effect");
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_DEFECTIVE_BOOSTS), "This Is Fire replaces Defective Boosts");
	CTR_TEST_CHECK_EQ(human.fireSpeedCap, 9000, "existing turbo immediately receives the super-pad speed tier");
	CTR_TEST_CHECK_EQ(turbo.fireSize, 8, "existing exhaust receives normal USF flame sizing");
	CTR_TEST_CHECK(human.reserves == 500 && human.turbo_outsideTimer == 100, "upgrading existing fire preserves both fuel fields");
	CTR_TEST_CHECK(human.numTurbos == 3 && human.actionsFlagSet == 0, "activation does not count or latch an extra boost");
	CTR_TEST_CHECK(partner.reserves == 0 && partner.fireSpeedCap == 0, "an unboosted human receives no free turbo or reserves");
	CTR_TEST_CHECK_EQ(bot.fireSpeedCap, 1500, "existing bot turbo remains unchanged");
	CTR_TEST_CHECK_EQ(Chaos_FireLevelForDriver(&bot, 128), 128, "new bot turbo remains ordinary");
	CTR_TEST_CHECK_EQ(Chaos_FireLevelForDriver(&partner, VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL + 128), VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL, "active effect selects the super-pad fire level");
	s_tracker.threadBuckets[TURBO].thread = NULL;

	const u32 types[] = {
		START_LINE_BOOST, POWER_SLIDE_HANG_TIME,
		TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD,
		TURBO_ITEM | FREEZE_RESERVES_ON_TURBO_PAD, TURBO_PAD | SUPER_ENGINE,
	};
	for (size_t i = 0; i < len(types); i++)
	{
		partner = initial;
		partner.driverID = 1;
		VehFire_Increment(&partner, 300, types[i], 128);
		CTR_TEST_CHECK_EQ(partner.fireSpeedCap, 9000, "start, drift, hang-time, pad, item and engine boosts receive USF");
		CTR_TEST_CHECK_EQ(partner.reserves, 300, "upgraded boost sources grant only their normal reserves");
	}
	VehFire_Increment(&human, 100, POWER_SLIDE_HANG_TIME, 128);
	CTR_TEST_CHECK_EQ(human.fireSpeedCap, 9000, "a weaker boost cannot cancel USF during the effect");
	CTR_TEST_CHECK_EQ(human.reserves, 600, "a new boost adds only its earned reserves");
	Tick(1000);
	CTR_TEST_CHECK_EQ(human.fireSpeedCap, 9000, "expiry does not remove earned USF");
	VehFire_Increment(&human, 100, POWER_SLIDE_HANG_TIME, 128);
	CTR_TEST_CHECK_EQ(human.fireSpeedCap, 1500, "the next weaker boost cancels USF normally after expiry");
	Chaos_Activate(CHAOS_EFFECT_THIS_IS_FIRE, 5000);
	Chaos_Activate(CHAOS_EFFECT_DEFECTIVE_BOOSTS, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_THIS_IS_FIRE), "Defective Boosts also replaces This Is Fire");
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
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 2000, "inactive Chaos leaves accumulated reserves alone");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 1000, "inactive Chaos leaves the outside turbo timer alone");
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 2000);
	const u32 actions = ACTION_JUMP_BUTTON_HELD | ACTION_MASK_WEAPON;
	CTR_TEST_CHECK_EQ(PreparedDriver(&human, &pad, &steering, actions), actions,
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
		PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
		CTR_TEST_CHECK_EQ(human.reserves, 0, "the next driver application clears newly granted reserves");
		CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 0, "the next driver application clears outside turbo time too");
		CTR_TEST_CHECK((human.actionsFlagSet & ACTION_NEW_BOOST) != 0, "retail boost event flags remain available for presentation");
	}
	struct Driver partner = {.driverID = 1, .reserves = 500, .turbo_outsideTimer = 200};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .reserves = 700, .turbo_outsideTimer = 300};
	PreparedDriver(&partner, &pad, &steering, partner.actionsFlagSet);
	PreparedDriver(&bot, &pad, &steering, bot.actionsFlagSet);
	PreparedDriver(NULL, &pad, &steering, 0);
	CTR_TEST_CHECK_EQ(partner.reserves, 0, "No Reserves also applies to the co-op partner");
	CTR_TEST_CHECK_EQ(partner.turbo_outsideTimer, 0, "the partner's two fuel fields are cleared together");
	CTR_TEST_CHECK_EQ(bot.reserves, 700, "No Reserves preserves bot reserves");
	CTR_TEST_CHECK_EQ(bot.turbo_outsideTimer, 300, "No Reserves preserves bot outside turbo time");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	VehFire_Increment(&human, 1000, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 100);
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
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
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "the refreshed restriction still clears reserves before its own expiry");
	Tick(500);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "final expiry does not restore reserves erased earlier");
	VehFire_Increment(&human, 1000, POWER_SLIDE_HANG_TIME, 100);
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "new reserves can accumulate normally after final expiry");
	sdata->chaosRuntime = saved;
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 0, "resident restoration restores the restriction without a separate driver snapshot");
	Chaos_Reset();
	VehFire_Increment(&human, 1000, TURBO_PAD | FREEZE_RESERVES_ON_TURBO_PAD, 100);
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
	CTR_TEST_CHECK_EQ(human.reserves, 1000, "reset immediately restores normal boost grants");
	CTR_TEST_CHECK_EQ(human.turbo_outsideTimer, 1000, "reset also restores ordinary outside turbo bookkeeping");
	Chaos_BeginRace(&s_tracker);
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 1000);
	s_tracker.gameMode1 |= ADVENTURE_ARENA;
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
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
	struct GamepadBuffer pad = {.stickLX = 128};
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
			driver->actionsFlagSet = PreparedDriver(driver, &pad, &steering, driver->actionsFlagSet);
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
	CTR_TEST_CHECK_EQ(human.kartScale, CHAOS_GIANT_SCALE, "activation changes the driver's scale target once");
	CTR_TEST_CHECK_EQ(partner.kartScale, CHAOS_GIANT_SCALE, "co-op partner receives the same scale target");
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
	CTR_TEST_CHECK_EQ(human.kartScale, CHAOS_GIANT_SCALE, "pause freezes the enlarged target");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 1000);
	CTR_TEST_CHECK_EQ(human.kartScale, CHAOS_GIANT_SCALE, "duplicate activation does not compound the scale target");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	Tick(1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_GIANT_KARTS), "the replacement expiry deactivates Giant Karts without a surviving copy");
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "deactivating Giant Karts restores the scale even for a finished human");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	Tick(2000);
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "final expiry restores the normal target");
	CTR_TEST_CHECK_EQ(partner.kartScale, 1, "final expiry restores the co-op partner's target");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 10000, "expiry restores speed");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 1000), 1000, "expiry restores jump force");
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 1000);
	CTR_TEST_CHECK_EQ(human.kartScale, CHAOS_GIANT_SCALE, "a new activation changes the target again");
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

static void TestSmallKartEnergy(void)
{
	CTR_TEST_SECTION("Small Kart Energy halves dimensions and uses normal size-based squash protection");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SMALL_KART_ENERGY] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_GIANT_KARTS] = true;
	sdata->chaosSettings.enabledCount += 2;
	struct Thread humanThread = {0};
	struct Instance humanInst = {.thread = &humanThread, .scale = {0xccc, 0xccc, 0xccc}};
	struct Driver human = {.driverID = 0, .instSelf = &humanInst, .wheelSize = 0xccc};
	struct Driver partner = {.driverID = 1};
	struct Thread botThread = {0};
	struct Instance botInst = {.thread = &botThread};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT, .instSelf = &botInst};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	s_tracker.drivers[2] = &bot;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SMALL_KART_ENERGY, 3000), "Small Kart Energy activates as a timed effect");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "the small size occupies a timed slot");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 0xccc), 0x666, "body dimensions shrink to half size");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, human.wheelSize), 0x666, "tires shrink with the body");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 25), 12, "odd collision radii round toward zero at half size");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, -25), -12, "signed scale offsets retain their direction");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 0), 0, "squashed zero height remains zero");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&partner, 100), 50, "the second human also shrinks");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&bot, 100), 100, "bots keep normal dimensions");
	CTR_TEST_CHECK_EQ(humanInst.scale.x, 0xccc, "activation preserves the existing scale interpolation");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 7000, "forward speed targets fall to seventy percent");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, -2000), -2000, "reverse speed follows the existing size-effect convention");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 4596), 3217, "jump force falls to seventy percent without altering constants");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 10000), 10000, "bots keep normal speed targets");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&bot, 4596), 4596, "bots keep normal jump force");
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&human, &partner), "equally small humans do not squash each other");
	g_config.maskProtectsFromDamage = true;
	human.actionsFlagSet |= ACTION_MASK_WEAPON;
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&bot, &human), "a protective mask prevents squash by a normal bot");
	human.actionsFlagSet &= ~ACTION_MASK_WEAPON;
	struct Shield shieldObject = {0};
	struct Thread shieldThread = {.object = &shieldObject};
	struct Instance shield = {.thread = &shieldThread};
	human.instBubbleHold = &shield;
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&human, &bot), "a shield absorbs the squash regardless of contact order");
	CTR_TEST_CHECK((shieldObject.flags & SHIELD_FLAG_POP_ON_DAMAGE) != 0 && human.instBubbleHold == NULL,
	               "shield protection uses normal shield consumption");
	CTR_TEST_CHECK(!VehPhysCrash_TrySquashSmallerKart(&bot, &human), "shield-granted immunity prevents an immediate repeat squash");
	human.invincibleTimer = 0;
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&human, &bot), "a normal bot can squash an unprotected half-sized human");
	CTR_TEST_CHECK_EQ(human.squishTimer, VEH_PICK_SQUISH_TIMER, "small karts use the existing squash duration");
	const int timer = human.squishTimer;
	bot.forcedJumpType = FORCED_JUMP_NONE;
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&bot, &human), "continued contact passes over an already squashed small kart");
	CTR_TEST_CHECK(human.squishTimer == timer && bot.forcedJumpType == FORCED_JUMP_NONE,
	               "continued contact does not renew damage or hop repeatedly");
	Chaos_Activate(CHAOS_EFFECT_GIANT_KARTS, 3000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SMALL_KART_ENERGY), "Giant Karts removes Small Kart Energy");
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 200, "replacement grows directly to the giant target");
	Chaos_Activate(CHAOS_EFFECT_SMALL_KART_ENERGY, 3000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_GIANT_KARTS), "Small Kart Energy removes Giant Karts");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 7000, "replacement does not retain the giant speed multiplier");
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(3000);
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 50, "pause freezes the small-size duration");
	s_tracker.gameMode1 &= ~PAUSE_1;
	Chaos_Activate(CHAOS_EFFECT_SMALL_KART_ENERGY, 1000);
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 50, "repeat activation does not compound shrinkage");
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 100, "finished humans return to retail dimensions");
	Tick(1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_SMALL_KART_ENERGY), "Small Kart Energy expires normally");
	CTR_TEST_CHECK_EQ(human.kartScale, 1, "expiry clears the small target even after finishing");
	human.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&partner, 100), 100, "expiry restores the partner's dimensions");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&human, 10000), 10000, "expiry restores speed targets");
	CTR_TEST_CHECK_EQ(Chaos_JumpForceForDriver(&human, 4596), 4596, "expiry restores jump force");
	Chaos_Activate(CHAOS_EFFECT_SMALL_KART_ENERGY, 1000);
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 100, "race reset restores normal size");
	human.squishTimer = 0;
	human.kartScale = 1.5f;
	CTR_TEST_CHECK(VehPhysCrash_TrySquashSmallerKart(&human, &bot), "one-and-a-half size remains distinguishable from normal size during squash comparison");
	human.kartScale = 0.75f;
	CTR_TEST_CHECK_EQ(VehPhysForce_ScaleValue(&human, 100), 75, "arbitrary fractional scales are not restricted to half-units");
}

static void TestKartParticleScale(void)
{
	CTR_TEST_SECTION("Kart particle scaling preserves color, rotation, timing and oscillator ownership");
	struct Driver driver = {.kartScale = 0.5f};
	MATRIX matrix = {.m = {{4096, -2048, 0}, {0, 4096, 0}, {0, 0, 4096}}, .t = {100, 200, 300}};
	VehPhysForce_ScaleMatrix(&driver, &matrix);
	CTR_TEST_CHECK(matrix.m[0][0] == 2048 && matrix.m[0][1] == -1024 && matrix.m[2][2] == 2048,
	               "attached effects scale their complete rotated geometry");
	CTR_TEST_CHECK(matrix.t[0] == 100 && matrix.t[1] == 200 && matrix.t[2] == 300,
	               "scaling an attached effect does not scale its world-space position");
	struct ParticleOscillator color = {.scale = 400, .offset = 200, .period = 12};
	struct ParticleOscillator size = {.next = &color, .previousValue = 100, .scale = 200, .offset = 400, .min = -100, .max = 500, .period = 16, .phase = 8};
	struct Particle particle = {
		.oscillator = &size, .framesLeftInLife = 30,
		.flagsOscillatorAxis = PARTICLE_AXIS_FLAG_SCALE_X | PARTICLE_AXIS_FLAG_COLOR_R,
		.axis = {
			[PARTICLE_AXIS_POS_X] = {.startVal = 100, .velocity = -20, .accel = 4},
			[PARTICLE_AXIS_ROT_X_OR_LINE_PREV_X] = {.startVal = 200, .velocity = 10},
			[PARTICLE_AXIS_ROT_Y_OR_LINE_PREV_Z] = {.startVal = 300},
			[PARTICLE_AXIS_SCALE_X_OR_LINE_SCALE] = {.startVal = 1536, .velocity = 400, .accel = -8},
			[PARTICLE_AXIS_SCALE_Y_OR_LINE_PREV_Y] = {.startVal = 1024},
			[PARTICLE_AXIS_COLOR_R] = {.startVal = 16384, .velocity = -2560},
		},
	};
	const struct Particle original = particle;
	VehPhysForce_ScaleParticle(&driver, &particle);
	CTR_TEST_CHECK(particle.axis[0].startVal == 50 && particle.axis[0].velocity == -10 && particle.axis[0].accel == 2,
	               "particle spread and motion shrink together");
	CTR_TEST_CHECK(particle.axis[5].startVal == 768 && particle.axis[5].velocity == 200 && particle.axis[5].accel == -4 && particle.axis[6].startVal == 512,
	               "particle size and its growth rate shrink together");
	CTR_TEST_CHECK(memcmp(&particle.axis[3], &original.axis[3], 2 * sizeof(struct ParticleAxis)) == 0,
	               "sprite angles and angular motion keep their original values");
	CTR_TEST_CHECK(memcmp(&particle.axis[7], &original.axis[7], 4 * sizeof(struct ParticleAxis)) == 0 && particle.framesLeftInLife == 30,
	               "particle color, fade, icon animation and lifetime stay unchanged");
	CTR_TEST_CHECK(size.scale == 100 && size.offset == 200 && size.previousValue == 50 && size.min == -50 && size.max == 250,
	               "size oscillator values follow the same scale as their axis");
	CTR_TEST_CHECK(size.period == 16 && size.phase == 8 && size.next == &color && color.scale == 400 && color.offset == 200,
	               "oscillator timing, links and color oscillation remain intact");
	particle.flagsSetColor = PARTICLE_SET_COLOR_FLAG_SPECIAL_LINE;
	VehPhysForce_ScaleParticle(&driver, &particle);
	CTR_TEST_CHECK(particle.axis[3].startVal == 100 && particle.axis[4].startVal == 150,
	               "line particles scale their previous endpoint rather than treating it as rotation");
	driver.kartScale = 2.0f;
	particle = original;
	particle.oscillator = NULL;
	VehPhysForce_ScaleParticle(&driver, &particle);
	CTR_TEST_CHECK(particle.axis[0].startVal == 200 && particle.axis[5].startVal == 3072 && particle.axis[5].velocity == 800,
	               "giant karts double particle dimensions and growth");
	driver.actionsFlagSet = ACTION_RACE_FINISHED;
	particle = original;
	particle.oscillator = NULL;
	const struct Particle finished = particle;
	VehPhysForce_ScaleParticle(&driver, &particle);
	CTR_TEST_CHECK(memcmp(&particle, &finished, sizeof(particle)) == 0, "finished drivers retain retail particles");
}

static void TestFreshStart(void)
{
	CTR_TEST_SECTION("Fresh Start replays a human-only start without resetting the race clock");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	struct Instance instances[2] = {0};
	struct Thread threads[2] = {0};
	struct Icon icon = {0};
	icon.texLayout.u1 = 24;
	icon.texLayout.v2 = 24;
	struct DB db = {0};
	s_tracker.backBuffer = &db;
	s_tracker.trafficLightsTimer = DOT_LIGHT_TIMER_SHOW_MIN - 1;
	for (int i = 0; i < 4; i++)
	{
		s_tracker.trafficLightIcon[i] = &icon;
	}
	for (int i = 0; i < 2; i++)
	{
		struct Driver *driver = s_tracker.drivers[i];
		driver->instSelf = &instances[i];
		s_tracker.cameraDC[i].driverToFollow = driver;
		instances[i].thread = &threads[i];
		driver->posCurr = (Vec3){100, 100000, 200};
		driver->velocity = (Vec3){100, 200, 300};
		driver->quadBlockHeight = -500;
		driver->reserves = 10000;
		driver->turbo_outsideTimer = 300;
		driver->jump_ForcedMS = 500;
		driver->actionsFlagSet = ACTION_AIRBORNE | ACTION_HIGH_JUMP;
		driver->heldItemID = HELD_ITEM_BOMB_3X;
		driver->numHeldItems = 3;
		driver->turnAngleCurr = 300;
		driver->simpTurnState = 40;
		driver->angle = 0x100;
		driver->rotCurr.y = 0x500;
		driver->rotCurr.w = 100;
		driver->turnWobbleAngle = 50;
	}
	s_tracker.drivers[1]->actionsFlagSet = ACTION_TOUCH_GROUND;
	struct Driver bot = {.driverID = 2, .speed = 9000, .reserves = 1000};
	s_tracker.drivers[2] = &bot;
	sdata->chaosSettings.effects[CHAOS_EFFECT_FRESH_START] = true;
	sdata->chaosSettings.effects[CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL] = true;
	sdata->chaosSettings.enabledCount += 2;
	Chaos_Activate(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, 30000);
	s_restartInitializations = 0;
	s_restartFallPoses = 0;
	s_startLightDraws = 0;
	s_startCountdownSounds = 0;
	s_startGreenSounds = 0;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_FRESH_START, 1), "restart duration is the retail countdown rather than the requested Chaos duration");
	CTR_TEST_CHECK_EQ(s_restartInitializations, 2, "both unfinished humans enter the retail starting state once");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "restart consumes no timed slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 1, "restart uses an instant notice");
	for (int i = 0; i < 2; i++)
	{
		struct Driver *driver = s_tracker.drivers[i];
		CTR_TEST_CHECK_EQ(driver->posCurr.y, 100000, "restart preserves the kart's height instead of teleporting");
		CTR_TEST_CHECK(driver->posCurr.x == 100 && driver->posCurr.z == 200 && driver->posPrev.y == driver->posCurr.y, "restart preserves horizontal progress without artificial displacement");
		CTR_TEST_CHECK(driver->velocity.x == 0 && driver->velocity.y == 0 && driver->velocity.z == 0, "restart clears motion on every axis");
		CTR_TEST_CHECK(driver->reserves == 0 && driver->turbo_outsideTimer == 0, "the explicit restart clears fire even during reserve protection");
		CTR_TEST_CHECK_EQ(driver->jump_ForcedMS, 0, "old jump state does not relaunch the kart");
		CTR_TEST_CHECK(driver->turnAngleCurr == 0 && driver->simpTurnState == 0, "restart clears the previous steering pose");
		CTR_TEST_CHECK(driver->angle == 0x500 && driver->rotCurr.y == 0x500 && driver->rotCurr.w == 0 && driver->turnWobbleAngle == 0,
		    "neutral pose synchronizes physical, visual and camera heading before the countdown");
		CTR_TEST_CHECK(driver->heldItemID == HELD_ITEM_BOMB_3X && driver->numHeldItems == 3, "restart preserves held inventory");
		CTR_TEST_CHECK_EQ(Chaos_StartTimerForDriver(driver), DOT_LIGHT_TIMER_HIDE, "the revving update waits for the full local countdown");
	}
	CTR_TEST_CHECK(sdata->chaosRuntime.restart[0].floating && sdata->chaosRuntime.restart[0].mask != NULL, "an airborne kart is held by its own rescue mask");
	CTR_TEST_CHECK_EQ(s_restartFallPoses, 1, "the suspended kart reuses the retail rescue falling pose");
	CTR_TEST_CHECK(!sdata->chaosRuntime.restart[1].floating && instances[1].animIndex == 0 && instances[1].animFrame == 20, "grounded karts return directly to the neutral forward-facing frame");
	for (int i = 0; i < 2; i++)
	{
		CTR_TEST_CHECK(Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[i]), "each human's camera starts the existing recovery blend at activation");
		CTR_TEST_CHECK(!Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[i]), "the camera blend is not restarted on every countdown frame");
	}
	struct CameraDC botCamera = {.driverToFollow = &bot};
	CTR_TEST_CHECK(!Chaos_ConsumeStartCameraBlend(&botCamera), "bot cameras do not consume a human restart transition");
	CTR_TEST_CHECK_EQ(Chaos_StartTimerForDriver(&bot), DOT_LIGHT_TIMER_SHOW_MIN - 1, "bots continue to see the completed global countdown");
	CTR_TEST_CHECK(bot.speed == 9000 && bot.reserves == 1000, "restart does not stop or drain bots");
	s_tracker.gameMode1 |= PAUSE_ALL;
	Tick(960);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.restart[0].timer, DOT_LIGHT_TIMER_HIDE, "pause freezes the local countdown");
	s_tracker.gameMode1 &= ~PAUSE_ALL;
	for (int i = 0; i < 4; i++)
	{
		Tick(960);
		Chaos_DrawStartLights(&s_tracker);
		Chaos_DrawStartLights(&s_tracker);
	}
	CTR_TEST_CHECK_EQ(s_startCountdownSounds, 3, "the three red lights sound once each across both viewports");
	CTR_TEST_CHECK_EQ(s_startGreenSounds, 1, "green sounds once even if the HUD is drawn again");
	CTR_TEST_CHECK_EQ(s_startLightDraws, 64, "the original four-light display renders independently in both human viewports");
	CTR_TEST_CHECK_EQ(Chaos_StartTimerForDriver(s_tracker.drivers[0]), 0, "green lets the normal revving update release the kart and grant its earned boost");
	CTR_TEST_CHECK_EQ(s_tracker.trafficLightsTimer, DOT_LIGHT_TIMER_SHOW_MIN - 1, "local lights never rewind the global race countdown");
	CTR_TEST_CHECK(!sdata->chaosRuntime.restart[0].floating && s_restartMasks[0].duration == 0, "green ends the suspension and fades its temporary mask");
	CTR_TEST_CHECK(s_tracker.drivers[0]->angle == 0x500 && s_tracker.drivers[0]->rotCurr.y == 0x500 &&
	    s_tracker.drivers[1]->angle == 0x500 && s_tracker.drivers[1]->rotCurr.y == 0x500,
	    "green preserves the held heading instead of returning to the old pre-drift direction");
	Tick(960);
	CTR_TEST_CHECK(!sdata->chaosRuntime.restart[0].active && !sdata->chaosRuntime.restart[1].active, "the green-light exit animation ends naturally");
	Chaos_Activate(CHAOS_EFFECT_FRESH_START, 1000);
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.restart[0].timer, DOT_LIGHT_TIMER_HIDE, "repeated activation starts a fresh countdown");
	s_tracker.cameraDC[0].cameraMode = CAMERA_MODE_FREECAM;
	CTR_TEST_CHECK(!Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[0]), "a non-follow camera is not redirected by the restart blend");
	s_tracker.cameraDC[0].cameraMode = 0;
	CTR_TEST_CHECK(Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[0]), "repeated activation requests a fresh blend when driver follow resumes");
	s_tracker.drivers[1]->actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_Activate(CHAOS_EFFECT_FRESH_START, 1000);
	Tick(1);
	CTR_TEST_CHECK(sdata->chaosRuntime.restart[0].active && !sdata->chaosRuntime.restart[1].active, "finished humans are excluded from the restarted sequence");
	s_startLightDraws = 0;
	Chaos_DrawStartLights(&s_tracker);
	CTR_TEST_CHECK_EQ(s_startLightDraws, 4, "only the unfinished human viewport receives restart lights");
	s_tracker.drivers[0]->kartState = KS_MASK_GRABBED;
	Chaos_Activate(CHAOS_EFFECT_FRESH_START, 1000);
	Tick(960);
	CTR_TEST_CHECK(sdata->chaosRuntime.restart[0].pending && !sdata->chaosRuntime.restart[0].active, "an ongoing rescue delays both the hold and its traffic lights");
	CTR_TEST_CHECK(!Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[0]), "pending restart does not disturb the camera during an ongoing rescue");
	s_tracker.drivers[0]->kartState = KS_ENGINE_REVVING;
	s_tracker.drivers[0]->KartStates.RevEngine.boolMaskGrab = true;
	Tick(960);
	CTR_TEST_CHECK(sdata->chaosRuntime.restart[0].pending, "the descending part of a rescue also finishes before the countdown starts");
	s_tracker.drivers[0]->kartState = KS_NORMAL;
	Tick(1);
	CTR_TEST_CHECK(sdata->chaosRuntime.restart[0].active && sdata->chaosRuntime.restart[0].timer == DOT_LIGHT_TIMER_HIDE, "rescue completion starts a full independent countdown");
	CTR_TEST_CHECK(Chaos_ConsumeStartCameraBlend(&s_tracker.cameraDC[0]), "the deferred restart blends only after rescue completion");
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(Chaos_StartTimerForDriver(s_tracker.drivers[0]), DOT_LIGHT_TIMER_SHOW_MIN - 1, "reset releases local start gating");
	s_tracker.trafficLightsTimer = DOT_LIGHT_TIMER_HIDE;
	s_tracker.gameMode1 = ADVENTURE_ARENA;
	CTR_TEST_CHECK(!Chaos_IsWaitingForStart(s_tracker.drivers[0]), "the adventure hub never waits on a positive global start timer without a local restart");
	s_tracker.drivers[2] = NULL;
}

static void TestYouveGotMail(void)
{
	CTR_TEST_SECTION("You've Got Mail launches once from the last bot at the leading human");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_YOUVE_GOT_MAIL] = true;
	sdata->chaosSettings.enabledCount++;
	s_mailLaunches = 0;
	struct Driver firstBot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct Driver lastBot = {.driverID = 7, .actionsFlagSet = ACTION_BOT};
	struct Driver finishedBot = {.driverID = 6, .actionsFlagSet = ACTION_BOT | ACTION_RACE_FINISHED};
	s_tracker.driversInRaceOrder[0] = &firstBot;
	s_tracker.driversInRaceOrder[1] = s_tracker.drivers[1];
	s_tracker.driversInRaceOrder[2] = s_tracker.drivers[0];
	s_tracker.driversInRaceOrder[3] = &lastBot;
	s_tracker.driversInRaceOrder[4] = &finishedBot;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_YOUVE_GOT_MAIL, 1000), "You've Got Mail activates as an instant effect");
	CTR_TEST_CHECK_EQ(s_mailLaunches, 1, "one activation launches exactly one orb");
	CTR_TEST_CHECK(s_mailSource == &lastBot && s_mailTarget == s_tracker.drivers[1], "the last unfinished bot targets the highest-ranked human");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 0, "mail consumes no timed slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 1, "mail announces through the instant banner");
	s_tracker.driversInRaceOrder[3] = NULL;
	s_tracker.drivers[1]->actionsFlagSet |= ACTION_RACE_FINISHED;
	s_tracker.drivers[0]->actionsFlagSet |= ACTION_BOT;
	Chaos_Activate(CHAOS_EFFECT_YOUVE_GOT_MAIL, 1000);
	CTR_TEST_CHECK(s_mailSource == &firstBot && s_mailTarget == s_tracker.drivers[0], "a source ahead of the target and a human on autopilot remain eligible");
	firstBot.actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_Activate(CHAOS_EFFECT_YOUVE_GOT_MAIL, 1000);
	CTR_TEST_CHECK_EQ(s_mailLaunches, 2, "mail without an unfinished bot performs no launch");
	firstBot.actionsFlagSet &= ~ACTION_RACE_FINISHED;
	s_tracker.drivers[0]->actionsFlagSet |= ACTION_RACE_FINISHED;
	Chaos_Activate(CHAOS_EFFECT_YOUVE_GOT_MAIL, 1000);
	CTR_TEST_CHECK_EQ(s_mailLaunches, 2, "mail without an unfinished human performs no launch");
	memset(s_tracker.driversInRaceOrder, 0, sizeof(s_tracker.driversInRaceOrder));
	Chaos_Reset();
}

static void TestArenaChampions(void)
{
	CTR_TEST_SECTION("Arena Champions doubles bot motion without compounding AI state");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_ARENA_CHAMPIONS] = true;
	sdata->chaosSettings.enabledCount++;
	Chaos_PublishEffects();
	CTR_TEST_CHECK(strstr(s_externalCatalogue, "arena-champions\n") != NULL, "Arena Champions is published even in a botless race");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_ARENA_CHAMPIONS, 1000), "Arena Champions activates without a bot-availability requirement");
	Chaos_BeginRace(&s_tracker);
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	struct Driver secondBot = {.driverID = 7, .actionsFlagSet = ACTION_BOT};
	s_tracker.drivers[2] = &bot;
	s_tracker.drivers[7] = &secondBot;
	struct Driver *human = s_tracker.drivers[0];
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 9000), 9000, "inactive Arena Champions preserves retail bot motion");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_ARENA_CHAMPIONS, 1000), "Arena Champions activates as a timed effect when bots are present");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numActive, 1, "Arena Champions occupies a timed slot");
	CTR_TEST_CHECK_EQ(sdata->chaosRuntime.numInstantNotices, 0, "Arena Champions is not an instant action");
	CTR_TEST_CHECK(Chaos_IsActiveForDriver(CHAOS_EFFECT_ARENA_CHAMPIONS, &bot) &&
	               Chaos_IsActiveForDriver(CHAOS_EFFECT_ARENA_CHAMPIONS, &secondBot), "all unfinished bot slots receive the effect");
	CTR_TEST_CHECK(!Chaos_IsActiveForDriver(CHAOS_EFFECT_ARENA_CHAMPIONS, human), "human slots are excluded");
	const int nominalSpeeds[] = {0, 500, 1000, 9000, 16000, 24000, 25600};
	int previousActualSpeed = 0;
	int previousNominalSpeed = 0;
	for (int i = 0; i < (int)len(nominalSpeeds); i++)
	{
		bot.botData.aiPhysics.speedLinear = nominalSpeeds[i];
		const int actualSpeed = Chaos_SpeedForDriver(&bot, bot.botData.aiPhysics.speedLinear);
		CTR_TEST_CHECK_EQ(actualSpeed, nominalSpeeds[i] * 2, "bot motion doubles through the full retail speed range, including boosted speeds");
		CTR_TEST_CHECK_EQ(actualSpeed - previousActualSpeed, (nominalSpeeds[i] - previousNominalSpeed) * 2,
		                  "the same AI acceleration produces twice the change in physical speed");
		CTR_TEST_CHECK_EQ(bot.botData.aiPhysics.speedLinear, nominalSpeeds[i], "motion scaling leaves persistent AI speed in retail units");
		CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, bot.botData.aiPhysics.speedLinear), actualSpeed,
		                  "repeated motion queries never compound the speed multiplier");
		previousActualSpeed = actualSpeed;
		previousNominalSpeed = nominalSpeeds[i];
	}
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(human, 9000), 9000, "human driving speed remains unchanged");
	sdata->chaosSettings.effects[CHAOS_EFFECT_SUPER_SPEED] = true;
	sdata->chaosSettings.enabledCount++;
	Chaos_Activate(CHAOS_EFFECT_SUPER_SPEED, 1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(human, 9000), 18000, "human Super Speed can coexist with bot Arena Champions");
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 9000), 18000, "human Super Speed does not double the bots a second time");
	human->actionsFlagSet |= ACTION_BOT;
	CTR_TEST_CHECK(!Chaos_IsActiveForDriver(CHAOS_EFFECT_ARENA_CHAMPIONS, human), "a human-owned slot on autopilot does not become an Arena Champions target");
	human->actionsFlagSet &= ~ACTION_BOT;
	secondBot.actionsFlagSet |= ACTION_RACE_FINISHED;
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&secondBot, 9000), 9000, "finished bots return to retail motion immediately");
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 9000), 9000, "expiry restores retail motion without undoing a modified constant");
	Chaos_Activate(CHAOS_EFFECT_ARENA_CHAMPIONS, 1000);
	Chaos_Reset();
	CTR_TEST_CHECK_EQ(Chaos_SpeedForDriver(&bot, 9000), 9000, "race teardown clears the bot multiplier");
}

static void TestClassicGTA(void)
{
	CTR_TEST_SECTION("Classic GTA provides per-human top-down views and viewport-local cutaways");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_CLASSIC_GTA] = true;
	sdata->chaosSettings.enabledCount++;
	struct Driver human = {.driverID = 0, .posCurr = {100 << 8, 200 << 8, 300 << 8}, .angle = 0x100};
	struct Driver partner = {.driverID = 1, .posCurr = {3000 << 8, 400 << 8, 5000 << 8}, .angle = 0x700};
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	s_tracker.drivers[0] = &human;
	s_tracker.drivers[1] = &partner;
	struct CameraDC *camera = &s_tracker.cameraDC[0];
	struct CameraDC *secondCamera = &s_tracker.cameraDC[1];
	struct PushBuffer *view = &s_tracker.pushBuffer[0];
	struct PushBuffer *secondView = &s_tracker.pushBuffer[1];
	*camera = (struct CameraDC){.driverToFollow = &human, .pushBuffer = view, .nearOrFar = 1, .cameraMode = 0};
	*secondCamera = (struct CameraDC){.driverToFollow = &partner, .pushBuffer = secondView};
	view->distanceToScreen_CURR = 256;
	view->pos = (SVec3){10, 20, 30};
	CTR_TEST_CHECK(!Chaos_ApplyCameraRules(camera), "inactive Classic GTA leaves the retail camera alone");
	CTR_TEST_CHECK_EQ(view->pos.y, 20, "the inactive hook does not move the camera");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_CLASSIC_GTA, 3000), "Classic GTA activates as a timed effect");
	CTR_TEST_CHECK(Chaos_ApplyCameraRules(camera) && Chaos_ApplyCameraRules(secondCamera), "both human viewports receive the override");
	CTR_TEST_CHECK(view->pos.x == 100 && view->pos.y == 200 + CHAOS_TOP_DOWN_HEIGHT && view->pos.z == 300,
	               "the eye stays vertically above the current kart position");
	CTR_TEST_CHECK(view->rot.x == 0x400 && view->rot.y == 0x900 && view->rot.z == 0,
	               "the camera looks vertically down with heading derived from the kart");
	CTR_TEST_CHECK(secondView->pos.x == 3000 && secondView->pos.z == 5000 && secondView->rot.y == 0xf00,
	               "split-screen cameras follow independent positions and headings");
	CTR_TEST_CHECK(camera->nearOrFar == 1 && camera->cameraMode == 0 && view->distanceToScreen_PREV == 256,
	               "the presentation override preserves the retail camera mode, zoom toggle and projection");
	const struct LevVertex vertices[] = {
		{.pos = {0, 400, 200}}, {.pos = {200, 400, 200}},
		{.pos = {0, 400, 400}}, {.pos = {200, 400, 400}},
		{.pos = {100, 400, 200}}, {.pos = {0, 400, 300}},
		{.pos = {100, 400, 300}}, {.pos = {200, 400, 300}},
		{.pos = {100, 400, 400}},
	};
	struct QuadBlock roof = {
		.index = {0, 2, 1, 3, 5, 4, 6, 8, 7},
		.bbox = {.min = {0, 400, 200}, .max = {200, 500, 400}},
	};
	const struct QuadBlock originalRoof = roof;
	CTR_TEST_CHECK(Chaos_HideCameraQuadBlock(view, &roof, vertices), "downward-facing tunnel geometry is cut away");
	CTR_TEST_CHECK(Chaos_HideCameraQuadBlock(secondView, &roof, vertices), "ceiling removal is stable outside the kart's current block footprint");
	struct QuadBlock road = {
		.index = {0, 1, 2, 3, 4, 5, 6, 7, 8},
		.quadFlags = QUADBLOCK_FLAG_GROUND,
		.bbox = {.min = {0, 200, 200}, .max = {200, 450, 400}},
	};
	human.underDriver = &road;
	Chaos_ApplyCameraRules(camera);
	CTR_TEST_CHECK(camera->ptrQuadBlock == &road && camera->quadBlockSearchHit,
	               "tunnel visibility uses the driver's ground block instead of a roof above the camera ray");
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &road, vertices), "sloping ground that crosses the cutaway height remains drivable and visible");
	struct QuadBlock uphill = road;
	uphill.bbox = (struct BoundingBox){.min = {0, 400, 500}, .max = {200, 700, 900}};
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &uphill, vertices), "elevated road ahead stays visible instead of being mistaken for a roof");
	uphill.quadFlags = 0;
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &uphill, vertices), "a ramp ahead stays visible even without the ground flag");
	struct QuadBlock downhill = road;
	downhill.bbox = (struct BoundingBox){.min = {0, -300, 500}, .max = {200, 0, 900}};
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &downhill, vertices), "descending road ahead is retained");
	struct QuadBlock bridge = roof;
	bridge.quadFlags = QUADBLOCK_FLAG_GROUND;
	CTR_TEST_CHECK(Chaos_HideCameraQuadBlock(view, &bridge, vertices), "a downward-facing underside is cut away regardless of the ground flag");
	int leafLists[2][2] = {{0}};
	int faceLists[2][3] = {{0}};
	int packedSource = 0;
	struct VisMem visibility = {
		.visLeafList = {leafLists[0], leafLists[1]},
		.visFaceList = {faceLists[0], faceLists[1]},
		.visLeafSrc = {&packedSource, &packedSource},
		.visFaceSrc = {&packedSource, &packedSource},
	};
	const struct mesh_info mesh = {.numBspNodes = 33, .numQuadBlock = 65};
	Chaos_ApplyLevelVisibilityRules(view, &visibility, &mesh, 0);
	CTR_TEST_CHECK(leafLists[0][0] == -1 && leafLists[0][1] == -1 &&
	               faceLists[0][0] == -1 && faceLists[0][1] == -1 && faceLists[0][2] == -1,
	               "overhead geometry selection includes leaves and faces excluded by the ground-level PVS");
	CTR_TEST_CHECK(leafLists[1][0] == 0 && faceLists[1][0] == 0 && visibility.visFaceSrc[1] == &packedSource,
	               "overhead visibility changes only the requested viewport");
	CTR_TEST_CHECK(visibility.visLeafSrc[0] == NULL && visibility.visFaceSrc[0] == NULL,
	               "visibility cache invalidation forces retail PVS decoding after expiry on the same block");
	road.quadFlags = 0;
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &road, vertices), "the driver's supporting ramp stays visible even without the ground flag");
	const struct QuadBlock unflaggedFloor = road;
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &unflaggedFloor, vertices), "unflagged floor remains visible even when it is not the current supporting block");
	const int boundaryPositions[] = {-100, 0, 100, 200, 300};
	for (int i = 0; i < (int)len(boundaryPositions); i++)
	{
		human.posCurr.x = boundaryPositions[i] * 256;
		human.posCurr.z = (200 + boundaryPositions[i]) * 256;
		CTR_TEST_CHECK(Chaos_HideCameraQuadBlock(view, &roof, vertices), "ceiling stays hidden across both X/Z bounding-box edges");
		CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &unflaggedFloor, vertices), "unflagged pavement stays visible across block edges");
	}
	human.posCurr.x = 100 << 8;
	human.posCurr.z = 300 << 8;
	struct LevVertex slopedVertices[len(vertices)];
	memcpy(slopedVertices, vertices, sizeof(vertices));
	for (int i = 0; i < (int)len(slopedVertices); i++)
	{
		slopedVertices[i].pos.y = slopedVertices[i].pos.x * 2 + slopedVertices[i].pos.z;
	}
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &unflaggedFloor, slopedVertices), "steep unflagged slopes retain their upward-facing pavement");
	struct LevVertex foldedVertices[len(vertices)];
	memcpy(foldedVertices, vertices, sizeof(vertices));
	foldedVertices[6].pos.x = -500;
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &roof, foldedVertices), "a mixed block containing upward-facing high-detail triangles is preserved");
	struct LevVertex wallVertices[len(vertices)];
	memcpy(wallVertices, slopedVertices, sizeof(vertices));
	for (int i = 0; i < (int)len(wallVertices); i++)
	{
		wallVertices[i].pos.x = 0;
	}
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &unflaggedFloor, wallVertices), "vertical panels are not mistaken for downward-facing ceilings");
	struct QuadBlock triangleRoof = roof;
	triangleRoof.index[3] = triangleRoof.index[2];
	triangleRoof.index[7] = UINT16_MAX;
	triangleRoof.index[8] = UINT16_MAX;
	CTR_TEST_CHECK(Chaos_HideCameraQuadBlock(view, &triangleRoof, vertices), "triangular blocks classify only their four valid high-detail faces");
	struct InstDef definition = {0};
	struct Instance decoration = {.instDef = &definition};
	CTR_TEST_CHECK(Chaos_HideCameraInstance(view, &decoration, 100, true), "static level models in front of the kart are cut away");
	CTR_TEST_CHECK(!Chaos_HideCameraInstance(view, &decoration, 100, false), "models outside the kart's screen position remain visible");
	CTR_TEST_CHECK(!Chaos_HideCameraInstance(view, &decoration, CHAOS_TOP_DOWN_HEIGHT * 4, true), "models at road height are not overhead occluders");
	struct Thread dynamicThread = {0};
	decoration.thread = &dynamicThread;
	CTR_TEST_CHECK(!Chaos_HideCameraInstance(view, &decoration, 100, true), "dynamic hazards and gameplay objects remain visible");
	CTR_TEST_CHECK(memcmp(&roof, &originalRoof, sizeof(roof)) == 0, "cutaway queries do not modify shared level geometry");
	s_tracker.gameMode1 |= PAUSE_1;
	Tick(3000);
	CTR_TEST_CHECK(Chaos_ApplyCameraRules(camera) && Chaos_HideCameraQuadBlock(view, &roof, vertices), "pause retains the top-down presentation and tunnel cutaway");
	s_tracker.gameMode1 &= ~PAUSE_1;
	human.posCurr.y = 300 << 8;
	human.angle = 0x300;
	Chaos_ApplyCameraRules(camera);
	CTR_TEST_CHECK(view->pos.y == 300 + CHAOS_TOP_DOWN_HEIGHT && view->rot.y == 0xb00, "height changes and turns update the top-down pose immediately");
	secondCamera->driverToFollow = &bot;
	CTR_TEST_CHECK(!Chaos_ApplyCameraRules(secondCamera), "a bot-follow camera does not receive the human-only override");
	secondCamera->driverToFollow = &partner;
	partner.actionsFlagSet = ACTION_RACE_FINISHED;
	CTR_TEST_CHECK(!Chaos_ApplyCameraRules(secondCamera), "finished humans regain their end-of-race camera");
	Tick(3000);
	view->pos = (SVec3){1, 2, 3};
	CTR_TEST_CHECK(!Chaos_ApplyCameraRules(camera) && view->pos.y == 2, "expiry hands presentation back to the live retail camera");
	CTR_TEST_CHECK(!Chaos_HideCameraQuadBlock(view, &roof, vertices), "expiry restores roof visibility immediately");
	leafLists[0][0] = 0;
	faceLists[0][0] = 0;
	Chaos_ApplyLevelVisibilityRules(view, &visibility, &mesh, 0);
	CTR_TEST_CHECK(leafLists[0][0] == 0 && faceLists[0][0] == 0, "expired Classic GTA leaves retail visibility lists alone");
	Chaos_Activate(CHAOS_EFFECT_CLASSIC_GTA, 1000);
	Chaos_Reset();
	CTR_TEST_CHECK(!Chaos_ApplyCameraRules(camera), "race teardown clears the camera override without a saved pose");
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
	struct GamepadBuffer pad = {.stickLX = 128};
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
	PreparedDriver(&bot, &pad, &steering, bot.actionsFlagSet);
	CTR_TEST_CHECK_EQ(bot.clockReceive, 3000, "Clock Curse does not change bot clocks");
	human.clockReceive = 0;
	human.actionsFlagSet |= ACTION_RACE_FINISHED;
	PreparedDriver(&human, &pad, &steering, human.actionsFlagSet);
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
	struct GamepadBuffer pad = {
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
	actions = PreparedDriver(&driver, &pad, &steering, actions);
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
		actions = PreparedDriver(&driver, &pad, &steering, actions);
		CTR_TEST_CHECK_EQ(actions, ACTION_TOUCH_GROUND, "held shoulders cannot retain a resolved hop on later frames");
		CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 0, "held shoulders cannot retain a jump buffer on later frames");
	}
	struct GamepadBuffer partnerPad = {.stickLX = 200, .buttonsHeldCurrFrame = BTN_SQUARE_one};
	CTR_TEST_CHECK_EQ(PreparedSteering(&partner, &partnerPad), 56, "each player resolves their own steering baseline");
	CTR_TEST_CHECK_EQ(steering, 224, "applying player two preserves player one's local steering");
	driver.kartState = KS_CRASHING;
	PreparedDriver(&driver, &pad, &steering, actions);
	CTR_TEST_CHECK_EQ(driver.kartState, KS_CRASHING, "No Hopping does not replace damage states with normal driving");
	Tick(1000);
	CTR_TEST_CHECK_EQ(PreparedSteering(&driver, &pad), pad.stickLX, "expiry uses live steering without restoring a snapshot");
	actions = PreparedJump(&driver, BTN_R1, BTN_R1, RAIN_CLOUD_EFFECT_NONE, ACTION_TOUCH_GROUND);
	CTR_TEST_CHECK((actions & ACTION_JUMP_BUTTON_HELD) != 0, "expiry lets retail hop input resume without restoring a snapshot");
	CTR_TEST_CHECK_EQ(driver.jump_TenBuffer, 10, "expiry lets retail input fill the jump buffer again");
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

static void TestHeldItemRemoval(void)
{
	CTR_TEST_SECTION("shared item removal preserves caller-owned timers and multiplayer bookkeeping");
	EnableGiftEffects();
	struct Driver *driver = &s_defaultDrivers[0];
	s_tracker.numPlyrCurrGame = 3;
	s_tracker.numPlayersWith3Missiles = 1;
	driver->heldItemID = HELD_ITEM_MISSILE_3X;
	driver->noItemTimer = 1;
	VehPhysProc_RemoveHeldItem(driver);
	CTR_TEST_CHECK_EQ(driver->heldItemID, HELD_ITEM_NONE, "ordinary removal empties the item slot");
	CTR_TEST_CHECK_EQ(s_tracker.numPlayersWith3Missiles, 0, "removing a triple missile releases its multiplayer holder count");
	CTR_TEST_CHECK_EQ(driver->noItemTimer, 1, "the ordinary caller retains control of cooldown decrementing");
	VehPhysProc_RemoveHeldItem(driver);
	CTR_TEST_CHECK_EQ(s_tracker.numPlayersWith3Missiles, 0, "removing an already empty slot does not decrement again");
	s_tracker.gameMode1 |= BATTLE_MODE;
	s_tracker.numPlayersWith3Missiles = 1;
	driver->heldItemID = HELD_ITEM_MISSILE_3X;
	VehPhysProc_RemoveHeldItem(driver);
	CTR_TEST_CHECK_EQ(s_tracker.numPlayersWith3Missiles, 1, "battle removal leaves the race-only missile counter untouched");
}

static void TestRandomItemGift(void)
{
	CTR_TEST_SECTION("Use This! uses Chaos RNG and respects inventory and orb ownership");
	EnableGiftEffects();
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
	Chaos_ClearItems(human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "discarding a held orb retains a deployed orb's flag");
	s_tracker.threadBuckets[TRACKING].thread = NULL;
	human->heldItemID = HELD_ITEM_WARPBALL;
	s_defaultDrivers[1].heldItemID = HELD_ITEM_WARPBALL;
	Chaos_ClearItems(human);
	CTR_TEST_CHECK((s_tracker.gameMode1 & WARPBALL_HELD) != 0, "another holder retains the orb flag");
	Chaos_ClearItems(&s_defaultDrivers[1]);
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
}

static void TestReserveGifts(void)
{
	CTR_TEST_SECTION("reserve gifts use normal fire grants and replace incompatible rules");
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
	human->reserves = INT16_MAX;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_RESERVE_INJECTION, 1000), "the gift remains eligible when reserves are already full");
	CTR_TEST_CHECK_EQ(human->reserves, INT16_MAX, "the gift cannot reduce a larger earned reserve balance");
	human->reserves = 2000;
	Chaos_Activate(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, 1000);
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 1968), 2000, "natural consumption cannot lower frozen reserves");
	VehFire_Increment(human, 500, POWER_SLIDE_HANG_TIME, CHAOS_THIRD_DRIFT_FIRE_LEVEL);
	CTR_TEST_CHECK_EQ(human->reserves, 2500, "boosts can still increase frozen reserves");
	VehPickState_ApplyDamage(human, VEH_PICK_DAMAGE_SPIN, NULL, 0, true);
	CTR_TEST_CHECK_EQ(human->reserves, 2500, "the actual damage path cannot clear frozen reserves");
	struct Driver bot = {.driverID = 2, .reserves = 2000, .actionsFlagSet = ACTION_BOT};
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(&bot, 0), 0, "bot fuel remains ordinary");
	const struct ChaosRuntime snapshot = sdata->chaosRuntime;
	Tick(1000);
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 0), 0, "expiry restores reserve losses");
	sdata->chaosRuntime = snapshot;
	CTR_TEST_CHECK_EQ(Chaos_ReservesForDriver(human, 0), 2500, "resident restore restores reserve protection");
	Chaos_Activate(CHAOS_EFFECT_NO_RESERVES, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL), "No Reserves replaces reserve protection");
	PreparedSteering(human, &(struct GamepadBuffer){0});
	CTR_TEST_CHECK_EQ(human->reserves, 0, "the replacement rule clears fuel immediately when applied");
	Chaos_Activate(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, 1000);
	CTR_TEST_CHECK(!Chaos_IsActive(CHAOS_EFFECT_NO_RESERVES), "reserve protection replaces No Reserves in the opposite order");
}

static void TestSenileMask(void)
{
	CTR_TEST_SECTION("Senile Mask adds false warnings without changing race direction or progress");
	SetupRace(CHAOS_TRIGGER_EXTERNAL);
	sdata->chaosSettings.effects[CHAOS_EFFECT_SENILE_MASK] = true;
	sdata->boolCanPlayVoicelines = true;
	struct Driver *human = s_tracker.drivers[0];
	struct Driver *partner = s_tracker.drivers[1];
	struct Driver bot = {.driverID = 2, .actionsFlagSet = ACTION_BOT};
	s_tracker.drivers[2] = &bot;
	human->distanceDrivenBackwards = 0;
	const struct Driver original = *human;
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK_EQ(s_wrongWayDraws, 0, "inactive Senile Mask adds no warnings");
	CTR_TEST_CHECK(!Chaos_PlayWrongWayVoice(), "inactive Senile Mask leaves retail voice arbitration alone");
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SENILE_MASK, 1000), "Senile Mask activates as a timed effect");
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK_EQ(s_wrongWayDraws, 3, "both correct-direction humans receive the shared flashing prompt, never bots");
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(s_wrongWayVoiceID, 0x1e, "the first eligible human uses the ordinary good-mask voice");
	CTR_TEST_CHECK_EQ(sdata->voicelineCooldown, 45, "Senile Mask halves the ordinary track-length-plus-delay cooldown");
	CTR_TEST_CHECK(memcmp(human, &original, sizeof(original)) == 0, "false warnings leave direction flags, distance, progress and inventory intact");
	for (int i = 0; i < 44; i++)
	{
		Voiceline_Update();
	}
	CTR_TEST_CHECK_EQ(s_wrongWayVoices, 1, "the warning cannot replay before its shortened cooldown");
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(s_wrongWayVoices, 1, "an occupied XA channel is not interrupted when the cooldown expires");
	sdata->XA_State = 0;
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(s_wrongWayVoices, 2, "the warning repeats without needing a real wrong-way direction transition");

	human->actionsFlagSet |= ACTION_DRIVING_WRONG_WAY;
	s_wrongWayDraws = 0;
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK_EQ(s_wrongWayDraws, 2, "a genuinely wrong-way human receives no false warning");
	CTR_TEST_CHECK(Chaos_OverridesWrongWayWarning(human) && Chaos_OverridesWrongWayWarning(partner),
	    "Senile Mask replaces retail warnings for both directions, supplying only its correct-direction warnings");
	// A retail warning may already be armed when the effect starts or direction changes.
	s_tracker.numPlyrCurrGame = 1;
	sdata->WrongWayDirection_bool = true;
	sdata->framesDrivingSameDirection = 31;
	sdata->boolCanPlayWrongWaySFX = true;
	sdata->XA_State = 0;
	sdata->voicelineCooldown = 0;
	const int voicesBeforeWrongWay = s_wrongWayVoices;
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(s_wrongWayVoices, voicesBeforeWrongWay, "an armed retail voice stays silent while the human is genuinely wrong-way");
	CTR_TEST_CHECK(sdata->boolCanPlayWrongWaySFX, "suppressing the retail voice does not consume its armed warning");
	s_tracker.numPlyrCurrGame = 2;
	sdata->XA_State = 0;
	sdata->voicelineCooldown = 0;
	Voiceline_Update();
	CTR_TEST_CHECK(s_voiceDriver == partner && s_wrongWayVoiceID == 0x3d, "2P speech uses the eligible partner's normal evil-mask voice");
	partner->actionsFlagSet |= ACTION_RACE_FINISHED;
	s_wrongWayDraws = 0;
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK_EQ(s_wrongWayDraws, 0, "finished humans receive no false warning");
	CTR_TEST_CHECK(!Chaos_PlayWrongWayVoice(), "no false voice is requested without an eligible human");
	partner->actionsFlagSet = 0;
	const int blockedModes[] = {PAUSE_1, DEBUG_MENU};
	for (int i = 0; i < (int)len(blockedModes); i++)
	{
		s_tracker.gameMode1 |= blockedModes[i];
		s_wrongWayDraws = 0;
		Chaos_DrawWrongWayWarnings(&s_tracker);
		CTR_TEST_CHECK(s_wrongWayDraws == 0 && !Chaos_PlayWrongWayVoice(), "pause and debug menus suppress false warnings and voices");
		s_tracker.gameMode1 &= ~blockedModes[i];
	}
	s_tracker.gameMode1 |= CRYSTAL_CHALLENGE;
	s_wrongWayDraws = 0;
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK_EQ(s_wrongWayDraws, 2, "false warnings also apply in crystal events without a retail wrong-way HUD path");
	s_tracker.gameMode1 &= ~CRYSTAL_CHALLENGE;
	CTR_TEST_CHECK(Chaos_Activate(CHAOS_EFFECT_SENILE_MASK, 1000) && sdata->chaosRuntime.numActive == 1,
	    "repeated activation refreshes one timed effect");
	sdata->XA_State = 0;
	sdata->voicelineCooldown = 0;
	s_voicePlayFails = true;
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(sdata->voicelineCooldown, 15, "missing voice assets retain a bounded retry delay");
	s_voicePlayFails = false;
	Tick(1000);
	s_wrongWayDraws = 0;
	Chaos_DrawWrongWayWarnings(&s_tracker);
	CTR_TEST_CHECK(s_wrongWayDraws == 0 && !Chaos_PlayWrongWayVoice(), "expiry removes both false warning hooks");
	CTR_TEST_CHECK(!Chaos_OverridesWrongWayWarning(human), "expiry restores the real wrong-way warning");

	// The ordinary one-shot warning must retain its original cooldown.
	s_tracker.numPlyrCurrGame = 1;
	sdata->WrongWayDirection_bool = true;
	sdata->framesDrivingSameDirection = 31;
	sdata->boolCanPlayWrongWaySFX = true;
	sdata->voicelineCooldown = 0;
	Voiceline_Update();
	CTR_TEST_CHECK_EQ(sdata->voicelineCooldown, 90, "retail wrong-way playback keeps its full cooldown");
	CTR_TEST_CHECK(!sdata->boolCanPlayWrongWaySFX, "retail wrong-way speech remains one-shot until rearmed");
	Chaos_Activate(CHAOS_EFFECT_SENILE_MASK, 1000);
	Chaos_Reset();
	CTR_TEST_CHECK(!Chaos_PlayWrongWayVoice(), "reset leaves no Senile Mask playback pending");
}

int main(void)
{
	TestSteeringAndDriverScope();
	TestSabotage();
	TestHydraulics();
	TestMyPlanetNeedsMe();
	TestLaunchSpeedLimit();
	TestWhoBoundThis();
	TestTrySomethingNew();
	TestWobbleSteering();
	TestGravityEffects();
	TestSpeedEffects();
	TestNoReserves();
	TestDefectiveBoosts();
	TestThisIsFire();
	TestNoItems();
	TestWumpaDrain();
	TestClockCurse();
	TestExplosiveCrates();
	TestHardCrates();
	TestGiantKarts();
	TestSmallKartEnergy();
	TestKartParticleScale();
	TestClassicGTA();
	TestArenaChampions();
	TestYouveGotMail();
	TestFreshStart();
	TestSenileMask();
	TestAnxiousMask();
	TestHopInputAndCloudOwnership();
	TestDriverApplication();
	TestInstantGifts();
	TestHeldItemRemoval();
	TestRandomItemGift();
	TestReserveGifts();
	return CtrTest_Summary();
}
