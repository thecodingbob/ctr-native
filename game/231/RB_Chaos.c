#include <common.h>
#include <chaos_gate.h>

static bool Chaos_CanRun(const struct GameTracker *gGT)
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
	sdata->chaosRuntime.intervalRemainingMS =
	    sdata->chaosSettings.triggerIntervalSeconds * CHAOS_MILLISECONDS_PER_SECOND;
}

bool Chaos_Activate(ChaosEffectId effect, s32 durationMS)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_CanRun(sdata->gGT) || durationMS <= 0 ||
	    !ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect) ||
	    runtime->numActive == CHAOS_MAX_ACTIVATIONS)
	{
		return false;
	}
	runtime->active[runtime->numActive++] = (struct ChaosActivation){effect, durationMS};
	return true;
}

bool Chaos_IsActive(ChaosEffectId effect)
{
	if (ChaosEffect_Def(effect) == NULL || !Chaos_CanRun(sdata->gGT))
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
			runtime->active[kept++] = entry;
		}
	}
	memset(&runtime->active[kept], 0, (runtime->numActive - kept) * sizeof(runtime->active[0]));
	runtime->numActive = kept;
}

void Chaos_Update(struct GameTracker *gGT)
{
	if (!Chaos_CanRun(gGT) || gGT->elapsedTimeMS <= 0)
	{
		return;
	}
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	Chaos_TickActivations(gGT->elapsedTimeMS);

	if (settings->trigger != CHAOS_TRIGGER_INTERVAL || settings->triggerIntervalSeconds == 0)
	{
		return;
	}
	const s32 intervalMS = settings->triggerIntervalSeconds * CHAOS_MILLISECONDS_PER_SECOND;
	const s32 durationMS = settings->durationSeconds * CHAOS_MILLISECONDS_PER_SECOND;
	runtime->intervalRemainingMS -= gGT->elapsedTimeMS;
	while (runtime->intervalRemainingMS <= 0)
	{
		runtime->intervalRemainingMS += intervalMS;
		if (durationMS > 0 && runtime->numActive < CHAOS_MAX_ACTIVATIONS)
		{
			const ChaosEffectId effect = ChaosEffect_Pick(settings->effects, settings->enabledCount, &sdata->chaosRng);
			Chaos_Activate(effect, durationMS);
		}
	}
}
