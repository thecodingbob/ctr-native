#include "VehCommon.h"

#ifndef VEH_TALK_MASK_HEAD
#define VEH_TALK_MASK_HEAD (sdata->s_head)
#endif

enum
{
	TALK_MASK_3D_MODEL_SCALE = 0x2000,
	TALK_MASK_STATIC_MODEL_SCALE = FP(1.0),
	TALK_MASK_SCALE_SHIFT = 0xc,

	TALK_MASK_MOUTH_SAMPLE_MULTIPLIER = 7,
	TALK_MASK_MOUTH_NEGATIVE_ROUND_BIAS = 0x3fff,
	TALK_MASK_MOUTH_SAMPLE_SHIFT = 0xe,
	TALK_MASK_MOUTH_SILENCE_FRAME_THRESHOLD = 2,
	TALK_MASK_MOUTH_SNAP_TARGET_THRESHOLD = 4,
	TALK_MASK_MOUTH_SNAP_DELTA_THRESHOLD = 4,
	TALK_MASK_MOUTH_FINAL_LERP_DELTA_THRESHOLD = 6,
	TALK_MASK_MOUTH_LERP_STEP = 1,

	TALK_MASK_INIT_MODEL = STATIC_AKUAKU,
	TALK_MASK_INIT_OBJECT_SIZE = 6,
	TALK_MASK_XA_TYPE = CDSYS_XA_TYPE_EXTRA,
};

CTR_STATIC_ASSERT((s32)TALK_MASK_INIT_MODEL == (s32)STATIC_AKUAKU);
CTR_STATIC_ASSERT((s32)STATIC_UKAUKA - 1 == (s32)STATIC_AKUAKU);

void VehTalkMask_ThTick(struct Thread *t)
{
	struct GameTracker *gGT;

	struct MaskHint *mhObj = t->object;
	struct Instance *mhInst = t->inst;

	s32 modelScale = TALK_MASK_3D_MODEL_SCALE;
	s32 scale;
	s32 xaSamplePeak;
	s32 numAnimFrames;
	s32 mouthSampleProduct;
	s32 targetMouthFrame;
	s32 currentMouthFrame;
	s32 mouthFrameDelta;
	u32 threadFlags;
	register s32 firstLerpTarget CTR_PSX_REGISTER("$4");

	if (VEH_MODEL_MASK_HINTS_3D != 0)
	{
		mhInst->model = VEH_MODEL_MASK_HINTS_3D;
		goto ModelSelected;
	}

	{
		gGT = GAME_TRACKER;

		if (gGT->drivers[0] != 0)
		{
			s16 boolGoodGuy;
			s32 modelOffset;
			u8 *modelAddress;

			boolGoodGuy = VehPickupItem_MaskBoolGoodGuy(gGT->drivers[0]);
			modelOffset = STATIC_UKAUKA * sizeof(gGT->modelPtr[0]);
			if (boolGoodGuy != 0)
			{
				modelOffset = STATIC_AKUAKU * sizeof(gGT->modelPtr[0]);
			}
			CTR_PSX_ADD_POINTER_OFFSET(modelAddress, gGT, modelOffset);
			mhInst->model = *(struct Model **)(modelAddress + offsetof(struct GameTracker, modelPtr));
			CTR_PSX_OBSERVE_MEMORY(mhInst->model);
			modelScale = TALK_MASK_STATIC_MODEL_SCALE;
		}
		else
		{
			modelScale = TALK_MASK_STATIC_MODEL_SCALE;
		}
	}

ModelSelected:

	xaSamplePeak = VEH_XA_MAX_SAMPLE_VALUE;
	scale = (mhObj->scale * modelScale) >> TALK_MASK_SCALE_SHIFT;
	sdata->talkMaskXASamplePeak = xaSamplePeak;
	mhInst->scale.z = scale;
	mhInst->scale.y = scale;
	mhInst->scale.x = scale;

	numAnimFrames = VehFrameInst_GetNumAnimFrames(mhInst, 0);

	mouthSampleProduct = sdata->talkMaskXASamplePeak * TALK_MASK_MOUTH_SAMPLE_MULTIPLIER;

	if (mouthSampleProduct < 0)
	{
		mouthSampleProduct += TALK_MASK_MOUTH_NEGATIVE_ROUND_BIAS;
	}

	targetMouthFrame = mouthSampleProduct >> TALK_MASK_MOUTH_SAMPLE_SHIFT;

	if (sdata->talkMaskMaxMouthFrame < targetMouthFrame)
	{
		sdata->talkMaskMaxMouthFrame = targetMouthFrame;
	}

	if (targetMouthFrame < TALK_MASK_MOUTH_SILENCE_FRAME_THRESHOLD)
	{
		targetMouthFrame = 0;
	}

	firstLerpTarget = targetMouthFrame;
	if (targetMouthFrame < TALK_MASK_MOUTH_SNAP_TARGET_THRESHOLD)
	{
		currentMouthFrame = mhInst->animFrame;
		goto LerpMouthFrame;
	}
	else
	{
		currentMouthFrame = mhInst->animFrame;
		mouthFrameDelta = currentMouthFrame - targetMouthFrame;

		if (mouthFrameDelta < 0)
		{
			mouthFrameDelta = -mouthFrameDelta;
		}

		if (mouthFrameDelta >= TALK_MASK_MOUTH_SNAP_DELTA_THRESHOLD)
		{
			goto SnapMouthFrame;
		}
	}

LerpMouthFrame:
	mhInst->animFrame = EngineSound_VolumeAdjust(firstLerpTarget, currentMouthFrame, TALK_MASK_MOUTH_LERP_STEP);
	goto SkipLerp;

SnapMouthFrame:
	mhInst->animFrame = (s16)targetMouthFrame;

SkipLerp:

	currentMouthFrame = mhInst->animFrame;

	// animFrame
	mouthFrameDelta = currentMouthFrame - targetMouthFrame;

	if (mouthFrameDelta < 0)
	{
		mouthFrameDelta = -mouthFrameDelta;
	}

	if (mouthFrameDelta < TALK_MASK_MOUTH_FINAL_LERP_DELTA_THRESHOLD)
	{
		mhInst->animFrame = EngineSound_VolumeAdjust(targetMouthFrame, currentMouthFrame, TALK_MASK_MOUTH_LERP_STEP);
	}
	else
	{
		mhInst->animFrame = (s16)targetMouthFrame;
	}

	// animation frame goes back and forth
	// 0x00: mouth close
	// 0x0C: mouth open

	if (mhInst->animFrame < 0)
	{
		mhInst->animFrame = 0;
	}
	else if ((numAnimFrames - 1) < mhInst->animFrame)
	{
		mhInst->animFrame = (s16)(numAnimFrames - 1);
	}

	if (sdata->talkMask_boolDead != 0)
	{
		threadFlags = t->flags;
		sdata->talkMask_boolDead = 0;

		// dead thread
		t->flags = threadFlags | THREAD_FLAG_DEAD;
	}
}

struct Instance *VehTalkMask_Init(void)
{
	struct Instance *mhInst;
	struct Thread *mhTh;

	VEH_MASK_THREAD_ALIVE = 1;
	VEH_TALK_MASK_DEAD = 0;

	mhInst = INSTANCE_BirthWithThread(TALK_MASK_INIT_MODEL, VEH_TALK_MASK_HEAD, SMALL, AKUAKU, VehTalkMask_ThTick, TALK_MASK_INIT_OBJECT_SIZE, 0);

	mhTh = mhInst->thread;
	mhTh->funcThDestroy = PROC_DestroyInstance;

	((struct MaskHint *)mhTh->object)->scale = 0;

	return mhInst;
}

void VehTalkMask_PlayXA(struct Instance *i, s32 id)
{
	struct Driver *d;
	register s32 originalID CTR_PSX_REGISTER("$17");
	register s32 xaID CTR_PSX_REGISTER("$16");

	(void)i;
	originalID = id;
	d = GAME_TRACKER->drivers[0];
	xaID = originalID;

	if (d != 0)
	{
		s16 boolGoodGuy = (s16)VehPickupItem_MaskBoolGoodGuy(d);

		if (boolGoodGuy == 0)
		{
			xaID = CTR_MipsAddLo(originalID, ADV_MASK_HINT_UKA_UKA_XA_OFFSET);
		}
	}

	CDSYS_XAPlay(TALK_MASK_XA_TYPE, (s16)xaID);
}

b32 VehTalkMask_boolNoXA(void)
{
	return VEH_XA_STATE == 0;
}

void VehTalkMask_End(void)
{
	CDSYS_XAPauseRequest();

	VEH_TALK_MASK_DEAD = 1;
	VEH_MASK_THREAD_ALIVE = 0;
}
