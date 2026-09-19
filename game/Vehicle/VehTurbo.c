#include "VehCommon.h"

enum
{
	TURBO_FIRE_SIZE_MIN = 4,
	TURBO_FIRE_SIZE_MAX = 8,
	TURBO_FIRE_MATRIX_SCALE_SHIFT = 3,
	TURBO_FIRE_LEFT_X_NUMERATOR = 9,
	TURBO_FIRE_LEFT_X_SHIFT = 0xb,
	TURBO_FIRE_RIGHT_X_NUMERATOR = -0x12,
	TURBO_FIRE_RIGHT_X_SHIFT = 0xc,
	TURBO_FIRE_Y_NUMERATOR = 3,
	TURBO_FIRE_Y_SHIFT = 8,
	TURBO_FIRE_Z_NUMERATOR = -0x34,
	TURBO_FIRE_Z_SHIFT = 0xc,
	TURBO_COOLDOWN_SIGN_SCALE = 0x10000,
	TURBO_ALPHA_RUMBLE_THRESHOLD = 2500,
	TURBO_RUMBLE_FRAMES = 4,
	TURBO_RUMBLE_FORCE = 4,
	TURBO_SECONDARY_MODEL_FRAME_OFFSET = 3,
	TURBO_ANIM_FRAME_COUNT = 8,
	TURBO_ANIM_FRAME_MASK = 7,
	TURBO_AUDIO_SLOT = 3,
	TURBO_AUDIO_VOLUME_BASE = 0x100,
	TURBO_AUDIO_ALPHA_SHIFT = 4,
	TURBO_AUDIO_VOLUME_MAX = 0x82,
	TURBO_AUDIO_DISTORT_STEP = 0x10,
	TURBO_AUDIO_DISTORT_MAX = 0x80,
	TURBO_AUDIO_DISTORT_INCREMENT_LIMIT = 0xc0,
	TURBO_AUDIO_SFX_ID = 0xe,
	TURBO_RESERVES_DISAPPEAR_THRESHOLD = 0x10,
	TURBO_ALPHA_FULL_MINUS_ONE = 0xfff,
	TURBO_FADE_FAST_STEP = 0x100,
	TURBO_FADE_SLOW_STEP = 0x40,
	TURBO_STOP_SFX_ID = -1,
};

#define VEH_TURBO_FIELD(cursor, type, offset) (*(type *)((cursor) + (offset)))

// NOTE(aalhendi): Retail emits all three GTE result stores from one base; native reads the same emulated registers as ordinary C values.
#if defined(CTR_NATIVE)
#define VEH_TURBO_STORE_TRANSLATION(out) \
	do                                   \
	{                                    \
		(out)[0] = (s32)MFC2(9);         \
		(out)[1] = (s32)MFC2(10);        \
		(out)[2] = (s32)MFC2(11);        \
	} while (0)
#else
#define VEH_TURBO_STORE_TRANSLATION(out)      \
	do                                        \
	{                                         \
		s32 *translation = (out);             \
		__asm__ volatile("swc2 $9,0(%0)\n\t"  \
		                 "swc2 $10,4(%0)\n\t" \
		                 "swc2 $11,8(%0)"     \
		                 :                    \
		                 : "r"(translation)   \
		                 : "memory");         \
	} while (0)
#endif

void VehTurbo_ProcessBucket(struct Thread *turboThread)
{
	register struct GameTracker *initialGameTracker CTR_PSX_REGISTER("$11");
	register struct GameTracker *loopGameTracker CTR_PSX_REGISTER("$9");
	struct Turbo *turbo;
	register struct Instance *primaryBase CTR_PSX_REGISTER("$5");
	register struct Instance *driverBase CTR_PSX_REGISTER("$2");
	register u32 primary CTR_PSX_REGISTER("$7");
	register u32 secondary CTR_PSX_REGISTER("$6");
	register u32 driver CTR_PSX_REGISTER("$5");
	register u32 clearMask CTR_PSX_REGISTER("$10");
	u32 range;
	u16 depth;
	register int i CTR_PSX_REGISTER("$8");

	clearMask = ~DRAW_SUCCESSFUL;
	if (turboThread == NULL)
	{
		return;
	}

	initialGameTracker = GAME_TRACKER;
	do
	{
		turbo = turboThread->object;
		primaryBase = turboThread->inst;
		secondary = (u32)turbo->inst;
		driverBase = turbo->driver->instSelf;

		i = 0;
		if (initialGameTracker->numPlyrCurrGame != 0)
		{
			loopGameTracker = GAME_TRACKER;
			primary = (u32)primaryBase;
			driver = (u32)driverBase;
			do
			{
				if ((VEH_TURBO_FIELD(driver, u32, 0xb8) & PUSHBUFFER_EXISTS) == 0)
				{
					VEH_TURBO_FIELD(secondary, u32, 0xb8) &= VEH_TURBO_FIELD(driver, u32, 0xb8) | clearMask;
					VEH_TURBO_FIELD(primary, u32, 0xb8) &= VEH_TURBO_FIELD(driver, u32, 0xb8) | clearMask;

					range = VEH_TURBO_FIELD(driver, u32, 0xe4);
					VEH_TURBO_FIELD(primary, u32, 0xe4) = range;
					VEH_TURBO_FIELD(secondary, u32, 0xe4) = range;
					range = VEH_TURBO_FIELD(driver, u32, 0xe8);
					VEH_TURBO_FIELD(primary, u32, 0xe8) = range;
					VEH_TURBO_FIELD(secondary, u32, 0xe8) = range;

					depth = VEH_TURBO_FIELD(driver, u16, 0xdc);
					VEH_TURBO_FIELD(primary, u16, 0xdc) = depth;
					VEH_TURBO_FIELD(secondary, u16, 0xdc) = depth;
					depth = VEH_TURBO_FIELD(driver, u16, 0xde);
					VEH_TURBO_FIELD(primary, u16, 0xde) = depth;
					VEH_TURBO_FIELD(secondary, u16, 0xde) = depth;
				}

				primary += sizeof(struct InstDrawPerPlayer);
				secondary += sizeof(struct InstDrawPerPlayer);
				driver += sizeof(struct InstDrawPerPlayer);
				i++;
			} while (i < loopGameTracker->numPlyrCurrGame);
		}

		turboThread = turboThread->siblingThread;
	} while (turboThread != NULL);
}

#undef VEH_TURBO_FIELD

void VehTurbo_ThDestroy(struct Thread *t)
{
	struct Turbo *turboObj = t->object;
	struct Driver *d = turboObj->driver;
	d->actionsFlagSet &= ~ACTION_TURBO_ITEM;

	INSTANCE_Death(turboObj->inst);
	INSTANCE_Death(t->inst);
}

void VehTurbo_ThTick(struct Thread *turboThread)
{
	struct Turbo *turbo = (struct Turbo *)turboThread->object;
	struct Driver *driver = turbo->driver;
	struct Instance *instance = turboThread->inst;
	struct Instance *instanceDriver = driver->instSelf;
	u32 *driverAudio = &driver->driverAudioPtrs[TURBO_AUDIO_SLOT];
	int fireSize;
	int firstSecondaryMatrixElement;
	s16 elapsedTime;
	u8 kartState;
	// NOTE(aalhendi): These transient bindings preserve the retail allocator choices; CTR_NATIVE ignores them and keeps the same C semantics.
	register u32 fireAudioDistort CTR_PSX_REGISTER("$4");
	register int fireSfxVolume CTR_PSX_REGISTER("$3");
	register u32 fireAudioDistortSource CTR_PSX_REGISTER("$2");
	register u32 fireAudioVolumeField CTR_PSX_REGISTER("$5");
	register u32 fireSfxParams CTR_PSX_REGISTER("$2");
	register u32 fireRecycleParams CTR_PSX_REGISTER("$6");
	u32 stopSfxParams;

	// NOTE(aalhendi): Retail thread ticks re-enter after ThTick_FastRET instead of returning as one-shot callbacks.
	do
	{
		if ((
		        // if not burnt
		        (driver->burnTimer == 0) &&

		        // if alpha of turbo is zero
		        ((u16)instance->alphaScale == 0)) &&


		    (instanceDriver->thread->modelIndex != DYNAMIC_GHOST))
		{
			// cut driverInst transparency in half
			instanceDriver->alphaScale = (u16)instanceDriver->alphaScale >> 1;
		}

		// if instance is split by water
		if ((instanceDriver->flags & SPLIT_LINE) != 0)
		{
			// turbos are now split by water, set vertical split height
			instance->flags |= SPLIT_LINE;
			instance->vertSplit = instanceDriver->vertSplit;
			turbo->inst->flags |= SPLIT_LINE;
			turbo->inst->vertSplit = instanceDriver->vertSplit;
		}

		// if instance is not split by water
		else
		{
			// instance flags
			instance->flags &= ~SPLIT_LINE;
			turbo->inst->flags &= ~SPLIT_LINE;
		}

		// if driver instance is reflective
		if ((instanceDriver->flags & REFLECTIVE) != 0)
		{
			// make turbo instances reflective
			// copy reflection height axis to instance
			instance->flags |= REFLECTIVE;
			instance->vertSplit = instanceDriver->vertSplit;
			turbo->inst->flags |= REFLECTIVE;
			turbo->inst->vertSplit = instanceDriver->vertSplit;
		}

		// if driver instance is not reflective
		else
		{
			// remove reflection from turbo instances
			instance->flags &= ~REFLECTIVE;
			turbo->inst->flags &= ~REFLECTIVE;
		}

		VehGteSetRotTransMatrix(&instanceDriver->matrix);

		fireSize = (int)turbo->fireSize;
		if (TURBO_FIRE_SIZE_MAX < (int)turbo->fireSize)
		{
			fireSize = TURBO_FIRE_SIZE_MAX;
		}
		if (fireSize < TURBO_FIRE_SIZE_MIN)
		{
			fireSize = TURBO_FIRE_SIZE_MIN;
		}

		// matrix of first turbo instance
		instance->matrix.m[0][0] = (s16)(instanceDriver->matrix.m[0][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[0][1] = (s16)(instanceDriver->matrix.m[0][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[0][2] = (s16)(instanceDriver->matrix.m[0][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[1][0] = (s16)(instanceDriver->matrix.m[1][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[1][1] = (s16)(instanceDriver->matrix.m[1][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[1][2] = (s16)(instanceDriver->matrix.m[1][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[2][0] = (s16)(instanceDriver->matrix.m[2][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[2][1] = (s16)(instanceDriver->matrix.m[2][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		instance->matrix.m[2][2] = (s16)(instanceDriver->matrix.m[2][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);

		MTC2(((u32)(driver->instSelf->scale.x * TURBO_FIRE_LEFT_X_NUMERATOR >> TURBO_FIRE_LEFT_X_SHIFT) & 0xffff) |
		         ((u32)(driver->instSelf->scale.y * TURBO_FIRE_Y_NUMERATOR >> TURBO_FIRE_Y_SHIFT) << 16),
		     0);
		MTC2(driver->instSelf->scale.z * TURBO_FIRE_Z_NUMERATOR >> TURBO_FIRE_Z_SHIFT, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		VEH_TURBO_STORE_TRANSLATION(&instance->matrix.t[0]);

		// matrix of second turbo instance, negate X axis
		// NOTE(aalhendi): Naming the first product leaves the secondary-instance load in the retail multiply latency slot.
		firstSecondaryMatrixElement = -(int)instanceDriver->matrix.m[0][0] * fireSize;
		turbo->inst->matrix.m[0][0] = (s16)(firstSecondaryMatrixElement >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[0][1] = (s16)(instanceDriver->matrix.m[0][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[0][2] = (s16)(instanceDriver->matrix.m[0][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[1][0] = (s16)(-(int)instanceDriver->matrix.m[1][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[1][1] = (s16)(instanceDriver->matrix.m[1][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[1][2] = (s16)(instanceDriver->matrix.m[1][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[2][0] = (s16)(-(int)instanceDriver->matrix.m[2][0] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[2][1] = (s16)(instanceDriver->matrix.m[2][1] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);
		turbo->inst->matrix.m[2][2] = (s16)(instanceDriver->matrix.m[2][2] * fireSize >> TURBO_FIRE_MATRIX_SCALE_SHIFT);

		MTC2(((u32)(driver->instSelf->scale.x * TURBO_FIRE_RIGHT_X_NUMERATOR >> TURBO_FIRE_RIGHT_X_SHIFT) & 0xffff) |
		         ((u32)(driver->instSelf->scale.y * TURBO_FIRE_Y_NUMERATOR >> TURBO_FIRE_Y_SHIFT) << 16),
		     0);
		MTC2(driver->instSelf->scale.z * TURBO_FIRE_Z_NUMERATOR >> TURBO_FIRE_Z_SHIFT, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		VEH_TURBO_STORE_TRANSLATION(&turbo->inst->matrix.t[0]);

		// decrease turbo visibility cooldown by elapsed milliseconds per frame, ~32
		elapsedTime = turbo->fireVisibilityCooldown - GAME_TRACKER->elapsedTimeMS;
		turbo->fireVisibilityCooldown = elapsedTime;

		// don't allow negatives
		if (elapsedTime * TURBO_COOLDOWN_SIGN_SCALE < 0)
		{
			turbo->fireVisibilityCooldown = 0;
		}

		if (turbo->fireVisibilityCooldown == 0)
		{
			// make fire visible now that there's no cooldown
			instance->flags &= ~HIDE_MODEL;
			turbo->inst->flags &= ~HIDE_MODEL;
		}

		if ((u16)instance->alphaScale < TURBO_ALPHA_RUMBLE_THRESHOLD)
		{
			// gamepad vibration
			GAMEPAD_ShockFreq(driver, TURBO_RUMBLE_FRAMES, TURBO_RUMBLE_FORCE);
		}

		// set new model pointer, one of eight
		instance->model = GAME_TRACKER->modelPtr[(int)turbo->fireAnimIndex + STATIC_TURBO_EFFECT];

		// set new model pointer, one of eight

		// STATIC_TURBO_EFFECT
		// STATIC_TURBO_EFFECT1
		// STATIC_TURBO_EFFECT2
		// STATIC_TURBO_EFFECT3
		// STATIC_TURBO_EFFECT4
		// STATIC_TURBO_EFFECT5
		// STATIC_TURBO_EFFECT6
		// STATIC_TURBO_EFFECT7
		turbo->inst->model =
		    GAME_TRACKER->modelPtr[(((int)turbo->fireAnimIndex + TURBO_SECONDARY_MODEL_FRAME_OFFSET) & TURBO_ANIM_FRAME_MASK) + STATIC_TURBO_EFFECT];

		turbo->fireAnimIndex++;

		// if eight or higher, back to zero
		if (turbo->fireAnimIndex >= TURBO_ANIM_FRAME_COUNT)
		{
			turbo->fireAnimIndex = 0;
		}

		if (turbo->fireDisappearCountdown > 0)
		{
			turbo->fireDisappearCountdown--;
		}

		// player of any kind
		if (instanceDriver->thread->modelIndex == DYNAMIC_PLAYER)
		{
			fireSfxVolume = TURBO_AUDIO_VOLUME_BASE;
			fireSfxVolume -= (u16)instance->alphaScale >> TURBO_AUDIO_ALPHA_SHIFT;

			if (fireSfxVolume < 0)
			{
				fireSfxVolume = 0;
			}
			else
			{
				if (TURBO_AUDIO_VOLUME_MAX < fireSfxVolume)
				{
					fireSfxVolume = TURBO_AUDIO_VOLUME_MAX;
				}
			}

			fireAudioDistortSource = (u32)turbo->fireAudioDistort;
			fireAudioDistort = fireAudioDistortSource + TURBO_AUDIO_DISTORT_STEP;

			if ((int)fireAudioDistort < 0)
			{
				fireAudioDistort = 0;
			}
			else
			{
				if ((int)fireAudioDistort > TURBO_AUDIO_DISTORT_MAX)
				{
					fireAudioDistort = TURBO_AUDIO_DISTORT_MAX;
				}
			}

			fireAudioVolumeField = fireSfxVolume << 16;
			fireAudioDistort <<= 8;
			if ((driver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
			{
				fireSfxParams = fireAudioDistort | HOWL_SFX_ECHO_FLAG;
				fireSfxParams = fireAudioVolumeField | fireSfxParams;
			}
			else
			{
				fireSfxParams = fireAudioVolumeField | fireAudioDistort;
			}

			// driver audio
			fireRecycleParams = fireSfxParams | HOWL_SFX_LR_CENTER;
			OtherFX_RecycleNew(driverAudio, TURBO_AUDIO_SFX_ID, fireRecycleParams);

			// manipulate turbo audio distort to change sound each frame
			if (turbo->fireAudioDistort < TURBO_AUDIO_DISTORT_INCREMENT_LIMIT)
			{
				turbo->fireAudioDistort++;
			}
		}

		if (instanceDriver->thread->modelIndex != DYNAMIC_GHOST)
		{
			kartState = driver->kartState;
			if ((kartState == KS_MASK_GRABBED) || (kartState == KS_CRASHING) || (kartState == KS_WARP_PAD))
			{
				goto RestoreDriverAlpha;
			}
		}
		goto UpdateFade;

	RestoreDriverAlpha:
		instanceDriver->alphaScale = driver->alphaScaleBackup;
		goto StopTurbo;

	UpdateFade:
		if ((driver->reserves < TURBO_RESERVES_DISAPPEAR_THRESHOLD) || (turbo->fireDisappearCountdown == 0))
		{
			if (TURBO_ALPHA_FULL_MINUS_ONE < (u16)instance->alphaScale)
			{
				goto StopTurbo;
			}

			if (turbo->fireDisappearCountdown == 0)
			{
				instance->alphaScale += TURBO_FADE_FAST_STEP;
				turbo->inst->alphaScale += TURBO_FADE_FAST_STEP;
			}
			else
			{
				instance->alphaScale += TURBO_FADE_SLOW_STEP;
				turbo->inst->alphaScale += TURBO_FADE_SLOW_STEP;
			}
		}
		else if (TURBO_ALPHA_FULL_MINUS_ONE < (u16)instance->alphaScale)
		{
			goto StopTurbo;
		}
		goto ContinueTurbo;

	StopTurbo:
		if (instanceDriver->thread->modelIndex == DYNAMIC_PLAYER)
		{
			stopSfxParams = HOWL_SFX_CENTER_NO_DISTORTION;
			if ((driver->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
			{
				stopSfxParams = HOWL_SFX_CENTER_NO_DISTORTION | HOWL_SFX_ECHO_FLAG;
			}

			OtherFX_RecycleNew(driverAudio, TURBO_STOP_SFX_ID, stopSfxParams);
		}
		turboThread->flags |= THREAD_FLAG_DEAD;

	ContinueTurbo:
		ThTick_FastRET(turboThread);
#if defined(CTR_NATIVE)
		// NOTE(aalhendi): Native ticks are ordinary callbacks; the retail fast-return trampoline is a no-op there.
		return;
#endif
	} while (1);
}

#undef VEH_TURBO_STORE_TRANSLATION
