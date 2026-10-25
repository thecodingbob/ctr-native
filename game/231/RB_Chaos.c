#include <common.h>
#include <chaos_gate.h>
#include <platform/native_chaos_ipc.h>
#include <platform/native_log.h>

enum
{
	CHAOS_RANDOM_GAP_MIN_PERCENT = 30,
};

static bool Chaos_TargetsDriver(ChaosEffectId effect, const struct Driver *driver)
{
	const ChaosEffectDef *def = ChaosEffect_Def(effect);
	return driver != NULL && def != NULL &&
	       (driver->actionsFlagSet & ACTION_RACE_FINISHED) == 0 &&
	       ChaosEffect_AppliesToDriver(def->scope, driver->driverID, sdata->gGT->numPlyrCurrGame);
}

static bool Chaos_EffectIsEligible(ChaosEffectId effect)
{
	if (ChaosEffect_Def(effect) == NULL)
	{
		return false;
	}
	for (int i = 0; i < (int)len(sdata->gGT->drivers); i++)
	{
		if (Chaos_TargetsDriver(effect, sdata->gGT->drivers[i]))
		{
			return true;
		}
	}
	return false;
}

static ChaosEffectId Chaos_PickEligibleEffect(void)
{
	bool eligible[CHAOS_EFFECT_COUNT];
	for (int id = 0; id < CHAOS_EFFECT_COUNT; id++)
	{
		eligible[id] = sdata->chaosSettings.effects[id] && Chaos_EffectIsEligible((ChaosEffectId)id);
	}
	return ChaosEffect_Pick(eligible, ChaosEffect_CountEnabled(eligible), &sdata->chaosRng);
}

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
	if (sdata->chaosRuntime.raceInitialized)
	{
		Chaos_SetKartScale(CHAOS_KART_SCALE_NORMAL);
	}
	memset(&sdata->chaosRuntime, 0, sizeof(sdata->chaosRuntime));
}

void Chaos_BeginRace(struct GameTracker *gGT)
{
	Chaos_Reset();
	Chaos_PublishEffects();
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

static void Chaos_OnEffectDeactivated(ChaosEffectId effect)
{
	if (effect == CHAOS_EFFECT_GIANT_KARTS)
	{
		Chaos_SetKartScale(CHAOS_KART_SCALE_NORMAL);
	}
}

static void Chaos_RemoveActivation(int index)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	ChaosEffectId effect = runtime->active[index].effect;
	int remaining = runtime->numActive - index - 1;
	memmove(&runtime->active[index], &runtime->active[index + 1], remaining * sizeof(runtime->active[0]));
	runtime->active[--runtime->numActive] = (struct ChaosActivation){0};
	Chaos_OnEffectDeactivated(effect);
}

static void Chaos_RemoveReplacedEffects(ChaosEffectId effect)
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
	else if (effect == CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL || effect == CHAOS_EFFECT_RESERVE_INJECTION)
	{
		replaced = CHAOS_EFFECT_NO_RESERVES;
	}
	else if (effect == CHAOS_EFFECT_NO_RESERVES)
	{
		replaced = CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL;
	}
	else if (effect == CHAOS_EFFECT_USE_THIS)
	{
		replaced = CHAOS_EFFECT_NO_ITEMS;
	}
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	for (int i = 0; i < runtime->numActive;)
	{
		if (runtime->active[i].effect == effect || runtime->active[i].effect == replaced)
		{
			Chaos_RemoveActivation(i);
		}
		else
		{
			i++;
		}
	}
}

static void Chaos_ApplyOneOff(const struct ChaosActivation *chaosActivation)
{
	const ChaosEffectId effect = chaosActivation->effect;
	if (effect == CHAOS_EFFECT_GIANT_KARTS)
	{
		Chaos_SetKartScale(CHAOS_GIANT_SCALE);
		return;
	}
	if (effect == CHAOS_EFFECT_CLOCK_CURSE)
	{
		OtherFX_Play(SOUND_CLOCK, 1);
	}
	for (int i = 0; i < (int)len(sdata->gGT->drivers); i++)
	{
		struct Driver *driver = sdata->gGT->drivers[i];
		if (!Chaos_TargetsDriver(effect, driver))
		{
			continue;
		}
		switch ((int)effect)
		{
		case CHAOS_EFFECT_USE_THIS:
			Chaos_GiveRandomItem(driver);
			break;
		case CHAOS_EFFECT_JUICED_UP:
			Chaos_GiveWumpas(driver);
			break;
		case CHAOS_EFFECT_IT_IS_I:
			Chaos_GiveMask(driver);
			break;
		case CHAOS_EFFECT_PROVIDENCE:
			Chaos_GiveShield(driver);
			break;
		case CHAOS_EFFECT_RESERVE_INJECTION:
			Chaos_GiveReserves(driver);
			break;
		case CHAOS_EFFECT_CLOCK_CURSE:
			VehPickState_ApplyDamage(driver, VEH_PICK_DAMAGE_SPIN, NULL, 0, true);
			driver->clockReceive = chaosActivation->remainingMS;
			break;
		default:
			break;
		}
	}
}

static void Chaos_EnqueueInstantNotice(const struct ChaosActivation *activation)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	// Refresh duplicates; overflow drops the oldest banner.
	for (int i = 0; i < runtime->numInstantNotices; i++)
	{
		if (runtime->instantNotices[i].effect == activation->effect)
		{
			memmove(&runtime->instantNotices[i], &runtime->instantNotices[i + 1],
			        (--runtime->numInstantNotices - i) * sizeof(*activation));
			break;
		}
	}
	if (runtime->numInstantNotices == CHAOS_MAX_INSTANT_NOTICES)
	{
		memmove(runtime->instantNotices, runtime->instantNotices + 1,
		        (--runtime->numInstantNotices) * sizeof(*activation));
	}
	runtime->instantNotices[runtime->numInstantNotices++] = *activation;
}

bool Chaos_Activate(ChaosEffectId effect, s32 durationMS)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	if (!Chaos_IsRunning(sdata->gGT) || durationMS <= 0 ||
	    !ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect) || !Chaos_EffectIsEligible(effect))
	{
		return false;
	}
	Chaos_RemoveReplacedEffects(effect);
	const bool instant = ChaosEffect_IsInstant(effect);
	if (!instant && runtime->numActive == CHAOS_MAX_ACTIVATIONS)
	{
		return false;
	}
	struct ChaosActivation activation = {
		.effect = effect,
		.remainingMS = durationMS,
		.noticeRemainingMS = CHAOS_ACTIVATION_NOTICE_MS + (instant ? 0 : CHAOS_ACTIVATION_DOCK_MS),
		.elapsedMS = 0,
	};
	if (instant)
	{
		Chaos_EnqueueInstantNotice(&activation);
	}
	else
	{
		runtime->active[runtime->numActive++] = activation;
	}
	Chaos_ApplyOneOff(&activation);
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

void Chaos_PublishEffects(void)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	char catalogue[NATIVE_CHAOS_CATALOGUE_SIZE] = {0};
	size_t used = 0;
	const struct GameTracker *gGT = sdata->gGT;
	const bool eligibleRace = gGT != NULL &&
	    ChaosGate_IsRaceEligible(gGT->gameMode1 & ~(PAUSE_ALL | LOADING),
	                             gGT->overlayIndex_Threads, true, CHAOS_GATE_ADVENTURE_ONLY);
	for (int i = 0; settings->armed && eligibleRace && i < CHAOS_EFFECT_COUNT; i++)
	{
		if (settings->effects[i] && Chaos_EffectIsEligible((ChaosEffectId)i))
		{
			const char *slug = ChaosEffect_Def((ChaosEffectId)i)->slug;
			const size_t length = strlen(slug);
			if (length + 1 >= sizeof(catalogue) - used)
			{
				Platform_LogError("[Chaos] Effects catalogue exceeds publication capacity.\n");
				return;
			}
			memcpy(catalogue + used, slug, length);
			used += length;
			catalogue[used++] = '\n';
		}
	}
	NativeChaosIPC_WriteEffectsCatalogue(catalogue);
}

static bool Chaos_ActivateExternalEffect(const char *slug)
{
	const ChaosEffectId effect = ChaosEffect_FromSlug(slug);
	if (effect == CHAOS_EFFECT_NONE)
	{
		Platform_LogError("[Chaos] Consumed invalid or unknown effect command.\n");
		return false;
	}
	if (!ChaosEffect_IsEnabled(sdata->chaosSettings.effects, effect))
	{
		Platform_LogError("[Chaos] Consumed disabled effect: %s\n", slug);
		return false;
	}
	if (!Chaos_Activate(effect, sdata->chaosSettings.durationSeconds * CHAOS_MILLISECONDS_PER_SECOND))
	{
		Platform_LogError("[Chaos] Consumed effect rejected by runtime: %s\n", slug);
		return false;
	}
	return true;
}

static void Chaos_PollExternal(void)
{
	if (sdata->chaosRuntime.intervalRemainingMS > 0)
	{
		return;
	}
	char command[NATIVE_CHAOS_COMMAND_SIZE];
	if (NativeChaosIPC_ReadCommand(command, sizeof(command)) == NATIVE_CHAOS_COMMAND &&
	    Chaos_ActivateExternalEffect(command))
	{
		sdata->chaosRuntime.intervalRemainingMS = Chaos_NextTriggerGap();
	}
}

bool Chaos_IsActiveForDriver(ChaosEffectId effect, const struct Driver *driver)
{
	return Chaos_TargetsDriver(effect, driver) && Chaos_IsActive(effect);
}

bool Chaos_OnPickup(struct Driver *driver)
{
	const struct ChaosSettings *settings = &sdata->chaosSettings;
	if (settings->trigger != CHAOS_TRIGGER_PICKUP || !Chaos_IsRunning(sdata->gGT) ||
	    driver == NULL || (driver->actionsFlagSet & (ACTION_RACE_FINISHED | ACTION_BOT)) != 0 ||
	    !ChaosEffect_AppliesToDriver(CHAOS_SCOPE_HUMANS, driver->driverID, sdata->gGT->numPlyrCurrGame) ||
	    settings->durationSeconds == 0)
	{
		return false;
	}
	const ChaosEffectId effect = Chaos_PickEligibleEffect();
	return Chaos_Activate(effect, settings->durationSeconds * CHAOS_MILLISECONDS_PER_SECOND);
}

void Chaos_OnTurboPadContact(struct Driver *driver, bool touching)
{
	if (sdata->chaosSettings.trigger != CHAOS_TRIGGER_PICKUP || !Chaos_IsRunning(sdata->gGT) || driver == NULL ||
	    !ChaosEffect_AppliesToDriver(CHAOS_SCOPE_HUMANS, driver->driverID, sdata->gGT->numPlyrCurrGame) ||
	    driver->driverID >= ADV_MULTIPLAYER_NUM_PLAYERS_MAX)
	{
		return;
	}
	bool *wasTouching = &sdata->chaosRuntime.touchingTurboPad[driver->driverID];
	const bool entered = touching && !*wasTouching;
	*wasTouching = touching;
	if (entered)
	{
		Chaos_OnPickup(driver);
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
			// Keep 256 signed when mirroring the local input.
			*steering = 2 * center - pad->stickLX;
		}
		else if (effect == CHAOS_EFFECT_WOBBLE_STEERING)
		{
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
	for (int i = 0; i < runtime->numInstantNotices;)
	{
		struct ChaosActivation *notice = &runtime->instantNotices[i];
		notice->elapsedMS += elapsedMS;
		notice->noticeRemainingMS -= elapsedMS;
		if (notice->noticeRemainingMS <= 0)
		{
			memmove(notice, notice + 1, (--runtime->numInstantNotices - i) * sizeof(*notice));
			memset(&runtime->instantNotices[runtime->numInstantNotices], 0, sizeof(*notice));
		}
		else
		{
			i++;
		}
	}
	for (int i = 0; i < runtime->numActive;)
	{
		struct ChaosActivation *entry = &runtime->active[i];
		if (entry->remainingMS <= elapsedMS)
		{
			Chaos_RemoveActivation(i);
			continue;
		}
		entry->remainingMS -= elapsedMS;
		entry->elapsedMS += elapsedMS;
		entry->noticeRemainingMS -= elapsedMS;
		if (entry->noticeRemainingMS < 0)
		{
			entry->noticeRemainingMS = 0;
		}
		i++;
	}
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
	if (settings->trigger == CHAOS_TRIGGER_EXTERNAL)
	{
		if (runtime->intervalRemainingMS > 0)
		{
			runtime->intervalRemainingMS -= gGT->elapsedTimeMS;
			if (runtime->intervalRemainingMS < 0)
			{
				runtime->intervalRemainingMS = 0;
			}
		}
		Chaos_PollExternal();
		return;
	}

	if ((settings->trigger != CHAOS_TRIGGER_INTERVAL && settings->trigger != CHAOS_TRIGGER_RANDOM) ||
	    settings->triggerIntervalSeconds == 0)
	{
		return;
	}
	const s32 durationMS = settings->durationSeconds * CHAOS_MILLISECONDS_PER_SECOND;
	runtime->intervalRemainingMS -= gGT->elapsedTimeMS;
	while (runtime->intervalRemainingMS <= 0)
	{
		if (durationMS > 0)
		{
			const ChaosEffectId effect = Chaos_PickEligibleEffect();
			Chaos_Activate(effect, durationMS);
		}
		runtime->intervalRemainingMS += Chaos_NextTriggerGap();
	}
}
