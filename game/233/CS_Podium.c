#include <common.h>

enum PodiumDanceModelOffset
{
	PODIUM_DANCE_CRASH = STATIC_CRASHDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_CORTEX = STATIC_CORTEXDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_COCO = STATIC_COCODANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_NGIN = STATIC_NGINDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_POLAR = STATIC_POLARDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PURA = STATIC_PURADANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PINSTRIPE = STATIC_PINSTRIPEDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PAPU = STATIC_PAPUDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_ROO = STATIC_ROODANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_JOE = STATIC_JOEDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_NTROPY = STATIC_NTROPYDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_PENTA = STATIC_PENDANCE - STATIC_CRASHDANCE,
	PODIUM_DANCE_FAKE_CRASH = STATIC_FAKEDANCE - STATIC_CRASHDANCE,
};

enum PodiumPrizeConstants
{
	PODIUM_CAMERA_DISTANCE_TO_SCREEN = 0x100,
	PODIUM_VOLUME_BACKUP_MASK = 0xff,
	PODIUM_CONFETTI_PARTICLE_COUNT = 200,
	PODIUM_SCENE_SPAWN_Y_OFFSET = 0x80,
	PODIUM_THIRD_POS_X = 299,
	PODIUM_THIRD_POS_Y = -85,
	PODIUM_THIRD_POS_Z = 0,
	PODIUM_THIRD_YAW_OFFSET = 0x600,
	PODIUM_SECOND_POS_X = -299,
	PODIUM_SECOND_POS_Y = -42,
	PODIUM_SECOND_POS_Z = 0,
	PODIUM_SECOND_YAW_OFFSET = 0x200,
	PODIUM_FIRST_POS_X = 0,
	PODIUM_FIRST_POS_Y = 0,
	PODIUM_FIRST_POS_Z = 0,
	PODIUM_TAWNA_POS_X = 0x1a8,
	PODIUM_TAWNA_POS_Y = -128,
	PODIUM_TAWNA_POS_Z = 0x140,
	PODIUM_TAWNA_YAW_OFFSET = -0x2aa,
	PODIUM_VICTORY_CAMERA_THREAD_FLAGS = SIZE_RELATIVE_POOL_BUCKET(sizeof(struct CsPodiumCameraThreadObj), NONE, SMALL, CAMERA),
	PODIUM_PRIZE_HUD_CENTER_X = 0x100,
	PODIUM_PRIZE_HUD_CENTER_Y = 0x6c,
	PODIUM_PRIZE_HUD_Y_OFFSET = 0x3c,
	PODIUM_PRIZE_FLY_TO_HUD_FRAMES = 0xf,
	PODIUM_PRIZE_DEFAULT_TARGET_Z = -0x200,
	PODIUM_PRIZE_TROPHY_TARGET_Z = -200,
	PODIUM_PRIZE_ROT_Y_STEP = 100,
	PODIUM_PRIZE_SPEC_LIGHT_PHASE_STEP = 0x3f,
	PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT = 11,
	PODIUM_PRIZE_SCALE_DOWN_STEP = 0x4b0,
	PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD = FP_ONE + 1,
	PODIUM_PRIZE_BOUNCE_FRAME_COUNT = 5,
	PODIUM_PRIZE_BOUNCE_SCALE_UP_BASE = 800,
	PODIUM_PRIZE_BOUNCE_SCALE_UP_STEP = 400,
	PODIUM_PRIZE_BOUNCE_SCALE_DOWN_STEP = 800,
	PODIUM_PRIZE_BOUNCE_LIMIT_BASE = 0x2000,
	PODIUM_PRIZE_BOUNCE_LIMIT_STEP = 0x28a,
	PODIUM_PRIZE_HEIGHT_INTERP_STEP = 0x14,
	PODIUM_PRIZE_ORBIT_INTERP_STEP = 1,
	PODIUM_PRIZE_UI_DEPTH_BIAS = 0x80,
	PODIUM_PRIZE_INITIAL_SCALE = FP_ONE * 2,
	PODIUM_PRIZE_TROPHY_INITIAL_SCALE = FP_ONE * 4,
	PODIUM_PRIZE_ORBIT_RADIUS = 0x40,
	PODIUM_PRIZE_HEIGHT_OFFSET = 0x200,
	PODIUM_PRIZE_PODIUM_Y_OFFSET = 0x1c0,
	PODIUM_REWARD_UNLOCK_SFX = 0x67,
	PODIUM_PRIZE_STOP_DINGOFIRE_FX = 0xaf,
	PODIUM_PRIZE_STOP_AKUMOUTH_FX = 0xae,
	PODIUM_PRIZE_FLY_TO_HUD_SFX = 0x9a,
	PODIUM_GEM_SPEC_LIGHT_VERTICAL_START = 0x5d3,
	PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_START = 0x718,
	PODIUM_GEM_SPEC_LIGHT_VERTICAL_END = 0x590,
	PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_END = 0x609,
	PODIUM_RELIC_SPEC_LIGHT_VERTICAL_START = 0x2ab,
	PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_START = 0x436,
	PODIUM_RELIC_SPEC_LIGHT_VERTICAL_END = 0x1eb,
	PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_END = 0x670,
	PODIUM_KEY_SPEC_LIGHT_VERTICAL_START = 0x1d9,
	PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_START = 0x5db,
	PODIUM_KEY_SPEC_LIGHT_VERTICAL_END = 0x2da,
	PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_END = 0x54b,
	PODIUM_MUSIC_POLAR = 7,
	PODIUM_MUSIC_CORTEX = 8,
	PODIUM_MUSIC_ROO = 9,
	PODIUM_MUSIC_CRASH = 10,
	PODIUM_MUSIC_PINSTRIPE = 0xb,
	PODIUM_MUSIC_TINY = 0xc,
};

void CS_DestroyPodium_StartDriving(void)
{
	struct Instance *inst;
	struct Driver *d;
	struct GameTracker *gGT;
	struct Thread *t;

	// enable HUD
	GAME_TRACKER->hudFlags |= HUD_FLAG_RACE_HUD;
	t = GAME_TRACKER->threadBuckets[OTHER].thread;

	// loop through all threads
	while (t != NULL)
	{
		if (t->funcThDestroy != CS_Podium_Prize_ThDestroy)
		{
			t->flags |= THREAD_FLAG_DEAD;
		}

		t = t->siblingThread;
	}

	d = GAME_TRACKER->drivers[0];

	// enable collisions for thread,
	// and make instance visible
	inst = d->instSelf;
	inst->thread->flags &= ~THREAD_FLAG_DISABLE_COLLISION;
	inst->flags &= ~(HIDE_MODEL);

	d->kartState = KS_ENGINE_REVVING;
	d->funcPtrs[DRIVER_FUNC_INIT] = VehPhysProc_Driving_Init;

	// if cutscene changed audio, restore backup
	if (CS_CONTROLS_AUDIO != 0)
	{
		// restore backup of volume variables
		howl_VolumeSet(HOWL_VOLUME_TYPE_FX, CS_FX_VOLUME_BACKUP);
		howl_VolumeSet(HOWL_VOLUME_TYPE_MUSIC, CS_MUSIC_VOLUME_BACKUP);
		howl_VolumeSet(HOWL_VOLUME_TYPE_VOICE, CS_VOICE_VOLUME_BACKUP);
	}

	// cam mode be zero to follow you
	gGT = GAME_TRACKER;
	gGT->cameraDC[0].cameraMode = 0;
	gGT->pushBuffer[0].distanceToScreen_PREV = PODIUM_CAMERA_DISTANCE_TO_SCREEN;
	gGT->pushBuffer[0].distanceToScreen_CURR = PODIUM_CAMERA_DISTANCE_TO_SCREEN;
}

void CS_Podium_Stand_ThTick(struct Thread *t)
{
	if (CS_FINISHED != 0)
	{
		t->flags |= THREAD_FLAG_DEAD;
	}
}

void CS_Podium_Stand_Init(struct CsThreadInitData *podiumData)
{
	struct Instance *inst = INSTANCE_BirthWithThread(STATIC_PODIUM, csPodiumNames.s_podium, SMALL, OTHER, CS_Podium_Stand_ThTick, 0, 0);

	// if the instance was built
	if (inst == NULL)
	{
		return;
	}

	// set funcThDestroy to remove instance from instance pool
	inst->thread->funcThDestroy = PROC_DestroyInstance;

	inst->matrix.t[0] = podiumData->podiumPos.x;
	inst->matrix.t[1] = podiumData->podiumPos.y;
	inst->matrix.t[2] = podiumData->podiumPos.z;

	inst->depthBiasNormal += 2;
	inst->depthBiasSecondary += 2;

	podiumData->derivedRot.x = podiumData->rot.x;
	podiumData->derivedRot.y = podiumData->rot.y;
	podiumData->derivedRot.z = podiumData->rot.z;

	ConvertRotToMatrix(&inst->matrix, SVec3Slot_AsVec3(&podiumData->derivedRot));
}

void CS_Podium_Prize_Spin(struct Instance *inst, struct Prize *prize)
{
	struct GamepadSystem *gGS;
	s16 prevAngle;
	s32 ratio, delta;
	u32 angle;
	SVec3 lightDir;

	prize->rot.y += PODIUM_PRIZE_ROT_Y_STEP;
	ConvertRotToMatrix(&inst->matrix, &prize->rot);
	if ((inst->flags & USE_SPECULAR_LIGHT) == 0)
	{
		return;
	}

	gGS = GAMEPADS;
	prevAngle = prize->specLightPhase;
	prize->specLightPhase = prevAngle + PODIUM_PRIZE_SPEC_LIGHT_PHASE_STEP;
	if ((gGS->gamepad[1].buttonsHeldCurrFrame & BTN_L1) != 0)
	{
		prize->specLightPhase = prevAngle;
	}

	{
		s32 sine1, cos1, sine2, cos2;

		// A triangular phase sweeps the light between its authored angles.
		angle = prize->specLightVerticalStart;
		delta = prize->specLightVerticalEnd - angle;
		ratio = (prize->specLightPhase & (ANG_TWO_PI - 1)) - ANG_PI;
		ratio = abs(ratio);
		angle = angle + ((delta * ratio) >> PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT);
		{
			const void *base = data.trigApprox;
			sine1 = CTR_ReadU32AlignedLE((const u8 *)base + (angle & 0x3ff) * 4);
		}
		if (angle & 0x400)
		{
			cos1 = (s16)sine1;
			sine1 = sine1 >> 16;
			if (angle & 0x800)
				sine1 = -sine1;
			else
				cos1 = -cos1;
		}
		else
		{
			cos1 = sine1 >> 16;
			sine1 = (s32)((u32)sine1 << 16);
			sine1 = sine1 >> 16;
			if (angle & 0x800)
			{
				cos1 = -cos1;
				sine1 = -sine1;
			}
		}
		// NOTE(aalhendi): GCC 2.8 otherwise keeps this cosine in v1 instead of
		// retail's a1. This allocation constraint emits no code on either target.
		CTR_PSX_CLOBBER("$3");
		lightDir.y = cos1;

		angle = prize->specLightHorizontalStart;
		delta = prize->specLightHorizontalEnd - angle;
		ratio = (prize->specLightPhase & (ANG_TWO_PI - 1)) - ANG_PI;
		ratio = abs(ratio);
		angle = angle + ((delta * ratio) >> PODIUM_PRIZE_SPEC_LIGHT_RATIO_SHIFT);
		{
			const void *base = data.trigApprox;
			sine2 = CTR_ReadU32AlignedLE((const u8 *)base + (angle & 0x3ff) * 4);
		}
		if (angle & 0x400)
		{
			cos2 = (s16)sine2;
			sine2 = sine2 >> 16;
			if (angle & 0x800)
				sine2 = -sine2;
			else
				cos2 = -cos2;
		}
		else
		{
			cos2 = sine2 >> 16;
			sine2 = (s32)((u32)sine2 << 16);
			sine2 = sine2 >> 16;
			if (angle & 0x800)
			{
				cos2 = -cos2;
				sine2 = -sine2;
			}
		}
		lightDir.x = (sine1 * cos2) >> FRACTIONAL_BITS;
		lightDir.z = (sine1 * sine2) >> FRACTIONAL_BITS;
	}
	Vector_SpecLightSpin3D(inst, &prize->rot, &lightDir);
}

void CS_Podium_Prize_ThTick3(struct Thread *th)
{
	struct GameTracker *gGT;
	struct Instance *inst;
	struct Prize *prize;
	s32 framesLeft;

	inst = th->inst;
	// NOTE(aalhendi): Preserve retail's instance load ahead of the countdown.
	CTR_PSX_OBSERVE_VALUE(inst);
	prize = th->object;
	prize->flyToHudFramesLeft--;
	framesLeft = prize->flyToHudFramesLeft;
	if (framesLeft == 0)
	{
		if (!CS_Camera_BoolGotoBoss())
		{
			u32 rewards = GAME_ADV_PROGRESS.rewards[ADV_PROGRESS_WORD_HINT];
			s16 hintID;

			if ((rewards & ADV_REWARD_HINT_MAP_INFORMATION_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_MAP_INFORMATION;
			}
			else if ((rewards & ADV_REWARD_HINT_WUMPA_FRUIT_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_WUMPA_FRUIT;
			}
			else if ((rewards & ADV_REWARD_HINT_TNT_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_TNT;
			}
			else if ((rewards & ADV_REWARD_HINT_HANG_TIME_TURBO_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_HANG_TIME_TURBO;
			}
			else if ((rewards & ADV_REWARD_HINT_POWER_SLIDE_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_POWER_SLIDE;
			}
			else if ((rewards & ADV_REWARD_HINT_TURBO_BOOST_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_TURBO_BOOST;
			}
			else if ((rewards & ADV_REWARD_HINT_BRAKE_SLIDE_MASK) == 0)
			{
				hintID = ADV_MASK_HINT_ID_BRAKE_SLIDE;
			}
			else
			{
				goto finish;
			}
			MainFrame_RequestMaskHint(hintID, 0);
		}

	finish:
		// Use the current tracker after the hint and camera callbacks.
		gGT = GAME_TRACKER;
		gGT->overlayTransition = 2;
		gGT->gameMode2 &= ~VEH_FREEZE_PODIUM;
		OtherFX_Play(PODIUM_REWARD_UNLOCK_SFX, 1);
		th->flags |= THREAD_FLAG_DEAD;
	}
	else
	{
		s32 y, frameMax;
		// NOTE(aalhendi): These two assignments and value lifetimes retain the
		// retail projection schedule. The constraints are native no-ops.
		register s32 x CTR_PSX_REGISTER("$4");
		register s32 startY CTR_PSX_REGISTER("$5");
		s32 startX;

		startX = prize->targetScreenPos.x;
		frameMax = prize->flyToHudFramesTotal;
		x = framesLeft * (PODIUM_PRIZE_HUD_CENTER_X - startX) / frameMax;
		startY = prize->targetScreenPos.y;
		y = framesLeft * (PODIUM_PRIZE_HUD_CENTER_Y - startY) / frameMax;
		{
			s32 sum = startX + x;
			CTR_PSX_ORDER_VALUES(x, startX);
			x = sum - PODIUM_PRIZE_HUD_CENTER_X;
		}
		x *= 0u - (u32)inst->matrix.t[2];
		if (x < 0)
		{
			x += FP8_ONE - 1;
		}
		y = CTR_MipsMulLo(startY + y - PODIUM_PRIZE_HUD_CENTER_Y, inst->matrix.t[2]);
		inst->matrix.t[0] = x >> FRACTIONAL_BITS_8;
		if (y < 0)
		{
			y += FP8_ONE - 1;
		}
		CTR_PSX_ORDER_VALUES(x, startY);
		inst->matrix.t[1] = y >> FRACTIONAL_BITS_8;

		inst->scale.x -= PODIUM_PRIZE_SCALE_DOWN_STEP;
		if (inst->scale.x < PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD)
		{
			inst->scale.x = FP_ONE;
		}
		inst->scale.y = inst->scale.z = inst->scale.x;
		CS_Podium_Prize_Spin(inst, prize);
	}
}

// Make the trophy bounce 3 times
// Then start ThTick3
void CS_Podium_Prize_ThTick2(struct Thread *th)
{
	struct Prize *prize = th->object;
	struct Instance *inst = th->inst;

	if (prize->bounceFrameIndex >= PODIUM_PRIZE_BOUNCE_FRAME_COUNT)
	{
		ThTick_SetAndExec(th, CS_Podium_Prize_ThTick3);
		return;
	}

	if ((prize->bounceFrameIndex & 1) == 0)
	{
		u16 scale = inst->scale.x + PODIUM_PRIZE_BOUNCE_SCALE_UP_BASE;
		u16 step = prize->bounceFrameIndex * PODIUM_PRIZE_BOUNCE_SCALE_UP_STEP;

		// NOTE(aalhendi): Keep the multiply's last shift in the scale load's
		// delay slot. GCC 2.8 otherwise schedules a non-emitting marker there.
		CTR_PSX_ORDER_VALUES(scale, step);
		inst->scale.x = scale + step;
		if ((prize->bounceFrameIndex + 1) * PODIUM_PRIZE_BOUNCE_LIMIT_STEP + PODIUM_PRIZE_BOUNCE_LIMIT_BASE < inst->scale.x)
		{
			prize->bounceFrameIndex++;
		}
	}
	else
	{
		inst->scale.x -= PODIUM_PRIZE_BOUNCE_SCALE_DOWN_STEP;
		if (inst->scale.x < PODIUM_PRIZE_SCALE_CLAMP_THRESHOLD)
		{
			prize->bounceFrameIndex++;
		}
	}

	inst->scale.y = inst->scale.z = inst->scale.x;
	CS_Podium_Prize_Spin(inst, prize);
}

void CS_Podium_Prize_ThTick1(struct Thread *th)
{
	struct Instance *inst = th->inst;
	struct Prize *prize = th->object;
	int trig;

	if (CS_PRIZE_DROP_READY != 0)
	{
		if (th->modelIndex != STATIC_BIG1)
		{
			inst->flags &= ~HIDE_MODEL;
		}

		prize->heightOffset = VehCalc_InterpBySpeed(prize->heightOffset, PODIUM_PRIZE_HEIGHT_INTERP_STEP, 0);
		prize->orbitRadius = VehCalc_InterpBySpeed(prize->orbitRadius, PODIUM_PRIZE_ORBIT_INTERP_STEP, 0);
	}

	trig = MATH_Sin(prize->rot.y);
	inst->matrix.t[0] = prize->posStart.x + ((prize->orbitRadius * trig) >> FRACTIONAL_BITS);
	inst->matrix.t[1] = prize->posStart.y + prize->heightOffset;

	trig = MATH_Cos(prize->rot.y);
	inst->matrix.t[2] = prize->posStart.z + ((prize->orbitRadius * trig) >> FRACTIONAL_BITS);

	if (CS_FINISHED != 0)
	{
		prize->flyToHudFramesTotal = PODIUM_PRIZE_FLY_TO_HUD_FRAMES;
		prize->flyToHudFramesLeft = PODIUM_PRIZE_FLY_TO_HUD_FRAMES;
		prize->bounceFrameIndex = 0;

		inst->depthBiasNormal = PODIUM_PRIZE_UI_DEPTH_BIAS;
		inst->depthBiasSecondary = PODIUM_PRIZE_UI_DEPTH_BIAS;

		{
			struct InstDrawPerPlayer *idpp = INST_GETIDPP(inst);
			idpp[0].pushBuffer = &GAME_TRACKER->pushBuffer_UI;
		}
		inst->scale.x = FP_ONE;
		inst->scale.y = FP_ONE;
		inst->scale.z = FP_ONE;

		inst->matrix.t[0] = 0;
		inst->matrix.t[1] = 0;
		inst->matrix.t[2] = prize->targetScreenPos.z;

		OtherFX_Stop2(PODIUM_PRIZE_STOP_DINGOFIRE_FX);
		OtherFX_Stop2(PODIUM_PRIZE_STOP_AKUMOUTH_FX);
		OtherFX_Play(PODIUM_PRIZE_FLY_TO_HUD_SFX, 1);

		ThTick_SetAndExec(th, CS_Podium_Prize_ThTick2);
	}
	else
	{
		CS_Podium_Prize_Spin(inst, prize);
	}
}

void CS_Podium_Prize_ThDestroy(struct Thread *t)
{
	// remove bits
	GAME_TRACKER->gameMode2 &= ~(INC_RELIC | INC_KEY | INC_TROPHY);
	PROC_DestroyInstance(t);
}

void CS_Podium_Prize_Init(u32 prizeModel, const char *prizeName, const SVec3Slot *podiumPos)
{
	struct Instance *inst;
	struct Prize *prize;
	s32 offsetX, offsetY, offsetZ;
	struct UiElement2D *hud;

	inst = INSTANCE_BirthWithThread(prizeModel, prizeName, MEDIUM, OTHER, CS_Podium_Prize_ThTick1, sizeof(struct Prize), NULL);
	if (inst == NULL)
	{
		if (CS_PHASE < CS_WAIT_INPUT)
		{
			CS_PHASE = CS_WAIT_INPUT;
		}

		// NOTE(aalhendi): Read the active tracker after birth, even on failure.
		GAME_TRACKER->gameMode2 &= ~VEH_FREEZE_PODIUM;
		return;
	}

	inst->scale.x = PODIUM_PRIZE_INITIAL_SCALE;
	inst->scale.y = PODIUM_PRIZE_INITIAL_SCALE;
	inst->scale.z = PODIUM_PRIZE_INITIAL_SCALE;
	inst->flags |= HIDE_MODEL;

	prize = inst->thread->object;
	inst->thread->funcThDestroy = CS_Podium_Prize_ThDestroy;
	prize->orbitRadius = PODIUM_PRIZE_ORBIT_RADIUS;
	prize->heightOffset = PODIUM_PRIZE_HEIGHT_OFFSET;
	prize->rot.x = 0;
	prize->rot.y = 0;
	prize->rot.z = 0;

	// Transform the prize's initial orbit offset with the podium's light matrix.
	MTC2(0, 0);
	MTC2(PODIUM_PRIZE_ORBIT_RADIUS, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_llv0();
	offsetX = MFC2_S(25);
	offsetY = MFC2_S(26);
	offsetZ = MFC2_S(27);

	prize->posStart.x = podiumPos->x + (s16)offsetX;
	prize->posStart.y = podiumPos->y + (s16)offsetY + PODIUM_PRIZE_PODIUM_Y_OFFSET;
	prize->posStart.z = podiumPos->z + (s16)offsetZ;
	prize->targetScreenPos.z = PODIUM_PRIZE_DEFAULT_TARGET_Z;

	hud = data.hudStructPtr[0];
	switch (prizeModel)
	{
	case STATIC_RELIC:
	{
		u32 *rewards = GAME_ADV_PROGRESS.rewards;
		s32 level = GAME_TRACKER->prevLEV;
		s32 platinumBit = level + ADV_REWARD_FIRST_PLATINUM_RELIC;

		if ((rewards[platinumBit >> 5] >> (platinumBit & 31)) & 1)
		{
			inst->colorRGBA = INST_COLOR_PLATINUM_RELIC;
		}
		else
		{
			s32 goldBit = level + ADV_REWARD_FIRST_GOLD_RELIC;
			if ((rewards[goldBit >> 5] >> (goldBit & 31)) & 1)
			{
				inst->colorRGBA = INST_COLOR_GOLD_RELIC;
			}
			else
			{
				inst->colorRGBA = INST_COLOR_SAPPHIRE_RELIC;
			}
		}
		prize->specLightVerticalStart = PODIUM_RELIC_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_RELIC_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_RELIC_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;

		prize->targetScreenPos.x = hud[UI_HUD_SLOT_RELIC].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_RELIC].y - PODIUM_PRIZE_HUD_Y_OFFSET;
		GAME_TRACKER->gameMode2 |= INC_RELIC;
		return;
	}

	case STATIC_GEM:
	{
		const void *cupBase = data.AdvCups;
		const struct AdventureCup *cup = (const void *)((const u8 *)cupBase + GAME_TRACKER->cup.cupID * sizeof(struct AdventureCup));

		inst->colorRGBA = ((u32)cup->color[0] << 20) | ((u32)cup->color[1] << 12) | ((u32)cup->color[2] << 4);
		prize->specLightVerticalStart = PODIUM_GEM_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_GEM_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_GEM_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;
		goto center_target;
	}

	case STATIC_KEY:
		inst->colorRGBA = INST_COLOR_KEY;
		prize->specLightVerticalStart = PODIUM_KEY_SPEC_LIGHT_VERTICAL_START;
		prize->specLightHorizontalStart = PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_START;
		prize->specLightVerticalEnd = PODIUM_KEY_SPEC_LIGHT_VERTICAL_END;
		prize->specLightHorizontalEnd = PODIUM_KEY_SPEC_LIGHT_HORIZONTAL_END;
		inst->flags |= USE_SPECULAR_LIGHT;

		prize->targetScreenPos.x = hud[UI_HUD_SLOT_KEY].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_KEY].y - PODIUM_PRIZE_HUD_Y_OFFSET;
		GAME_TRACKER->gameMode2 |= INC_KEY;
		return;

	case STATIC_TROPHY:
		prize->targetScreenPos.x = hud[UI_HUD_SLOT_TROPHY].x;
		prize->targetScreenPos.y = hud[UI_HUD_SLOT_TROPHY].y - PODIUM_PRIZE_HUD_Y_OFFSET;
		prize->targetScreenPos.z = PODIUM_PRIZE_TROPHY_TARGET_Z;

		inst->scale.x = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;
		inst->scale.y = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;
		inst->scale.z = PODIUM_PRIZE_TROPHY_INITIAL_SCALE;
		GAME_TRACKER->gameMode2 |= INC_TROPHY;
		return;

	case STATIC_BIG1:
		inst->flags |= HIDE_MODEL;
		goto center_target;

	default:
	center_target:
		prize->targetScreenPos.x = PODIUM_PRIZE_HUD_CENTER_X;
		prize->targetScreenPos.y = PODIUM_PRIZE_HUD_CENTER_Y;
		return;
	}
}

const struct CsPodiumNames csPodiumNames = {
    .s_podium = "podium",
    .s_third = "third",
    .s_second = "second",
    .s_first = "first",
    .s_tawna = "tawna",
    .s_prize = "prize",
    .s_victorycam = "victorycam",
};

void CS_Podium_FullScene_Init(void)
{
	struct Instance *driverInstSelf;
	struct Thread *victoryCamThread;
	u32 podiumMusic;
	struct CsThreadInitData InitData;
	MATRIX podiumMatrix;

	struct CsThreadInitData *init;
	struct Driver *driver;

	struct GameTracker *gGT;

	// assume cutscene did not manipulate audio
	CS_CONTROLS_AUDIO = 0;

	// Make a backup of FX volume, masked to a byte
	CS_FX_VOLUME_BACKUP = howl_VolumeGet(HOWL_VOLUME_TYPE_FX);
	CS_FX_VOLUME_BACKUP &= PODIUM_VOLUME_BACKUP_MASK;

	// Make a backup of Music volume, masked to a byte
	CS_MUSIC_VOLUME_BACKUP = howl_VolumeGet(HOWL_VOLUME_TYPE_MUSIC);
	CS_MUSIC_VOLUME_BACKUP &= PODIUM_VOLUME_BACKUP_MASK;

	// Make a backup of Voice volume, masked to a byte
	CS_VOICE_VOLUME_BACKUP = howl_VolumeGet(HOWL_VOLUME_TYPE_VOICE);
	CS_VOICE_VOLUME_BACKUP &= PODIUM_VOLUME_BACKUP_MASK;

	// Cutscene is now starting
	CS_FINISHED = 0;
	CS_PHASE = CS_CAMERA_PAN;

	CS_PRIZE_DROP_READY = 0;

	driver = GAME_TRACKER->drivers[0];
	driverInstSelf = driver->instSelf;

	CS_PODIUM_CAMERA_FRAME = 0;

	driverInstSelf->flags |= HIDE_MODEL;

	VehPhysProc_FreezeEndEvent_Init(driverInstSelf->thread, driver);

	// Number of Winners = 1
	// this means Draw Confetti on one window
	{
		struct GameTracker *gtHud;
		gtHud = GAME_TRACKER;
		gtHud->numWinners = 1;

		// Set winnerIndex[0] to 0, to draw
		// confetti on the first pushBuffer
		gtHud->winnerIndex[0] = 0;

		gtHud->confetti.numParticles_max = PODIUM_CONFETTI_PARTICLE_COUNT;
		gtHud->confetti.vanishRate = PODIUM_CONFETTI_PARTICLE_COUNT;
		gtHud->hudFlags &= HUD_FLAG_CLEAR_RACE_HUD_MASK;

		// Draw Confetti
		gtHud->renderFlags |= RENDER_FLAG_CONFETTI;
	}
	gGT = GAME_TRACKER;
	gGT->gameMode2 |= VEH_FREEZE_PODIUM;

	// position and rotation of podium scene
	// Y coordinate (podiumPos.y) has added height

	InitData.podiumPos.x = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->pos.x;
	init = &InitData;
	init->podiumPos.y = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->pos.y + PODIUM_SCENE_SPAWN_Y_OFFSET;
	init->podiumPos.z = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->pos.z;
	init->rot.x = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->rot.x;
	init->rot.y = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->rot.y;
	init->rot.z = gGT->level1->ptrSpawnType2_PosRot[1].coords.posRot->rot.z;

	// convert 3 rotation shorts into rotation matrix
	ConvertRotToMatrix(&podiumMatrix, SVec3Slot_AsVec3(&init->rot));
	// Move position of trophy girl
	CTR_GteLoadLightMatrix(&podiumMatrix);

	// CameraDC, this makes the camera stop following you as it does while racing, it must be zero to follow you
	gGT = GAME_TRACKER;
	gGT->cameraDC[0].cameraMode = CAMERA_MODE_FREECAM;

	// if someone placed third
	if (gGT->podium_modelIndex_Third != '\0')
	{
		init->characterPos.x = PODIUM_THIRD_POS_X;
		init->characterPos.y = PODIUM_THIRD_POS_Y;
		init->characterPos.z = PODIUM_THIRD_POS_Z;

		// create thread for "third"
		CS_Thread_Init(gGT->podium_modelIndex_Third, &csPodiumNames.s_third[0], init, PODIUM_THIRD_YAW_OFFSET, 0);
	}

	// if someone placed second
	{
		struct GameTracker *gtSecond;
		gtSecond = GAME_TRACKER;
		if (gtSecond->podium_modelIndex_Second != '\0')
		{
			init->characterPos.x = PODIUM_SECOND_POS_X;
			init->characterPos.y = PODIUM_SECOND_POS_Y;
			init->characterPos.z = PODIUM_SECOND_POS_Z;

			// create thread for "second"
			CS_Thread_Init(gtSecond->podium_modelIndex_Second, &csPodiumNames.s_second[0], init, PODIUM_SECOND_YAW_OFFSET, 0);
		}
	}
	init->characterPos.x = PODIUM_FIRST_POS_X;
	init->characterPos.y = PODIUM_FIRST_POS_Y;
	init->characterPos.z = PODIUM_FIRST_POS_Z;

	// create thread for "first"
	CS_Thread_Init(GAME_TRACKER->podium_modelIndex_First, CS_PODIUM_FIRST_NAME, init, 0, 0);

	init->characterPos.x = PODIUM_TAWNA_POS_X;
	init->characterPos.y = PODIUM_TAWNA_POS_Y;
	init->characterPos.z = PODIUM_TAWNA_POS_Z;

	// create thread for trophy girl (internally called "tawna")
	CS_Thread_Init(GAME_TRACKER->podium_modelIndex_tawna, CS_PODIUM_TAWNA_NAME, init, PODIUM_TAWNA_YAW_OFFSET, 0);

	CS_Podium_Prize_Init(GAME_TRACKER->podiumRewardID, CS_PODIUM_PRIZE_NAME, &init->podiumPos);

	CS_Podium_Stand_Init(init);

	victoryCamThread =
	    (struct Thread *)PROC_BirthWithObject(PODIUM_VICTORY_CAMERA_THREAD_FLAGS, (void *)CS_Camera_ThTick_Podium, CS_PODIUM_VICTORYCAM_NAME, NULL);

	// if it allocated correctly
	if (victoryCamThread != 0)
	{
		struct CsPodiumCameraThreadObj *podiumCamera = victoryCamThread->object;
		podiumCamera->pathFrame32 = 0;
	}

	// change victory music based on who is first in the podium
	switch (GAME_TRACKER->podium_modelIndex_First - STATIC_CRASHDANCE)
	{
	// Crash, Coco, Fake Crash
	case PODIUM_DANCE_CRASH:
	case PODIUM_DANCE_COCO:
	case PODIUM_DANCE_FAKE_CRASH:
		// Crash's music
		podiumMusic = PODIUM_MUSIC_CRASH;
		break;

	// Polar Pura
	case PODIUM_DANCE_POLAR:
	case PODIUM_DANCE_PURA:
		// Polar and Pura's music
		podiumMusic = PODIUM_MUSIC_POLAR;
		break;

	// Cortex, NGin, NTrophy
	case PODIUM_DANCE_CORTEX:
	case PODIUM_DANCE_NGIN:
	case PODIUM_DANCE_NTROPY:
		// Cortex's music
		podiumMusic = PODIUM_MUSIC_CORTEX;
		break;

	// papu, roo, penta
	case PODIUM_DANCE_PAPU:
	case PODIUM_DANCE_ROO:
	case PODIUM_DANCE_PENTA:
		// Ripper Roo's music
		podiumMusic = PODIUM_MUSIC_ROO;
		break;

	// pinstripe kjoe
	case PODIUM_DANCE_PINSTRIPE:
	case PODIUM_DANCE_JOE:
		// Pinstripe's music
		podiumMusic = PODIUM_MUSIC_PINSTRIPE;
		break;

	// Tiny, Dingo, Oxide
	default:
		// Default music is Tiny Tiger's
		podiumMusic = PODIUM_MUSIC_TINY;
		break;
	}

	CDSYS_XAPlay(CDSYS_XA_TYPE_MUSIC, podiumMusic);

	return;
}
