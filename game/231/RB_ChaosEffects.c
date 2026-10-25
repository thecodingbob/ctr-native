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
};

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

static void Chaos_ApplyClockCurse(struct Driver *driver, s32 durationMS)
{
	VehPickState_ApplyDamage(driver, VEH_PICK_DAMAGE_SPIN, NULL, 0, true);
	driver->clockReceive = durationMS;
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

static u32 Chaos_PreventHopping(struct Driver *driver, u32 actions)
{
	driver->jump_TenBuffer = 0;
	return actions & ~ACTION_JUMP_BUTTON_HELD;
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
