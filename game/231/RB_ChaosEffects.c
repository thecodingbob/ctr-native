#include <common.h>

// Effect implementations; RB_Chaos.c owns activation, dispatch and timing.
static bool Chaos_TargetsDriver(ChaosEffectId effect, const struct Driver *driver);

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

static void Chaos_SetKartScale(u8 scale)
{
	struct GameTracker *gGT = sdata->gGT;
	for (int i = 0; i < gGT->numPlyrCurrGame && i < ADV_MULTIPLAYER_NUM_PLAYERS_MAX; i++)
	{
		gGT->drivers[i]->kartScale = scale;
	}
}

static void Chaos_GiveRandomItem(struct Driver *driver)
{
	VehPhysGeneral_GiveRandomItem(driver, ChaosEffect_NextRandom(&sdata->chaosRng));
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
		VehFire_SetReserves(driver, CHAOS_INJECTION_RESERVES);
	}
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
	if (driver->reserves != 0 && Chaos_IsActiveForDriver(CHAOS_EFFECT_DEFECTIVE_BOOSTS, driver))
	{
		// Retail already added the fire bonus; subtract it twice to invert it.
		speed -= 2 * driver->fireSpeedCap;
		if (speed <= 0)
		{
			return 0;
		}
	}
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_GIANT_KARTS, driver))
	{
		speed = speed * CHAOS_GIANT_SPEED_PERCENT / CHAOS_PERCENT_SCALE;
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
		speed *= CHAOS_SUPER_SPEED_MULTIPLIER;
	}
	return speed;
}

int Chaos_JumpForceForDriver(const struct Driver *driver, int force)
{
	return Chaos_IsActiveForDriver(CHAOS_EFFECT_GIANT_KARTS, driver) ?
	       force * CHAOS_GIANT_SPEED_PERCENT / CHAOS_PERCENT_SCALE : force;
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

void Chaos_ApplySurfaceRules(const struct Driver *driver, struct Scrub *scrub)
{
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, driver))
	{
		scrub->flags |= SCRUB_FLAG_KEEP_RESERVES;
	}
}

int Chaos_ReservesForDriver(const struct Driver *driver, int reserves)
{
	if (Chaos_IsRunning(sdata->gGT) && Chaos_TargetsDriver(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL, driver))
	{
		// Reserve Injection fills the signed timer; subsequent grants must not wrap it.
		if (reserves > CHAOS_RESERVES_MAX)
		{
			reserves = CHAOS_RESERVES_MAX;
		}
		if (reserves < driver->reserves && Chaos_IsActive(CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL))
		{
			return driver->reserves;
		}
	}
	return reserves;
}

void Chaos_ApplyRescueRules(struct Driver *driver)
{
	if (!Chaos_IsActiveForDriver(CHAOS_EFFECT_ANXIOUS_MASK, driver))
	{
		return;
	}
	const struct Terrain *terrain = VehAfterColl_GetTerrain(driver->currBlockTouching->terrain_type);
	if (terrain->speedMultiplier < VEH_PHYS_FORCE_TERRAIN_SCALE_NEUTRAL)
	{
		driver->collisionFlags |= DRIVER_COLL_FLAG_MASK_GRAB_REQUEST;
	}
}

void Chaos_ApplyCrateRules(struct Driver *driver, struct Instance *crateInst)
{
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_EXPLOSIVE_CRATES, driver))
	{
		RB_Burst_InitVisual(crateInst);
		PlaySound3D(HOWL_SFX_TNT_EXPLOSION, crateInst);
		VehPickState_ApplyDamage(driver, VEH_PICK_DAMAGE_BLAST, NULL, 0, true);
	}
}

bool Chaos_IsHardCrateForDriver(const struct Instance *crateInst, const struct Thread *collidingTh,
                               const struct ScratchpadStruct *sps)
{
	return crateInst->scale.x == 0x1000 && sps->Input1.modelID == DYNAMIC_PLAYER &&
	       Chaos_IsActiveForDriver(CHAOS_EFFECT_HARD_CRATES, collidingTh->object);
}

void Chaos_ApplyItemRules(struct Driver *driver)
{
	if (Chaos_IsActiveForDriver(CHAOS_EFFECT_NO_ITEMS, driver))
	{
		VehPhysGeneral_ClearHeldItem(driver);
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
