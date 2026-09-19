#include "VehCommon.h"

enum
{
	VEH_PHYS_ANGULAR_STICK_MIN_SPEED = 0x10,
	VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD = 0x300,
	VEH_PHYS_ANGULAR_STEER_ACCEL_COMPARE_SPEED = 0x2ff,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE = 0x32,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE = 100,
	VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE = 50,
	VEH_PHYS_ANGULAR_DRIFT_SPINOUT_TIME = 0x140,
	VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT = 0x10,
	VEH_PHYS_ANGULAR_CLASS_SPEED_HALF_SHIFT = 0x11,
	VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL = 0x100,
	VEH_PHYS_ANGULAR_TURN_ASSIST_MIN_DELTA = 3,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA = 10,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_TIMER = 8,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY = 0x14,
	VEH_PHYS_ANGULAR_TURN_WOBBLE_DISABLE_ANGLE = 0x32,
	VEH_PHYS_ANGULAR_AIR_TURN_SPEED_MAX = 0x600,
	VEH_PHYS_ANGULAR_ANGLE_MASK = 0xfff,
	VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT = 5,
	VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT = 0xd,
	VEH_PHYS_ANGULAR_BRAKE_LEAN_SCALE = 10,

	VEH_PHYS_JUMP_NORMAL_Y_MIN = 0x15,
	VEH_PHYS_JUMP_TURBO_PAD_ACCEL = 8000,
	VEH_PHYS_JUMP_REVERSE_SLIDE_SPEED_COMPARE = 0x2ff,
	VEH_PHYS_JUMP_TERRAIN_SCALE_NEUTRAL = 0x100,
	VEH_PHYS_JUMP_FAST_SQRT_ITERATIONS = 0x10,
	VEH_PHYS_JUMP_SPEED_FIXED_SHIFT = 8,
	VEH_PHYS_JUMP_HIGH_TIMER_MS = 0x180,
	VEH_PHYS_JUMP_FORCED_MS = 0xa0,
	VEH_PHYS_JUMP_COOLDOWN_MS = 0x180,
	VEH_PHYS_JUMP_RUMBLE_CHANNEL = 8,
	VEH_PHYS_JUMP_RUMBLE_FORCE = 0x7f,
	VEH_PHYS_JUMP_SPRING_SFX = 9,
	VEH_PHYS_JUMP_NORMAL_SFX = 8,
	VEH_PHYS_JUMP_FORCED_SFX = 0x7e,
	VEH_PHYS_JUMP_VERTICAL_SPEED_DEFAULT = 0x3700,
	VEH_PHYS_JUMP_VERTICAL_SPEED_MAX = 0x5000,
	VEH_PHYS_JUMP_SPEEDOMETER_REVERSE_THRESHOLD = 0x100,
	VEH_PHYS_JUMP_SPEEDOMETER_DECAY_SHIFT = 3,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD = 0xd,
	VEH_PHYS_JUMP_SPEEDOMETER_TIMER_MASK = 7,
	VEH_PHYS_JUMP_SPEEDOMETER_TIMER_SCALE = 0x300,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT = 4,
	VEH_PHYS_JUMP_SPEEDOMETER_BLEND_NEW = 3,
	VEH_PHYS_JUMP_NORMAL_VECTOR_STRIDE = 8,
};


void VehPhysGeneral_PhysAngular(struct Thread *thread, struct Driver *driverArg)
{
	register struct Driver *driver CTR_PSX_REGISTER("$18") = driverArg;
	int speedApprox;
	int elapsedTimeMS;
	int classSpeed_original;
	register int classSpeedPacked CTR_PSX_REGISTER("$6");
	register int turnResistMaxProduct CTR_PSX_REGISTER("$4");
	register int turnResistMinProduct CTR_PSX_REGISTER("$3");
	int driverSpeed;
	struct Terrain *terrain;
	register int rotCurrW_original CTR_PSX_REGISTER("$17");
	register int turnResistMinBitshift CTR_PSX_REGISTER("$19");
	register int turnResistMaxBitshift CTR_PSX_REGISTER("$21");
	register int driftAngleValue CTR_PSX_REGISTER("$22");
	int turnResistMin;
	register int wobbleTimer CTR_PSX_REGISTER("$20");
	u32 actionsFlagSet;
	register int turnAngleValue CTR_PSX_REGISTER("$23");
	int rotCurrW_interp;
	int modelRotVelocity;
	int interpolatedRot;

	(void)thread;
	CTR_PSX_OBSERVE_MEMORY(actionsFlagSet);
	CTR_PSX_OBSERVE_MEMORY(elapsedTimeMS);
	CTR_PSX_OBSERVE_MEMORY(terrain);
	CTR_PSX_OBSERVE_MEMORY(speedApprox);

	{
		register int remainingRot CTR_PSX_REGISTER("$16");
		int previousRot;

		remainingRot = driver->rotCurr.w;
		{
			register struct GameTracker *gGT CTR_PSX_REGISTER("$3");
			register int absRemaining CTR_PSX_REGISTER("$2");
			register int lerpStep CTR_PSX_REGISTER("$6");

			VEH_LOAD_GAME_TRACKER(gGT);
			absRemaining = remainingRot;
			if (remainingRot < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(absRemaining);
			}
			elapsedTimeMS = gGT->elapsedTimeMS;
			lerpStep = CTR_MipsSra(absRemaining, 3);
			if (lerpStep == 0)
			{
				lerpStep = 1;
			}
			{
				register int maxLerpStep CTR_PSX_REGISTER("$3") = (u8)driver->const_DriftCameraLerpStep;

				if (maxLerpStep < lerpStep)
				{
					lerpStep = maxLerpStep;
				}
			}

			previousRot = VehCalc_InterpBySpeed(driver->rotPrev.w, 8, lerpStep);
		}
		{
#if defined(CTR_NATIVE)
			register int frameTime CTR_PSX_REGISTER("$8") = elapsedTimeMS;

			driver->rotPrev.w = (s16)previousRot;
			frameTime = CTR_MipsMulLo(previousRot, frameTime);
			interpolatedRot = VehCalc_InterpBySpeed(remainingRot, CTR_MipsSra(frameTime, 5), 0);
#else
			register int interpolationResult CTR_PSX_REGISTER("$2") = previousRot;
			int hiState;
			int loState;

			// NOTE(aalhendi): Retail keeps the frame-time load, multiply result,
			// store, and interpolation call in one fixed scheduling window. The
			// operands and clobbers expose the embedded call's complete O32 boundary.
			__asm__ volatile(".word 0x8fa8001c\n\t"
			                 ".word 0x00000000\n\t"
			                 ".word 0x00480018\n\t"
			                 ".word 0x02002021\n\t"
			                 ".word 0x00003021\n\t"
			                 ".word 0xa64202fa\n\t"
			                 ".word 0x00004012\n\t"
			                 ".word 0x0c0163d5\n\t"
			                 ".word 0x00082943\n\t"
			                 "# %0 %3 %4 %5"
			                 : "+r"(interpolationResult), "=h"(hiState), "=l"(loState)
			                 : "r"(remainingRot), "r"(driver), "m"(elapsedTimeMS)
			                 : "$1", "$3", "$4", "$5", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "memory");
			interpolatedRot = interpolationResult;
#endif
		}
	}

	{
		register u32 loadedActions CTR_PSX_REGISTER("$8") = driver->actionsFlagSet;

		actionsFlagSet = loadedActions;
	}
	CTR_PSX_RELOAD(actionsFlagSet);
	CTR_PSX_MEMORY_BARRIER();
	{
		int initialDirection;
		register int rawSpeed CTR_PSX_REGISTER("$2");
		register int baseSpeed CTR_PSX_REGISTER("$3");

		CTR_PSX_LOAD_SIGNED_HALF(initialDirection, driver, offsetof(struct Driver, forwardDir), driver->forwardDir);
		CTR_PSX_BIND_VALUE_CLOBBER(initialDirection, "$4");
		{
			register int simpleTurn CTR_PSX_REGISTER("$3");

			CTR_PSX_LOAD_SIGNED_BYTE(simpleTurn, driver, offsetof(struct Driver, simpTurnState), driver->simpTurnState);
			CTR_PSX_OBSERVE_VALUE(simpleTurn);
			driver->rotCurr.w = (s16)interpolatedRot;
			CTR_PSX_MEMORY_BARRIER();
			CTR_PSX_LOAD_SIGNED_HALF(rawSpeed, driver, offsetof(struct Driver, speedApprox), driver->speedApprox);
			rotCurrW_original = CTR_MipsSll(simpleTurn, 8);
		}
		CTR_PSX_LOAD_SIGNED_HALF(baseSpeed, driver, offsetof(struct Driver, baseSpeed), driver->baseSpeed);
		if (rawSpeed < 1)
		{
			if (baseSpeed < 0)
			{
				initialDirection = -1;
				driver->forwardDir = -1;
			}
			if (-1 < rawSpeed)
			{
				goto LAB_8005fd74;
			}
		}
		else
		{
		LAB_8005fd74:
			if (-1 < baseSpeed)
			{
				initialDirection = 1;
				driver->forwardDir = 1;
			}
		}
		if (initialDirection < 0)
		{
#if defined(CTR_NATIVE)
			rotCurrW_original = CTR_MipsNegLo(rotCurrW_original);
			actionsFlagSet ^= ACTION_STEER_LEFT;
#else
			register u32 flippedActions CTR_PSX_REGISTER("$8");

			__asm__("lw %0,%1" : "=r"(flippedActions) : "m"(actionsFlagSet));
			rotCurrW_original = CTR_MipsNegLo(rotCurrW_original);
			flippedActions ^= ACTION_STEER_LEFT;
			__asm__("sw %1,%0" : "=m"(actionsFlagSet) : "r"(flippedActions));
#endif
		}
		{
			register u32 testedActions CTR_PSX_REGISTER("$8");

			CTR_PSX_LOAD_WORD(testedActions, actionsFlagSet);
			{
				register u32 touchesGround CTR_PSX_REGISTER("$2");

				if (rawSpeed < 0)
				{
					rawSpeed = CTR_MipsNegLo(rawSpeed);
				}
				speedApprox = rawSpeed;
				touchesGround = testedActions & ACTION_TOUCH_GROUND;
				CTR_PSX_OBSERVE_VALUE(testedActions);
				if (touchesGround != 0)
				{
					register u32 turboPadSteps CTR_PSX_REGISTER("$2") = driver->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK;

					if (turboPadSteps == 0)
					{
						register int mapValue CTR_PSX_REGISTER("$4") = speedApprox;
						register int mapMinimum CTR_PSX_REGISTER("$5") = VEH_PHYS_ANGULAR_STICK_MIN_SPEED;
						register int mapMaximum CTR_PSX_REGISTER("$6") = VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD;
						register int rangeMinimum CTR_PSX_REGISTER("$7");

#if defined(CTR_NATIVE)
						rangeMinimum = 0;
#else
						// NOTE(aalhendi): Tying the zero to the preceding arguments keeps
						// retail's final argument setup after the three immediate loads.
						__asm__("move %0,$0" : "=r"(rangeMinimum) : "r"(mapValue), "r"(mapMinimum), "r"(mapMaximum));
#endif
						rotCurrW_original = VehCalc_MapToRange(mapValue, mapMinimum, mapMaximum, rangeMinimum, rotCurrW_original);
					}
				}
			}
		}
		{
			register struct Terrain *loadedTerrain CTR_PSX_REGISTER("$8") = driver->terrainMeta1;

			terrain = loadedTerrain;
		}
		{
			register int currentRate CTR_PSX_REGISTER("$5");
			int mirrored;

			CTR_PSX_LOAD_SIGNED_HALF(currentRate, driver, offsetof(struct Driver, rotationSpinRate), driver->rotationSpinRate);
			if (rotCurrW_original == 0)
			{
#if defined(CTR_NATIVE)
				int rate;

				CTR_PSX_FORGET_VALUE(rotCurrW_original);

				rate =
				    CTR_MipsSra(CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE),
				                              terrain->turnResponseScale),
				                8);

				currentRate = VehCalc_InterpBySpeed(currentRate, rate, 0);
#else
				int rateProduct;
				int rateInput;
				register int rateScale CTR_PSX_REGISTER("$4");
				register int interpolationResult CTR_PSX_REGISTER("$2");
				int hiState;

				CTR_PSX_FORGET_VALUE(rotCurrW_original);
				rateInput = CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_COAST_SCALE);
				rateScale = terrain->turnResponseScale;
				__asm__ volatile("mult %2,%3" : "=h"(hiState), "=l"(rateProduct) : "r"(rateInput), "r"(rateScale));
				// NOTE(aalhendi): Retail consumes LO between its call setup and delay
				// slot. Exposing both multiply results also models the embedded call's
				// complete O32 boundary without changing that schedule.
				__asm__ volatile(".word 0x00003021\n\t"
				                 ".word 0x00a02021\n\t"
				                 ".word 0x00004012\n\t"
				                 ".word 0x0c0163d5\n\t"
				                 ".word 0x00082a03\n\t"
				                 "# %0 %1 %2 %3"
				                 : "=r"(interpolationResult), "+r"(currentRate), "+l"(rateProduct), "+h"(hiState)
				                 :
				                 : "$1", "$3", "$4", "$6", "$7", "$8", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "memory");
				currentRate = interpolationResult;
#endif
			}
			else
			{
				b32 crossedTarget;

				mirrored = 0;
				if (rotCurrW_original < 0)
				{
					rotCurrW_original = CTR_MipsNegLo(rotCurrW_original);
					currentRate = CTR_MipsNegLo(currentRate);
					mirrored = 1;
				}
				if (currentRate < rotCurrW_original)
				{
					int rate;
					register struct Terrain *rateTerrain CTR_PSX_REGISTER("$8");
					int rateInput;
					register int rateScale CTR_PSX_REGISTER("$4");

					CTR_PSX_RELOAD(terrain);
					rateTerrain = terrain;
#if defined(CTR_NATIVE)
					(void)rateInput;
					(void)rateScale;
					rate = CTR_MipsSra(
					    CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE),
					                  rateTerrain->turnResponseScale),
					    8);
#else
					rateInput = CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_ACCEL_SCALE);
					rateScale = rateTerrain->turnResponseScale;
					__asm__ volatile("mult %1,%2\n\tmflo $8\n\tsra %0,$8,8" : "=r"(rate) : "r"(rateInput), "r"(rateScale) : "$8");
#endif
					currentRate = CTR_MipsAddLo(currentRate, rate);
					crossedTarget = rotCurrW_original < currentRate;
					goto LAB_8005fee4;
				}
				if (rotCurrW_original < currentRate)
				{
					int rate;
					register struct Terrain *rateTerrain CTR_PSX_REGISTER("$8");
					int rateInput;
					register int rateScale CTR_PSX_REGISTER("$4");

					CTR_PSX_RELOAD(terrain);
					rateTerrain = terrain;
#if defined(CTR_NATIVE)
					(void)rateInput;
					(void)rateScale;
					rate = CTR_MipsSra(
					    CTR_MipsMulLo(CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE),
					                  rateTerrain->turnResponseScale),
					    8);
#else
					rateInput = CTR_MipsAddLo(driver->const_TurnInputDelay, (s8)driver->turnConst * VEH_PHYS_ANGULAR_TURN_RESPONSE_DECEL_SCALE);
					rateScale = rateTerrain->turnResponseScale;
					__asm__ volatile("mult %1,%2\n\tmflo $8\n\tsra %0,$8,8" : "=r"(rate) : "r"(rateInput), "r"(rateScale) : "$8");
#endif
					currentRate = CTR_MipsSubLo(currentRate, rate);
					crossedTarget = currentRate < rotCurrW_original;
				LAB_8005fee4:
					if (crossedTarget)
					{
						currentRate = rotCurrW_original;
					}
				}
				if (mirrored)
				{
					currentRate = CTR_MipsNegLo(currentRate);
				}
			}

#if defined(CTR_NATIVE)
			rotCurrW_original = (int)(s16)currentRate;
#else
			{
				register int narrowedRate CTR_PSX_REGISTER("$2") = CTR_MipsSll(currentRate, 16);

				__asm__("sra %0,%1,16" : "=r"(rotCurrW_original) : "r"(narrowedRate));
			}
#endif
			driver->rotationSpinRate = (s16)currentRate;
		}
	}

	{
		register int spinoutTime CTR_PSX_REGISTER("$16") = (int)driver->timeUntilDriftSpinout;

		if (spinoutTime != 0)
		{
			register int mapValue CTR_PSX_REGISTER("$4");
			register int mapMinimum CTR_PSX_REGISTER("$5");
			register int mapMaximum CTR_PSX_REGISTER("$6");
			register int rangeMinimum CTR_PSX_REGISTER("$7");
			register int previousFrameDrift CTR_PSX_REGISTER("$2");

			mapValue = spinoutTime;
			mapMinimum = 0;
			mapMaximum = VEH_PHYS_ANGULAR_DRIFT_SPINOUT_TIME;
			rangeMinimum = mapMinimum;
#if defined(CTR_NATIVE)
			spinoutTime = CTR_MipsSubLo(spinoutTime, elapsedTimeMS);
			previousFrameDrift = (int)driver->previousFrameMultDrift;
#else
			// NOTE(aalhendi): Keep the elapsed-time reload and previous-frame
			// load adjacent so the rotation store remains in the branch delay slot.
			__asm__ volatile("lw $8,%2\n\t"
			                 "lh %1,%4(%3)\n\t"
			                 "subu %0,%0,$8"
			                 : "+r"(spinoutTime), "=r"(previousFrameDrift)
			                 : "m"(elapsedTimeMS), "r"(driver), "I"(offsetof(struct Driver, previousFrameMultDrift)), "r"(mapValue), "r"(mapMinimum),
			                   "r"(mapMaximum), "r"(rangeMinimum)
			                 : "$8");
#endif
			rotCurrW_interp = VehCalc_MapToRange(mapValue, mapMinimum, mapMaximum, rangeMinimum, previousFrameDrift);
			rotCurrW_original = CTR_MipsAddLo(rotCurrW_original, rotCurrW_interp);
			if (spinoutTime < 0)
			{
				spinoutTime = 0;
			}
			driver->timeUntilDriftSpinout = (s16)spinoutTime;
		}
	}

	classSpeedPacked = CTR_MipsSll((u16)driver->const_Speed_ClassStat, VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT);
	classSpeed_original = CTR_MipsSra(classSpeedPacked, VEH_PHYS_ANGULAR_CLASS_SPEED_SHIFT);
	turnResistMaxProduct = CTR_MipsMulLo((u8)driver->const_turnResistMax, classSpeed_original);
	turnResistMinProduct = CTR_MipsMulLo((u8)driver->const_turnResistMin, classSpeed_original);
	turnAngleValue = driver->turnAngleLerpVel;
	CTR_PSX_LOAD_SIGNED_HALF_AFTER(modelRotVelocity, driver, offsetof(struct Driver, const_modelRotVelMax), driver->const_modelRotVelMax, turnAngleValue);
	turnResistMaxBitshift = CTR_MipsSra(turnResistMaxProduct, 8);
	turnResistMinBitshift = CTR_MipsSra(turnResistMinProduct, 8);

	// gas and brake together
	{
		register u32 brakeActions CTR_PSX_REGISTER("$8");
		register b32 brakeWithAccel CTR_PSX_REGISTER("$2");

#if defined(CTR_NATIVE)
		brakeActions = actionsFlagSet;
		brakeWithAccel = (b32)(brakeActions & ACTION_BRAKE_WITH_ACCEL);
#else
		__asm__("lw %0,%1" : "=r"(brakeActions) : "m"(actionsFlagSet), "r"(modelRotVelocity));
		__asm__("andi %0,%1,32" : "=r"(brakeWithAccel) : "r"(brakeActions), "r"(turnResistMaxBitshift));
#endif
		if (brakeWithAccel != 0)
		{
			turnResistMaxBitshift = CTR_MipsSra(turnResistMaxProduct, 9);
			{
				register int brakeSpeed CTR_PSX_REGISTER("$8") = speedApprox;
				register b32 belowSkidSpeed CTR_PSX_REGISTER("$2") = brakeSpeed < (VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD + 1);

				if (!belowSkidSpeed)
				{
					// driver is leaving skids
					driver->actionsFlagSet |= ACTION_BACK_SKID;
				}
			}
			turnResistMinBitshift = CTR_MipsSra(turnResistMinProduct, 9);
			if (driver->baseSpeed != 0)
			{
				register int modelSpeed CTR_PSX_REGISTER("$4");
				register int minSpeed CTR_PSX_REGISTER("$5") = VEH_PHYS_ANGULAR_STEER_SPEED_THRESHOLD;
				register int classHalfSpeed CTR_PSX_REGISTER("$6");
				register int minModelVelocity CTR_PSX_REGISTER("$7");

				CTR_PSX_LOAD_SIGNED_HALF(modelSpeed, driver, offsetof(struct Driver, speed), driver->speed);
				CTR_PSX_LOAD_SIGNED_HALF(minModelVelocity, driver, offsetof(struct Driver, const_modelRotVelMin), driver->const_modelRotVelMin);
				classHalfSpeed = CTR_MipsSra(classSpeedPacked, VEH_PHYS_ANGULAR_CLASS_SPEED_HALF_SHIFT);
#if !defined(CTR_NATIVE)
				// NOTE(aalhendi): Retail loads both signed arguments before shifting
				// the packed class speed; this barrier preserves that local order.
				__asm__ volatile("" : "+r"(modelSpeed), "+r"(classHalfSpeed), "+r"(minModelVelocity));
#endif
				if (modelSpeed < 0)
				{
					modelSpeed = CTR_MipsNegLo(modelSpeed);
				}
				// Rotate the model to exaggerate steering above the steering speed threshold.
				modelRotVelocity = VehCalc_MapToRange(modelSpeed, minSpeed, classHalfSpeed, minModelVelocity, modelRotVelocity);
			}
			else
			{
				modelRotVelocity = (int)driver->const_modelRotVelMin;
			}
		}
	}
	{
		register int turnResistance CTR_PSX_REGISTER("$20");
		register int lerpTarget CTR_PSX_REGISTER("$7");
		register int compareSpeed CTR_PSX_REGISTER("$8");
		register b32 belowTurnResistance CTR_PSX_REGISTER("$2");
#if defined(CTR_NATIVE)
		register int resistanceMinArg CTR_PSX_REGISTER("$5") = turnResistMinBitshift;
		register int resistanceMaxArg CTR_PSX_REGISTER("$6") = turnResistMaxBitshift;
#endif

		// this prevents you from steering sharp at low speeds
#if defined(CTR_NATIVE)
		driftAngleValue = CTR_MipsSll(CTR_MipsAddLo((u8)driver->const_TurnRate, CTR_MipsSll((s8)driver->turnConst, 1) / 5), 8);
		driverSpeed = (int)driver->speed;
		if (driverSpeed < 0)
		{
			driverSpeed = CTR_MipsNegLo(driverSpeed);
		}
		turnResistance = VehCalc_MapToRange(driverSpeed, resistanceMinArg, resistanceMaxArg, driftAngleValue, 0);
#else
		register int divideMagic CTR_PSX_REGISTER("$2");
		register int doubledTurn CTR_PSX_REGISTER("$3");
		int hiState;
		int loState;

		divideMagic = 0x66660000;
		doubledTurn = (s8)driver->turnConst;
		divideMagic |= 0x6667;
		doubledTurn = CTR_MipsSll(doubledTurn, 1);
		// NOTE(aalhendi): Retail interleaves the signed division by five with
		// the MapToRange argument setup. This bounded window preserves its HI
		// latency, stack argument, call delay slot, and following speed reload.
		__asm__ volatile(".word 0x00620018\n\t"
		                 ".word 0x02602821\n\t"
		                 ".word 0x9247043a\n\t"
		                 ".word 0x8644038c\n\t"
		                 ".word 0x02a03021\n\t"
		                 ".word 0xafa00010\n\t"
		                 ".word 0x04810002\n\t"
		                 ".word 0x00000000\n\t"
		                 ".word 0x00042023\n\t"
		                 ".word 0x00031fc3\n\t"
		                 ".word 0x00004010\n\t"
		                 ".word 0x00081043\n\t"
		                 ".word 0x00431023\n\t"
		                 ".word 0x00e23821\n\t"
		                 ".word 0x0007b200\n\t"
		                 ".word 0x0c0163e7\n\t"
		                 ".word 0x02c03821\n\t"
		                 ".word 0x8fa80024\n\t"
		                 ".word 0x0040a021\n\t"
		                 "# %0 %1 %2 %3 %4 %5 %6 %7"
		                 : "=r"(turnResistance), "=r"(driftAngleValue), "=r"(compareSpeed), "+r"(divideMagic), "+r"(doubledTurn)
		                 : "r"(driver), "r"(turnResistMinBitshift), "r"(turnResistMaxBitshift)
		                 : "$1", "$4", "$5", "$6", "$7", "$9", "$10", "$11", "$12", "$13", "$14", "$15", "$24", "$25", "$31", "memory");
		// NOTE(aalhendi): Model the embedded call's HI/LO outputs without
		// splitting the live argument window or emitting another instruction.
		__asm__ volatile("" : "=h"(hiState), "=l"(loState));
#endif
		CTR_PSX_CAPTURE_REGISTER(compareSpeed, speedApprox);
		CTR_PSX_KEEP_VALUE(compareSpeed);

		lerpTarget = 0;
		belowTurnResistance = compareSpeed < turnResistMinBitshift;
		if (!belowTurnResistance)
		{
			register int rotationRateAbs CTR_PSX_REGISTER("$16") = rotCurrW_original;

			if (rotCurrW_original < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(rotationRateAbs);
			}
			if (turnResistance < rotationRateAbs)
			{
				register int fireSpeed CTR_PSX_REGISTER("$4");
				register int minResistance CTR_PSX_REGISTER("$5") = turnResistMinBitshift;
				register int maxResistance CTR_PSX_REGISTER("$6");

				CTR_PSX_LOAD_SIGNED_HALF(fireSpeed, driver, offsetof(struct Driver, fireSpeed), driver->fireSpeed);
				maxResistance = turnResistMaxBitshift;
				CTR_PSX_OBSERVE_VALUE(maxResistance);
				if (fireSpeed < 0)
				{
					fireSpeed = CTR_MipsNegLo(fireSpeed);
				}
				lerpTarget = VehCalc_MapToRange(fireSpeed, minResistance, maxResistance, lerpTarget, modelRotVelocity);
				lerpTarget = VehCalc_MapToRange(rotationRateAbs, turnResistance, driftAngleValue, 0, lerpTarget);
				if (rotCurrW_original < 0)
				{
					CTR_PSX_NEGATE_IN_PLACE(lerpTarget);
				}
			}
		}

		// spins camera from side of driver, to back of driver,
		// when the drifting ends. "LerpToForwards"
		{
			register struct Driver *lerpDriver CTR_PSX_REGISTER("$4");
			register int lerpTurn CTR_PSX_REGISTER("$6");
			register int lerpResult CTR_PSX_REGISTER("$2");
#if !defined(CTR_NATIVE)
			register int narrowedTurnAngle CTR_PSX_REGISTER("$3");
#endif

			CTR_PSX_COPY_VALUE(lerpDriver, driver);
			CTR_PSX_LOAD_SIGNED_HALF(driftAngleValue, driver, offsetof(struct Driver, turnAngleCurr), driver->turnAngleCurr);
			CTR_PSX_COPY_VALUE(lerpTurn, turnAngleValue);
			lerpResult = VehPhysGeneral_LerpToForwards(lerpDriver, driftAngleValue, lerpTurn, lerpTarget);

#if defined(CTR_NATIVE)
			driver->turnAngleLerpVel = (s16)lerpResult;
			turnAngleValue = (int)(s16)lerpResult;
#else
			narrowedTurnAngle = CTR_MipsSll(lerpResult, 16);
			__asm__ volatile("sh %1,%0" : "=m"(driver->turnAngleLerpVel) : "r"(lerpResult));
			__asm__("sra %0,%1,16" : "=r"(turnAngleValue) : "r"(narrowedTurnAngle));
#endif
		}
	}

	{
		register struct Terrain *turnTerrain CTR_PSX_REGISTER("$8") = terrain;

		if (turnTerrain->turnAngleScale != VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL)
		{
#if defined(CTR_NATIVE)
			turnAngleValue = CTR_MipsSra(CTR_MipsMulLo(turnTerrain->turnAngleScale, turnAngleValue), 8);
#else
			int terrainProduct = CTR_MipsMulLo(turnTerrain->turnAngleScale, turnAngleValue);

			__asm__("mflo $8\n\tsra %0,$8,8" : "=r"(turnAngleValue) : "l"(terrainProduct) : "$8");
#endif
		}
	}
	{
		register int turnFrameTime CTR_PSX_REGISTER("$8");

		CTR_PSX_LOAD_WORD(turnFrameTime, elapsedTimeMS);
		CTR_PSX_KEEP_VALUE(turnFrameTime);
#if defined(CTR_NATIVE)
		driftAngleValue = CTR_MipsAddLo(driftAngleValue, CTR_MipsSra(CTR_MipsMulLo(turnAngleValue, turnFrameTime), VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT));
#else
		{
			int integrationProduct = CTR_MipsMulLo(turnAngleValue, turnFrameTime);
			int integrationStep;

			__asm__("mflo $8\n\tsra %0,$8,5" : "=r"(integrationStep) : "l"(integrationProduct) : "$8");
			driftAngleValue = CTR_MipsAddLo(driftAngleValue, integrationStep);
		}
#endif
		CTR_PSX_KEEP_VALUE(driftAngleValue);
	}
	CTR_PSX_STORE_HALF(driver->turnAngleCurr, driftAngleValue);
	{
		register int accelerationSpeed CTR_PSX_REGISTER("$8") = speedApprox;
		register b32 belowAccelerationSpeed CTR_PSX_REGISTER("$2");

		belowAccelerationSpeed = accelerationSpeed < (VEH_PHYS_ANGULAR_STEER_ACCEL_COMPARE_SPEED + 1);
		CTR_PSX_OBSERVE_VALUE(accelerationSpeed);
		if (!belowAccelerationSpeed)
		{
			register u32 accelerationActions CTR_PSX_REGISTER("$8") = actionsFlagSet;
			register u32 touchesGround CTR_PSX_REGISTER("$2") = accelerationActions & ACTION_TOUCH_GROUND;

			CTR_PSX_OBSERVE_VALUE(accelerationActions);
			if (touchesGround != 0)
			{
				int steerAcceleration;
				register int rotationRateScaled CTR_PSX_REGISTER("$4");
				register int steerVelocityLimit CTR_PSX_REGISTER("$5");
				register int steeringFrames CTR_PSX_REGISTER("$16") = driver->numFramesSpentSteering;
				int rotationRateAbs;
				int steerAccelerationResult;

				steerAccelerationResult = VehCalc_SteerAccel(steeringFrames, (int)driver->const_SteerAccel_Stage2_FirstFrame,
				                                             (int)driver->const_SteerAccel_Stage2_FrameLength, (int)driver->const_SteerAccel_Stage4_FirstFrame,
				                                             (int)driver->const_SteerAccel_Stage1_MinSteer, (int)driver->const_SteerAccel_Stage1_MaxSteer);
				rotationRateScaled = driver->const_SteerAccelTurnVelScale;
				CTR_PSX_KEEP_VALUE(rotationRateScaled);
				rotationRateAbs = rotCurrW_original;
				if (rotCurrW_original < 0)
				{
					rotationRateAbs = CTR_MipsNegLo(rotationRateAbs);
				}

#if defined(CTR_NATIVE)
				rotationRateScaled = CTR_MipsMulLo(rotationRateScaled, rotationRateAbs);
#else
				__asm__ volatile("mult %0,%1" : : "r"(rotationRateScaled), "r"(rotationRateAbs));
#endif
				steeringFrames = CTR_MipsAddLo(steeringFrames, 1);
				steerAcceleration = steerAccelerationResult;
#if defined(CTR_NATIVE)
				rotationRateScaled = CTR_MipsSra(rotationRateScaled, 8);
#else
				__asm__ volatile("mflo $8\n\tsra %0,$8,8" : "=r"(rotationRateScaled) : : "$8");
#endif
				driver->numFramesSpentSteering = (s16)steeringFrames;

				// the higher the value of steerAcceleration the more steering is "locked up"
				// try setting mov r3, xxxx at 80060170 for proof
				if (rotationRateScaled < steerAcceleration)
				{
					steerAcceleration = rotationRateScaled;
				}

				// steering left or right
				{
					register u32 steeringActions CTR_PSX_REGISTER("$8") = actionsFlagSet;
					register u32 steeringLeft CTR_PSX_REGISTER("$2") = steeringActions & ACTION_STEER_LEFT;

					CTR_PSX_OBSERVE_VALUE(steeringActions);
					if (steeringLeft != 0)
					{
						steerAcceleration = CTR_MipsNegLo(steerAcceleration);
					}
				}

				CTR_PSX_LOAD_SIGNED_HALF(steerVelocityLimit, driver, offsetof(struct Driver, const_SteerAccelTurnVelLimit),
				                         driver->const_SteerAccelTurnVelLimit);
				CTR_PSX_KEEP_VALUE(steerVelocityLimit);
				if (0 < rotCurrW_original)
				{
					int updatedRotation = CTR_MipsAddLo(rotCurrW_original, steerAcceleration);

					CTR_PSX_OBSERVE_VALUE(updatedRotation);
					rotationRateScaled = CTR_MipsNegLo(steerVelocityLimit);
					if (updatedRotation < rotationRateScaled)
					{
						rotCurrW_original = rotationRateScaled;
						goto LAB_VehPhysGeneral_SteerDone;
					}
				}
				if (rotCurrW_original < 0)
				{
					int updatedRotation = CTR_MipsAddLo(rotCurrW_original, steerAcceleration);

					if (steerVelocityLimit < updatedRotation)
					{
						rotCurrW_original = steerVelocityLimit;
						goto LAB_VehPhysGeneral_SteerDone;
					}
				}
				rotCurrW_original = CTR_MipsAddLo(rotCurrW_original, steerAcceleration);
			LAB_VehPhysGeneral_SteerDone:;
			}
		}
	}
	{
		register int wobbleVelocity CTR_PSX_REGISTER("$16");
		register struct Terrain *wobbleTerrain CTR_PSX_REGISTER("$8");
		int modelRotTwice;
		int turnAssistThreshold;

		turnResistMaxBitshift = (int)driver->turnWobbleAngle;
		wobbleTerrain = terrain;
		wobbleTimer = (int)driver->turnWobbleTimer;
		wobbleVelocity = (int)driver->turnWobbleVelocity;
		if ((wobbleTerrain->flags & TERRAIN_FLAG_SKIP_TURN_ASSIST) == 0)
		{
			register u32 wobbleActions CTR_PSX_REGISTER("$8") = actionsFlagSet;
			register u32 touchesGround CTR_PSX_REGISTER("$2") = wobbleActions & ACTION_TOUCH_GROUND;

			CTR_PSX_OBSERVE_VALUE(wobbleActions);
			if (touchesGround != 0)
			{
				modelRotTwice = CTR_MipsSll(modelRotVelocity, 1);
				CTR_PSX_OBSERVE_VALUE(modelRotTwice);
				turnResistMin = driftAngleValue;
				if (driftAngleValue < 0)
				{
					turnResistMin = CTR_MipsNegLo(driftAngleValue);
				}
				turnAssistThreshold = CTR_MipsSra(CTR_MipsAddLo(modelRotTwice, modelRotVelocity), 2);
				if (turnAssistThreshold < turnResistMin)
				{
					rotCurrW_interp = turnAngleValue;
					if (turnAngleValue < 0)
					{
						rotCurrW_interp = CTR_MipsNegLo(turnAngleValue);
					}
					if (rotCurrW_interp < VEH_PHYS_ANGULAR_TURN_ASSIST_MIN_DELTA)
					{
						rotCurrW_interp = turnResistMaxBitshift;
						if (turnResistMaxBitshift < 0)
						{
							rotCurrW_interp = CTR_MipsNegLo(turnResistMaxBitshift);
						}
						if (rotCurrW_interp < VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA)
						{
							wobbleTimer = VEH_PHYS_ANGULAR_TURN_WOBBLE_TIMER;
							wobbleVelocity = VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY;
							if (driftAngleValue < 0)
							{
								wobbleVelocity = -VEH_PHYS_ANGULAR_TURN_WOBBLE_VELOCITY;
							}
						}
					}
					goto LAB_80060284;
				}
			}
		}
		wobbleTimer = 0;
	LAB_80060284:
		rotCurrW_interp = turnResistMaxBitshift;
		if (turnResistMaxBitshift < 0)
		{
			rotCurrW_interp = CTR_MipsNegLo(turnResistMaxBitshift);
		}
		if (VEH_PHYS_ANGULAR_TURN_WOBBLE_DISABLE_ANGLE < rotCurrW_interp)
		{
			wobbleTimer = 0;
		}
		if (wobbleTimer != 0)
		{
			wobbleTimer = CTR_MipsSubLo(wobbleTimer, 1);
			turnResistMaxBitshift = CTR_MipsAddLo(turnResistMaxBitshift, wobbleVelocity);
		}
		else
		{
			register int wobbleAngleArg CTR_PSX_REGISTER("$4");
			register int wobbleStepAbs CTR_PSX_REGISTER("$5");

			wobbleVelocity = VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA;
			if (0 < turnResistMaxBitshift)
			{
				wobbleVelocity = CTR_MipsNegLo(VEH_PHYS_ANGULAR_TURN_WOBBLE_MIN_DELTA);
			}
			CTR_PSX_COPY_VALUE(wobbleAngleArg, turnResistMaxBitshift);
			wobbleStepAbs = wobbleVelocity;
			if (wobbleVelocity < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(wobbleStepAbs);
			}
			CTR_PSX_FORGET_VALUE(wobbleTimer);
			turnResistMaxBitshift = VehCalc_InterpBySpeed(wobbleAngleArg, wobbleStepAbs, 0);
		}
		{
			register int mapSpeed CTR_PSX_REGISTER("$4");
			register int mapMinimum CTR_PSX_REGISTER("$5");
			register int mapMaximum CTR_PSX_REGISTER("$6");
			register int mappedTurn CTR_PSX_REGISTER("$2");
			register int rotationDelta CTR_PSX_REGISTER("$3");
			int absRotationDelta;
			register int frameTime CTR_PSX_REGISTER("$8");
			register int integratedAngle CTR_PSX_REGISTER("$2");

#if defined(CTR_NATIVE)
			mapSpeed = speedApprox;
			mapMinimum = 0;
			mapMaximum = VEH_PHYS_ANGULAR_AIR_TURN_SPEED_MAX;
#else
			__asm__("move %0,$0" : "=r"(mapMinimum));
			__asm__("li %0,0x600" : "=r"(mapMaximum));
			__asm__("lw %0,%1" : "=r"(mapSpeed) : "m"(speedApprox));
#endif
			CTR_PSX_LOAD_SIGNED_HALF(turnResistMinBitshift, driver, offsetof(struct Driver, angle), driver->angle);
			CTR_PSX_OBSERVE_VALUE(turnResistMinBitshift);
			driver->turnWobbleTimer = (s16)wobbleTimer;
			driver->turnWobbleAngle = (s16)turnResistMaxBitshift;
			driver->turnWobbleVelocity = (s16)wobbleVelocity;
			mappedTurn = VehCalc_MapToRange(mapSpeed, mapMinimum, mapMaximum, turnAngleValue, 0);

			CTR_PSX_LOAD_WORD(frameTime, elapsedTimeMS);
			frameTime = CTR_MipsMulLo(mappedTurn, frameTime);
			CTR_PSX_SHIFT_RIGHT_ARITHMETIC(rotationDelta, frameTime, VEH_PHYS_ANGULAR_TURN_INTEGRATION_SHIFT);
			absRotationDelta = rotationDelta;
			if (rotationDelta < 0)
			{
				absRotationDelta = CTR_MipsNegLo(rotationDelta);
			}
			if (1 < absRotationDelta)
			{
				register int updatedAngle CTR_PSX_REGISTER("$2") = CTR_MipsSubLo(turnResistMinBitshift, rotationDelta);

				CTR_PSX_OBSERVE_VALUE(updatedAngle);
				turnResistMinBitshift = updatedAngle & VEH_PHYS_ANGULAR_ANGLE_MASK;
			}
			CTR_PSX_LOAD_WORD(frameTime, elapsedTimeMS);
			CTR_PSX_KEEP_VALUE(frameTime);
#if defined(CTR_NATIVE)
			frameTime = CTR_MipsMulLo(rotCurrW_original, frameTime);
			driver->ampTurnState = (s16)rotCurrW_original;
			integratedAngle = CTR_MipsSra(frameTime, VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT);
#else
			__asm__ volatile("mult %0,%1" : : "r"(rotCurrW_original), "r"(frameTime));
			driver->ampTurnState = (s16)rotCurrW_original;
			__asm__ volatile("mflo $8\n\tsra %0,$8,13" : "=r"(integratedAngle) : : "$8");
#endif
			integratedAngle = CTR_MipsAddLo(turnResistMinBitshift, integratedAngle);
			CTR_PSX_OBSERVE_VALUE(integratedAngle);
			turnResistMinBitshift = integratedAngle & VEH_PHYS_ANGULAR_ANGLE_MASK;
		}
	}
	driver->angle = (u16)turnResistMinBitshift;

	{
		register int visualRotation CTR_PSX_REGISTER("$2") = CTR_MipsAddLo(CTR_MipsAddLo(turnResistMinBitshift, driftAngleValue), turnResistMaxBitshift);

		(driver->rotCurr).y = (s16)visualRotation;
	}

	{
		register u32 leanActions CTR_PSX_REGISTER("$8");
		register u32 preventsAcceleration CTR_PSX_REGISTER("$2");

		CTR_PSX_LOAD_WORD_DELAYED(leanActions, actionsFlagSet);
		preventsAcceleration = leanActions & ACTION_ACCEL_PREVENTION;
		CTR_PSX_OBSERVE_VALUE(leanActions);
		if (preventsAcceleration != 0)
		{
			goto LAB_VehPhysGeneral_BrakeLean;
		}
		{
			if (driver->accelTapCount < DRIVER_ACCEL_TAP_STEER_COUNT)
			{
				goto LAB_VehPhysGeneral_TerrainLean;
			}
		LAB_VehPhysGeneral_BrakeLean:
			rotCurrW_original = CTR_MipsSra(rotCurrW_original * VEH_PHYS_ANGULAR_BRAKE_LEAN_SCALE, 8);
			goto LAB_VehPhysGeneral_LeanDone;

		LAB_VehPhysGeneral_TerrainLean:
		{
			register int neutralScale CTR_PSX_REGISTER("$2") = VEH_PHYS_ANGULAR_TERRAIN_SCALE_NEUTRAL;
			register struct Terrain *leanTerrain CTR_PSX_REGISTER("$8");

			CTR_PSX_LOAD_WORD(leanTerrain, terrain);
			CTR_PSX_KEEP_VALUE(leanTerrain);
			if (leanTerrain->turnLeanScale != neutralScale)
			{
				register int leanProduct CTR_PSX_REGISTER("$8") = CTR_MipsMulLo(rotCurrW_original, leanTerrain->turnLeanScale);

				CTR_PSX_KEEP_VALUE(leanProduct);
				CTR_PSX_SHIFT_RIGHT_ARITHMETIC(rotCurrW_original, leanProduct, 8);
			}
		}
		LAB_VehPhysGeneral_LeanDone:;
		}
	}

	{
		register int axisFrameTime CTR_PSX_REGISTER("$8");
		register int axisDelta CTR_PSX_REGISTER("$3");

		CTR_PSX_LOAD_WORD(axisFrameTime, elapsedTimeMS);
		CTR_PSX_KEEP_VALUE(axisFrameTime);
		axisFrameTime = CTR_MipsMulLo(rotCurrW_original, axisFrameTime);
		CTR_PSX_SHIFT_RIGHT_ARITHMETIC(axisDelta, axisFrameTime, VEH_PHYS_ANGULAR_AXIS_INTEGRATION_SHIFT);
		driver->axisRotationX = (s16)(CTR_MipsAddLo((u16)driver->axisRotationX, axisDelta) & VEH_PHYS_ANGULAR_ANGLE_MASK);
	}

	VehPhysForce_RotAxisAngle(&driver->matrixMovingDir, CTR_VECTOR_DATA(&driver->AxisAngle1_normalVec), turnResistMinBitshift);
	CTR_GteSetRotMatrix(&driver->matrixMovingDir);
	VehPhysForce_CounterSteer(driver);
}

int VehPhysGeneral_LerpQuarterStrength(int current, int desired)
{
	register int result CTR_PSX_REGISTER("$2") = current;

	if (desired != 0)
	{
		desired = CTR_MipsSra(desired, 2);

		if (desired == 0)
		{
			desired = 1;
		}
	}

	if (desired <= result)
	{
		result = desired;
	}

	return result;
}

int VehPhysGeneral_LerpToForwards(struct Driver *d, int currentAngle, int currentVelocity, int targetAngle)
{
	register struct Driver *driver CTR_PSX_REGISTER("$17") = d;
	register int velocity CTR_PSX_REGISTER("$18") = currentVelocity;
	register int target CTR_PSX_REGISTER("$16") = targetAngle;
	b32 mirrored = false;
	int desiredVelocity;

	CTR_PSX_KEEP_VALUE(driver);
	CTR_PSX_KEEP_VALUE(velocity);
	CTR_PSX_KEEP_VALUE(target);

	driver->turnAngleLerpTarget = 0;
	if ((target < 0) || ((target == 0 && (currentAngle < 0))))
	{
		mirrored = true;
		currentAngle = CTR_MipsNegLo(currentAngle);
		velocity = CTR_MipsNegLo(velocity);
		target = CTR_MipsNegLo(target);
	}

	desiredVelocity = 0;
	if (driver->wallRubTimer != DRIVER_WALL_RUB_TIMER_START)
	{
		if (target < currentAngle)
		{
			u32 lerpStrength;
			u32 baseStrength;
			int angleDelta;

			if (driver->const_modelRotVelMax < currentAngle)
			{
				baseStrength = (u8)driver->const_ModelTurnReturnStrength;
				angleDelta = CTR_MipsSubLo(currentAngle, target);
				CTR_PSX_BIND_VALUE_CLOBBER(baseStrength, "$3");
				lerpStrength = CTR_MipsSll(baseStrength, 4);
				CTR_PSX_BIND_VALUE_CLOBBER(lerpStrength, "$3");
				lerpStrength = CTR_MipsSubLo(lerpStrength, baseStrength);
			}
			else
			{
				lerpStrength = (u8)driver->const_ModelTurnReturnStrength;
				angleDelta = CTR_MipsSubLo(currentAngle, target);
			}
			desiredVelocity = VehPhysGeneral_LerpQuarterStrength(lerpStrength, angleDelta);
			desiredVelocity = CTR_MipsNegLo(desiredVelocity);
		}
		else
		{
			if (currentAngle < target)
			{
				if (currentAngle < 0)
				{
					desiredVelocity =
					    VehPhysGeneral_LerpQuarterStrength((u8)driver->const_ModelTurnNegativeReturnStrength, CTR_MipsSubLo(target, currentAngle));
				}
				else
				{
					desiredVelocity = VehPhysGeneral_LerpQuarterStrength((u8)driver->const_ModelTurnCounterSteerStrength, CTR_MipsSubLo(target, currentAngle));
					driver->turnAngleLerpTarget = (s16)target;
				}
			}
		}
	}

	// Interpolate rotation by speed
	velocity = VehCalc_InterpBySpeed(velocity, driver->const_ModelTurnVelocityLerp, desiredVelocity);
	if (mirrored)
	{
		CTR_PSX_FORGET_VALUE(velocity);
		CTR_PSX_CLOBBER("$2");
		return CTR_MipsNegLo(velocity);
	}
	return velocity;
}

int VehPhysGeneral_JumpGetVelY(s16 *normalVec, Vec3 *speedXYZ)
{
	int normalY;
	register int absNormalY CTR_PSX_REGISTER("$2");
	int result;
	int dotProduct;

	normalY = normalVec[1];
	absNormalY = normalY;

	if (normalY < 0)
	{
		CTR_PSX_KEEP_VALUE(absNormalY);
		absNormalY = CTR_MipsNegLo(absNormalY);
	}

	result = 0;
	if (absNormalY < VEH_PHYS_JUMP_NORMAL_Y_MIN)
	{
		register int returnValue CTR_PSX_REGISTER("$2");

		CTR_PSX_BIND_VALUE_CLOBBER(result, "$2");
		returnValue = result;
		CTR_PSX_MEMORY_BARRIER();
		return returnValue;
	}

	dotProduct = CTR_MipsAddLo(CTR_MipsMulLo(speedXYZ->x, normalVec[0]), CTR_MipsMulLo(speedXYZ->z, normalVec[2]));
	result = CTR_MipsDiv(dotProduct, normalY);
	CTR_PSX_BIND_VALUE_CLOBBER(result, "$2");
	return result;
}

static inline int VehPhysGeneral_Jump_Abs(int value)
{
	return value < 0 ? CTR_MipsNegLo(value) : value;
}

static inline int VehPhysGeneral_Jump_Div2TowardZero(int value)
{
	return CTR_MipsSra(CTR_MipsAddLo(value, (u32)value >> 31), 1);
}

static inline int VehPhysGeneral_Jump_Div4TowardZero(int value)
{
	if (value < 0)
	{
		value = CTR_MipsAddLo(value, 3);
	}

	return CTR_MipsSra(value, 2);
}

void VehPhysGeneral_JumpAndFriction(struct Thread *t, struct Driver *driverArg)
{
	register struct Driver *d CTR_PSX_REGISTER("$17") = driverArg;
	Vec3 movement;
	int speedLoss;
	register int acceleration CTR_PSX_REGISTER("$16");
	int bestJumpVelY;
	register int jumpVelY CTR_PSX_REGISTER("$4");
	s16 *normalVec;
	int jumpVelYSquared;
	int verticalSpeed;
	register int maxVerticalSpeed CTR_PSX_REGISTER("$4");
	int speed;
	int speedApprox;

	(void)t;

	// NOTE(aalhendi): Retail loads matrixMovingDir once, then reuses the same CP2
	// rotation registers for every jump/friction impulse in this function.
	CTR_GteSetRotMatrix(&d->matrixMovingDir);

	if ((d->kartState != KS_DRIFTING) && ((d->actionsFlagSet & ACTION_MASK_WEAPON) == 0) && (d->reserves == 0))
	{
		register int ampTurn CTR_PSX_REGISTER("$4");
		register int rangeStart CTR_PSX_REGISTER("$5") = 0;
		register int mappedStart CTR_PSX_REGISTER("$7") = rangeStart;
		register int rangeEnd CTR_PSX_REGISTER("$6");
		register int mappedEnd CTR_PSX_REGISTER("$2");
		int mappedDecrease;
		register int baseSpeed CTR_PSX_REGISTER("$6");
		register int baseSpeedBits CTR_PSX_REGISTER("$5");
		register int absBaseSpeed CTR_PSX_REGISTER("$3");
		register int turnDecrease CTR_PSX_REGISTER("$4");
		int nextBaseSpeed;

		CTR_PSX_KEEP_VALUE(mappedStart);
		CTR_PSX_LOAD_UNSIGNED_HALF(ampTurn, d, offsetof(struct Driver, ampTurnState), d->ampTurnState);
		CTR_PSX_LOAD_UNSIGNED_BYTE(rangeEnd, d, offsetof(struct Driver, const_BackwardTurnRate), d->const_BackwardTurnRate);
		CTR_PSX_LOAD_SIGNED_HALF(mappedEnd, d, offsetof(struct Driver, const_TurnDecreaseRate), d->const_TurnDecreaseRate);
		CTR_PSX_SHIFT_LEFT_IN_PLACE(ampTurn, 16);
		CTR_PSX_SHIFT_RIGHT_ARITHMETIC_IN_PLACE(ampTurn, 24);
		if (ampTurn < 0)
		{
			ampTurn = CTR_MipsNegLo(ampTurn);
		}
		mappedDecrease = VehCalc_MapToRange(ampTurn, rangeStart, rangeEnd, mappedStart, mappedEnd);
		turnDecrease = mappedDecrease;
		CTR_PSX_LOAD_SIGNED_HALF(baseSpeed, d, offsetof(struct Driver, baseSpeed), d->baseSpeed);
		CTR_PSX_LOAD_UNSIGNED_HALF(baseSpeedBits, d, offsetof(struct Driver, baseSpeed), d->baseSpeed);
		absBaseSpeed = baseSpeed;

		if (baseSpeed < 0)
		{
			CTR_PSX_NEGATE_IN_PLACE(absBaseSpeed);
		}
		if (absBaseSpeed < turnDecrease)
		{
			turnDecrease = absBaseSpeed;
		}

		nextBaseSpeed = CTR_MipsSubLo(baseSpeedBits, turnDecrease);
		if (baseSpeed < 0)
		{
			turnDecrease = CTR_MipsNegLo(turnDecrease);
			CTR_PSX_SUBTRACT(nextBaseSpeed, baseSpeedBits, turnDecrease);
		}
		d->baseSpeed = (s16)nextBaseSpeed;
	}

	if (d->wallRubTimer != 0)
	{
		int wallBaseSpeed = d->baseSpeed;
		int wallRubSpeedLimit = d->wallRubSpeedLimit;

		if (wallRubSpeedLimit < wallBaseSpeed)
		{
			d->baseSpeed = d->wallRubSpeedLimit;
		}

		if (d->baseSpeed < CTR_MipsNegLo(d->wallRubSpeedLimit))
		{
			u16 wallRubSpeedLimit = d->wallRubSpeedLimit;

			d->baseSpeed = (s16)CTR_MipsNegLo(wallRubSpeedLimit);
		}
	}

	movement.x = d->velocity.x;
	movement.y = d->velocity.y;
	movement.z = d->velocity.z;
	speedLoss = 0;
	CTR_PSX_FORGET_VALUE(speedLoss);

	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0)
	{
		goto CHECK_FOR_ANY_JUMP;
	}

	acceleration = speedLoss;
	CTR_PSX_KEEP_VALUE(acceleration);

	if (((d->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK) != 0) && (d->baseSpeed > 0))
	{
		goto SET_HIGH_ACCEL;
	}
	if (d->baseSpeed != 0)
	{
		if (((d->terrainMeta1->flags & TERRAIN_FLAG_ACCEL_WHILE_REVERSE_SLIDING) == 0) || (d->baseSpeed < 1) || (d->speedApprox >= 0))
		{
			int speedApprox = d->speedApprox;
			int absSpeedApprox = speedApprox;

			if (speedApprox < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(absSpeedApprox);
			}

			if ((absSpeedApprox > VEH_PHYS_JUMP_REVERSE_SLIDE_SPEED_COMPARE) && ((d->baseSpeed < 1) || (speedApprox < 1)) &&
			    ((d->baseSpeed >= 0) || (speedApprox >= 0)))
			{
				goto PROCESS_ACCEL;
			}
		}

		{
			register int classAcceleration CTR_PSX_REGISTER("$4");
			register int constantAcceleration CTR_PSX_REGISTER("$3");
			register int constantInput CTR_PSX_REGISTER("$2");

			constantInput = (s8)d->accelConst;
			constantInput = CTR_MipsSll(constantInput, 5);
			constantAcceleration = constantInput / 5;
			CTR_PSX_LOAD_SIGNED_HALF(classAcceleration, d, offsetof(struct Driver, const_Accel_ClassStat), d->const_Accel_ClassStat);
			CTR_PSX_KEEP_VALUE(constantAcceleration);
			acceleration = CTR_MipsAddLo(classAcceleration, constantAcceleration);
		}

		if ((d->stepFlagSet & COLL_STEP_TRIGGER_TURBO_PAD_MASK) == 0)
		{
			goto SCALE_TERRAIN_ACCEL;
		}
		if (d->baseSpeed > 0)
		{
			goto SET_HIGH_ACCEL;
		}
		goto PROCESS_ACCEL;
	}
	goto PROCESS_ACCEL;

SET_HIGH_ACCEL:
	acceleration = VEH_PHYS_JUMP_TURBO_PAD_ACCEL;
	goto PROCESS_ACCEL;

SCALE_TERRAIN_ACCEL:
	if ((d->reserves != 0) && (d->baseSpeed > 0))
	{
		acceleration = d->const_Accel_Reserves;
	}
	{
		int slowUntilSpeed = d->terrainMeta1->slowUntilSpeed;

		if ((slowUntilSpeed != VEH_PHYS_JUMP_TERRAIN_SCALE_NEUTRAL) && ((d->actionsFlagSet & ACTION_MASK_WEAPON) == 0))
		{
			acceleration = CTR_MipsSra(CTR_MipsMulLo(slowUntilSpeed, acceleration), VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);
		}
	}

PROCESS_ACCEL:
{
	register int rotatedX CTR_PSX_REGISTER("$4");
	register int rotatedY CTR_PSX_REGISTER("$5");
	register int rotatedZ CTR_PSX_REGISTER("$6");
	u32 movementLengthSq;
	b32 clampToForwardImpulse;
	int accelerationProduct = CTR_MipsMulLo(acceleration, GAME_TRACKER->elapsedTimeMS);
	int speedMagnitude;
	register int lossBaseSpeed CTR_PSX_REGISTER("$3");

	CTR_PSX_SHIFT_RIGHT_ARITHMETIC(acceleration, accelerationProduct, 5);
	MTC2(0, 0);
	MTC2((u32)(u16)acceleration, 1);
	CTR_PSX_GTE_PIPELINE_DELAY();
	gte_mvmva(1, 0, 0, 3, 0);
#if defined(CTR_NATIVE)
	rotatedX = MFC2_S(25);
	rotatedY = MFC2_S(26);
	rotatedZ = MFC2_S(27);
#else
	__asm__ volatile("mfc2 %0,$25" : "=r"(rotatedX));
	__asm__ volatile("mfc2 %0,$26" : "=r"(rotatedY));
	__asm__ volatile("mfc2 %0,$27" : "=r"(rotatedZ));
#endif
	{
		register int directionOrImpulse CTR_PSX_REGISTER("$2") = d->baseSpeed;

		if (directionOrImpulse >= 0)
		{
			goto APPLY_FORWARD_IMPULSE;
		}
		{
			directionOrImpulse = CTR_MipsNegLo(acceleration);
			d->forwardAccelImpulse = (s16)directionOrImpulse;

			{
				int movementX = movement.x;
				register int movementZ CTR_PSX_REGISTER("$3") = movement.z;

				movement.x = CTR_MipsSubLo(movementX, rotatedX);
				CTR_PSX_MEMORY_BARRIER();
				movement.z = CTR_MipsSubLo(movementZ, rotatedZ);
				movement.y = CTR_MipsSubLo(movement.y, rotatedY);
				CTR_PSX_MEMORY_BARRIER();
			}

			CTR_PSX_NEGATE(directionOrImpulse, rotatedX);
			d->forwardAccelVector.x = (s16)directionOrImpulse;
			CTR_PSX_NEGATE(directionOrImpulse, rotatedY);
			d->forwardAccelVector.y = (s16)directionOrImpulse;
			CTR_PSX_NEGATE(directionOrImpulse, rotatedZ);
			d->forwardAccelVector.z = (s16)directionOrImpulse;
		}
		goto FINISH_FORWARD_IMPULSE;

	APPLY_FORWARD_IMPULSE:
	{
		d->forwardAccelImpulse = (s16)acceleration;

		movement.x = CTR_MipsAddLo(movement.x, rotatedX);
		movement.y = CTR_MipsAddLo(movement.y, rotatedY);
		movement.z = CTR_MipsAddLo(movement.z, rotatedZ);

		d->forwardAccelVector.x = (s16)rotatedX;
		d->forwardAccelVector.y = (s16)rotatedY;
		d->forwardAccelVector.z = (s16)rotatedZ;
		CTR_PSX_MEMORY_BARRIER();
	}
	FINISH_FORWARD_IMPULSE:;
	}

	movementLengthSq =
	    (u32)CTR_MipsAddLo(CTR_MipsAddLo(CTR_MipsMulLo(movement.x, movement.x), CTR_MipsMulLo(movement.y, movement.y)), CTR_MipsMulLo(movement.z, movement.z));
	speedMagnitude = (int)VehCalc_FastSqrt(movementLengthSq, VEH_PHYS_JUMP_FAST_SQRT_ITERATIONS);
	CTR_PSX_LOAD_SIGNED_HALF(lossBaseSpeed, d, offsetof(struct Driver, baseSpeed), d->baseSpeed);
	speedLoss = (s32)((u32)speedMagnitude >> VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);
	CTR_PSX_KEEP_VALUE(speedLoss);
	if (lossBaseSpeed < 0)
	{
		lossBaseSpeed = CTR_MipsNegLo(lossBaseSpeed);
	}
	speedLoss = CTR_MipsSubLo(speedLoss, lossBaseSpeed);

	clampToForwardImpulse = acceleration < speedLoss;
	if (speedLoss < 0)
	{
		speedLoss = 0;
		clampToForwardImpulse = acceleration < speedLoss;
	}
	if (clampToForwardImpulse)
	{
		speedLoss = acceleration;
	}

	if (((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0) || (d->jump_ForcedMS == 0))
	{
		goto CHECK_FOR_ANY_JUMP;
	}

	if (d->jump_HighJumpTimerMS != 0)
	{
		d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
	}

	if (d->kartState == KS_BLASTED)
	{
		GAMEPAD_ShockFreq(d, VEH_PHYS_JUMP_RUMBLE_CHANNEL, 0);
		GAMEPAD_ShockForce1(d, VEH_PHYS_JUMP_RUMBLE_CHANNEL, VEH_PHYS_JUMP_RUMBLE_FORCE);
	}
}

	goto PROCESS_JUMP;

CHECK_FOR_ANY_JUMP:
	if (((d->actionsFlagSet & ACTION_WEAPON_FIRE_REQUEST) != 0) && (d->heldItemID == HELD_ITEM_SPRING))
	{
		d->actionsFlagSet &= ~ACTION_WEAPON_FIRE_REQUEST;

		if ((d->jump_CoyoteTimerMS != 0) && (d->jump_CooldownMS == 0))
		{
			int jumpForce;

			d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;

			jumpForce = CTR_MipsAddLo(CTR_MipsSll(d->const_JumpForce, 3), d->const_JumpForce);
			d->jump_InitialVelY = (s16)VehPhysGeneral_Jump_Div4TowardZero(jumpForce);

			OtherFX_Play_Echo(VEH_PHYS_JUMP_SPRING_SFX, 1, (d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);

			d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
			goto PROCESS_JUMP;
		}

		d->noItemTimer = 0;
	}

	if (d->forcedJumpType == FORCED_JUMP_NONE)
	{
		goto NORMAL_JUMP;
	}
	{
		if ((d->jump_ForcedMS == 0) || (d->jump_InitialVelY == d->const_JumpForce))
		{
			OtherFX_Play(VEH_PHYS_JUMP_FORCED_SFX, 1);
		}

		d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;

		if (d->forcedJumpType == FORCED_JUMP_HIGH)
		{
			int jumpForce;

			d->jump_HighJumpTimerMS = VEH_PHYS_JUMP_HIGH_TIMER_MS;
			jumpForce = CTR_MipsAddLo(CTR_MipsSll(d->const_JumpForce, 1), d->const_JumpForce);
			d->jump_InitialVelY = (s16)jumpForce;
		}
		else
		{
			int jumpForce = CTR_MipsAddLo(CTR_MipsSll(d->const_JumpForce, 1), d->const_JumpForce);

			d->jump_InitialVelY = (s16)VehPhysGeneral_Jump_Div2TowardZero(jumpForce);
		}

		d->forcedJumpType = FORCED_JUMP_NONE;
	}
	goto PROCESS_JUMP;

NORMAL_JUMP:
	if ((d->jump_CoyoteTimerMS == 0) || (d->jump_TenBuffer == 0) || (d->jump_CooldownMS != 0))
	{
		goto CHECK_ANTI_GRAVITY;
	}

	d->jump_ForcedMS = VEH_PHYS_JUMP_FORCED_MS;
	d->numberOfJumps = (s16)CTR_MipsAddLo((u16)d->numberOfJumps, 1);
	d->jump_InitialVelY = d->const_JumpForce;

	OtherFX_Play_Echo(VEH_PHYS_JUMP_NORMAL_SFX, 1, (d->actionsFlagSet & ACTION_ENGINE_ECHO) != 0);

PROCESS_JUMP:
	d->jump_CooldownMS = VEH_PHYS_JUMP_COOLDOWN_MS;
	d->jump_TenBuffer = 0;
	d->actionsFlagSet |= ACTION_JUMP_STARTED | ACTION_TURBO_INPUT_LATCH;

	{
		register int normalOffset CTR_PSX_REGISTER("$18");
		register int normalIndex CTR_PSX_REGISTER("$19");

		bestJumpVelY = 0;
		normalIndex = bestJumpVelY;
		normalOffset = offsetof(struct Driver, AxisAngle4_normalVec);
		do
		{
			register int absoluteJumpVelY CTR_PSX_REGISTER("$3");
			register int absoluteBestJumpVelY CTR_PSX_REGISTER("$2");

			jumpVelY = VehPhysGeneral_JumpGetVelY((s16 *)((u8 *)d + normalOffset), &movement);
			absoluteJumpVelY = jumpVelY;
			if (jumpVelY < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(absoluteJumpVelY);
			}
			absoluteBestJumpVelY = bestJumpVelY;
			if (bestJumpVelY < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(absoluteBestJumpVelY);
			}
			if (absoluteBestJumpVelY < absoluteJumpVelY)
			{
				bestJumpVelY = jumpVelY;
			}
			normalOffset = CTR_MipsAddLo(normalOffset, VEH_PHYS_JUMP_NORMAL_VECTOR_STRIDE);
			normalIndex = CTR_MipsAddLo(normalIndex, 1);
		} while (normalIndex < 1);
	}

	normalVec = CTR_VECTOR_DATA(&(d->AxisAngle1_normalVec));
	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) == 0)
	{
		normalVec = CTR_VECTOR_DATA(&(d->AxisAngle2_normalVec));
	}

	jumpVelY = VehPhysGeneral_JumpGetVelY(normalVec, &movement);

	{
		register int absoluteJumpVelY CTR_PSX_REGISTER("$3");
		register int absoluteBestJumpVelY CTR_PSX_REGISTER("$2");

		absoluteJumpVelY = jumpVelY;
		if (jumpVelY < 0)
		{
			CTR_PSX_NEGATE_IN_PLACE(absoluteJumpVelY);
		}
		absoluteBestJumpVelY = bestJumpVelY;
		if (bestJumpVelY < 0)
		{
			CTR_PSX_NEGATE_IN_PLACE(absoluteBestJumpVelY);
		}
		if (absoluteBestJumpVelY < absoluteJumpVelY)
		{
			bestJumpVelY = jumpVelY;
		}
	}
	jumpVelYSquared = CTR_MipsMulLo(bestJumpVelY, bestJumpVelY);

	verticalSpeed = VehCalc_FastSqrt(
	    (u32)CTR_MipsSra(CTR_MipsAddLo(jumpVelYSquared, CTR_MipsMulLo(d->jump_InitialVelY, d->jump_InitialVelY)), VEH_PHYS_JUMP_SPEED_FIXED_SHIFT),
	    VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);

	{
		register int verticalSpeedCap CTR_PSX_REGISTER("$3") = GAME_TRACKER->level1->jumpVerticalSpeedCap;

		CTR_PSX_OBSERVE_VALUE(verticalSpeedCap);
		maxVerticalSpeed = CTR_MipsSll(verticalSpeedCap, VEH_PHYS_JUMP_SPEED_FIXED_SHIFT);
	}
	bestJumpVelY = CTR_MipsSubLo(verticalSpeed, bestJumpVelY);
	if (maxVerticalSpeed == 0)
	{
		maxVerticalSpeed = VEH_PHYS_JUMP_VERTICAL_SPEED_DEFAULT;
	}
	else if (maxVerticalSpeed > VEH_PHYS_JUMP_VERTICAL_SPEED_MAX)
	{
		maxVerticalSpeed = VEH_PHYS_JUMP_VERTICAL_SPEED_MAX;
	}

	if (maxVerticalSpeed < bestJumpVelY)
	{
		bestJumpVelY = maxVerticalSpeed;
	}

	if (movement.y < bestJumpVelY)
	{
		movement.y = bestJumpVelY;
	}
	goto NOT_JUMPING;

CHECK_ANTI_GRAVITY:
	if ((d->actionsFlagSet & ACTION_TOUCH_GROUND) != 0)
	{
		if (d->underDriver != NULL)
		{
			register int antiGravityNormal CTR_PSX_REGISTER("$4");
			int antiGravitySpeed;
			int antiGravityVelocity;
			register int rotatedX CTR_PSX_REGISTER("$4");
			register int rotatedY CTR_PSX_REGISTER("$5");
			register int rotatedZ CTR_PSX_REGISTER("$6");

			CTR_PSX_LOAD_SIGNED_BYTE(antiGravityNormal, d->underDriver, offsetof(struct QuadBlock, mulNormVecY), d->underDriver->mulNormVecY);
			if (antiGravityNormal != 0)
			{
				antiGravitySpeed = d->speedApprox;
				if (antiGravitySpeed < 0)
				{
					antiGravitySpeed = VehPhysGeneral_Jump_Abs(antiGravitySpeed);
				}
				CTR_PSX_KEEP_VALUE(antiGravitySpeed);

				antiGravityVelocity = CTR_MipsSra(CTR_MipsMulLo(antiGravityNormal, antiGravitySpeed), 8);
				MTC2(CTR_MipsSll(antiGravityVelocity, 16), 0);
				MTC2(0, 1);
				CTR_PSX_GTE_PIPELINE_DELAY();
				gte_mvmva(1, 0, 0, 3, 0);
#if defined(CTR_NATIVE)
				rotatedX = MFC2_S(25);
				rotatedY = MFC2_S(26);
				rotatedZ = MFC2_S(27);
#else
				__asm__ volatile("mfc2 %0,$25" : "=r"(rotatedX));
				__asm__ volatile("mfc2 %0,$26" : "=r"(rotatedY));
				__asm__ volatile("mfc2 %0,$27" : "=r"(rotatedZ));
#endif

				movement.x = CTR_MipsAddLo(movement.x, rotatedX);
				movement.y = CTR_MipsAddLo(movement.y, rotatedY);
				movement.z = CTR_MipsAddLo(movement.z, rotatedZ);
			}
		}
	}

NOT_JUMPING:
	VEH_CONVERT_VEC_TO_SPEED(d, &movement);

	speed = CTR_MipsSubLo((u16)d->speed, speedLoss);
	d->speed = (s16)speed;
	if (d->speed < 0)
	{
		d->speed = 0;
	}

	speedApprox = d->speedApprox;
	{
		s16 nextNeedleValue;

		if (speedApprox < 0)
		{
			register int absoluteSpeed CTR_PSX_REGISTER("$2");

			CTR_PSX_FORGET_VALUE(speedApprox);
			absoluteSpeed = speedApprox;
			if (speedApprox < 0)
			{
				CTR_PSX_NEGATE_IN_PLACE(absoluteSpeed);
			}

			if (absoluteSpeed < VEH_PHYS_JUMP_SPEEDOMETER_REVERSE_THRESHOLD)
			{
				nextNeedleValue =
				    (s16)CTR_MipsSubLo((u16)d->speedometerNeedleValue, CTR_MipsSra(d->speedometerNeedleValue, VEH_PHYS_JUMP_SPEEDOMETER_DECAY_SHIFT));
			}
			else
			{
				register int needleValue CTR_PSX_REGISTER("$4");
				register int blendedValue CTR_PSX_REGISTER("$3");
				register struct GameTracker *gGT CTR_PSX_REGISTER("$2");
				register int timerValue CTR_PSX_REGISTER("$2");
				register int timerLowPart CTR_PSX_REGISTER("$4");

				CTR_PSX_LOAD_SIGNED_HALF(needleValue, d, offsetof(struct Driver, speedometerNeedleValue), d->speedometerNeedleValue);
				VEH_LOAD_GAME_TRACKER(gGT);
#if defined(CTR_NATIVE)
				blendedValue = needleValue * VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD;
				timerValue = gGT->timer & VEH_PHYS_JUMP_SPEEDOMETER_TIMER_MASK;
				timerLowPart = CTR_MipsSll(timerValue, 8);
				timerValue = CTR_MipsSll(timerValue, 9);
				timerValue = CTR_MipsAddLo(timerValue, timerLowPart);
				blendedValue = CTR_MipsAddLo(blendedValue, timerValue);
#else
				__asm__("sll %0,%1,1" : "=r"(blendedValue) : "r"(needleValue));
				__asm__("addu %0,%0,%1" : "+r"(blendedValue) : "r"(needleValue));
				__asm__("sll %0,%0,2" : "+r"(blendedValue));
				__asm__("lw %0,7404(%1)" : "=r"(timerValue) : "r"(gGT), "m"(gGT->timer));
				__asm__("addu %0,%0,%1" : "+r"(blendedValue) : "r"(needleValue));
				__asm__("andi %0,%0,7" : "+r"(timerValue));
				__asm__("sll %0,%1,8" : "=r"(timerLowPart) : "r"(timerValue));
				__asm__("sll %0,%0,9" : "+r"(timerValue));
				__asm__("addu %0,%0,%1" : "+r"(timerValue) : "r"(timerLowPart));
				__asm__("addu %0,%0,%1" : "+r"(blendedValue) : "r"(timerValue));
#endif
				blendedValue = (s32)((u32)blendedValue >> VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT);
				nextNeedleValue = (s16)blendedValue;
			}
		}
		else
		{
			register int blendedValue CTR_PSX_REGISTER("$3") =
			    CTR_MipsAddLo(d->speedometerNeedleValue * VEH_PHYS_JUMP_SPEEDOMETER_BLEND_OLD, speedApprox * VEH_PHYS_JUMP_SPEEDOMETER_BLEND_NEW);

			CTR_PSX_SHIFT_RIGHT_ARITHMETIC_IN_PLACE(blendedValue, VEH_PHYS_JUMP_SPEEDOMETER_BLEND_SHIFT);
			nextNeedleValue = (s16)blendedValue;
		}
		d->speedometerNeedleValue = nextNeedleValue;
	}
}

enum ItemSet
{
	ITEMSET_Invalid = -1,
	ITEMSET_Race1 = 0,
	ITEMSET_Race2,
	ITEMSET_Race3,
	ITEMSET_Race4,
	ITEMSET_BattleDefault,
	ITEMSET_BattleCustom,
	ITEMSET_CrystalChallenge,
	ITEMSET_BossRace,
	ITEMSET_Count,
	ITEMSET_RNG_BUCKET_COUNT = 0xc8,
	ITEMSET_FALLBACK_ITEM_COUNT = HELD_ITEM_MISSILE_3X + 1,
	ITEMSET_WEAPON_COUNT_RACE1 = 0x14,
	ITEMSET_WEAPON_COUNT_RACE2 = 0x34,
	ITEMSET_WEAPON_COUNT_RACE3 = 0x14,
	ITEMSET_WEAPON_COUNT_RACE4 = 0x13,
	ITEMSET_WEAPON_COUNT_BATTLE_DEFAULT = 0x14,
	ITEMSET_WEAPON_COUNT_BOSS = 0x14,
	ITEMSET_RNG_RANDOM_SHIFT = 3,
	ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK_WARPBALL = 3,
	ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK = 4,
	ITEMSET_BOSS_LOSSES_REPLACE_CLOCK = 5,
	ITEMSET_THREE_MISSILES_MIN_PLAYERS = 3,
	ITEMSET_THREE_MISSILES_HELD_LIMIT = 2,
};

// Itemset infographic (outdated):
// https://discord.com/channels/330945093416779787/550106151887568906/734368526294450267
void VehPhysGeneral_SetHeldItem(struct Driver *driver)
{
	int itemSet = ITEMSET_Invalid;
	int mode;
	int onePlayerRank;
	register int rank CTR_PSX_REGISTER("$3");
	u32 rng;
	register int selectedItem CTR_PSX_REGISTER("$2");
	register struct GameTracker *gGT CTR_PSX_REGISTER("$4");

	// NOTE(aalhendi): Retail classifies the field before choosing the mode-specific item set.
	mode = GAME_TRACKER->numPlyrCurrGame + GAME_TRACKER->numBotsNextGame;

	if ((GAME_TRACKER->gameMode1 & BATTLE_MODE) != 0)
	{
		// 6th Itemset (Battle Mode Custom Itemset)
		itemSet = ITEMSET_BattleCustom;

		// 5th Itemset (Battle Mode Default Itemset)
		if (GAME_TRACKER->battleSetup.enabledWeapons == BATTLE_DEFAULT_WEAPON_FLAGS)
		{
			itemSet = ITEMSET_BattleDefault;
		}
	}

	// Not in Battle Mode
	else
	{
		if ((GAME_TRACKER->gameMode1 & CRYSTAL_CHALLENGE) != 0)
		{
			// 7th Itemset (Crystal Challenge)
			itemSet = ITEMSET_CrystalChallenge;
		}
		else
		{
			switch (mode)
			{
			// 1P Arcade
			case 8:
				onePlayerRank = driver->driverRank;

				// if in 2nd place, get itemSet2
				if (onePlayerRank == 1)
				{
					goto Itemset2;
				}
				itemSet = onePlayerRank / 2;
				break;

			// 2P Arcade
			case 6:
				rank = driver->driverRank;
				if (rank == 0)
				{
					goto Itemset1;
				}
				if (rank == 5)
				{
					goto Itemset4;
				}
				selectedItem = (rank - 1) / 2;
				itemSet = selectedItem + 1;
				break;

			case 5:
				itemSet = driver->driverRank;
				if (itemSet == 4)
				{
					itemSet = ITEMSET_Race4;
				}
				break;

			case 4:
				itemSet = driver->driverRank;
				break;

			// 3P VS race
			case 3:
				if (driver->driverRank == 0)
				{
					goto Itemset1;
				}
				itemSet = ITEMSET_Race4;
				if (driver->driverRank == 1)
				{
					itemSet = ITEMSET_Race3;
					rng = MixRNG_Scramble();
					if ((rng & 1) != 0)
					{
						goto Itemset2;
					}
				}
				break;

			Itemset2:
				itemSet = ITEMSET_Race2;
				break;

			Itemset4:
				CTR_PSX_OBSERVE_MEMORY(driver->heldItemID);
				itemSet = ITEMSET_Race4;
				break;

			// Boss race
			case 2:
				itemSet = ITEMSET_BossRace;
				if (driver->driverRank == 0)
				{
					goto Itemset1;
				}
				break;

			Itemset1:
				itemSet = ITEMSET_Race1;
			}
		}
	}

	// if you have 4th-place itemset on first lap,
	// then override to 3rd place
	if (itemSet == ITEMSET_Race4 && driver->lapIndex == 0)
	{
		itemSet = ITEMSET_Race3;
	}

	// Decide item for Driver
	rng = (MixRNG_Scramble() >> ITEMSET_RNG_RANDOM_SHIFT) % ITEMSET_RNG_BUCKET_COUNT;

	switch (itemSet)
	{
	case ITEMSET_Race1:
		selectedItem = VEH_ITEM_SET_RACE1[(rng * ITEMSET_WEAPON_COUNT_RACE1) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;
	case ITEMSET_Race2:
		selectedItem = VEH_ITEM_SET_RACE2[(rng * ITEMSET_WEAPON_COUNT_RACE2) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;
	case ITEMSET_Race3:
		selectedItem = VEH_ITEM_SET_RACE3[(rng * ITEMSET_WEAPON_COUNT_RACE3) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;
	case ITEMSET_Race4:
		// NOTE(aalhendi): Race4 storage is 0x14 bytes, but retail samples 0x13 entries.
		selectedItem = VEH_ITEM_SET_RACE4[(rng * ITEMSET_WEAPON_COUNT_RACE4) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;
	case ITEMSET_BossRace:
		selectedItem = VEH_ITEM_SET_BOSS_RACE[(rng * ITEMSET_WEAPON_COUNT_BOSS) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;
	case ITEMSET_BattleDefault:
		selectedItem = VEH_ITEM_SET_BATTLE_DEFAULT[(rng * ITEMSET_WEAPON_COUNT_BATTLE_DEFAULT) / ITEMSET_RNG_BUCKET_COUNT];
		CTR_PSX_OBSERVE_VALUE(selectedItem);
		driver->heldItemID = (u8)selectedItem;
		break;

	// NOTE(aalhendi): Overlay 230 builds this custom item set as a word array;
	// Vehicle reads the same shared representation here.
	case ITEMSET_BattleCustom:
		driver->heldItemID = GAME_TRACKER->battleSetup.RNG_itemSetCustom[((s32)rng * GAME_TRACKER->battleSetup.numWeapons) / ITEMSET_RNG_BUCKET_COUNT];
		break;

	case ITEMSET_CrystalChallenge:
		// Item is bomb at Rocky Road, Nitro Court
		// Item is turbo at Skull Rock and Rampage Ruins
		rank = GAME_TRACKER->levelID;
		if (rank == SKULL_ROCK || rank == RAMPAGE_RUINS)
		{
			driver->heldItemID = HELD_ITEM_TURBO;
			break;
		}
		selectedItem = HELD_ITEM_BOMB_1X;
		goto SetItem;

	// "-1st place": Undecided rank
	default:
	{
		s32 fallbackRng = MixRNG_Scramble();
		selectedItem = fallbackRng % ITEMSET_FALLBACK_ITEM_COUNT;
	}
	SetItem:
		driver->heldItemID = (u8)selectedItem;
	}

	// In Boss race
	if (GAME_TRACKER->gameMode1 & ADVENTURE_BOSS)
	{
		s8 bossFails = GAME_ADV_PROGRESS.timesLostBossRace[GAME_TRACKER->bossID];

		if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK_WARPBALL)
		{
			// Replace Clock, Mask,  with 3 Missiles
			if ((u32)driver->heldItemID - HELD_ITEM_MASK < (HELD_ITEM_WARPBALL - HELD_ITEM_MASK + 1))
			{
				driver->heldItemID = HELD_ITEM_MISSILE_3X;
			}
		}

		else if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_MASK_CLOCK)
		{
			// Replace Clock, Mask with 3 Missiles
			if ((u32)driver->heldItemID - HELD_ITEM_MASK < (HELD_ITEM_CLOCK - HELD_ITEM_MASK + 1))
			{
				driver->heldItemID = HELD_ITEM_MISSILE_3X;
			}
		}

		else if (bossFails < ITEMSET_BOSS_LOSSES_REPLACE_CLOCK && driver->heldItemID == HELD_ITEM_CLOCK)
		{
			// Replace Clock with 3 Missiles
			driver->heldItemID = HELD_ITEM_MISSILE_3X;
		}

		// Replace 3 Missiles with 1 Missile if racing Komodo Joe
		if (GAME_TRACKER->levelID == DRAGON_MINES && driver->heldItemID == HELD_ITEM_MISSILE_3X)
		{
			driver->heldItemID = HELD_ITEM_MISSILE_1X;
		}
	}

	// Replace unused Spring item with Turbo
	if (driver->heldItemID == HELD_ITEM_SPRING)
	{
		driver->heldItemID = HELD_ITEM_TURBO;
	}

	// Make sure only 1 Warpball is instanced at once
	if (driver->heldItemID == HELD_ITEM_WARPBALL)
	{
		VEH_LOAD_GAME_TRACKER(gGT);

		// if somebody has warpball already, then give 3 missiles
		if ((gGT->gameMode1 & WARPBALL_HELD) != 0)
		{
			driver->heldItemID = HELD_ITEM_MISSILE_3X;
		}

		// if nobody has warpball, then set flag that somebody has it
		else
		{
			gGT->gameMode1 |= WARPBALL_HELD;
		}
	}

	VEH_LOAD_GAME_TRACKER(gGT);
	if (
	    // if three or more players
	    gGT->numPlyrCurrGame >= ITEMSET_THREE_MISSILES_MIN_PLAYERS &&

	    // if not in battle mode
	    ((gGT->gameMode1 & BATTLE_MODE) == 0) &&

	    // if you got 3 missiles
	    driver->heldItemID == HELD_ITEM_MISSILE_3X)
	{
		// if less than 2 drivers have 3 missiles, then increase number of drivers that have it
		if (gGT->numPlayersWith3Missiles < ITEMSET_THREE_MISSILES_HELD_LIMIT)
		{
			gGT->numPlayersWith3Missiles++;
		}

		// if 2 drivers already have 3 missiles, now you have 1 missile
		else
		{
			driver->heldItemID = HELD_ITEM_MISSILE_1X;
		}
	}

	// Set number of held items
	if ((u32)driver->heldItemID - HELD_ITEM_BOMB_3X < (HELD_ITEM_MISSILE_3X - HELD_ITEM_BOMB_3X + 1))
	{
		driver->numHeldItems = HELD_ITEM_STACK_COUNT;
	}

	if (driver->heldItemID == HELD_ITEM_MASK)
	{
		driver->maskIsAku = (s8)VehPickupItem_ApplyMaskMode(driver);
	}
	else
	{
		driver->maskIsAku = -1;
	}

	return;
}

enum
{
	VEH_BASE_SPEED_MAX_WUMPA = 9,
	VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER = 5,
	VEH_BASE_SPEED_STAT_BLEND_SHIFT = 0xc,
	VEH_BASE_SPEED_STAT_DIVISOR = 5,
	VEH_BASE_SPEED_STAT_OFFSET = 1,
	VEH_BASE_SPEED_WUMPA_DIVISOR = 10,
	VEH_BASE_SPEED_DAMAGE_HALF_SHIFT = 1,
	VEH_BASE_SPEED_CLOCK_RANK_BASE = 0x14,
	VEH_BASE_SPEED_CLOCK_DAMAGE_SHIFT = 4,
	VEH_BASE_SPEED_NET_CAP = 0x6400,
};


int VehPhysGeneral_GetBaseSpeed(struct Driver *driver)
{
	int netSpeedStat;
	int statAdditional;
	int speedAdditional;
	register int turboMultiplier CTR_PSX_REGISTER("$8");
	int netSpeedCap;
	register int subtract CTR_PSX_REGISTER("$7");
	int clockEffect;
	register struct Driver *d CTR_PSX_REGISTER("$6") = driver;
#if !defined(CTR_NATIVE)
	register int divideMagic CTR_PSX_REGISTER("$4");
#endif
	int scaledSpeedStat;
#if !defined(CTR_NATIVE)
	int productHigh;
	int productSign;
#endif

	CTR_PSX_LOAD_SIGNED_BYTE(speedAdditional, d, offsetof(struct Driver, numWumpas), d->numWumpas);
	if (speedAdditional > VEH_BASE_SPEED_MAX_WUMPA)
	{
		speedAdditional = VEH_BASE_SPEED_MAX_WUMPA;
	}
	CTR_PSX_BIND_VALUE_CLOBBER(speedAdditional, "$5");
	CTR_PSX_LOAD_SIGNED_BYTE(turboMultiplier, d, offsetof(struct Driver, turboConst), d->turboConst);
#if !defined(CTR_NATIVE)
	// NOTE(aalhendi): Retail keeps one signed-division magic value live
	// across the clamp, then reuses it for both divisions below.
	divideMagic = 0x66660000;
#endif
	if (turboMultiplier > VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER)
	{
		turboMultiplier = VEH_BASE_SPEED_MAX_TURBO_MULTIPLIER;
	}
#if !defined(CTR_NATIVE)
	CTR_PSX_KEEP_VALUE(divideMagic);
#endif

#if defined(CTR_NATIVE)
	scaledSpeedStat = d->const_AccelSpeed_ClassStat;
	statAdditional = d->const_Speed_ClassStat;
	netSpeedStat = CTR_MipsSubLo(CTR_MipsSll(CTR_MipsSubLo(scaledSpeedStat, statAdditional), VEH_BASE_SPEED_STAT_BLEND_SHIFT) / VEH_BASE_SPEED_STAT_DIVISOR,
	                             VEH_BASE_SPEED_STAT_OFFSET);
#else
	scaledSpeedStat = d->const_AccelSpeed_ClassStat;
	__asm__("lh %0,%1" : "=r"(statAdditional) : "m"(d->const_Speed_ClassStat), "r"(scaledSpeedStat));
	divideMagic |= 0x6667;
	scaledSpeedStat = CTR_MipsSll(CTR_MipsSubLo(scaledSpeedStat, statAdditional), VEH_BASE_SPEED_STAT_BLEND_SHIFT);
	__asm__("mult %1,%2" : "=h"(productHigh) : "r"(scaledSpeedStat), "r"(divideMagic));
	productSign = CTR_MipsSra(scaledSpeedStat, 31);
	netSpeedStat = CTR_MipsSubLo(CTR_MipsSubLo(CTR_MipsSra(productHigh, 1), productSign), VEH_BASE_SPEED_STAT_OFFSET);
#endif

	{
		register int wumpaProduct CTR_PSX_REGISTER("$2");
		register int speedSum CTR_PSX_REGISTER("$2");

		wumpaProduct = CTR_MipsMulLo(speedAdditional, netSpeedStat);
#if defined(CTR_NATIVE)
		speedAdditional = wumpaProduct / VEH_BASE_SPEED_WUMPA_DIVISOR;
		turboMultiplier = CTR_MipsMulLo(turboMultiplier, netSpeedStat);
#else
		// NOTE(aalhendi): Preserve the two-instruction gap emitted before the retail division multiply.
		__asm__ volatile("nop\n\tnop");
		__asm__ volatile("mult %1,%2" : "=h"(speedAdditional) : "r"(wumpaProduct), "r"(divideMagic));
		turboMultiplier = CTR_MipsMulLo(turboMultiplier, netSpeedStat);
#endif
		CTR_PSX_BIND_VALUE_CLOBBER(turboMultiplier, "$12");
#if !defined(CTR_NATIVE)
		productSign = CTR_MipsSra(wumpaProduct, 31);
		speedAdditional = CTR_MipsSubLo(CTR_MipsSra(speedAdditional, 2), productSign);
#endif
		CTR_PSX_BIND_VALUE_CLOBBER(speedAdditional, "$13");
		speedSum = CTR_MipsAddLo(speedAdditional, turboMultiplier);
		CTR_PSX_BIND_VALUE_CLOBBER(speedSum, "$11");
		speedAdditional = CTR_MipsSra(speedSum, VEH_BASE_SPEED_STAT_BLEND_SHIFT);
	}
	CTR_PSX_BIND_VALUE_CLOBBER(speedAdditional, "$2");

	if ((d->actionsFlagSet & ACTION_MASK_WEAPON) != 0)
	{
	        int maskSpeed = d->const_MaskSpeed * g_config.maskExtraSpeedMultiplier / 100;
		speedAdditional = CTR_MipsAddLo(speedAdditional, maskSpeed);
	}

	if (d->reserves != 0)
	{
		netSpeedCap = d->const_SingleTurboSpeed;
		statAdditional = CTR_MipsAddLo(statAdditional, d->fireSpeedCap);
		netSpeedCap = CTR_MipsAddLo(netSpeedCap, CTR_MipsSll(CTR_MipsSubLo(d->const_SacredFireSpeed, netSpeedCap), 1));
		netSpeedCap = CTR_MipsSubLo(netSpeedCap, d->fireSpeedCap);

		if (netSpeedCap < 0)
		{
			netSpeedCap = 0;
		}
		if (netSpeedCap < speedAdditional)
		{
			speedAdditional = netSpeedCap;
		}
	}
	statAdditional = CTR_MipsAddLo(statAdditional, speedAdditional);

	subtract = 0;

	if (d->instTntRecv != NULL)
	{
		register int damagedSpeed CTR_PSX_REGISTER("$2");

		damagedSpeed = (u16)d->const_DamagedSpeed;
		CTR_PSX_BIND_VALUE_CLOBBER(damagedSpeed, "$7");
		subtract = CTR_MipsSra(CTR_MipsSll(damagedSpeed, 16), 16 + VEH_BASE_SPEED_DAMAGE_HALF_SHIFT);
	}

	if ((d->burnTimer != 0) || (d->squishTimer != 0) || (d->rainCloudEffect == RAIN_CLOUD_EFFECT_SLOW))
	{
		subtract = d->const_DamagedSpeed;
	}

	if (d->clockReceive != 0)
	{
		clockEffect =
		    CTR_MipsSra(CTR_MipsMulLo(d->const_DamagedSpeed, CTR_MipsSubLo(VEH_BASE_SPEED_CLOCK_RANK_BASE, d->driverRank)), VEH_BASE_SPEED_CLOCK_DAMAGE_SHIFT);
		if (subtract < clockEffect)
		{
			subtract = clockEffect;
		}
	}

	statAdditional = CTR_MipsSubLo(statAdditional, subtract);

	if (VEH_BASE_SPEED_NET_CAP < statAdditional)
	{
		statAdditional = VEH_BASE_SPEED_NET_CAP;
	}

	return statAdditional;
}
