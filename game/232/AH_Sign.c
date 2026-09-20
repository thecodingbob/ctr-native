#include <common.h>

struct AHSignScratch
{
	SVec3Slot probeTop;
	SVec3Slot probeBottom;
	SVec3Slot normal;
	struct ScratchpadStruct sps;
};

CTR_STATIC_ASSERT(offsetof(struct AHSignScratch, probeTop) == 0x00);
CTR_STATIC_ASSERT(offsetof(struct AHSignScratch, probeBottom) == 0x08);
CTR_STATIC_ASSERT(offsetof(struct AHSignScratch, normal) == 0x10);
CTR_STATIC_ASSERT(offsetof(struct AHSignScratch, sps) == 0x18);

void AH_Sign_LInB(struct Instance *inst)
{
	struct AHSignScratch *scratch = CTR_SCRATCHPAD_PTR(struct AHSignScratch, 0x108);
	s16 x, y, z;
	// NOTE(aalhendi): Keep retail's X-normal allocation while building the
	// collision probe. Native is free to allocate the same C value normally.
	register s32 normalX CTR_PSX_REGISTER("$11");

	normalX = inst->matrix.m[0][2] >> 6;
	scratch->normal.x = normalX;
	scratch->normal.y = inst->matrix.m[1][2] >> 6;
	scratch->normal.z = inst->matrix.m[2][2] >> 6;

	scratch->sps.Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND | QUADBLOCK_FLAG_COLLISION_SURFACE;
	scratch->sps.Union.QuadBlockColl.searchFlags = COLL_SEARCH_HIGH_LOD;
	scratch->sps.Union.QuadBlockColl.quadFlagsIgnored = 0;
	scratch->sps.ptr_mesh_info = GAME_TRACKER->level1->ptr_mesh_info;

	x = inst->matrix.t[0] + scratch->normal.x * 2;
	scratch->probeTop.x = x;
	y = inst->matrix.t[1] + scratch->normal.y * 2;
	scratch->probeTop.y = y;
	z = inst->matrix.t[2] + scratch->normal.z * 2;
	scratch->probeTop.z = z;
	scratch->probeBottom.x = x - scratch->normal.x * 4;
	scratch->probeBottom.y = y - scratch->normal.y * 4;
	scratch->probeBottom.z = z - scratch->normal.z * 4;

	COLL_SearchBSP_CallbackQUADBLK(SVec3Slot_AsVec3(&scratch->probeTop), SVec3Slot_AsVec3(&scratch->probeBottom), &scratch->sps, 0);
	if (scratch->sps.boolDidTouchQuadblock != 0)
	{
		scratch->normal.x = -scratch->normal.x;
		scratch->normal.y = -scratch->normal.y;
		scratch->normal.z = -scratch->normal.z;
	}
	inst->compressedNormalAndDriverIndex = ((u16)scratch->normal.x & 0xff) | (((u16)scratch->normal.y & 0xff) << 8) | (((u16)scratch->normal.z & 0xff) << 16);
}
