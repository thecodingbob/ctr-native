#include "VehCommon.h"

#define VEH_EMITTER_STRING_INNER(value) #value
#define VEH_EMITTER_STRING(value)       VEH_EMITTER_STRING_INNER(value)

#if defined(CTR_NATIVE)
#define VEH_EMITTER_LOAD_TERRAIN_FLAGS(result, nativeValue)                     ((result) = (nativeValue))
#define VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(result, nativeValue, dependency) ((result) = (nativeValue))
#define VEH_EMITTER_TIE_THREAD_ARGUMENT(result, nativeValue)                    ((result) = (nativeValue))
#define VEH_EMITTER_TIE_THREAD_ARGUMENT_AFTER(result, nativeValue, dependency)  ((result) = (nativeValue))
#define VEH_EMITTER_RETAIN_THREAD_ARGUMENT(result, dependency)                  ((void)(result))
#define VEH_EMITTER_TERRAIN_FLAGS_ARG(nativeValue)                              (nativeValue)
#define VEH_EMITTER_PATH_TAG(pathTag)                                           ((void)(pathTag))
#define VEH_EMITTER_MUL_BEGIN(result, lhs, rhs)                                 ((result) = CTR_MipsMulLo((lhs), (rhs)))
#define VEH_EMITTER_MUL_READ_SHIFT(result, product)                             ((result) = CTR_MipsSra((product), FRACTIONAL_BITS))
#define VEH_EMITTER_MUL_RAW(lhs, rhs)                                           ((void)0)
#define VEH_EMITTER_READ_LO(result)                                             ((void)(result))
#define VEH_EMITTER_HILO_DELAY()                                                ((void)0)
#define VEH_EMITTER_STORE_MAC_DIRECT(out)                                       CTR_GteStoreMAC((s32 *)(out))
#define VEH_EMITTER_STORE_EXHAUST_VELOCITY(out)                                 VehEmitter_StoreMAC(&(out)->vx)
#define VEH_EMITTER_ZERO_AFTER_LOAD(value, dependency)                          ((value) = 0)
#else
// NOTE(aalhendi): Retail overlaps the third spark-axis multiply with the first
// two particle updates, then reads LO immediately before consuming that axis.
#define VEH_EMITTER_LOAD_TERRAIN_FLAGS(result, nativeValue)                     __asm__ volatile("lw %0,56($sp)\n\tnop" : "=r"(result) : : "memory")
#define VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(result, nativeValue, dependency) __asm__ volatile("lw %0,104($sp)" : "=r"(result) : "r"(dependency) : "memory")
// NOTE(aalhendi): GCC 2.8.1 excludes specified local registers from its reload
// pass. These empty constraints recover retail's t3 thread reload without
// emitting instructions or exposing the register choice to native compilers.
#define VEH_EMITTER_TIE_THREAD_ARGUMENT(result, nativeValue)                    __asm__("" : "=r"(result) : "0"(nativeValue))
#define VEH_EMITTER_TIE_THREAD_ARGUMENT_AFTER(result, nativeValue, dependency)  __asm__("" : "=r"(result) : "0"(nativeValue), "r"(dependency))
#define VEH_EMITTER_RETAIN_THREAD_ARGUMENT(result, dependency)                  __asm__("" : "=r"(result) : "0"(result), "r"(dependency))
#define VEH_EMITTER_TERRAIN_FLAGS_ARG(nativeValue)                              0
#define VEH_EMITTER_PATH_TAG(pathTag)                                           __asm__ volatile(".Lveh_emitter_skid_" VEH_EMITTER_STRING(pathTag) ":")
#define VEH_EMITTER_MUL_BEGIN(result, lhs, rhs)                                 __asm__ volatile("nop\n\tnop\n\tmult %0,%1" : : "r"(lhs), "r"(rhs))
#define VEH_EMITTER_MUL_READ_SHIFT(result, product)                             __asm__ volatile("mflo %1\n\tsra %0,%1,12" : "=r"(result), "=r"(product))
#define VEH_EMITTER_MUL_RAW(lhs, rhs)                                           __asm__ volatile("mult %0,%1" : : "r"(lhs), "r"(rhs))
#define VEH_EMITTER_READ_LO(result)                                             __asm__ volatile("mflo %0" : "=r"(result))
#define VEH_EMITTER_HILO_DELAY()                                                __asm__ volatile("nop\n\tnop")
#define VEH_EMITTER_STORE_MAC_DIRECT(out) __asm__ volatile("swc2 $25,0(%0)\n\tswc2 $26,4(%0)\n\tswc2 $27,8(%0)" : : "r"(out) : "memory")
#define VEH_EMITTER_STORE_EXHAUST_VELOCITY(out) \
	__asm__ volatile("mfc2 $2,$25\n\tmfc2 $3,$26\n\tmfc2 $4,$27\n\tsw $2,40($sp)\n\tsw $3,44($sp)\n\tsw $4,48($sp)" : : : "$2", "$3", "$4", "memory")
#define VEH_EMITTER_ZERO_AFTER_LOAD(value, dependency) \
	do                                                 \
	{                                                  \
		__asm__ volatile("nop" : : "r"(dependency));   \
		(value) = 0;                                   \
	} while (0)
#endif

enum
{
	VEH_EMITTER_AXIS_COUNT = 3,
	VEH_EMITTER_EXHAUST_ICON_LOW = 1,
	VEH_EMITTER_EXHAUST_ICON_WATER = 7,
	VEH_EMITTER_EXHAUST_WATER_Y_LIMIT = FP8_ONE,
	VEH_EMITTER_EXHAUST_VEL_Y = 0x400,
	VEH_EMITTER_EXHAUST_VEL_Z = -0x400,
	VEH_EMITTER_EXHAUST_POS_X_NUM = 9,
	VEH_EMITTER_EXHAUST_POS_X_SHIFT = 3,
	VEH_EMITTER_EXHAUST_POS_Y_NUM = 7,
	VEH_EMITTER_EXHAUST_POS_Y_SHIFT = 1,
	VEH_EMITTER_EXHAUST_POS_Z_NUM = -0x38,
	VEH_EMITTER_EXHAUST_POS_Z_SHIFT = 4,
	VEH_EMITTER_EXHAUST_POS_SECOND_X_NUM = -0x12,
	VEH_EMITTER_EXHAUST_POS_SECOND_X_SHIFT = 4,
	VEH_EMITTER_TURBO_METER_COLOR_MIN = 129,
	VEH_EMITTER_TURBO_ROOM_WARNING_PAD = 2,
	VEH_EMITTER_TURBO_ROOM_WARNING_SCALE = 32,
	VEH_EMITTER_GROUND_SPARK_COUNT = 10,
	VEH_EMITTER_GROUND_SPARK_RNG_MASK = 0x7ff,
	VEH_EMITTER_TERRAIN_MIN_SPEED = 0x300,
	VEH_EMITTER_TERRAIN_NORMAL_TIRE_COUNT = 2,
	VEH_EMITTER_TERRAIN_DRIFT_TIRE_COUNT = 4,
	VEH_EMITTER_TIRE_LEFT_X = -0x1e,
	VEH_EMITTER_TIRE_RIGHT_X = 0x1e,
	VEH_EMITTER_TIRE_BACK_Z = -0x14,
	VEH_EMITTER_TIRE_FRONT_Z = 0x28,
	VEH_EMITTER_TERRAIN_TIRE_Y = 0x0a,
	VEH_EMITTER_SKID_LATERAL_SCALE = 15,
	VEH_EMITTER_SKID_WIDTH_SCALE = 10,
	VEH_EMITTER_SKID_COLOR_BIAS = 2,
	VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT = 1,
	VEH_EMITTER_SKID_HALF_LATERAL_SHIFT = 1,
	VEH_EMITTER_MUD_SPLASH_NORMAL_COUNT = 1,
	VEH_EMITTER_MUD_SPLASH_LANDING_COUNT = 10,
	VEH_EMITTER_MUD_SPLASH_ICON = 0x0d,
	VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT = 4,
	VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE = 1 << VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT,
	VEH_EMITTER_PARTICLE_WORLD_SCALE = FP8_ONE,
	VEH_EMITTER_MUD_MIN_SPEED = FP8_ONE * 5,
	VEH_EMITTER_LANDING_SPARK_MIN_SPEED = FP8_ONE * 6,
	VEH_EMITTER_LANDING_SPARK_MIN_JUMP = FP8_ONE * 22,
	VEH_EMITTER_TERRAIN_AUDIO_SPEED_MAX = 5000,
	VEH_EMITTER_TERRAIN_AUDIO_VOLUME_MAX = 200,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_SPEED_MAX = 12000,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MIN = 0x6c,
	VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MAX = 0xd2,
	VEH_EMITTER_TERRAIN_RUMBLE_MIN_SPEED = FP8_ONE * 2,
	VEH_EMITTER_LANDING_RUMBLE_FRAMES = 3,
	VEH_EMITTER_RUMBLE_FORCE_MAX = 0xff,
	VEH_EMITTER_WALL_RUB_TIMER_FULL = 0xf0,
	VEH_EMITTER_WALL_RUB_SFX = 0x14,
	VEH_EMITTER_WALL_RUB_VOLUME_STEP = 0x14,
	VEH_EMITTER_SKID_AUDIO_MIN_SPEED = (FP8_ONE * 2) + 1,
	VEH_EMITTER_SKID_AUDIO_SPEED_MIN = 2000,
	VEH_EMITTER_SKID_AUDIO_SPEED_MAX = 12000,
	VEH_EMITTER_SKID_AUDIO_VOLUME_MIN = 0x14,
	VEH_EMITTER_SKID_AUDIO_VOLUME_MAX = 0xaa,
	VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW = 0x92,
	VEH_EMITTER_SKID_AUDIO_DISTORT_FAST = 0x78,
	VEH_EMITTER_WALL_SPARK_MIN_SPEED = FP8_ONE * 2,
	VEH_EMITTER_WALL_RUMBLE_FRAME_LIMIT = 450,
	VEH_EMITTER_WALL_RUMBLE_FRAMES = 8,
	VEH_EMITTER_WALL_RUMBLE_FORCE = 0x7f,
	VEH_EMITTER_WALL_SPARK_LEFT_X = -0x2200,
	VEH_EMITTER_WALL_SPARK_RIGHT_X = 0x2200,
	VEH_EMITTER_WALL_SPARK_Y = 0x0a00,
	VEH_EMITTER_WALL_SPARK_REVERSE_Z = -0x1400,
	VEH_EMITTER_WALL_SPARK_FORWARD_Z = 0x2800,
	VEH_EMITTER_WALL_SPARK_SCRATCH_HALF_COUNT = 6,
	VEH_EMITTER_ALPHA_FULL = 0x1000,
	VEH_EMITTER_JOG_GROUND = 0x27,
	VEH_EMITTER_JOG_WOBBLE_ALT = 0xf0,
	VEH_EMITTER_JOG_WOBBLE_TIMER_MASK = 3,
	VEH_EMITTER_JOG_WOBBLE_DURATION = 0x100,
	VEH_EMITTER_JOG_AIR_LANDING_BOOST_MAX = 0x80,
	VEH_EMITTER_JOG_AIR_TURN_NEGATIVE = 0x12,
	VEH_EMITTER_JOG_AIR_TURN_POSITIVE = 0x22,
	VEH_EMITTER_JOG_AIR_DURATION = 0x20,
};


struct Particle *VehEmitter_Exhaust(struct Driver *d, VECTOR *exhaustPos, VECTOR *exhaustVel)
{
	int exhaustType;
	struct ParticleEmitter *emSet;
	u8 numPlyr;
	struct Particle *p;

	if (d->invisibleTimer != 0)
	{
		return 0;
	}

	if ((d->instSelf->flags & HIDE_MODEL) != 0)
	{
		return 0;
	}

	emSet = &data.emSet_Exhaust_High[0];
	numPlyr = GAME_TRACKER->numPlyrCurrGame;
    if (!sdata->highDetailSplitScreenLevel) {
      if (numPlyr >= 3)
      {
        CTR_PSX_FORGET_VALUE(emSet);
        emSet = &data.emSet_Exhaust_Low[0];
      }
      else
      {
        CTR_PSX_FORGET_VALUE(emSet);
        if (numPlyr >= 2)
        {
          emSet = &data.emSet_Exhaust_Med[0];
        }
      }
    }

	// NOTE(aalhendi): Converted bots retain a non-robot model index, so the
	// runtime model identity is the retail discriminator for exhaust LOD.
	if (d->instSelf->thread->modelIndex == DYNAMIC_ROBOT_CAR)
	{
		emSet = &data.emSet_Exhaust_Low[0];
	}
	exhaustType = VEH_EMITTER_EXHAUST_ICON_LOW;

	if (((d->instSelf->flags & SPLIT_LINE) != 0) && ((exhaustPos->vy - exhaustVel->vy) + d->posCurr.y < VEH_EMITTER_EXHAUST_WATER_Y_LIMIT))
	{
		// bubble texture
		exhaustType = VEH_EMITTER_EXHAUST_ICON_WATER;
		emSet = &data.emSet_Exhaust_Water[0];
	}

	p = Particle_Init(0, GAME_TRACKER->iconGroup[exhaustType], emSet);

	if (p == NULL)
	{
		goto done;
	}

	p->axis[0].startVal += exhaustPos->vx - exhaustVel->vx;
	p->axis[0].velocity = (s16)exhaustVel->vx;
	p->axis[1].startVal += exhaustPos->vy - exhaustVel->vy;
	p->axis[1].velocity = (s16)exhaustVel->vy;
	p->axis[2].startVal += exhaustPos->vz - exhaustVel->vz;
	p->axis[2].velocity = (s16)exhaustVel->vz;

	p->otIndexOffset = d->instSelf->depthBiasNormal;
	p->owner.driverInst = d->instSelf;

	if (exhaustType == VEH_EMITTER_EXHAUST_ICON_WATER)
	{
		p->funcPtr = Particle_FuncPtr_ExhaustUnderwater;
	}

	// if engine revving
	if (d->kartState == KS_ENGINE_REVVING)
	{
		if (d->revEngineState != 1)
		{
			goto done;
		}
	}

	// if not engine revving
	else
	{
		s16 meterLeft = d->turbo_MeterRoomLeft;
		if ((meterLeft < VEH_EMITTER_TURBO_METER_COLOR_MIN) ||
		    (((d->const_turboLowRoomWarning + VEH_EMITTER_TURBO_ROOM_WARNING_PAD) * VEH_EMITTER_TURBO_ROOM_WARNING_SCALE) < meterLeft))
		{
			goto done;
		}
	}

	p->flagsSetColor &= ~PARTICLE_SET_COLOR_FLAG_DRAW_MODE_MASK;
	p->flagsSetColor |= PARTICLE_SET_COLOR_FLAG_DRAW_MODE_40;

done:
	return p;
}

void VehEmitter_Sparks_Ground(struct Driver *d, struct ParticleEmitter *emSet)
{
	s32 outZ[3];
	s32 outX0;
	register s32 outX1 CTR_PSX_REGISTER("$23");
	register s32 outX2 CTR_PSX_REGISTER("$22");
	register s32 outZ20 CTR_PSX_REGISTER("$21");
	register s32 outZ21 CTR_PSX_REGISTER("$20");
	register s32 outZ22 CTR_PSX_REGISTER("$19");
	int count;
	register s32 gteValue CTR_PSX_REGISTER("$7") = 0;

	CTR_PSX_OBSERVE_MEMORY(emSet);

	MTC2(gteValue, 0);
	gteValue = -0x1800;
	MTC2(gteValue, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	gteValue = MFC2_S(25);
	CTR_PSX_GTE_READ_DELAY();
	outZ[0] = gteValue;
	gteValue = MFC2_S(26);
	CTR_PSX_GTE_READ_DELAY();
	outZ[1] = gteValue;
	gteValue = MFC2_S(27);
	CTR_PSX_GTE_READ_DELAY();
	outZ[2] = gteValue;

	gteValue = 0x1800;
	MTC2(gteValue, 0);
	gteValue = 0;
	MTC2(gteValue, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	outX0 = MFC2_S(25);
	outX1 = MFC2_S(26);
	outX2 = MFC2_S(27);

	MTC2(gteValue, 0);
	gteValue = -0x200;
	MTC2(gteValue, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	outZ20 = MFC2_S(25);
	outZ21 = MFC2_S(26);
	outZ22 = MFC2_S(27);

	count = VEH_EMITTER_GROUND_SPARK_COUNT;
	do
	{
		struct Particle *p = Particle_Init(0, GAME_TRACKER->iconGroup[0], emSet);

		if (p != NULL)
		{
			u32 rawRng;
			u32 rng;
			register s32 productX CTR_PSX_REGISTER("$8");
			register s32 productY CTR_PSX_REGISTER("$4");

			rawRng = (u32)RngDeadCoed(&GAME_TRACKER->deadcoed_struct);
			rng = rawRng & VEH_EMITTER_GROUND_SPARK_RNG_MASK;

			if ((rawRng & 1) != 0)
			{
				rng = -rng;
			}
			productX = CTR_MipsMulLo((s32)rng, outX0);
			CTR_PSX_KEEP_VALUE(productX);
			productY = CTR_MipsMulLo((s32)rng, outX1);
			CTR_PSX_KEEP_VALUE(productY);
			{
				register s32 productZ CTR_PSX_REGISTER("$5");
				register s32 velocityDelta CTR_PSX_REGISTER("$2");
				register s32 currentVelocity CTR_PSX_REGISTER("$3");

				VEH_EMITTER_MUL_BEGIN(productZ, (s32)rng, outX2);
				velocityDelta = CTR_MipsSra(productX, FRACTIONAL_BITS);
				CTR_PSX_KEEP_VALUE(velocityDelta);
				currentVelocity = (u16)p->axis[0].velocity;
				CTR_PSX_KEEP_VALUE(currentVelocity);
				velocityDelta = outZ20 + velocityDelta;
				CTR_PSX_KEEP_VALUE(velocityDelta);
				currentVelocity += velocityDelta;
				p->axis[0].velocity = (s16)currentVelocity;
				velocityDelta = CTR_MipsSra(productY, FRACTIONAL_BITS);
				CTR_PSX_KEEP_VALUE(velocityDelta);
				currentVelocity = (u16)p->axis[1].velocity;
				CTR_PSX_KEEP_VALUE(currentVelocity);
				velocityDelta = outZ21 + velocityDelta;
				CTR_PSX_KEEP_VALUE(velocityDelta);
				currentVelocity += velocityDelta;
				p->axis[1].velocity = (s16)currentVelocity;
				currentVelocity = (u16)p->axis[2].velocity;
				CTR_PSX_KEEP_VALUE(currentVelocity);
				VEH_EMITTER_MUL_READ_SHIFT(velocityDelta, productZ);
				CTR_PSX_KEEP_VALUE(productZ);
				CTR_PSX_KEEP_VALUE(velocityDelta);
				velocityDelta = outZ22 + velocityDelta;
				CTR_PSX_KEEP_VALUE(velocityDelta);
				currentVelocity += velocityDelta;
				p->axis[2].velocity = (s16)currentVelocity;
			}

			{
				register s32 particleVelocity CTR_PSX_REGISTER("$2");
				register s32 particleStart CTR_PSX_REGISTER("$3");

				particleVelocity = p->axis[0].velocity;
				CTR_PSX_KEEP_VALUE(particleVelocity);
				gteValue = outZ[0];
				CTR_PSX_KEEP_VALUE(gteValue);
				particleStart = p->axis[0].startVal;
				CTR_PSX_KEEP_VALUE(particleStart);
				particleVelocity = gteValue + particleVelocity;
				particleStart += particleVelocity;
				p->axis[0].startVal = particleStart;

				particleVelocity = p->axis[1].velocity;
				CTR_PSX_KEEP_VALUE(particleVelocity);
				gteValue = outZ[1];
				CTR_PSX_KEEP_VALUE(gteValue);
				particleStart = p->axis[1].startVal;
				CTR_PSX_KEEP_VALUE(particleStart);
				particleVelocity = gteValue + particleVelocity;
				particleStart += particleVelocity;
				p->axis[1].startVal = particleStart;

				particleVelocity = p->axis[2].velocity;
				CTR_PSX_KEEP_VALUE(particleVelocity);
				gteValue = outZ[2];
				CTR_PSX_KEEP_VALUE(gteValue);
				particleStart = p->axis[2].startVal;
				CTR_PSX_KEEP_VALUE(particleStart);
				particleVelocity = gteValue + particleVelocity;
				particleStart += particleVelocity;
				p->axis[2].startVal = particleStart;
			}

			p->owner.driverInst = d->instSelf;
			p->otIndexOffset = d->instSelf->depthBiasNormal;
		}

		count--;
	} while (count != 0);
}

void VehEmitter_Terrain_Ground(struct Driver *d, struct ParticleEmitter *emSet)
{
	register struct Driver *driver CTR_PSX_REGISTER("$20") = d;
	register struct ParticleEmitter *emitter CTR_PSX_REGISTER("$21");
	register int numTires CTR_PSX_REGISTER("$19");
	register s32 posX CTR_PSX_REGISTER("$16");
	register s32 posY CTR_PSX_REGISTER("$17");
	register s32 posZ CTR_PSX_REGISTER("$18");
	register s32 gteValue CTR_PSX_REGISTER("$7");
	int speed;
	int flags;

	CTR_PSX_KEEP_VALUE(driver);
	flags = driver->actionsFlagSet;
	emitter = emSet;

	if ((flags & ACTION_TOUCH_GROUND) == 0)
	{
		return;
	}

	if ((flags & ACTION_ACCEL_PREVENTION) != 0)
	{
		return;
	}

	speed = driver->fireSpeed;
	if (speed < 0)
	{
		speed = -speed;
	}
	if (speed < VEH_EMITTER_TERRAIN_MIN_SPEED)
	{
		speed = driver->speedApprox;
		if (speed < 0)
		{
			speed = -speed;
		}
		if (speed < VEH_EMITTER_TERRAIN_MIN_SPEED)
		{
			return;
		}
	}

	// if sliding, spawn on 4 tires, otherwise just 2
	CTR_PSX_CLOBBER("$19");
	numTires = KS_DRIFTING;
	if (driver->kartState == numTires)
	{
		numTires = VEH_EMITTER_TERRAIN_DRIFT_TIRE_COUNT;
	}

	while (numTires != 0)
	{
		switch (numTires)
		{
		case 4:
			gteValue = CTR_PackS16Pair(VEH_EMITTER_TIRE_LEFT_X, VEH_EMITTER_TERRAIN_TIRE_Y);
			MTC2(gteValue, 0);
			gteValue = VEH_EMITTER_TIRE_FRONT_Z;
			MTC2(gteValue, 1);
			break;
		case 3:
			gteValue = CTR_PackS16Pair(VEH_EMITTER_TIRE_RIGHT_X, VEH_EMITTER_TERRAIN_TIRE_Y);
			MTC2(gteValue, 0);
			gteValue = VEH_EMITTER_TIRE_FRONT_Z;
			MTC2(gteValue, 1);
			break;
		case 2:
			gteValue = CTR_PackS16Pair(VEH_EMITTER_TIRE_LEFT_X, VEH_EMITTER_TERRAIN_TIRE_Y);
			MTC2(gteValue, 0);
			gteValue = VEH_EMITTER_TIRE_BACK_Z;
			MTC2(gteValue, 1);
			break;
		default:
			gteValue = CTR_PackS16Pair(VEH_EMITTER_TIRE_RIGHT_X, VEH_EMITTER_TERRAIN_TIRE_Y);
			MTC2(gteValue, 0);
			gteValue = VEH_EMITTER_TIRE_BACK_Z;
			MTC2(gteValue, 1);
			break;
		}

		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rtv0();
		posX = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);

		{
			struct Particle *p = Particle_Init(0, GAME_TRACKER->iconGroup[0], emitter);

			if (p != NULL)
			{
				register u32 packedVelocity CTR_PSX_REGISTER("$3");
				s32 velocityY;

				p->axis[0].startVal += CTR_MipsSll(posX, 8);
				p->axis[1].startVal += CTR_MipsSll(posY, 8);
				p->axis[2].startVal += CTR_MipsSll(posZ, 8);
				CTR_PSX_MEMORY_BARRIER();

				velocityY = p->axis[1].velocity;
				packedVelocity = (u16)p->axis[0].velocity;
				velocityY = CTR_MipsSll(velocityY, 16);
				packedVelocity |= (u32)velocityY;
				CTR_PSX_KEEP_VALUE(packedVelocity);
				MTC2(packedVelocity, 0);
				gteValue = (u16)p->axis[2].velocity;
				MTC2(gteValue, 1);
				CTR_PSX_GTE_PIPELINE_DELAY();
				gte_rtv0();
				posX = MFC2_S(25);
				posY = MFC2_S(26);
				posZ = MFC2_S(27);

				p->axis[0].velocity = (s16)posX;
				p->axis[1].velocity = (s16)posY;
				p->axis[2].velocity = (s16)posZ;
				p->owner.driverInst = driver->instSelf;
				p->otIndexOffset = driver->instSelf->depthBiasNormal;
			}
		}

		numTires--;
		CTR_PSX_KEEP_VALUE(numTires);
	}
}

void VehEmitter_Sparks_Wall(struct Driver *d, struct ParticleEmitter *emSet)
{
	register struct Driver *driver CTR_PSX_REGISTER("$16") = d;
	register struct ParticleEmitter *emitter CTR_PSX_REGISTER("$20");
	register s32 tireX CTR_PSX_REGISTER("$17");
	register s32 tireY CTR_PSX_REGISTER("$18");
	register s32 tireZ CTR_PSX_REGISTER("$19");
	register s32 gteValue CTR_PSX_REGISTER("$8");
	s32 otherTireX;
	s32 otherTireY;
	s32 otherTireZ;
	s32 distanceX;
	s32 distanceY;
	s32 frameAgainstWall;
	register u32 packedValue CTR_PSX_REGISTER("$2");
	int speedAbs;
	struct Particle *p;

	CTR_PSX_KEEP_VALUE(driver);
	emitter = emSet;

	if (driver->fireSpeed == 0)
	{
		speedAbs = driver->speedApprox;
		if (speedAbs < 0)
		{
			speedAbs = -speedAbs;
		}
		if (speedAbs < (VEH_EMITTER_WALL_SPARK_MIN_SPEED + 1))
		{
			goto resetRumble;
		}
	}

	CTR_PSX_LOAD_SIGNED_HALF(frameAgainstWall, driver, 0x50, driver->frameAgainstWall);
	if (frameAgainstWall < VEH_EMITTER_WALL_RUMBLE_FRAME_LIMIT)
	{
		GAMEPAD_ShockFreq(driver, VEH_EMITTER_WALL_RUMBLE_FRAMES, 0);
		GAMEPAD_ShockForce1(driver, VEH_EMITTER_WALL_RUMBLE_FRAMES, VEH_EMITTER_WALL_RUMBLE_FORCE);
		driver->frameAgainstWall++;
		goto rumbleDone;
	}

resetRumble:
	driver->frameAgainstWall = 0;

rumbleDone:
	if (driver->speedApprox >= (VEH_EMITTER_WALL_SPARK_MIN_SPEED + 1))
	{
		gteValue = CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_LEFT_X, VEH_EMITTER_WALL_SPARK_Y);
		MTC2(gteValue, 0);
		gteValue = VEH_EMITTER_WALL_SPARK_FORWARD_Z;
		MTC2(gteValue, 1);
		gteValue = CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_RIGHT_X, VEH_EMITTER_WALL_SPARK_Y);
		MTC2(gteValue, 2);
		gteValue = VEH_EMITTER_WALL_SPARK_FORWARD_Z;
		MTC2(gteValue, 3);
	}
	else
	{
		if (driver->speedApprox >= -VEH_EMITTER_WALL_SPARK_MIN_SPEED)
		{
			return;
		}
		CTR_PSX_CLOBBER("$8");

		gteValue = CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_LEFT_X, VEH_EMITTER_WALL_SPARK_Y);
		MTC2(gteValue, 0);
		gteValue = VEH_EMITTER_WALL_SPARK_REVERSE_Z;
		MTC2(gteValue, 1);
		gteValue = CTR_PackS16Pair(VEH_EMITTER_WALL_SPARK_RIGHT_X, VEH_EMITTER_WALL_SPARK_Y);
		MTC2(gteValue, 2);
		gteValue = VEH_EMITTER_WALL_SPARK_REVERSE_Z;
		MTC2(gteValue, 3);
	}

	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	tireX = MFC2_S(25);
	tireY = MFC2_S(26);
	tireZ = MFC2_S(27);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv1();
	otherTireX = MFC2_S(25);
	otherTireY = MFC2_S(26);
	otherTireZ = MFC2_S(27);

	packedValue = (u16)tireX;
	CTR_PSX_KEEP_VALUE(packedValue);
	distanceX = CTR_MipsSll(tireY, 16);
	packedValue |= (u32)distanceX;
	CTC2(packedValue, 8);
	packedValue = (u16)tireZ;
	CTR_PSX_KEEP_VALUE(packedValue);
	distanceX = CTR_MipsSll(otherTireX, 16);
	packedValue |= (u32)distanceX;
	CTC2(packedValue, 9);
	packedValue = (u16)otherTireY;
	CTR_PSX_KEEP_VALUE(packedValue);
	distanceX = CTR_MipsSll(otherTireZ, 16);
	packedValue |= (u32)distanceX;
	CTC2(packedValue, 10);

	distanceX = CTR_MipsSubLo(CTR_MipsSll(driver->posWallColl.x, 8), driver->posCurr.x);
	distanceY = CTR_MipsSubLo(CTR_MipsSll(driver->posWallColl.y, 8), driver->posCurr.y);
	{
		register u32 packedDistance CTR_PSX_REGISTER("$3");
		s32 distanceZ;

		packedDistance = (u16)distanceX;
		distanceY = CTR_MipsSll(distanceY, 16);
		packedDistance |= (u32)distanceY;
		CTR_PSX_KEEP_VALUE(packedDistance);
		MTC2(packedDistance, 0);
		distanceZ = CTR_MipsSubLo(CTR_MipsSll(driver->posWallColl.z, 8), driver->posCurr.z);
		MTC2(distanceZ, 1);
	}
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_llv0();
	distanceX = MFC2_S(25);
	distanceY = MFC2_S(26);
	CTR_PSX_GTE_READ_DELAY();

	if (distanceX < distanceY)
	{
		tireX = otherTireX;
		tireY = otherTireY;
		tireZ = otherTireZ;
	}

	p = Particle_Init(0, GAME_TRACKER->iconGroup[0], emitter);
	if (p != NULL)
	{
		register u32 packedVelocity CTR_PSX_REGISTER("$3");
		register s32 particlePosition CTR_PSX_REGISTER("$2");
		register s32 particlePositionZ CTR_PSX_REGISTER("$3");
		s32 velocityY;

		particlePosition = p->axis[0].startVal;
		particlePositionZ = p->axis[2].startVal;
		CTR_PSX_KEEP_VALUE(particlePosition);
		CTR_PSX_KEEP_VALUE(particlePositionZ);
		particlePosition += tireX;
		p->axis[0].startVal = particlePosition;
		particlePosition = p->axis[1].startVal;
		particlePositionZ += tireZ;
		p->axis[2].startVal = particlePositionZ;
		particlePosition += tireY;
		p->axis[1].startVal = particlePosition;
		CTR_PSX_MEMORY_BARRIER();

		velocityY = p->axis[1].velocity;
		packedVelocity = (u16)p->axis[0].velocity;
		velocityY = CTR_MipsSll(velocityY, 16);
		packedVelocity |= (u32)velocityY;
		CTR_PSX_KEEP_VALUE(packedVelocity);
		MTC2(packedVelocity, 0);
		gteValue = (u16)p->axis[2].velocity;
		MTC2(gteValue, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rtv0();
		tireX = MFC2_S(25);
		tireY = MFC2_S(26);
		tireZ = MFC2_S(27);

		p->axis[0].velocity = (s16)tireX;
		p->axis[1].velocity = (s16)tireY;
		p->axis[2].velocity = (s16)tireZ;
		p->owner.driverInst = driver->instSelf;
	}
}

static inline void VehEmitter_SetRotMatrix(MATRIX *m)
{
	const CtrPackedU32 *matrixWords = (const CtrPackedU32 *)m;
	register u32 word0 CTR_PSX_REGISTER("$12");
	register u32 word1 CTR_PSX_REGISTER("$13");
	register u32 word2 CTR_PSX_REGISTER("$14");

	CTR_PSX_KEEP_VALUE(matrixWords);
	word0 = matrixWords[0];
	word1 = matrixWords[1];
	CTC2(word0, 0);
	CTC2(word1, 1);
	word0 = matrixWords[2];
	word1 = matrixWords[3];
	word2 = matrixWords[4];
	CTC2(word0, 2);
	CTC2(word1, 3);
	CTC2(word2, 4);
}

static inline void VehEmitter_StoreMAC(s32 *out)
{
	register s32 x CTR_PSX_REGISTER("$2");
	register s32 y CTR_PSX_REGISTER("$3");
	register s32 z CTR_PSX_REGISTER("$4");

	x = MFC2_S(25);
	y = MFC2_S(26);
	z = MFC2_S(27);
	out[0] = x;
	out[1] = y;
	out[2] = z;
}

static inline void VehEmitter_Skidmarks(struct Instance *instanceArg, struct Driver *driverArg, TerrainFlags terrainFlags, u8 flagsArg)
{
	register struct Driver *d CTR_PSX_REGISTER("$19") = driverArg;
	register struct Instance *inst CTR_PSX_REGISTER("$23") = instanceArg;
	register u8 color CTR_PSX_REGISTER("$21");
	register int rawColor CTR_PSX_REGISTER("$2");
	register u8 flags CTR_PSX_REGISTER("$22") = flagsArg;
	int sin;
	int cos;
	register int lateralX CTR_PSX_REGISTER("$17");
	register int lateralZ CTR_PSX_REGISTER("$20");
	register int widthX CTR_PSX_REGISTER("$16");
	register int widthZ CTR_PSX_REGISTER("$10");
	register int posX CTR_PSX_REGISTER("$6");
	register int posY CTR_PSX_REGISTER("$9");
	register int posZ CTR_PSX_REGISTER("$7");
	register int skidOffset CTR_PSX_REGISTER("$2");
	register u8 forcedFlags CTR_PSX_REGISTER("$2");
#if !defined(CTR_NATIVE)
	int rawSin;
	int rawWidthZ;
	register int negWidthZ CTR_PSX_REGISTER("$2");
	int angle;
	register int widthProduct CTR_PSX_REGISTER("$3");
#endif
	int x;
	int z;
	register u32 frame CTR_PSX_REGISTER("$8");
	register TerrainFlags flagsValue CTR_PSX_REGISTER("$11");
	register u8 *currentMarkBase CTR_PSX_REGISTER("$3");
	register u8 *previousMarkBase CTR_PSX_REGISTER("$5");
	register int halfWidthX CTR_PSX_REGISTER("$4");
	register int halfWidthZ CTR_PSX_REGISTER("$3");

	CTR_PSX_KEEP_VALUE(flags);
	if ((inst->flags & SPLIT_LINE) == 0)
	{
		goto skidColorNormal;
	}
	rawColor = inst->depthBiasSecondary;
	goto skidColorDone;

skidColorNormal:
	rawColor = inst->depthBiasNormal;

skidColorDone:

	color = rawColor + VEH_EMITTER_SKID_COLOR_BIAS;
	MATH_Cos(d->axisRotationX);
#if defined(CTR_NATIVE)
	sin = MATH_Sin(d->axisRotationX);
	lateralZ = VEH_EMITTER_SKID_LATERAL_SCALE;
	CTR_PSX_KEEP_VALUE(lateralZ);
	sin *= lateralZ;
	cos = MATH_Cos(d->axisRotationX);
	lateralX = VEH_EMITTER_SKID_WIDTH_SCALE;
	MATH_Sin(d->axisRotationX);
	widthX = MATH_Cos(d->axisRotationX);
	MATH_Sin(d->axisRotationX);
	widthX *= lateralX;
	cos *= lateralZ;
	MATH_Cos(d->axisRotationX);
	widthZ = MATH_Sin(d->axisRotationX);
	widthZ *= -lateralX;
	lateralX = sin >> FRACTIONAL_BITS;
	lateralZ = cos >> FRACTIONAL_BITS;
	widthX >>= FRACTIONAL_BITS;
	widthZ >>= FRACTIONAL_BITS;
#else
	rawSin = MATH_Sin(d->axisRotationX);
	lateralZ = VEH_EMITTER_SKID_LATERAL_SCALE;
	CTR_PSX_KEEP_VALUE(lateralZ);
	VEH_EMITTER_MUL_RAW(rawSin, lateralZ);
	angle = d->axisRotationX;
	VEH_EMITTER_READ_LO(sin);
	cos = MATH_Cos(angle);
	lateralX = VEH_EMITTER_SKID_WIDTH_SCALE;
	MATH_Sin(d->axisRotationX);
	widthX = MATH_Cos(d->axisRotationX);
	MATH_Sin(d->axisRotationX);
	VEH_EMITTER_MUL_RAW(widthX, lateralX);
	VEH_EMITTER_READ_LO(widthProduct);
	VEH_EMITTER_HILO_DELAY();
	VEH_EMITTER_MUL_RAW(cos, lateralZ);
	angle = d->axisRotationX;
	VEH_EMITTER_READ_LO(cos);
	widthX = widthProduct >> FRACTIONAL_BITS;
	MATH_Cos(angle);
	angle = d->axisRotationX;
	lateralZ = cos >> FRACTIONAL_BITS;
	rawWidthZ = MATH_Sin(angle);
	VEH_EMITTER_MUL_RAW(rawWidthZ, lateralX);
	lateralX = sin >> FRACTIONAL_BITS;
	VEH_EMITTER_READ_LO(widthProduct);
	negWidthZ = -widthProduct;
	widthZ = negWidthZ >> FRACTIONAL_BITS;
#endif

	VehGteSetRotTransMatrix(&inst->matrix);

	if ((d->actionsFlagSet & ACTION_BACK_SKID) != 0)
	{
		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_BACK_LEFT;
		flagsValue = (u16)VEH_EMITTER_TIRE_LEFT_X;
		MTC2(flagsValue, 0);
		flagsValue = VEH_EMITTER_TIRE_BACK_Z;
		MTC2(flagsValue, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		posX = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);
		skidOffset = sin >> (FRACTIONAL_BITS + VEH_EMITTER_SKID_HALF_LATERAL_SHIFT);
		x = posX - skidOffset;
		skidOffset = lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		frame = d->skidmarkFrameIndex;
		z = posZ - skidOffset;
		skidOffset = (u32)frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		currentMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(currentMarkBase);
		skidOffset = x + widthX;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge0.x = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge0.y = (s16)posY;
		skidOffset = z + widthZ;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge0.z = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge1.x = (s16)(x - widthX);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.edge1.z = (s16)(z - widthZ);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xc4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			currentMarkBase[0xcb] = flags | 1;
		}
		else
		{
			currentMarkBase[0xcb] = flags;
		}
		x += lateralX;
		z += lateralZ;
		skidOffset = frame - 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = (u32)frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		previousMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(previousMarkBase);
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge0.x = (s16)(x + (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge0.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge0.z = (s16)(z + (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge1.x = (s16)(x - (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.edge1.z = (s16)(z - (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xc4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			previousMarkBase[0xcb] = flags | 1;
		}
		else
		{
			previousMarkBase[0xcb] = flags;
		}

		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_BACK_RIGHT;
		flagsValue = VEH_EMITTER_TIRE_RIGHT_X;
		MTC2(flagsValue, 0);
		flagsValue = VEH_EMITTER_TIRE_BACK_Z;
		MTC2(flagsValue, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		posX = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);
		skidOffset = frame + 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		x = posX - skidOffset;
		skidOffset = lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		z = posZ - skidOffset;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		currentMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(currentMarkBase);
		skidOffset = x + widthX;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge0.x = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge0.y = (s16)posY;
		skidOffset = z + widthZ;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge0.z = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge1.x = (s16)(x - widthX);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.edge1.z = (s16)(z - widthZ);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xd4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			currentMarkBase[0xdb] = flags | 1;
		}
		else
		{
			currentMarkBase[0xdb] = flags;
		}
		x += lateralX;
		z += lateralZ;
		skidOffset = frame - 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		previousMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(previousMarkBase);
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge0.x = (s16)(x + (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge0.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge0.z = (s16)(z + (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge1.x = (s16)(x - (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.edge1.z = (s16)(z - (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xd4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			previousMarkBase[0xdb] = flags | 1;
		}
		else
		{
			previousMarkBase[0xdb] = flags;
		}
	}

	if ((d->actionsFlagSet & ACTION_FRONT_SKID) != 0)
	{
		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_FRONT_LEFT;
		flagsValue = (u16)VEH_EMITTER_TIRE_LEFT_X;
		MTC2(flagsValue, 0);
		flagsValue = VEH_EMITTER_TIRE_FRONT_Z;
		MTC2(flagsValue, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		posX = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);
		skidOffset = lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		x = posX - skidOffset;
		skidOffset = lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		frame = d->skidmarkFrameIndex;
		z = posZ - skidOffset;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		currentMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(currentMarkBase);
		skidOffset = x + widthX;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge0.x = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge0.y = (s16)posY;
		skidOffset = z + widthZ;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge0.z = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge1.x = (s16)(x - widthX);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.edge1.z = (s16)(z - widthZ);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xe4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			currentMarkBase[0xeb] = flags | 1;
		}
		else
		{
			currentMarkBase[0xeb] = flags;
		}
		x += lateralX;
		z += lateralZ;
		skidOffset = frame - 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		previousMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(previousMarkBase);
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge0.x = (s16)(x + (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge0.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge0.z = (s16)(z + (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge1.x = (s16)(x - (widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.edge1.z = (s16)(z - (widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT));
		((union VehEmitterSkidmark *)(previousMarkBase + 0xe4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			previousMarkBase[0xeb] = flags | 1;
		}
		else
		{
			previousMarkBase[0xeb] = flags;
		}

		d->skidmarkEnableFlags |= DRIVER_SKIDMARK_FRONT_RIGHT;
		flagsValue = VEH_EMITTER_TIRE_RIGHT_X;
		MTC2(flagsValue, 0);
		flagsValue = VEH_EMITTER_TIRE_FRONT_Z;
		MTC2(flagsValue, 1);
		CTR_PSX_GTE_PIPELINE_DELAY();
		gte_rt();
		posX = MFC2_S(25);
		posY = MFC2_S(26);
		posZ = MFC2_S(27);
		skidOffset = frame + 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = lateralX >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		x = posX - skidOffset;
		skidOffset = lateralZ >> VEH_EMITTER_SKID_HALF_LATERAL_SHIFT;
		z = posZ - skidOffset;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		currentMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(currentMarkBase);
		skidOffset = x + widthX;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge0.x = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge0.y = (s16)posY;
		skidOffset = z + widthZ;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge0.z = (s16)skidOffset;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge1.x = (s16)(x - widthX);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.edge1.z = (s16)(z - widthZ);
		((union VehEmitterSkidmark *)(currentMarkBase + 0xf4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			currentMarkBase[0xfb] = flags | 1;
		}
		else
		{
			currentMarkBase[0xfb] = flags;
		}
		x += lateralX;
		z += lateralZ;
		skidOffset = frame - 1;
		frame = skidOffset & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		skidOffset = frame << 6;
		CTR_PSX_KEEP_VALUE(skidOffset);
		previousMarkBase = (u8 *)d + skidOffset;
		CTR_PSX_KEEP_VALUE(previousMarkBase);
		halfWidthX = widthX >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT;
		skidOffset = x + halfWidthX;
		halfWidthZ = widthZ >> VEH_EMITTER_SKID_PREVIOUS_WIDTH_SHIFT;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge0.x = (s16)skidOffset;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge0.y = (s16)posY;
		skidOffset = z + halfWidthZ;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge0.z = (s16)skidOffset;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge1.x = (s16)(x - halfWidthX);
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge1.y = (s16)posY;
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.edge1.z = (s16)(z - halfWidthZ);
		((union VehEmitterSkidmark *)(previousMarkBase + 0xf4))->fields.color = color;
		VEH_EMITTER_LOAD_TERRAIN_FLAGS(flagsValue, terrainFlags);
		skidOffset = flagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(skidOffset);
		if (skidOffset != 0)
		{
			forcedFlags = flags | 1;
			previousMarkBase[0xfb] = forcedFlags;
			goto skidmarksDone;
		}
		else
		{
			previousMarkBase[0xfb] = flags;
			goto skidmarksDone;
		}
	}

skidmarksDone:;
}

static inline void VehEmitter_MudSplash(struct Driver *driverArg)
{
	register struct Driver *d CTR_PSX_REGISTER("$19") = driverArg;
	struct Particle *p;

	if ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) != 0)
	{
		register int count CTR_PSX_REGISTER("$16") = VEH_EMITTER_MUD_SPLASH_LANDING_COUNT;

		do
		{
			p = Particle_Init(0, GAME_TRACKER->iconGroup[VEH_EMITTER_MUD_SPLASH_ICON], &data.emSet_MudSplash[0]);
			if (p != NULL)
			{
				p->otIndexOffset = d->instSelf->depthBiasNormal;
				p->owner.driverInst = d->instSelf;
				p->driverID = d->driverID;
				p->axis[0].startVal += (int)p->axis[0].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
				p->axis[2].startVal += (int)p->axis[2].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
				p->axis[0].accel -= p->axis[0].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
				p->axis[2].accel -= p->axis[2].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
			}
			count--;
		} while (count != 0);
	}
	else
	{
		p = Particle_Init(0, GAME_TRACKER->iconGroup[VEH_EMITTER_MUD_SPLASH_ICON], &data.emSet_MudSplash[0]);
		if (p == NULL)
		{
			return;
		}

		p->otIndexOffset = d->instSelf->depthBiasNormal;
		p->owner.driverInst = d->instSelf;
		p->driverID = d->driverID;

		p->axis[0].startVal += (int)p->axis[0].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
		p->axis[2].startVal += (int)p->axis[2].velocity * VEH_EMITTER_MUD_SPLASH_VELOCITY_SCALE;
		p->axis[0].accel -= p->axis[0].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
		p->axis[2].accel -= p->axis[2].velocity >> VEH_EMITTER_MUD_SPLASH_VELOCITY_SHIFT;
	}
}

void VehEmitter_DriverMain(struct Thread *thread, struct Driver *driverArg)
{
	register struct Driver *d CTR_PSX_REGISTER("$19") = driverArg;
	register struct Terrain *terrain CTR_PSX_REGISTER("$18");
	register struct Instance *inst CTR_PSX_REGISTER("$23");
	VECTOR exhaustPos;
	VECTOR exhaustVel;
	volatile TerrainFlags terrainFlags;
	register int skidSound CTR_PSX_REGISTER("$20");
	register u32 skidFlags CTR_PSX_REGISTER("$22");
	register int absSpeedApprox CTR_PSX_REGISTER("$4");
	register TerrainFlags terrainFlagTest CTR_PSX_REGISTER("$2");
	u32 skidActions;
	register int jogValue CTR_PSX_REGISTER("$5");
	int jogDuration;
	register struct Driver *jogDriver CTR_PSX_REGISTER("$4");
	register int initialModelIndex CTR_PSX_REGISTER("$2");
	register int playerModelIndex CTR_PSX_REGISTER("$3");

	{
		register u32 skidHistory CTR_PSX_REGISTER("$3");
		register u32 skidIndex CTR_PSX_REGISTER("$2");
		register u32 skidMask CTR_PSX_REGISTER("$4");
		register struct Thread *initialThread CTR_PSX_REGISTER("$11");

		skidMask = 0xffffff;
		CTR_PSX_KEEP_VALUE(skidMask);
		skidHistory = d->skidmarkEnableFlags;
		VEH_EMITTER_TIE_THREAD_ARGUMENT_AFTER(initialThread, thread, skidHistory);
		skidIndex = d->skidmarkFrameIndex;
		terrain = d->terrainMeta1;
		skidHistory = (skidHistory << DRIVER_SKIDMARK_HISTORY_SHIFT) & skidMask;
		skidIndex--;
		VEH_EMITTER_RETAIN_THREAD_ARGUMENT(initialThread, skidIndex);
		inst = initialThread->inst;
		skidIndex &= DRIVER_SKIDMARK_FRAME_INDEX_MASK;
		d->skidmarkEnableFlags = skidHistory;
		d->skidmarkFrameIndex = (u8)skidIndex;
	}
	{
		register TerrainFlags initialTerrainFlags CTR_PSX_REGISTER("$11");
		register struct Thread *modelThread CTR_PSX_REGISTER("$11");

		initialTerrainFlags = terrain->flags;
		CTR_PSX_KEEP_VALUE(initialTerrainFlags);
		terrainFlags = initialTerrainFlags;
		VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(modelThread, thread, initialTerrainFlags);
		initialModelIndex = modelThread->modelIndex;
	}
	skidSound = terrain->skidSound;
	playerModelIndex = DYNAMIC_PLAYER;
	CTR_PSX_KEEP_VALUE(playerModelIndex);

	if (initialModelIndex == playerModelIndex)
	{
		register TerrainFlags terrainFlagsValue CTR_PSX_REGISTER("$11");
		register int soundID CTR_PSX_REGISTER("$17") = -1;
		register int vol CTR_PSX_REGISTER("$16");
		register int mapMin CTR_PSX_REGISTER("$5");
		register int volumeSpeedMax CTR_PSX_REGISTER("$6");
		register int volumeMin CTR_PSX_REGISTER("$7");
		register int distort CTR_PSX_REGISTER("$5");
		register u32 volumeFlags CTR_PSX_REGISTER("$6");
		register u32 audioFlags CTR_PSX_REGISTER("$2");
		register u32 *terrainAudioPtr CTR_PSX_REGISTER("$4");

		if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0)
		{
			terrainFlagsValue = terrainFlags;
			CTR_PSX_KEEP_VALUE(terrainFlagsValue);
			terrainFlagTest = terrainFlagsValue & TERRAIN_FLAG_ONESHOT_GROUND_SOUND;
			CTR_PSX_KEEP_VALUE(terrainFlagTest);
			if (terrainFlagTest == 0)
			{
				soundID = terrain->sound;
			}
		}
		mapMin = 0;
		CTR_PSX_KEEP_VALUE(mapMin);
		volumeSpeedMax = VEH_EMITTER_TERRAIN_AUDIO_SPEED_MAX;
		volumeMin = mapMin;
		CTR_PSX_KEEP_VALUE(volumeSpeedMax);
		CTR_PSX_KEEP_VALUE(volumeMin);

		{
			register int volumeMax CTR_PSX_REGISTER("$2");
			int speed = d->speedApprox;
			volumeMax = VEH_EMITTER_TERRAIN_AUDIO_VOLUME_MAX;
			CTR_PSX_KEEP_VALUE(volumeMax);
			if (speed < 0)
			{
				speed = -speed;
			}
			vol = VehCalc_MapToRange(speed, mapMin, volumeSpeedMax, volumeMin, volumeMax);
		}

		{
			register int distortInputMin CTR_PSX_REGISTER("$5");
			register int distortSpeedMax CTR_PSX_REGISTER("$6");
			register int distortMin CTR_PSX_REGISTER("$7");
			register int distortMax CTR_PSX_REGISTER("$2");
			int speed = d->speedApprox;
			distortInputMin = 0;
			distortSpeedMax = VEH_EMITTER_TERRAIN_AUDIO_DISTORT_SPEED_MAX;
			distortMin = VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MIN;
			CTR_PSX_KEEP_VALUE(distortInputMin);
			CTR_PSX_KEEP_VALUE(distortSpeedMax);
			CTR_PSX_KEEP_VALUE(distortMin);
			distortMax = VEH_EMITTER_TERRAIN_AUDIO_DISTORT_MAX;
			CTR_PSX_KEEP_VALUE(distortMax);
			if (speed < 0)
			{
				speed = -speed;
			}
			distort = VehCalc_MapToRange(speed, distortInputMin, distortSpeedMax, distortMin, distortMax);
		}
		terrainAudioPtr = &d->driverAudioPtrs[1];
		distort <<= 8;
		volumeFlags = (u32)vol << 16;
		if ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
		{
			audioFlags = (u32)distort | 0x1000000;
			audioFlags = volumeFlags | audioFlags;
		}
		else
		{
			audioFlags = volumeFlags | (u32)distort;
		}
		volumeFlags = audioFlags | HOWL_SFX_LR_CENTER;
		OtherFX_RecycleNew(terrainAudioPtr, soundID, volumeFlags);

		if ((d->actionsFlagSet & ACTION_BOT) == 0)
		{
			{
				int speed = d->speedApprox;
				if (speed < 0)
				{
					speed = -speed;
				}
				if (speed > VEH_EMITTER_TERRAIN_RUMBLE_MIN_SPEED)
				{
					GAMEPAD_ShockFreq(d, terrain->vibrationData[0], terrain->vibrationData[1]);
					GAMEPAD_ShockForce2(d, terrain->vibrationData[2], terrain->vibrationData[3]);
				}
			}

			if ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) != 0)
			{
				int absJump = d->jumpHeightPrev;
				if (absJump < 0)
				{
					absJump = -absJump;
				}
				if (absJump > VEH_EMITTER_LANDING_SPARK_MIN_JUMP)
				{
					GAMEPAD_ShockForce1(d, VEH_EMITTER_LANDING_RUMBLE_FRAMES, VEH_EMITTER_RUMBLE_FORCE_MAX);
				}
			}
		}
	}

  if (GAME_TRACKER->numPlyrCurrGame < 2 || sdata->highDetailSplitScreenLevel)
	{
		register TerrainFlags terrainFlagsValue CTR_PSX_REGISTER("$11");
		int absSpeed = d->speed;
		register int wallSound CTR_PSX_REGISTER("$17");

		if (absSpeed < 0)
		{
			absSpeed = -absSpeed;
		}
		if ((absSpeed > VEH_EMITTER_MUD_MIN_SPEED) && (d->currentTerrain == TERRAIN_MUD))
		{
			VehEmitter_MudSplash(d);
		}

		terrainFlagsValue = terrainFlags;
		CTR_PSX_KEEP_VALUE(terrainFlagsValue);
		terrainFlagTest = terrainFlagsValue & TERRAIN_FLAG_LANDING_SPARKS;
		CTR_PSX_KEEP_VALUE(terrainFlagTest);
		if ((terrainFlagTest != 0) && ((d->actionsFlagSet & ACTION_STARTED_TOUCH_GROUND) != 0))
		{
			int landingSpeed = d->speedApprox;
			int absJump;

			if (landingSpeed < 0)
			{
				landingSpeed = -landingSpeed;
			}
			if (landingSpeed > VEH_EMITTER_LANDING_SPARK_MIN_SPEED)
			{
				absJump = d->jumpHeightPrev;
				if (absJump < 0)
				{
					absJump = -absJump;
				}
				if (absJump > VEH_EMITTER_LANDING_SPARK_MIN_JUMP)
				{
					VehGteSetRotTransMatrix(&inst->matrix);
					VehEmitter_Sparks_Ground(d, &data.emSet_GroundSparks[0]);
				}
			}
		}

		if (terrain->em_OddFrame != NULL)
		{
			struct ParticleEmitter *emSet;

			VehGteSetRotTransMatrix(&inst->matrix);
			CTR_PSX_MEMORY_BARRIER();
			emSet = terrain->em_EvenFrame;
			if ((emSet == NULL) || ((GAME_TRACKER->timer & 1) == 0))
			{
				emSet = terrain->em_OddFrame;
			}
			VehEmitter_Terrain_Ground(d, emSet);
		}

		if ((d->wallRubTimer == VEH_EMITTER_WALL_RUB_TIMER_FULL) && (d->kartState != KS_MASK_GRABBED))
		{
			VehGteSetRotTransMatrix(&inst->matrix);
			VehEmitter_Sparks_Wall(d, &data.emSet_WallSparks[0]);
			wallSound = VEH_EMITTER_WALL_RUB_SFX;
			d->engineVol += VEH_EMITTER_WALL_RUB_VOLUME_STEP;
			if ((s16)d->engineVol > HOWL_SFX_VOLUME_MAX)
			{
				d->engineVol = HOWL_SFX_VOLUME_MAX;
			}
		}
		else
		{
			if (d->wallRubTimer == 0)
			{
				d->frameAgainstWall = 0;
			}
			d->engineVol -= VEH_EMITTER_WALL_RUB_VOLUME_STEP;
			if ((s16)d->engineVol < 0)
			{
				d->engineVol = 0;
			}
			wallSound = VEH_EMITTER_WALL_RUB_SFX;
			if ((s16)d->engineVol == 0)
			{
				wallSound = -1;
			}
		}
		{
			register struct Thread *wallThread CTR_PSX_REGISTER("$11");

			VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(wallThread, thread, wallSound);
			if (wallThread->modelIndex == DYNAMIC_PLAYER)
			{
				u32 *wallAudioPtr = &d->driverAudioPtrs[2];
				u32 wallAudioFlags = (u32)(s16)d->engineVol << 16;

				if ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
				{
					wallAudioFlags |= 0x1008080;
				}
				else
				{
					wallAudioFlags |= 0x8080;
				}
				OtherFX_RecycleNew(wallAudioPtr, wallSound, wallAudioFlags);
			}
		}
	}

	{
		register TerrainFlags terrainFlagsValue CTR_PSX_REGISTER("$11");

		terrainFlagsValue = terrainFlags;
		CTR_PSX_KEEP_VALUE(terrainFlagsValue);
		terrainFlagTest = terrainFlagsValue & TERRAIN_FLAG_FORCE_SKIDMARKS;
		CTR_PSX_KEEP_VALUE(terrainFlagTest);
		if (terrainFlagTest != 0)
		{
			d->actionsFlagSet |= ACTION_BACK_SKID | ACTION_FRONT_SKID;
		}
	}

	{
		int matrixArray = d->matrixArray;

		if (matrixArray == BAKED_GTE_MATRIX_WHEELIE_START)
		{
			goto enableBackSkid;
		}
		if (matrixArray == BAKED_GTE_MATRIX_NONE)
		{
			goto wheelieSkidDone;
		}
		if (matrixArray > BAKED_GTE_MATRIX_WHEELIE_RECOVER)
		{
			goto wheelieSkidDone;
		}
		goto clearFrontSkid;

	enableBackSkid:
		d->actionsFlagSet |= ACTION_BACK_SKID;

	clearFrontSkid:
		d->actionsFlagSet &= ~ACTION_FRONT_SKID;

	wheelieSkidDone:;
	}

	skidActions = d->actionsFlagSet;
	if ((skidActions & ACTION_TOUCH_GROUND) == 0)
	{
		goto eraseSkidAudio;
	}
	if ((skidActions & (ACTION_BACK_SKID | ACTION_FRONT_SKID)) == 0)
	{
		goto eraseSkidAudio;
	}
	{
		int rawSpeedApprox = d->speedApprox;

		absSpeedApprox = rawSpeedApprox;
		if (rawSpeedApprox < 0)
		{
			absSpeedApprox = -absSpeedApprox;
		}
	}
	if (absSpeedApprox < VEH_EMITTER_SKID_AUDIO_MIN_SPEED)
	{
		goto eraseSkidAudio;
	}

	{
		register int playerModel CTR_PSX_REGISTER("$2");
		register int skidModelIndex CTR_PSX_REGISTER("$3");
		register struct Thread *skidThread CTR_PSX_REGISTER("$11");

		playerModel = DYNAMIC_PLAYER;
		CTR_PSX_KEEP_VALUE(playerModel);
		VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(skidThread, thread, playerModel);
		skidModelIndex = skidThread->modelIndex;
		VEH_EMITTER_ZERO_AFTER_LOAD(skidFlags, skidModelIndex);
		if (skidModelIndex != playerModel)
		{
			goto skidPlayerAudioDone;
		}
		CTR_PSX_FORGET_VALUE(skidFlags);
		{
			register int absTurn CTR_PSX_REGISTER("$17");
			register int vol CTR_PSX_REGISTER("$16");
			register int distort CTR_PSX_REGISTER("$5");
			register int inputMin CTR_PSX_REGISTER("$5");
			register int inputMax CTR_PSX_REGISTER("$6");
			register int outputMin CTR_PSX_REGISTER("$7");
			register int outputMax CTR_PSX_REGISTER("$2");
			register int rawTurn CTR_PSX_REGISTER("$3");
			register int halfTurn CTR_PSX_REGISTER("$3");
			register int panFlags CTR_PSX_REGISTER("$6");
			register u32 volumeFlags CTR_PSX_REGISTER("$7");
			register u32 audioFlags CTR_PSX_REGISTER("$2");
			register u32 *audioPtr CTR_PSX_REGISTER("$4");
			int mappedVolume;
			int panOffset;

			inputMin = VEH_EMITTER_SKID_AUDIO_SPEED_MIN;
			inputMax = VEH_EMITTER_SKID_AUDIO_SPEED_MAX;
			outputMin = VEH_EMITTER_SKID_AUDIO_VOLUME_MIN;
			CTR_PSX_KEEP_VALUE(inputMin);
			CTR_PSX_KEEP_VALUE(inputMax);
			CTR_PSX_KEEP_VALUE(outputMin);
			rawTurn = d->simpTurnState;
			outputMax = VEH_EMITTER_SKID_AUDIO_VOLUME_MAX;
			CTR_PSX_KEEP_VALUE(outputMax);
			absTurn = rawTurn;
			if (rawTurn < 0)
			{
				absTurn = -absTurn;
			}
			mappedVolume = VehCalc_MapToRange(absSpeedApprox, inputMin, inputMax, outputMin, outputMax);
			halfTurn = absTurn >> 1;
			CTR_PSX_KEEP_VALUE(halfTurn);
			vol = mappedVolume + halfTurn;
			{
				inputMin = VEH_EMITTER_SKID_AUDIO_SPEED_MIN;
				inputMax = VEH_EMITTER_SKID_AUDIO_SPEED_MAX;
				outputMin = VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW;
				CTR_PSX_KEEP_VALUE(inputMin);
				CTR_PSX_KEEP_VALUE(inputMax);
				CTR_PSX_KEEP_VALUE(outputMin);
				absSpeedApprox = d->speedApprox;
				CTR_PSX_KEEP_VALUE(absSpeedApprox);
				outputMax = VEH_EMITTER_SKID_AUDIO_DISTORT_FAST;
				CTR_PSX_KEEP_VALUE(outputMax);
				if (absSpeedApprox < 0)
				{
					absSpeedApprox = -absSpeedApprox;
				}
				distort = VehCalc_MapToRange(absSpeedApprox, inputMin, inputMax, outputMin, outputMax);
			}
			if (d->kartState == KS_DRIFTING)
			{
				int drift = d->turnWobbleAngle;
				if (drift < 0)
				{
					drift = -drift;
				}
				distort -= drift;
				if (distort < 0)
				{
					distort = skidFlags;
				}
			}
			distort += absTurn;
			if (distort > VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW)
			{
				distort = VEH_EMITTER_SKID_AUDIO_DISTORT_SLOW;
			}
			panFlags = HOWL_SFX_LR_CENTER;
			CTR_PSX_KEEP_VALUE(panFlags);
			audioPtr = &d->driverAudioPtrs[0];
			volumeFlags = (u32)vol << 16;
			panOffset = CTR_MipsSra(CTR_MipsSll((u8)d->simpTurnState, 24), 26);
			panFlags = CTR_MipsSubLo(panFlags, panOffset);
			distort <<= 8;
			if ((d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0)
			{
				audioFlags = (u32)distort | 0x1000000;
				audioFlags = volumeFlags | audioFlags;
			}
			else
			{
				audioFlags = volumeFlags | (u32)distort;
			}
			panFlags = (int)(audioFlags | (u32)panFlags);
			CTR_PSX_KEEP_VALUE(panFlags);
			OtherFX_RecycleNew(audioPtr, skidSound, (u32)panFlags);
		}

	skidPlayerAudioDone:;
	}
	VehEmitter_Skidmarks(inst, d, VEH_EMITTER_TERRAIN_FLAGS_ARG(terrainFlags), skidFlags);
	goto skidAudioDone;

eraseSkidAudio:
	if (d->driverAudioPtrs[0] != 0)
	{
		OtherFX_Stop1((int)d->driverAudioPtrs[0]);
		d->driverAudioPtrs[0] = 0;
	}

skidAudioDone:

{
	struct GameTracker *gGT;
	u32 numPlyr;
	register TerrainFlags terrainFlagsValue CTR_PSX_REGISTER("$11");
	register struct Thread *exhaustThread CTR_PSX_REGISTER("$11");
	register int exhaustModelIndex CTR_PSX_REGISTER("$3");
	register int robotModelIndex CTR_PSX_REGISTER("$2");
	register int packedPosition CTR_PSX_REGISTER("$3");
	register int packedComponent CTR_PSX_REGISTER("$2");
	register int positionZ CTR_PSX_REGISTER("$2");
	register VECTOR *exhaustPosPtr CTR_PSX_REGISTER("$16");
	register VECTOR *exhaustVelPtr CTR_PSX_REGISTER("$17");
	register struct Driver *exhaustDriverArg CTR_PSX_REGISTER("$4");
	register VECTOR *exhaustPosArg CTR_PSX_REGISTER("$5");

	VEH_EMITTER_TIE_THREAD_ARGUMENT(exhaustThread, thread);
	exhaustModelIndex = exhaustThread->modelIndex;
	CTR_PSX_KEEP_VALUE(exhaustModelIndex);
	robotModelIndex = DYNAMIC_ROBOT_CAR;
	CTR_PSX_KEEP_VALUE(robotModelIndex);
	if (exhaustModelIndex == robotModelIndex)
	{
		if ((GAME_TRACKER->timer & 3) != (d->driverID & 3))
		{
			goto exhaustDone;
		}
	}
	else
	{
		if (d->revEngineState == 2)
		{
			goto exhaustDone;
		}

		gGT = GAME_TRACKER;
		numPlyr = gGT->numPlyrCurrGame;
		if (numPlyr >= 2 && !sdata->highDetailSplitScreenLevel)
		{
			if (numPlyr == 2)
			{
				if ((gGT->timer & 1) == d->driverID)
				{
					goto exhaustAllowed;
				}
			}
			if ((gGT->timer & 3) != d->driverID)
			{
				goto exhaustDone;
			}
		}

	exhaustAllowed:
		if (d->failedBoostExhaustTimer == 0)
		{
			int meterLeft = d->turbo_MeterRoomLeft;

			if ((meterLeft < VEH_EMITTER_TURBO_METER_COLOR_MIN) ||
			    (((d->const_turboLowRoomWarning + VEH_EMITTER_TURBO_ROOM_WARNING_PAD) * VEH_EMITTER_TURBO_ROOM_WARNING_SCALE) < meterLeft))
			{
				register struct Thread *searchThread CTR_PSX_REGISTER("$11");

				VEH_EMITTER_LOAD_THREAD_ARGUMENT_AFTER(searchThread, thread, meterLeft);
				if (PROC_SearchForModel(searchThread->childThread, STATIC_TURBO_EFFECT) != NULL)
				{
					goto exhaustDone;
				}
			}
		}
	}

	if (d->failedBoostExhaustTimer != 0)
	{
		d->failedBoostExhaustTimer--;
	}

	VehEmitter_SetRotMatrix(&inst->matrix);
	terrainFlagsValue = CTR_PackS16Pair(0, VEH_EMITTER_EXHAUST_VEL_Y);
	MTC2(terrainFlagsValue, 0);
	terrainFlagsValue = VEH_EMITTER_EXHAUST_VEL_Z;
	MTC2(terrainFlagsValue, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	VEH_EMITTER_STORE_EXHAUST_VELOCITY(&exhaustVel);

	packedPosition = ((inst->scale.x * VEH_EMITTER_EXHAUST_POS_X_NUM) >> VEH_EMITTER_EXHAUST_POS_X_SHIFT) & 0xffff;
	packedComponent = ((inst->scale.y * VEH_EMITTER_EXHAUST_POS_Y_NUM) >> VEH_EMITTER_EXHAUST_POS_Y_SHIFT) << 16;
	packedPosition |= packedComponent;
	MTC2(packedPosition, 0);
	positionZ = (inst->scale.z * VEH_EMITTER_EXHAUST_POS_Z_NUM) >> VEH_EMITTER_EXHAUST_POS_Z_SHIFT;
	MTC2(positionZ, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	exhaustPosPtr = &exhaustPos;
	VEH_EMITTER_STORE_MAC_DIRECT(exhaustPosPtr);
	exhaustDriverArg = d;
	exhaustPosArg = exhaustPosPtr;
	CTR_PSX_OBSERVE_VALUE(exhaustDriverArg);
	CTR_PSX_OBSERVE_VALUE(exhaustPosArg);
	exhaustVelPtr = &exhaustVel;
	VehEmitter_Exhaust(exhaustDriverArg, exhaustPosArg, exhaustVelPtr);

	packedPosition = ((inst->scale.x * VEH_EMITTER_EXHAUST_POS_SECOND_X_NUM) >> VEH_EMITTER_EXHAUST_POS_SECOND_X_SHIFT) & 0xffff;
	packedComponent = ((inst->scale.y * VEH_EMITTER_EXHAUST_POS_Y_NUM) >> VEH_EMITTER_EXHAUST_POS_Y_SHIFT) << 16;
	packedPosition |= packedComponent;
	MTC2(packedPosition, 0);
	positionZ = (inst->scale.z * VEH_EMITTER_EXHAUST_POS_Z_NUM) >> VEH_EMITTER_EXHAUST_POS_Z_SHIFT;
	MTC2(positionZ, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_rtv0();
	VEH_EMITTER_STORE_MAC_DIRECT(exhaustPosPtr);
	VehEmitter_Exhaust(d, exhaustPosPtr, exhaustVelPtr);
}

exhaustDone:

	if (d->burnTimer != 0)
	{
		d->alphaScaleBackup = VEH_EMITTER_ALPHA_FULL;
		d->instSelf->alphaScale = VEH_EMITTER_ALPHA_FULL;
	}

	if (d->invisibleTimer != 0)
	{
		inst->alphaScale = VEH_EMITTER_ALPHA_FULL;
	}

	if ((d->kartState != KS_NORMAL) && (d->kartState != KS_DRIFTING))
	{
		d->actionsFlagSet &= ~ACTION_AIRBORNE;
	}

	jogDriver = d;
	if ((d->kartState == KS_ENGINE_REVVING) || (d->kartState == KS_MASK_GRABBED))
	{
		goto jogGroundSetValue;
	}

	// NOTE(aalhendi): Retail reaches the ground jog call through two entries;
	// keeping both paths explicit preserves its branch-delay scheduling.
	CTR_PSX_MEMORY_BARRIER();
	jogValue = VEH_EMITTER_JOG_GROUND;
	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0)
	{
		goto jogGroundCall;
	}

	{
		int landingBoost;
		int canJog;
		int airJogValue;

		landingBoost = d->jump_LandingBoost;
		canJog = landingBoost < VEH_EMITTER_JOG_AIR_LANDING_BOOST_MAX;
		airJogValue = 0;
		if (canJog != 0)
		{
			airJogValue = VEH_EMITTER_JOG_AIR_TURN_NEGATIVE;
			if ((d->simpTurnState < 0) || ((airJogValue = VEH_EMITTER_JOG_AIR_TURN_POSITIVE), (d->simpTurnState > 0)))
			{
				VEH_EMITTER_JOG_CON1_CURRENT_DRIVER(d, airJogValue, VEH_EMITTER_JOG_AIR_DURATION);
				jogDriver = d;
			}
			airJogValue = 0;
		}
		jogValue = airJogValue;
		jogDuration = jogValue;
	}
	goto jogFinalCall;

jogGroundSetValue:
	jogValue = VEH_EMITTER_JOG_GROUND;

jogGroundCall:
	VEH_EMITTER_JOG_CON2(jogDriver, jogValue, 0);
	if (d->turnWobbleAngle == 0)
	{
		return;
	}
	jogDriver = d;
	if ((GAME_TRACKER->timer & VEH_EMITTER_JOG_WOBBLE_TIMER_MASK) != 0)
	{
		CTR_PSX_MEMORY_BARRIER();
		jogValue = VEH_EMITTER_JOG_WOBBLE_ALT;
	}
	else
	{
		CTR_PSX_MEMORY_BARRIER();
		jogValue = VEH_EMITTER_JOG_GROUND;
	}
	jogDuration = VEH_EMITTER_JOG_WOBBLE_DURATION;

jogFinalCall:
	VEH_EMITTER_JOG_CON2(jogDriver, jogValue, jogDuration);
}
