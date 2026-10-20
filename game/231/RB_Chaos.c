#include <common.h>
#include <chaos_gate.h>

enum
{
	CHAOS_RANDOM_GAP_MIN_PERCENT = 30,
	CHAOS_PERCENT_SCALE = 100,
	CHAOS_STICK_CENTER = 0x80,
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

bool Chaos_Activate(ChaosEffectId effect, s32 durationMS)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_IsRunning(sdata->gGT) || durationMS <= 0 ||
	    !ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect) ||
	    runtime->numActive == CHAOS_MAX_ACTIVATIONS)
	{
		return false;
	}
	runtime->active[runtime->numActive++] = (struct ChaosActivation){
		.effect = effect,
		.remainingMS = durationMS,
		.noticeRemainingMS = CHAOS_ACTIVATION_NOTICE_MS + CHAOS_ACTIVATION_DOCK_MS,
	};
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
		else if (effect == CHAOS_EFFECT_NO_HOPPING)
		{
			driver->jump_TenBuffer = 0;
			actions &= ~ACTION_JUMP_BUTTON_HELD;
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
