#include <common.h>

struct RBDefaultScratch
{
	SVec3Slot probeTop;
	SVec3Slot probeBottom;
	struct ScratchpadStruct sps;
};

CTR_STATIC_ASSERT(offsetof(struct RBDefaultScratch, probeTop) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct RBDefaultScratch, probeBottom) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct RBDefaultScratch, sps) == 0x10);


void RB_Default_LInB(struct Instance *inst)
{
	struct RBDefaultScratch *scratch = CTR_SCRATCHPAD_PTR(struct RBDefaultScratch, 0x108);
	s16 x;
	s16 y;
	s16 z;
	// high-LOD coll (8 triangles)
	scratch->sps.Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND | QUADBLOCK_FLAG_COLLISION_SURFACE;
	scratch->sps.Union.QuadBlockColl.quadFlagsIgnored = 0;
	scratch->sps.Union.QuadBlockColl.searchFlags = COLL_SEARCH_HIGH_LOD;
	scratch->sps.ptr_mesh_info = GAME_TRACKER->level1->ptr_mesh_info;

	// NOTE(aalhendi): Both ends of the vertical probe reuse each truncated
	// position component, preserving retail's single load per axis.
	x = inst->matrix.t[0];
	scratch->probeTop.x = x;
	y = inst->matrix.t[1];
	scratch->probeTop.y = y - 0x180;
	z = inst->matrix.t[2];
	scratch->probeTop.z = z;
	scratch->probeBottom.x = x;
	scratch->probeBottom.y = y + 0x80;
	scratch->probeBottom.z = z;

	COLL_SearchBSP_CallbackQUADBLK(SVec3Slot_AsVec3(&scratch->probeTop), SVec3Slot_AsVec3(&scratch->probeBottom), &scratch->sps, 0);

	RB_MakeInstanceReflective(&scratch->sps, inst);
}
