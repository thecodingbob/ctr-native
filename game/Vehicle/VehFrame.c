#include "VehCommon.h"

enum
{
	VEH_FRAME_ANIM_DRIVE = 0,
	VEH_FRAME_ANIM_STATIONARY = 1,
	VEH_FRAME_ANIM_MATRIX_FIRST = 2,
	VEH_FRAME_ANIM_AIRBORNE = 3,
	VEH_FRAME_ANIM_END_START = 4,
	VEH_FRAME_MATRIX_ANIM_COUNT = 2,
	VEH_FRAME_NUM_FRAMES_MASK = 0x7fff,
	VEH_FRAME_BURN_SMOKE_ICON_GROUP = 1,
	VEH_FRAME_JUMP_AIR_THRESHOLD = 0x600,
	VEH_FRAME_AIR_GAP_THRESHOLD = 0x8000,
	VEH_FRAME_BURN_TIMER_LIMIT = 0x1e0,
	VEH_FRAME_BURN_TIMER_SHIFT = 5,
	VEH_FRAME_BURN_FRAME_PERIOD = 5,
	VEH_FRAME_BURN_FRAME_SHIFT = 2,
	VEH_FRAME_BURN_FRAME_BIAS = 8,
	VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT = 0x40,
	VEH_FRAME_TRANSITION_DEFAULT_SPEED = 2,
	VEH_FRAME_TRANSITION_DRIVE_SPEED = 6,
	VEH_FRAME_TRANSITION_MATRIX_SPEED = 1,
	VEH_FRAME_INTERP_SPEED_NORMAL = 1,
	VEH_FRAME_SPIN_INTERP_SPEED = 4,
	VEH_FRAME_LAST_SPIN_INTERP_SPEED = 3,
	VEH_FRAME_AIRBORNE_MATRIX_BASE = BAKED_GTE_MATRIX_JUMP_BASE,
	VEH_FRAME_OXIDE_MATRIX_ARRAY = BAKED_GTE_MATRIX_JUMP_OXIDE,
};


int VehFrameInst_GetStartFrame(int animIndex, int numFrames)
{
	switch (animIndex)
	{
	// midpoint
	case VEH_FRAME_ANIM_DRIVE:
		return numFrames >> 1;

	// end
	case VEH_FRAME_ANIM_END_START:
		return (numFrames - 1);

	// start
	default:
		return 0;
	}
}

u32 VehFrameInst_GetNumAnimFrames(struct Instance *inst, int animIndex)
{
	struct ModelHeader *mh;
	struct ModelAnim *anim;

	if (inst->model == NULL)
	{
		return 0;
	}
	if (inst->model->numHeaders <= 0)
	{
		return 0;
	}
	if (inst->model->headers == NULL)
	{
		return 0;
	}

	mh = inst->model->headers;

	if (animIndex >= (int)mh->numAnimations)
	{
		return 0;
	}
	if (mh->ptrAnimations == NULL)
	{
		return 0;
	}

	anim = mh->ptrAnimations[animIndex];

	if (anim == NULL)
	{
		return 0;
	}

	return anim->numFrames & VEH_FRAME_NUM_FRAMES_MASK;
}

static inline void VehFrameProc_Driving_SpawnBurnSmoke(struct Driver *d)
{
	struct Particle *p = Particle_Init(0, GAME_TRACKER->iconGroup[VEH_FRAME_BURN_SMOKE_ICON_GROUP], &data.emSet_BurnSmoke[0]);

	if (p != NULL)
	{
		p->otIndexOffset = d->instSelf->depthBiasNormal;
		p->owner.driverInst = d->instSelf;
		p->driverID = d->driverID;
	}
}

void VehFrameProc_Driving(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int desiredAnim = VEH_FRAME_ANIM_DRIVE;
	int numFrames;

	if ((d->instTntRecv == NULL) && (d->kartState != KS_WARP_PAD))
	{
		if (d->fireSpeed < 0)
		{
			desiredAnim = (d->speedApprox < 1) ? VEH_FRAME_ANIM_STATIONARY : VEH_FRAME_ANIM_DRIVE;
		}

		if (((d->jumpHeightCurr > VEH_FRAME_JUMP_AIR_THRESHOLD) || (inst->animIndex == VEH_FRAME_ANIM_AIRBORNE)) &&
		    (d->posCurr.y - d->quadBlockHeight > VEH_FRAME_AIR_GAP_THRESHOLD))
		{
			desiredAnim = VEH_FRAME_ANIM_AIRBORNE;
		}
	}

	numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	if (numFrames <= 0)
	{
		return;
	}

	{
		register int currAnim CTR_PSX_REGISTER("$5") = inst->animIndex;

		if (desiredAnim != currAnim)
		{
			goto TransitionAnimation;
		}

	ProcessCurrentAnimation:
		if (desiredAnim == VEH_FRAME_ANIM_DRIVE)
		{
			goto DriveAnimation;
		}
		if (desiredAnim == VEH_FRAME_ANIM_AIRBORNE)
		{
			goto AirborneAnimation;
		}
		goto OtherAnimation;

	DriveAnimation:
	{
		int targetFrame;

		if (d->instTntRecv != NULL)
		{
			targetFrame = numFrames >> 1;
		}
		else
		{
			s16 burnTimer = d->burnTimer;

			if ((burnTimer != 0) && (burnTimer < VEH_FRAME_BURN_TIMER_LIMIT))
			{
				targetFrame = CTR_MipsAddLo(
				    CTR_MipsSubLo(CTR_MipsSll((burnTimer >> VEH_FRAME_BURN_TIMER_SHIFT) % VEH_FRAME_BURN_FRAME_PERIOD, VEH_FRAME_BURN_FRAME_SHIFT),
				                  VEH_FRAME_BURN_FRAME_BIAS),
				    CTR_MipsSra(numFrames, 1));
				inst->animFrame = targetFrame;
				VehFrameProc_Driving_SpawnBurnSmoke(d);
			}
			else
			{
				if ((d->actionsFlagSet & ACTION_ACCEL_PREVENTION) != 0)
				{
					targetFrame =
					    VehCalc_MapToRange(-d->ampTurnState, -VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT, VEH_FRAME_TURN_ACCEL_PREVENTION_LIMIT, 0, numFrames - 1);
				}
				else
				{
					int turnInput = d->simpTurnState;
					int turnMax = (u8)d->const_TurnRate;

					targetFrame = VehCalc_MapToRange(-turnInput, -turnMax, turnMax, 0, numFrames - 1);
				}
			}
		}

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, targetFrame);
		return;
	}

	AirborneAnimation:
	{
		s32 characterID;
		s32 matrixArray;

		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, numFrames - 1);

		if (d->kartState == KS_MASK_GRABBED)
		{
			return;
		}

		characterID = GAME_CHARACTER_IDS[d->driverID];
		if (characterID == PENTA_PENGUIN)
		{
			characterID = COCO_BANDICOOT;
		}
		if (characterID == FAKE_CRASH)
		{
			characterID = CRASH_BANDICOOT;
		}

		if (characterID == NITROS_OXIDE)
		{
			characterID = VEH_FRAME_OXIDE_MATRIX_ARRAY - VEH_FRAME_AIRBORNE_MATRIX_BASE;
		}
		matrixArray = characterID + VEH_FRAME_AIRBORNE_MATRIX_BASE;

		d->matrixArray = matrixArray;
		d->matrixIndex = (u8)inst->animFrame;
		return;
	}

	OtherAnimation:
		inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_INTERP_SPEED_NORMAL, numFrames - 1);
		return;

	TransitionAnimation:
		if (currAnim == VEH_FRAME_ANIM_MATRIX_FIRST)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, currAnim) - 1;
		}
		else
		{
			numFrames = VehFrameInst_GetStartFrame(currAnim, numFrames);
		}

		if (inst->animFrame == numFrames)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, desiredAnim);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = desiredAnim;
			inst->animFrame = VehFrameInst_GetStartFrame(desiredAnim, numFrames);
			d->matrixArray = BAKED_GTE_MATRIX_NONE;
			d->matrixIndex = 0;
			goto ProcessCurrentAnimation;
		}
		else
		{
			int speed;
			int transitionAnim = inst->animIndex;

			switch (transitionAnim)
			{
			case VEH_FRAME_ANIM_DRIVE:
				speed = VEH_FRAME_TRANSITION_DRIVE_SPEED;
				break;
			case VEH_FRAME_ANIM_MATRIX_FIRST:
				d->matrixIndex = inst->animFrame;
				speed = VEH_FRAME_TRANSITION_MATRIX_SPEED;
				break;
			default:
				speed = VEH_FRAME_TRANSITION_DEFAULT_SPEED;
				break;
			}

			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, speed, numFrames);

			if ((u32)(inst->animIndex - VEH_FRAME_ANIM_MATRIX_FIRST) < VEH_FRAME_MATRIX_ANIM_COUNT)
			{
				d->matrixIndex = (u8)inst->animFrame;
				if (d->matrixIndex == 0)
				{
					d->matrixArray = BAKED_GTE_MATRIX_NONE;
					d->matrixIndex = 0;
				}
			}

			return;
		}
	}
}

void VehFrameProc_Spinning(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int numFrames = VehFrameInst_GetNumAnimFrames(inst, inst->animIndex);
	int targetFrame;

	if (numFrames <= 0)
	{
		return;
	}

	if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
	{
		targetFrame = VehFrameInst_GetStartFrame(inst->animIndex, numFrames);

		if ((u32)(inst->animIndex - VEH_FRAME_ANIM_MATRIX_FIRST) < VEH_FRAME_MATRIX_ANIM_COUNT)
		{
			d->matrixArray = BAKED_GTE_MATRIX_NONE;
			d->matrixIndex = 0;
		}

		if (inst->animFrame == targetFrame)
		{
			numFrames = VehFrameInst_GetNumAnimFrames(inst, VEH_FRAME_ANIM_DRIVE);
			if (numFrames <= 0)
			{
				return;
			}

			inst->animIndex = VEH_FRAME_ANIM_DRIVE;
		}

		if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
		{
			inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_SPIN_INTERP_SPEED, targetFrame);
			return;
		}
	}

	targetFrame = 0;
	if (d->KartStates.Spinning.spinDir >= 0)
	{
		targetFrame = numFrames - 1;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_SPIN_INTERP_SPEED, targetFrame);
}

void VehFrameProc_LastSpin(struct Thread *t, struct Driver *d)
{
	struct Instance *inst = t->inst;
	int numFrames;
	int targetFrame;

	if (inst->animIndex != VEH_FRAME_ANIM_DRIVE)
	{
		VehFrameProc_Spinning(t, d);
		return;
	}

	numFrames = VehFrameInst_GetNumAnimFrames(inst, VEH_FRAME_ANIM_DRIVE);
	if (numFrames <= 0)
	{
		return;
	}

	targetFrame = inst->animFrame;

	if (d->turnAngleCurr > 0)
	{
		if (d->turnAngleLerpVel < 0)
		{
			targetFrame = numFrames - 1;
		}
	}

	if ((d->turnAngleCurr < 0) && (d->turnAngleLerpVel > 0))
	{
		targetFrame = 0;
	}

	inst->animFrame = VehCalc_InterpBySpeed(inst->animFrame, VEH_FRAME_LAST_SPIN_INTERP_SPEED, targetFrame);
}
