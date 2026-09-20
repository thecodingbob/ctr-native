#include <common.h>

void RB_MakeInstanceReflective(struct ScratchpadStruct *sps, struct Instance *inst)
{
	u16 quadFlags;
	u16 normalX;
	u16 normalY;
	u16 normalZ;
	struct GameTracker *gGT;

	if ((sps->boolDidTouchQuadblock != 0) && (sps->boolDidTouchHitbox == 0))
	{
		normalX = sps->hit.plane.normal.x;
		normalY = sps->hit.plane.normal.y;
		normalZ = sps->hit.plane.normal.z;
		// NOTE(aalhendi): Keep the flat packing expression here; nested inline
		// helpers change the old compiler's component-load order.
		inst->compressedNormalAndDriverIndex = ((normalX >> INST_COMPRESSED_NORMAL_SHIFT) & INST_COMPRESSED_NORMAL_MASK) |
		                                       (((normalY >> INST_COMPRESSED_NORMAL_SHIFT) & INST_COMPRESSED_NORMAL_MASK) << INST_COMPRESSED_NORMAL_Y_SHIFT) |
		                                       (((normalZ >> INST_COMPRESSED_NORMAL_SHIFT) & INST_COMPRESSED_NORMAL_MASK) << INST_COMPRESSED_NORMAL_Z_SHIFT);
		gGT = GAME_TRACKER;

		if (1 < gGT->numPlyrCurrGame)
		{
			return;
		}

		quadFlags = sps->hit.ptrQuadblock->quadFlags;

		if ((quadFlags & QUADBLOCK_FLAG_COLLISION_SURFACE) == 0)
		{
			if ((quadFlags & QUADBLOCK_FLAG_REFLECT_SPLIT_LINE_1) != 0)
			{
				inst->flags |= REFLECTIVE;
				inst->vertSplit = gGT->level1->splitLines[1];
				return;
			}

			if ((quadFlags & QUADBLOCK_FLAG_REFLECT_SPLIT_LINE_0) != 0)
			{
				inst->flags |= REFLECTIVE;
				inst->vertSplit = gGT->level1->splitLines[0];
				return;
			}
		}
	}
	else
	{
		inst->compressedNormalAndDriverIndex = INST_CompressNormalVector(0, FP_ONE, 0);
	}

	inst->flags &= ~REFLECTIVE;
}
