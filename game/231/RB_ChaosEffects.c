#include <common.h>

// Effect operations only; RB_Chaos.c checks activation and driver eligibility.

enum
{
	CHAOS_PERCENT_SCALE = 100,
	CHAOS_STICK_CENTER = 0x80,
	CHAOS_STICK_RANGE = 0x80,
	CHAOS_WOBBLE_RANGE_NUMERATOR = 3,
	CHAOS_WOBBLE_RANGE_DENOMINATOR = 4,
	CHAOS_LOW_GRAVITY_DIVISOR = 4,
	CHAOS_HEAVY_GRAVITY_MULTIPLIER = 3,
	CHAOS_SPEED_CAP_DIVISOR = 2,
	CHAOS_SUPER_SPEED_MULTIPLIER = 2,
	CHAOS_KART_SCALE_NORMAL = 1,
	CHAOS_GIANT_SCALE = 2,
	CHAOS_GIANT_SPEED_PERCENT = 130,
	CHAOS_THIRD_DRIFT_FIRE_LEVEL = 0x80,
	CHAOS_WUMPA_PICKUP_FRAMES = 5,
	CHAOS_LAUNCH_JUMP_MULTIPLIER = 6,
	CHAOS_ANALOG_ACTION_DEAD_ZONE = 0x30,
};

static const u32 s_chaosControlButtons[CHAOS_CONTROL_COUNT] = {
	BTN_CROSS_one, BTN_SQUARE_one, BTN_CIRCLE, BTN_L1, BTN_R1,
};

static u32 Chaos_AnalogActions(const struct GamepadBuffer *pad)
{
	u32 actions = 0;
	if (pad->stickRY < CHAOS_STICK_CENTER - CHAOS_ANALOG_ACTION_DEAD_ZONE)
	{
		actions |= BTN_CROSS_one;
	}
	if (pad->stickRY > CHAOS_STICK_CENTER + CHAOS_ANALOG_ACTION_DEAD_ZONE ||
	    pad->stickLY > CHAOS_STICK_CENTER + CHAOS_ANALOG_ACTION_DEAD_ZONE)
	{
		actions |= BTN_SQUARE_one;
	}
	return actions;
}

static void Chaos_ShuffleControls(struct ChaosControlShuffle *controls, const struct GamepadBuffer *pad)
{
	bool changed;
	do
	{
		for (int i = 0; i < CHAOS_CONTROL_COUNT; i++)
		{
			controls->destination[i] = i;
		}
		for (int i = CHAOS_CONTROL_COUNT - 1; i > 0; i--)
		{
			const int other = ChaosEffect_NextRandom(&sdata->chaosRng) % (i + 1);
			const u8 saved = controls->destination[i];
			controls->destination[i] = controls->destination[other];
			controls->destination[other] = saved;
		}
		changed = false;
		for (int i = 0; i < CHAOS_CONTROL_COUNT; i++)
		{
			changed |= controls->destination[i] != i;
		}
	} while (!changed);
	controls->analogHeld = Chaos_AnalogActions(pad);
	controls->analogPrev = controls->analogHeld;
}

static u32 Chaos_RemapButtons(const struct ChaosControlShuffle *controls, u32 buttons)
{
	const u32 mask = BTN_CROSS_one | BTN_SQUARE_one | BTN_CIRCLE | BTN_L1 | BTN_R1;
	u32 remapped = buttons & ~mask;
	for (int i = 0; i < CHAOS_CONTROL_COUNT; i++)
	{
		if ((buttons & s_chaosControlButtons[i]) != 0)
		{
			remapped |= s_chaosControlButtons[controls->destination[i]];
		}
	}
	return remapped;
}

static void Chaos_RemapControls(const struct ChaosControlShuffle *controls, struct GamepadBuffer *pad)
{
	const u32 mask = BTN_CROSS_one | BTN_SQUARE_one | BTN_CIRCLE | BTN_L1 | BTN_R1;
	const u32 held = pad->buttonsHeldCurrFrame | controls->analogHeld;
	const u32 previous = pad->buttonsHeldPrevFrame | controls->analogPrev;
	pad->buttonsHeldCurrFrame = Chaos_RemapButtons(controls, held);
	pad->buttonsHeldPrevFrame = Chaos_RemapButtons(controls, previous);
	pad->buttonsTapped = Chaos_RemapButtons(controls, (pad->buttonsTapped & ~mask) | (held & ~previous & mask));
	pad->buttonsReleased = Chaos_RemapButtons(controls, (pad->buttonsReleased & ~mask) | (previous & ~held & mask));
	pad->stickRY = CHAOS_STICK_CENTER;
	if (pad->stickLY > CHAOS_STICK_CENTER)
	{
		pad->stickLY = CHAOS_STICK_CENTER;
	}
}

static void Chaos_SetKartScale(u8 scale, struct Driver *driver)
{
	driver->kartScale = scale;
}

static void Chaos_ClearItems(struct Driver *driver)
{
	VehPhysProc_RemoveHeldItem(driver);
	driver->numHeldItems = 0;
	driver->itemRollTimer = 0;
	driver->noItemTimer = 0;
	driver->maskIsAku = -1;
	driver->actionsFlagSet &= ~ACTION_WEAPON_FIRE_REQUEST;
}

static void Chaos_GiveRandomItem(struct Driver *driver)
{
	// Distinct retail Race2 items, with the orb last so it can be excluded.
	static const u8 items[] = {
		HELD_ITEM_TURBO, HELD_ITEM_BOMB_1X, HELD_ITEM_MISSILE_1X,
		HELD_ITEM_TNT, HELD_ITEM_POTION, HELD_ITEM_SHIELD, HELD_ITEM_MASK,
		HELD_ITEM_CLOCK, HELD_ITEM_BOMB_3X, HELD_ITEM_MISSILE_3X, HELD_ITEM_WARPBALL,
	};
	Chaos_ClearItems(driver);
	struct GameTracker *gGT = GAME_TRACKER;
	const u32 random = ChaosEffect_NextRandom(&sdata->chaosRng);
	const int count = len(items) - ((gGT->gameMode1 & WARPBALL_HELD) != 0);
	VehPhysProc_AssignHeldItem(driver, items[random % count]);
	if (driver->heldItemID == HELD_ITEM_MASK)
	{
		driver->maskIsAku = g_config.maskMode == MASK_MODE_RANDOM ? (s8)((random >> 16) & 1) :
		                    (s8)VehPickupItem_ApplyMaskMode(driver);
	}
	VehPhysProc_PlayItemAcquireSound(driver);
}

static void Chaos_GiveWumpas(struct Driver *driver)
{
	const int missing = DRIVER_WUMPA_JUICED_COUNT - driver->numWumpas;
	if (missing <= 0)
	{
		return;
	}
	if ((sdata->gGT->gameMode1 & CRYSTAL_CHALLENGE) != 0)
	{
		// Crystal events use this pickup queue for crystals, not Wumpas.
		RB_Player_ModifyWumpa(driver, missing);
		return;
	}
	if (driver->PickupWumpaHUD.numCollected < missing)
	{
		if (driver->PickupWumpaHUD.numCollected == 0)
		{
			const RECT *viewport = &sdata->gGT->pushBuffer[driver->driverID].rect;
			driver->PickupWumpaHUD.startX = viewport->x + viewport->w / 2;
			driver->PickupWumpaHUD.startY = viewport->y + viewport->h / 2;
			driver->PickupWumpaHUD.cooldown = CHAOS_WUMPA_PICKUP_FRAMES;
		}
		driver->PickupWumpaHUD.numCollected = missing;
	}
}

static void Chaos_GiveMask(struct Driver *driver)
{
	const s8 heldMask = driver->maskIsAku;
	driver->maskIsAku = -1;
	VehPickupItem_MaskUseWeapon(driver, true);
	driver->maskIsAku = heldMask;
}

static void Chaos_GiveShield(struct Driver *driver)
{
	VehPickupItem_ShootNow(driver, HELD_ITEM_SHIELD, 0);
}

static void Chaos_GiveReserves(struct Driver *driver)
{
	if (driver->reserves <= 0 || driver->fireSpeedCap <= 0)
	{
		VehFire_Increment(driver, 0, POWER_SLIDE_HANG_TIME, CHAOS_THIRD_DRIFT_FIRE_LEVEL);
	}
	if (driver->reserves < CHAOS_INJECTION_RESERVES)
	{
		driver->reserves = CHAOS_INJECTION_RESERVES;
	}
}

static void Chaos_UpgradeTurbo(struct Driver *driver)
{
	if (driver->reserves > 0 && driver->fireSpeedCap > 0)
	{
		struct Turbo *turbo = NULL;
		for (struct Thread *thread = GAME_TRACKER->threadBuckets[TURBO].thread; thread != NULL; thread = thread->siblingThread)
		{
			if (((struct Turbo *)thread->object)->driver == driver)
			{
				turbo = thread->object;
				break;
			}
		}
		VehFire_SetLevel(driver, turbo, VEH_PHYS_FORCE_SUPER_TURBO_PAD_FIRE_LEVEL);
	}
}

static void Chaos_ApplyClockCurse(struct Driver *driver, s32 durationMS)
{
	VehPickState_ApplyDamage(driver, VEH_PICK_DAMAGE_SPIN, NULL, 0, true);
	driver->clockReceive = durationMS;
}

static void Chaos_LaunchDriver(struct Driver *driver)
{
	Vec3 velocity;
	VehPhysForce_ConvertSpeedToVec(driver, &velocity);
	velocity.y += Chaos_JumpForceForDriver(driver, driver->const_JumpForce) * CHAOS_LAUNCH_JUMP_MULTIPLIER;
	VehPhysCrash_ConvertVecToSpeed(driver, &velocity);
	driver->velocity = velocity;
}

static int Chaos_LowGravity(int gravity)
{
	return gravity / CHAOS_LOW_GRAVITY_DIVISOR;
}

static int Chaos_HeavyGravity(int gravity)
{
	return gravity * CHAOS_HEAVY_GRAVITY_MULTIPLIER;
}

static int Chaos_DefectiveBoostSpeed(const struct Driver *driver, int speed)
{
	if (driver->reserves != 0)
	{
		// Retail already added the fire bonus; subtract it twice to invert it.
		speed -= 2 * driver->fireSpeedCap;
	}
	return speed > 0 ? speed : 0;
}

static int Chaos_GiantForce(int force)
{
	return force * CHAOS_GIANT_SPEED_PERCENT / CHAOS_PERCENT_SCALE;
}

static int Chaos_CappedSpeed(const struct Driver *driver, int speed)
{
	const int cap = driver->const_Speed_ClassStat / CHAOS_SPEED_CAP_DIVISOR;
	return speed > cap ? cap : speed;
}

static int Chaos_SuperSpeed(int speed)
{
	return speed * CHAOS_SUPER_SPEED_MULTIPLIER;
}

static void Chaos_ClearReserves(struct Driver *driver)
{
	driver->reserves = 0;
	driver->turbo_outsideTimer = 0;
}

static int Chaos_KeepReserves(const struct Driver *driver, int reserves)
{
	return reserves < driver->reserves ? driver->reserves : reserves;
}

static void Chaos_RequestOffroadRescue(struct Driver *driver)
{
	const struct Terrain *terrain = VehAfterColl_GetTerrain(driver->currBlockTouching->terrain_type);
	if (terrain->speedMultiplier < VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL)
	{
		driver->collisionFlags |= DRIVER_COLL_FLAG_MASK_GRAB_REQUEST;
	}
}

static void Chaos_ExplodeCrate(struct Driver *driver, struct Instance *crateInst)
{
	RB_Burst_InitVisual(crateInst);
	PlaySound3D(HOWL_SFX_TNT_EXPLOSION, crateInst);
	VehPickState_ApplyDamage(driver, VEH_PICK_DAMAGE_BLAST, NULL, 0, true);
}

static int Chaos_InvertSteering(const struct GamepadBuffer *pad)
{
	const int center = pad->rwd != NULL ? pad->rwd->gamepadCenter : CHAOS_STICK_CENTER;
	// Keep 256 signed when mirroring the local input.
	return 2 * center - pad->stickLX;
}

static void Chaos_DisableBraking(struct GamepadBuffer *pad)
{
	pad->buttonsHeldCurrFrame &= ~BTN_SQUARE_one;
	if (pad->stickRY > CHAOS_STICK_CENTER)
	{
		pad->stickRY = CHAOS_STICK_CENTER;
	}
	if (pad->stickLY > CHAOS_STICK_CENTER)
	{
		pad->stickLY = CHAOS_STICK_CENTER;
	}
}

static u32 Chaos_PreventHopping(struct Driver *driver, u32 actions)
{
	driver->jump_TenBuffer = 0;
	return actions & ~ACTION_JUMP_BUTTON_HELD;
}

static void Chaos_RequestHop(struct Driver *driver)
{
	// Renew for this physics step; normal jump eligibility and force stay intact.
	if (driver->jump_TenBuffer == 0)
	{
		driver->jump_TenBuffer = 1;
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
