#include "VehCommon.h"
#include <race_field_limits.h>

enum
{
	VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET = 0x80,
	VEH_SPAWN_COLL_PROBE_TOP_Y_OFFSET = 0x100,
};

CTR_STATIC_ASSERT(RACE_FIELD_CONFIG_MAX_RACERS == 2 * len(((struct Level *)0)->DriverSpawn));

static void VehStartGrid_GetTransform(const struct Level *level, unsigned int index, SVec3 *pos, SVec3 *rot)
{
	const unsigned int count = len(level->DriverSpawn);
	const unsigned int source = index % count;
	*pos = level->DriverSpawn[source].pos;
	*rot = level->DriverSpawn[source].rot;
	if (index < count)
	{
		return;
	}

	// Repeat the two authored rows behind themselves, using twice the average
	// row separation. Include Y to carry the original slope into the extra rows.
	const int half = count / 2;
	int dx = 0, dy = 0, dz = 0;
	for (int i = 0; i < half; i++)
	{
		dx += level->DriverSpawn[i + half].pos.x - level->DriverSpawn[i].pos.x;
		dy += level->DriverSpawn[i + half].pos.y - level->DriverSpawn[i].pos.y;
		dz += level->DriverSpawn[i + half].pos.z - level->DriverSpawn[i].pos.z;
	}
	pos->x = (s16)(pos->x + 2 * dx / half);
	pos->y = (s16)(pos->y + 2 * dy / half);
	pos->z = (s16)(pos->z + 2 * dz / half);
}

static void VehSpawn_InitGroundProbe(const struct GameTracker *tracker, struct ScratchpadStruct *sps)
{
	sps->Union.QuadBlockColl.quadFlagsWanted = QUADBLOCK_FLAG_GROUND | QUADBLOCK_FLAG_COLLISION_SURFACE;
	sps->Union.QuadBlockColl.quadFlagsIgnored = 0;
	sps->Union.QuadBlockColl.searchFlags =
	    tracker->numPlyrCurrGame < 3 || sdata->highDetailSplitScreenLevel ? COLL_SEARCH_HIGH_LOD : 0;
	sps->ptr_mesh_info = tracker->level1->ptr_mesh_info;
}

static void VehSpawn_ProbeGround(const SVec3 *bottom, struct ScratchpadStruct *sps)
{
	SVec3 top = *bottom;
	top.y = (s16)CTR_MipsSubLo((u16)bottom->y, VEH_SPAWN_COLL_PROBE_TOP_Y_OFFSET);
	COLL_SearchBSP_CallbackQUADBLK(&top, bottom, sps, 0);
}
