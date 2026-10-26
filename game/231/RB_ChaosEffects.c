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
	CHAOS_SMALL_FORCE_PERCENT = 70,
	CHAOS_GIANT_SPEED_PERCENT = 130,
	CHAOS_THIRD_DRIFT_FIRE_LEVEL = 0x80,
	CHAOS_WUMPA_PICKUP_FRAMES = 5,
	CHAOS_LAUNCH_JUMP_MULTIPLIER = 6,
	CHAOS_ANALOG_ACTION_DEAD_ZONE = 0x30,
	CHAOS_ENGINE_AUDIO_CLASS_STRIDE = 4,
	CHAOS_TOP_DOWN_HEIGHT = 0xa00,
	CHAOS_TOP_DOWN_PITCH = 0x400,
	CHAOS_TOP_DOWN_YAW_OFFSET = 0x800,
	CHAOS_TOP_DOWN_CUTAWAY_HEIGHT = 0x80,
	CHAOS_WRONG_WAY_CADENCE_MULTIPLIER = 2,
};

static const float s_chaosSmallScale = 0.5f;

static void Chaos_PlaySenileVoice(struct Driver *driver)
{
	Voiceline_PlayWrongWay(driver);
	sdata->voicelineCooldown /= CHAOS_WRONG_WAY_CADENCE_MULTIPLIER;
}

static void Chaos_SetTopDownCamera(struct CameraDC *camera, const struct Driver *driver)
{
	struct PushBuffer *pushBuffer = camera->pushBuffer;
	const int x = driver->posCurr.x >> 8;
	const int y = driver->posCurr.y >> 8;
	const int z = driver->posCurr.z >> 8;
	const int height = y + CHAOS_TOP_DOWN_HEIGHT;
	pushBuffer->pos.x = (s16)x;
	pushBuffer->pos.y = (s16)(height > INT16_MAX ? INT16_MAX : height);
	pushBuffer->pos.z = (s16)z;
	pushBuffer->rot.x = CHAOS_TOP_DOWN_PITCH;
	pushBuffer->rot.y = (s16)(driver->angle + CHAOS_TOP_DOWN_YAW_OFFSET);
	pushBuffer->rot.z = 0;
	pushBuffer->distanceToScreen_PREV = pushBuffer->distanceToScreen_CURR;
	struct QuadBlock *ground = driver->underDriver != NULL ? driver->underDriver : driver->currBlockTouching;
	if (ground != NULL)
	{
		camera->ptrQuadBlock = ground;
		camera->quadBlockSearchHit = true;
	}
}

static bool Chaos_TopDownHidesQuadBlock(const struct Driver *driver, const struct QuadBlock *block, const struct LevVertex *vertices)
{
	if (block == driver->underDriver || block == driver->currBlockTouching)
	{
		return false;
	}
	// Same high-detail triangles and normal winding as COLL_FIXED_QUADBLK.
	static const u8 triangles[][3] = {
		{0, 4, 5}, {4, 6, 5}, {6, 4, 1}, {5, 6, 2},
		{8, 6, 7}, {7, 3, 8}, {1, 7, 6}, {2, 6, 8},
	};
	const int numTriangles = block->index[2] == block->index[3] ? 4 : (int)len(triangles);
	bool ceiling = false;
	for (int i = 0; i < numTriangles; i++)
	{
		const SVec3 *a = &vertices[block->index[triangles[i][0]]].pos;
		const SVec3 *b = &vertices[block->index[triangles[i][1]]].pos;
		const SVec3 *c = &vertices[block->index[triangles[i][2]]].pos;
		const s64 normalY = (s64)(c->z - a->z) * (b->x - a->x) - (s64)(c->x - a->x) * (b->z - a->z);
		if (normalY > 0)
		{
			// Keep any road-facing part, including unflagged ramps and mixed blocks.
			return false;
		}
		ceiling |= normalY < 0;
	}
	// Ceiling classification is independent of kart position and block bounds.
	return ceiling;
}

static bool Chaos_TopDownHidesInstance(const struct Driver *driver, const struct PushBuffer *pushBuffer,
                                     const struct Instance *instance, int nearestDepth, bool coversKart)
{
	if (!coversKart || instance->instDef == NULL || instance->thread != NULL)
	{
		return false;
	}
	const int ceiling = (driver->posCurr.y >> 8) + VehPhysForce_ScaleValue(driver, CHAOS_TOP_DOWN_CUTAWAY_HEIGHT);
	return nearestDepth < (pushBuffer->pos.y - ceiling) * 4;
}

static void Chaos_SetTopDownVisibility(struct VisMem *visibility, const struct mesh_info *mesh, int playerIndex)
{
	// Ground-level PVS omits road hidden behind crests and tunnel entrances.
	// Let the overhead camera's frustum select geometry instead.
	memset(visibility->visLeafList[playerIndex], 0xff, ((mesh->numBspNodes + 0x1f) >> 5) * sizeof(int));
	memset(visibility->visFaceList[playerIndex], 0xff, ((mesh->numQuadBlock + 0x1f) >> 5) * sizeof(int));
	// Force retail lists to be decoded again after expiry, even on the same block.
	visibility->visLeafSrc[playerIndex] = NULL;
	visibility->visFaceSrc[playerIndex] = NULL;
}

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

static void Chaos_SetKartScale(float scale, struct Driver *driver)
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

static void Chaos_NeutralRestartPose(struct Driver *driver)
{
	driver->matrixArray = BAKED_GTE_MATRIX_NONE;
	driver->matrixIndex = 0;
	driver->simpTurnState = 0;
	driver->ampTurnState = 0;
	driver->turnAngleCurr = 0;
	driver->turnAnglePrev = 0;
	driver->turnAngleLerpTarget = 0;
	driver->turnAngleLerpVel = 0;
	driver->rotationSpinRate = 0;
	driver->turnWobbleTimer = 0;
	driver->turnWobbleAngle = 0;
	driver->turnWobbleVelocity = 0;
	// Driving reconstructs visual yaw from angle plus drift/wobble offsets.
	driver->angle = ANG_MODULO_TWO_PI(driver->rotCurr.y);
	driver->rotCurr.x = 0;
	driver->rotCurr.z = 0;
	driver->rotCurr.w = 0;
	driver->rotPrev = driver->rotCurr;
	driver->instSelf->animIndex = 0;
	driver->instSelf->animFrame = VehFrameInst_GetStartFrame(0, VehFrameInst_GetNumAnimFrames(driver->instSelf, 0));
}

static void Chaos_EndRestartHold(struct ChaosRestart *restart, struct Driver *driver)
{
	if (restart->mask != NULL)
	{
		restart->mask->duration = 0;
		restart->mask = NULL;
	}
	if (restart->floating && driver->kartState == KS_ENGINE_REVVING)
	{
		Chaos_NeutralRestartPose(driver);
	}
	restart->floating = false;
}

static void Chaos_UpdateRestartMask(struct ChaosRestart *restart, const struct Driver *driver)
{
	if (restart->mask != NULL)
	{
		restart->mask->duration = MASK_HEAD_DURATION_NORMAL;
		restart->mask->pos = (SVec3){driver->posCurr.x >> 8, driver->posCurr.y >> 8, driver->posCurr.z >> 8};
	}
}

static void Chaos_RestartDriver(struct Driver *driver, struct ChaosRestart *restart)
{
	restart->active = true;
	restart->pending = false;
	restart->timer = DOT_LIGHT_TIMER_HIDE;
	restart->lightsPrevious = DOT_LIGHT_TIMER_HIDE;
	restart->cameraBlendPending = true;
	restart->floating = (driver->actionsFlagSet & ACTION_AIRBORNE) != 0;
	driver->posPrev = driver->posCurr;
	driver->velocity = (Vec3){0};
	driver->speed = 0;
	driver->speedApprox = 0;
	driver->baseSpeed = 0;
	driver->fireSpeed = 0;
	driver->ySpeed = 0;
	driver->reserves = 0;
	driver->turbo_outsideTimer = 0;
	driver->turbo_MeterRoomLeft = 0;
	driver->jump_TenBuffer = 0;
	driver->jump_ForcedMS = 0;
	driver->jump_InitialVelY = 0;
	driver->jump_HighJumpTimerMS = 0;
	driver->jump_LandingBoost = 0;
	driver->NoInputTimer = 0;
	Chaos_NeutralRestartPose(driver);
	driver->actionsFlagSet &= ~(ACTION_HIGH_JUMP | ACTION_JUMP_STARTED |
	    ACTION_JUMP_BUTTON_HELD | ACTION_BACK_SKID | ACTION_FRONT_SKID | ACTION_WARP);
	VehStuckProc_RevEngine_Init(driver->instSelf->thread, driver);
	if (restart->floating)
	{
		VehStuckProc_MaskGrab_SetFallPose(driver);
		restart->mask = VehPickupItem_MaskCreateRescue(driver);
		if (restart->mask != NULL)
		{
			restart->mask->rot.z |= MASK_HEAD_ROT_WORLD_SPACE;
			Chaos_UpdateRestartMask(restart, driver);
		}
	}
}

static void Chaos_TickRestart(struct ChaosRestart *restart, struct Driver *driver, s32 elapsedMS)
{
	restart->timer -= elapsedMS;
	Chaos_UpdateRestartMask(restart, driver);
	if (restart->timer <= 0 || (driver->actionsFlagSet & ACTION_RACE_FINISHED))
	{
		Chaos_EndRestartHold(restart, driver);
	}
	if (restart->timer < DOT_LIGHT_TIMER_SHOW_MIN || (driver->actionsFlagSet & ACTION_RACE_FINISHED))
	{
		restart->active = false;
	}
}

void Chaos_DrawStartLights(struct GameTracker *gGT)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	u32 drawn = 0;
	for (int i = 0; i < (int)len(runtime->restart); i++)
	{
		struct ChaosRestart *restart = &runtime->restart[i];
		if (!restart->active || (drawn & (1u << i)))
		{
			continue;
		}
		u32 players = 0;
		const s32 previous = restart->lightsPrevious;
		for (int j = i; j < (int)len(runtime->restart); j++)
		{
			struct ChaosRestart *other = &runtime->restart[j];
			if (other->active && other->timer == restart->timer && other->lightsPrevious == previous)
			{
				players |= 1u << j;
				other->lightsPrevious = other->timer;
			}
		}
		DotLights_Sequence(gGT, restart->timer, previous, players);
		drawn |= players;
	}
}

static void Chaos_SendWarpball(void)
{
	struct GameTracker *gGT = GAME_TRACKER;
	struct Driver *source = NULL;
	struct Driver *target = NULL;
	for (int rank = 0; rank < (int)len(gGT->driversInRaceOrder); rank++)
	{
		struct Driver *driver = gGT->driversInRaceOrder[rank];
		if (driver == NULL || (driver->actionsFlagSet & ACTION_RACE_FINISHED))
		{
			continue;
		}
		if (driver->driverID >= gGT->numPlyrCurrGame)
		{
			source = driver;
		}
		else if (target == NULL)
		{
			target = driver;
		}
	}
	if (source != NULL && target != NULL)
	{
		VehPickupItem_ShootWarpball(source, target);
	}
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

static void Chaos_ChangeEngineAudio(const struct Driver *driver, int previous, int next)
{
	EngineAudio_Stop(previous * CHAOS_ENGINE_AUDIO_CLASS_STRIDE + driver->driverID);
	EngineAudio_InitOnce(next * CHAOS_ENGINE_AUDIO_CLASS_STRIDE + driver->driverID, HOWL_SFX_CENTER_NO_DISTORTION);
}

static void Chaos_ChangeDrivingClass(struct Driver *driver)
{
	struct ChaosRuntime *runtime = &sdata->chaosRuntime;
	const int player = driver->driverID;
	const int current = Chaos_EngineClassForDriver(driver, GAME_CHARACTER_METADATA[GAME_CHARACTER_IDS[player]].engineID);
	int next = ChaosEffect_NextRandom(&sdata->chaosRng) % (NUM_CLASSES - 1);
	if (next >= current)
	{
		next++;
	}
	runtime->engineClass[player] = next;
	runtime->engineClassChanged[player] = true;
	VehBirth_SetConstsForClass(driver, next);
	Chaos_ChangeEngineAudio(driver, current, next);
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

static int Chaos_SmallForce(int force)
{
	return force * CHAOS_SMALL_FORCE_PERCENT / CHAOS_PERCENT_SCALE;
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
