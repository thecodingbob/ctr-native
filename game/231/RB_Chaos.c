#include <common.h>
#include <chaos_gate.h>

enum
{
	CHAOS_RANDOM_GAP_MIN_PERCENT = 30,
	CHAOS_PERCENT_SCALE = 100,
	CHAOS_STICK_CENTER = 0x80,
	CHAOS_STICK_RANGE = 0x80,
	CHAOS_WOBBLE_RANGE_NUMERATOR = 3,
	CHAOS_WOBBLE_RANGE_DENOMINATOR = 4,
	CHAOS_LOW_GRAVITY_DIVISOR = 4,
	CHAOS_HEAVY_GRAVITY_MULTIPLIER = 3,
	CHAOS_SPEED_CAP_DIVISOR = 2,
	CHAOS_SUPER_SPEED_MULTIPLIER = 2,
};

static s32 Chaos_NextTriggerGap(void)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	const s32 intervalMS = settings->triggerIntervalSeconds * CHAOS_MILLISECONDS_PER_SECOND;
	if (settings->trigger != CHAOS_TRIGGER_RANDOM || intervalMS == 0 ||
	    settings->durationSeconds == 0 || settings->enabledCount == 0)
	{
		return intervalMS;
	}
	const s32 minMS = intervalMS * CHAOS_RANDOM_GAP_MIN_PERCENT / CHAOS_PERCENT_SCALE;
	const u32 sample = ChaosEffect_NextRandom(&sdata->chaosRng);
	return minMS + (s32)((u64)sample * (intervalMS - minMS) / CHAOS_RANDOM_SAMPLE_MAX);
}

bool Chaos_IsRunning(const struct GameTracker *gGT)
{
	return sdata->chaosRuntime.raceInitialized && sdata->chaosSettings.armed &&
	       sdata->ptrActiveMenu == NULL && sdata->Loading.stage == LOAD_IDLE &&
	       (gGT->gameMode1 & DEBUG_MENU) == 0 &&
	       ChaosGate_IsRaceEligible(gGT->gameMode1, gGT->overlayIndex_Threads,
	                                gGT->trafficLightsTimer < 1, CHAOS_GATE_ADVENTURE_ONLY);
}

void Chaos_Reset(void)
{
	memset(&sdata->chaosRuntime, 0, sizeof(sdata->chaosRuntime));
}

void Chaos_BeginRace(struct GameTracker *gGT)
{
	Chaos_Reset();
	// Initialization may precede unpausing; GO gates updates, not race setup.
	if (!sdata->chaosSettings.armed ||
	    !ChaosGate_IsRaceEligible(gGT->gameMode1 & ~(PAUSE_ALL | LOADING),
	                             gGT->overlayIndex_Threads, true, CHAOS_GATE_ADVENTURE_ONLY))
	{
		return;
	}
	sdata->chaosRuntime.raceInitialized = true;
	sdata->chaosRuntime.intervalRemainingMS = Chaos_NextTriggerGap();
}

static void Chaos_RemoveConflictingEffects(ChaosEffectId effect)
{
	ChaosEffectId replaced = CHAOS_EFFECT_NONE;
	if (effect == CHAOS_EFFECT_INVERTED_STEERING)
	{
		replaced = CHAOS_EFFECT_WOBBLE_STEERING;
	}
	else if (effect == CHAOS_EFFECT_WOBBLE_STEERING)
	{
		replaced = CHAOS_EFFECT_INVERTED_STEERING;
	}
	else if (effect == CHAOS_EFFECT_LOW_GRAVITY)
	{
		replaced = CHAOS_EFFECT_HEAVY_KARTS;
	}
	else if (effect == CHAOS_EFFECT_HEAVY_KARTS)
	{
		replaced = CHAOS_EFFECT_LOW_GRAVITY;
	}
	else if (effect == CHAOS_EFFECT_SPEED_CAPPED)
	{
		replaced = CHAOS_EFFECT_SUPER_SPEED;
	}
	else if (effect == CHAOS_EFFECT_SUPER_SPEED)
	{
		replaced = CHAOS_EFFECT_SPEED_CAPPED;
	}
	if (replaced == CHAOS_EFFECT_NONE)
	{
		return;
	}
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	int kept = 0;
	for (int i = 0; i < runtime->numActive; i++)
	{
		if (runtime->active[i].effect != replaced)
		{
			runtime->active[kept++] = runtime->active[i];
		}
	}
	memset(&runtime->active[kept], 0, (runtime->numActive - kept) * sizeof(runtime->active[0]));
	runtime->numActive = kept;
}

bool Chaos_Activate(ChaosEffectId effect, s32 durationMS)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_IsRunning(sdata->gGT) || durationMS <= 0 ||
	    !ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect))
	{
		return false;
	}
	Chaos_RemoveConflictingEffects(effect);
	if (runtime->numActive == CHAOS_MAX_ACTIVATIONS)
	{
		return false;
	}
	runtime->active[runtime->numActive++] = (struct ChaosActivation){
		.effect = effect,
		.remainingMS = durationMS,
		.noticeRemainingMS = CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS,
	        .elapsedMS = 0;
	};
	OtherFX_Play_LowLevel(HOWL_SFX_SWISH, 1, HOWL_SFX_DEFAULT_FLAGS | HOWL_SFX_GAIN_DOUBLE_FLAG);
	return true;
}

bool Chaos_IsActive(ChaosEffectId effect)
{
	if (ChaosEffect_Def(effect) == NULL || !Chaos_IsRunning(sdata->gGT))
	{
		return false;
	}
	for (int i = 0; i < sdata->chaosRuntime.numActive; i++)
	{
		if (sdata->chaosRuntime.active[i].effect == effect)
		{
			return true;
		}
	}
	return false;
}

bool Chaos_IsActiveForDriver(ChaosEffectId effect, const struct Driver *driver)
{
	const ChaosEffectDef *def = ChaosEffect_Def(effect);
	return driver != NULL && def != NULL &&
	       (driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0 &&
	       ChaosEffect_AppliesToDriver(def->scope, driver->driverID, sdata->gGT->numPlyrCurrGame) &&
	       Chaos_IsActive(effect);
}

int Chaos_GravityForDriver(const struct Driver *driver)
{
	const int gravity = driver->const_Gravity;
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_LOW_GRAVITY, driver))
	{
		return gravity / CHAOS_LOW_GRAVITY_DIVISOR;
	}
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_HEAVY_KARTS, driver))
	{
		return gravity * CHAOS_HEAVY_GRAVITY_MULTIPLIER;
	}
	return gravity;
}

int Chaos_SpeedForDriver(const struct Driver *driver, int speed)
{
	if (speed <= 0)
	{
		return speed;
	}
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_SPEED_CAPPED, driver))
	{
		const int cap = driver->const_Speed_ClassStat / CHAOS_SPEED_CAP_DIVISOR;
		if (speed > cap)
		{
			return cap;
		}
	}
	else if (Chaos_IsActiveForDriver(CHAOS_EFFECT_SUPER_SPEED, driver))
	{
		return speed * CHAOS_SUPER_SPEED_MULTIPLIER;
	}
	return speed;
}

int Chaos_AccelerationForDriver(const struct Driver *driver, int acceleration)
{
	if (driver->baseSpeed > 0 && acceleration > 0 &&
	    Chaos_IsActiveForDriver(CHAOS_EFFECT_SUPER_SPEED, driver))
	{
		return acceleration * CHAOS_SUPER_SPEED_MULTIPLIER;
	}
	return acceleration;
}

void Chaos_ApplyReserveRules(struct Driver *driver)
{
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_NO_RESERVES, driver))
	{
		driver->reserves = 0;
		driver->turbo_outsideTimer = 0;
	}
}

void Chaos_ApplyItemRules(struct Driver *driver)
{
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_NO_ITEMS, driver))
	{
	  struct GameTracker *gGT = sdata->gGT;
	  driver->heldItemID = HELD_ITEM_NONE;
	  driver->numHeldItems = 0;
	  driver->itemRollTimer = 0;
	  driver->noItemTimer = 0;
	  driver->actionsFlagSet &= ~ACTION_WEAPON_FIRE_REQUEST;
	  gGT->gameMode1 &= ~WARPBALL_HELD;
	}
}

static int Chaos_WobbleSteering(const struct ChaosActivation *entry, const struct GamepadBuffer *pad)
{
	const int center = pad->rwd != NULL ? pad->rwd->gamepadCenter : CHAOS_STICK_CENTER;
	const int range = pad->rwd != NULL ? pad->rwd->range : CHAOS_STICK_RANGE;
	if (range <= 0)
	{
		return pad->stickLX;
	}
	const int amplitude = range * CHAOS_WOBBLE_RANGE_NUMERATOR / CHAOS_WOBBLE_RANGE_DENOMINATOR;
	const u32 angle = (entry->elapsedMS % CHAOS_WOBBLE_PERIOD_MS) * ANG_TWO_PI / CHAOS_WOBBLE_PERIOD_MS;
	int steering = pad->stickLX + MATH_Sin(angle) * amplitude / FP(1.0);
	if (steering < center - range)
	{
		steering = center - range;
	}
	else if (steering > center + range)
	{
		steering = center + range;
	}
	return steering;
}

static void Chaos_TickWumpaDrain(s32 elapsedMS, struct Driver *driver)
{
	const s32 frameMS = sdata->gGT->elapsedTimeMS;
	const s32 previousMS = elapsedMS > frameMS ? elapsedMS - frameMS : 0;
	const bool didPulse = elapsedMS / CHAOS_WUMPA_DRAIN_PERIOD_MS > previousMS / CHAOS_WUMPA_DRAIN_PERIOD_MS;
	if (didPulse && driver->numWumpas > 0)
	{
	      driver->numWumpas -= 1;
	}

}

u32 Chaos_ApplyDriver(struct Driver *driver, const struct GamepadBuffer *pad, int *steering, u32 actions)
{
	const struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_IsRunning(sdata->gGT) || (driver->actionsFlagSet & ACTION_RACE_FINISHED) != 0)
	{
		return actions;
	}
	for (int i = 0; i < runtime->numActive; i++)
	{
		const ChaosEffectId effect = runtime->active[i].effect;
		const ChaosEffectDef *def = ChaosEffect_Def(effect);
		if (def == NULL || !ChaosEffect_AppliesToDriver(def->scope, driver->driverID, sdata->gGT->numPlyrCurrGame))
		{
			continue;
		}
		if (effect == CHAOS_EFFECT_INVERTED_STEERING)
		{
			const int center = pad->rwd != NULL ? pad->rwd->gamepadCenter : CHAOS_STICK_CENTER;
			// Resolve from the baseline so duplicates do not invert twice; keep 256 signed.
			*steering = 2 * center - pad->stickLX;
		}
		else if (effect == CHAOS_EFFECT_WOBBLE_STEERING)
		{
			// Resolve from live input so repeated Wobble activations do not add waves.
			*steering = Chaos_WobbleSteering(&runtime->active[i], pad);
		}
		else if (effect == CHAOS_EFFECT_NO_HOPPING)
		{
			driver->jump_TenBuffer = 0;
			actions &= ~ACTION_JUMP_BUTTON_HELD;
		}
		else if (effect == CHAOS_EFFECT_NO_RESERVES)
		{
			Chaos_ApplyReserveRules(driver);
		}
		else if (effect == CHAOS_EFFECT_WUMPA_DRAIN)
		{
			Chaos_TickWumpaDrain(runtime->active[i].elapsedMS, driver);
		}
	}
	return actions;
}

static void Chaos_TickActivations(s32 elapsedMS)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	int kept = 0;
	for (int i = 0; i < runtime->numActive; i++)
	{
		struct ChaosActivation entry = runtime->active[i];
		if (entry.remainingMS > elapsedMS)
		{
			entry.remainingMS -= elapsedMS;
			entry.elapsedMS += elapsedMS;
			entry.noticeRemainingMS -= elapsedMS;
			if (entry.noticeRemainingMS < 0)
			{
				entry.noticeRemainingMS = 0;
			}
			runtime->active[kept++] = entry;
		}
	}
	memset(&runtime->active[kept], 0, (runtime->numActive - kept) * sizeof(runtime->active[0]));
	runtime->numActive = kept;
}

void Chaos_Update(struct GameTracker *gGT)
{
	if (!Chaos_IsRunning(gGT) || gGT->elapsedTimeMS <= 0)
	{
		return;
	}
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	Chaos_TickActivations(gGT->elapsedTimeMS);

	if ((settings->trigger != CHAOS_TRIGGER_INTERVAL && settings->trigger != CHAOS_TRIGGER_RANDOM) ||
	    settings->triggerIntervalSeconds == 0)
	{
		return;
	}
	const s32 durationMS = settings->durationSeconds * CHAOS_MILLISECONDS_PER_SECOND;
	runtime->intervalRemainingMS -= gGT->elapsedTimeMS;
	while (runtime->intervalRemainingMS <= 0)
	{
		if (durationMS > 0 && runtime->numActive < CHAOS_MAX_ACTIVATIONS)
		{
			const ChaosEffectId effect = ChaosEffect_Pick(settings->effects, settings->enabledCount, &sdata->chaosRng);
			Chaos_Activate(effect, durationMS);
		}
		runtime->intervalRemainingMS += Chaos_NextTriggerGap();
	}
}
