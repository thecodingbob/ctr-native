#include <common.h>

enum CsInstanceConstants
{
	CS_ANIM_FRAME_COUNT_MASK = 0x7fff,
	CS_ANIM_BONE_AXIS_STRIDE = 3,
	CS_ANIM_BONE_DATA_OFFSET = 0x1c,
	CS_ANIM_BONE_VALUE_X_OFFSET = 0,
	CS_ANIM_BONE_VALUE_Z_OFFSET = 1,
	CS_ANIM_BONE_VALUE_Y_OFFSET = 2,
	CS_ANIM_BONE_TARGET_X_OFFSET = 3,
	CS_ANIM_BONE_TARGET_Z_OFFSET = 4,
	CS_ANIM_BONE_TARGET_Y_OFFSET = 5,
};

void CS_Instance_GetFrameData(struct Instance *inst, int animIndex, s32 animFrame, SVec3 *pos, SVec3 *rotOut, int offset)
{
	int isOdd;
	int numFrames;
	struct ModelAnim *ptrAnim;
	s16 *framePos;
	u8 *bonePtr;
	u32 boneValueX, boneValueY, boneValueZ;
	u32 boneTargetX, boneTargetY, boneTargetZ;
	struct ModelHeader *headers;

	headers = inst->model->headers;
	ptrAnim = headers->ptrAnimations[animIndex];

	if (animFrame < 0)
	{
		animFrame = 0;
	}

	numFrames = (s16)ptrAnim->numFrames;
	isOdd = 0;

	if (numFrames < 0)
	{
		// A negative frame count stores every other frame; odd frames interpolate.
		numFrames = -numFrames;
		isOdd = animFrame & 1;
		animFrame = animFrame >> 1;
	}

	if (animFrame >= (numFrames - 1))
	{
		animFrame = numFrames - 1;
		isOdd = 0;
	}

	framePos = (s16 *)((char *)ptrAnim + ((s16)ptrAnim->frameSize * (int)animFrame + sizeof(struct ModelAnim)));

	{
		int boneOff = offset * CS_ANIM_BONE_AXIS_STRIDE + CS_ANIM_BONE_DATA_OFFSET;
		bonePtr = (u8 *)framePos + boneOff;
	}

	boneValueX = (u32)bonePtr[CS_ANIM_BONE_VALUE_X_OFFSET];
	boneValueY = (u32)bonePtr[CS_ANIM_BONE_VALUE_Y_OFFSET];
	boneValueZ = (u32)bonePtr[CS_ANIM_BONE_VALUE_Z_OFFSET];
	boneTargetX = (u32)bonePtr[CS_ANIM_BONE_TARGET_X_OFFSET];
	boneTargetY = (u32)bonePtr[CS_ANIM_BONE_TARGET_Y_OFFSET];
	boneTargetZ = (u32)bonePtr[CS_ANIM_BONE_TARGET_Z_OFFSET];

	if (isOdd)
	{
		framePos = (s16 *)((char *)framePos + (s16)ptrAnim->frameSize);
		{
			int boneOff = offset * CS_ANIM_BONE_AXIS_STRIDE + CS_ANIM_BONE_DATA_OFFSET;
			bonePtr = (u8 *)framePos + boneOff;
		}

		boneValueX = (int)(boneValueX + bonePtr[CS_ANIM_BONE_VALUE_X_OFFSET]) >> 1;
		boneValueY = (int)(boneValueY + bonePtr[CS_ANIM_BONE_VALUE_Y_OFFSET]) >> 1;
		boneValueZ = (int)(boneValueZ + bonePtr[CS_ANIM_BONE_VALUE_Z_OFFSET]) >> 1;
		boneTargetX = (int)(boneTargetX + bonePtr[CS_ANIM_BONE_TARGET_X_OFFSET]) >> 1;
		boneTargetY = (int)(boneTargetY + bonePtr[CS_ANIM_BONE_TARGET_Y_OFFSET]) >> 1;
		boneTargetZ = (int)(boneTargetZ + bonePtr[CS_ANIM_BONE_TARGET_Z_OFFSET]) >> 1;
	}

	{
		s16 instScale = inst->scale.x;
		s32 partialY, partialZ;
		s32 scaledY, scaledZ;

		// Keep the bone direction unscaled, and apply the two Q12 position scales separately.
		boneTargetX = (int)boneValueX - (int)boneTargetX;
		boneValueX = (int)boneValueX + framePos[0];
		boneValueX = (int)boneValueX * instScale;
		boneValueX = ((int)boneValueX >> FRACTIONAL_BITS) * headers->scale.x;
		boneTargetY = (int)boneValueY - (int)boneTargetY;
		boneValueY = (int)boneValueY + framePos[1];
		partialY = (int)boneValueY * instScale;
		scaledY = (partialY >> FRACTIONAL_BITS) * headers->scale.y;
		boneTargetZ = (int)boneValueZ - (int)boneTargetZ;
		boneValueZ = (int)boneValueZ + framePos[2];
		partialZ = (int)boneValueZ * instScale;
		scaledZ = (partialZ >> FRACTIONAL_BITS) * headers->scale.z;
		boneValueX = (int)boneValueX >> FRACTIONAL_BITS;
		boneValueY = scaledY >> FRACTIONAL_BITS;
		boneValueZ = scaledZ >> FRACTIONAL_BITS;
	}
	CTR_GteLoadLightMatrix(&inst->matrix);

	MTC2(CTR_PackS16Pair(boneValueX, boneValueY), 0);
	MTC2(boneValueZ, 1);
	CTR_GteLoadDelay();
	gte_llv0();

	boneValueX = MFC2_S(25);
	boneValueY = MFC2_S(26);
	boneValueZ = MFC2_S(27);
	pos->x = boneValueX;
	pos->y = boneValueY;
	pos->z = boneValueZ;

	if (rotOut != NULL)
	{
		MTC2(CTR_PackS16Pair(boneTargetX, boneTargetY), 0);
		MTC2(boneTargetZ, 1);
		CTR_GteLoadDelay();
		gte_llv0();

		boneTargetX = MFC2_S(25);
		boneTargetY = MFC2_S(26);
		boneTargetZ = MFC2_S(27);
		rotOut->x = ratan2(-boneTargetY, SquareRoot0_stub(boneTargetX * boneTargetX + boneTargetZ * boneTargetZ));
		rotOut->y = ratan2(boneTargetX, boneTargetZ);
		rotOut->z = 0;
	}
}

int CS_Instance_GetNumAnimFrames(struct Instance *modelInst, int animIndex, int LOD)
{
	struct Model *model;
	struct ModelHeader *header;
	struct ModelAnim *anim;

	if (modelInst == NULL)
	{
		return 0;
	}

	model = modelInst->model;
	if (model == NULL)
	{
		return 0;
	}

	if (LOD >= model->numHeaders)
	{
		return 0;
	}

	header = &model->headers[LOD];
	if (header == NULL)
	{
		return 0;
	}

	if (animIndex >= (int)header->numAnimations)
	{
		return 0;
	}

	if (header->ptrAnimations == NULL)
	{
		return 0;
	}

	anim = header->ptrAnimations[animIndex];
	if (anim == NULL)
	{
		return 0;
	}

	return (anim->numFrames & CS_ANIM_FRAME_COUNT_MASK);
}

int CS_Instance_SafeCheckAnimFrame(struct Instance *inst, int animIndex, int LOD, int desiredFrame)
{
	int numFrames;
	// Default return value
	int animFrame = desiredFrame;

	if (inst == NULL)
	{
		return animFrame;
	}

	if (desiredFrame <= 0)
	{
		return 0;
	}

	numFrames = CS_Instance_GetNumAnimFrames(inst, animIndex, LOD);

	if (numFrames > 0)
	{
		if (numFrames <= desiredFrame)
		{
			return numFrames - 1;
		}
		return desiredFrame;
	}
	return 0;
}

b32 CS_Instance_BoolPlaySound(struct CutsceneObj *cs, struct Instance *desiredInst)
{
	struct Instance **visInstSrc;
	struct Instance *visible;
	struct InstDrawPerPlayer *idpp;

	if ((desiredInst == NULL) || ((cs->flags & CS_FLAG_SOUND_ONSCREEN_ONLY) == 0))
	{
		return 1;
	}

	// pointer to array of visible instances
	visInstSrc = GAME_TRACKER->cameraDC[0].visInstSrc;

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Same native low-RAM guard as AH_WarpPad_ThTick:
	// a null camera list behaves like "desired instance is not visible."
	if (visInstSrc == NULL)
	{
		return 0;
	}
#endif

	visible = *visInstSrc;
	while (visible != NULL)
	{
		visInstSrc++;
		if (visible == desiredInst)
		{
			break;
		}
		visible = *visInstSrc;
	}

	if (visible == NULL)
	{
		return 0;
	}
	idpp = INST_GETIDPP(desiredInst);
	return (s32)(idpp[0].instFlags & DRAW_SUCCESSFUL) > 0;
}

void CS_Instance_InitMatrix(void)
{
	MATRIX scale;
	MATRIX mat;
	u32 i;
	int j;
	struct Ovr233InitMatrixTableEntry *table;
	if (CS_MATRIX_INITIALIZED)
		return;
	CS_MATRIX_INITIALIZED = 1;
	CTR_WriteU32AlignedLE(&scale.m[0][0], 0);
	CTR_WriteU32AlignedLE(&scale.m[0][2], 0);
	CTR_WriteU32AlignedLE(&scale.m[1][1], 0);
	CTR_WriteU32AlignedLE(&scale.m[2][0], 0);
	scale.m[2][2] = 0;
	for (i = 0; i < 4; i++)
	{
		table = &CS_MATRIX_TABLE[i];
		if (table->count != 0 && table->data != NULL)
		{
			for (j = 0; j < table->count; j++)
			{
				struct CsInitMatrixEntry *entry = &table->data[j];
				ConvertRotToMatrix(&mat, &entry->matrix.fields.rot);
				scale.m[0][0] = entry->matrix.fields.scale.x;
				scale.m[1][1] = entry->matrix.fields.scale.y;
				scale.m[2][2] = entry->matrix.fields.scale.z;
				MatrixRotate(&entry->matrix, &scale, &mat);
			}
		}
	}
}
