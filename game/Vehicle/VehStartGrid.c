#include "VehCommon.h"
#include <race_field.h>

enum
{
	VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET = 0x80,
	VEH_SPAWN_COLL_PROBE_TOP_Y_OFFSET = 0x100,
};

// The LEV only stores the first eight transforms. Repeat that formation once
// behind itself, translating by twice the separation of its front/rear halves.
// This produces candidates, not a guarantee that the track extends beneath them.
CTR_STATIC_ASSERT(RACE_FIELD_CONFIG_MAX_RACERS == 2 * len(((struct Level *)0)->DriverSpawn));

bool VehStartGrid_GetTransform(const struct Level *level, unsigned int index, SVec3 *pos, SVec3 *rot)
{
	const unsigned int count = len(level->DriverSpawn);
	if (index >= RACE_FIELD_CONFIG_MAX_RACERS)
	{
		return false;
	}

	unsigned int source = index % count;
	SVec3 candidate = level->DriverSpawn[source].pos;
	if (index >= count)
	{
		const unsigned int half = count / 2;
		int dx = 0, dy = 0, dz = 0;
		for (unsigned int i = 0; i < half; i++)
		{
			dx += level->DriverSpawn[i + half].pos.x - level->DriverSpawn[i].pos.x;
			dy += level->DriverSpawn[i + half].pos.y - level->DriverSpawn[i].pos.y;
			dz += level->DriverSpawn[i + half].pos.z - level->DriverSpawn[i].pos.z;
		}
		dx = 2 * dx / (int)half;
		dy = 2 * dy / (int)half;
		dz = 2 * dz / (int)half;
		int x = candidate.x + dx;
		int y = candidate.y + dy;
		int z = candidate.z + dz;
		if ((dx == 0 && dz == 0) || x < INT16_MIN || x > INT16_MAX || z < INT16_MIN || z > INT16_MAX ||
		    y < INT16_MIN + VEH_SPAWN_COLL_PROBE_TOP_Y_OFFSET - VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET ||
		    y > INT16_MAX - VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET)
		{
			return false;
		}
		for (unsigned int i = 0; i < count; i++)
		{
			if (level->DriverSpawn[i].pos.x == x && level->DriverSpawn[i].pos.z == z)
			{
				return false;
			}
		}
		candidate.x = (s16)x;
		candidate.y = (s16)y;
		candidate.z = (s16)z;
	}
	*pos = candidate;
	*rot = level->DriverSpawn[source].rot;
	return true;
}

// Shared with VehBirth.c in the vehicle unity build.
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

static bool VehStartGrid_IsSafeGround(const struct ScratchpadStruct *sps)
{
	return sps->boolDidTouchQuadblock && sps->hit.ptrQuadblock != NULL && sps->hit.plane.normal.y > 0 &&
	       (sps->hit.ptrQuadblock->quadFlags & (QUADBLOCK_FLAG_KILL_PLANE | QUADBLOCK_FLAG_NO_COLLISION_RESPONSE)) == 0;
}

static bool VehStartGrid_HasFootprint(const SVec3 *bottom, const SVec3 *center, struct ScratchpadStruct *sps)
{
	static const int offsets[][2] = {
	    {THREAD_DRIVER_HIT_RADIUS, 0}, {-THREAD_DRIVER_HIT_RADIUS, 0},
	    {0, THREAD_DRIVER_HIT_RADIUS}, {0, -THREAD_DRIVER_HIT_RADIUS},
	};
	for (unsigned int i = 0; i < len(offsets); i++)
	{
		int x = bottom->x + offsets[i][0];
		int z = bottom->z + offsets[i][1];
		if (x < INT16_MIN || x > INT16_MAX || z < INT16_MIN || z > INT16_MAX)
		{
			return false;
		}
		SVec3 edge = {.x = (s16)x, .y = bottom->y, .z = (s16)z};
		VehSpawn_ProbeGround(&edge, sps);
		if (!VehStartGrid_IsSafeGround(sps) || abs(sps->Union.QuadBlockColl.hitPos.y - center->y) > THREAD_DRIVER_HIT_RADIUS)
		{
			return false;
		}
	}
	return true;
}

// Returns a contiguous prefix of slot indices. Call after collision is ready
// but before assigning grid order: rebuild that order for the returned count.
// The LEV's own slots (and their retail spawn behavior) remain trusted.
int VehStartGrid_ValidateSize(const struct GameTracker *tracker, int requested)
{
	const int retail = len(((struct Level *)0)->DriverSpawn);
	if (requested <= retail)
	{
		return requested;
	}
	if (tracker->level1 == NULL || tracker->level1->ptr_mesh_info == NULL)
	{
		return retail;
	}
	if (requested > RACE_FIELD_CONFIG_MAX_RACERS)
	{
		requested = RACE_FIELD_CONFIG_MAX_RACERS;
	}

	struct ScratchpadStruct *sps = CTR_SCRATCHPAD_PTR(struct ScratchpadStruct, 0x108);
	VehSpawn_InitGroundProbe(tracker, sps);
	SVec3 centers[RACE_FIELD_CONFIG_MAX_RACERS];
	const int diameter = 2 * THREAD_DRIVER_HIT_RADIUS;
	for (int i = 0; i < requested; i++)
	{
		SVec3 bottom, rot;
		if (!VehStartGrid_GetTransform(tracker->level1, i, &bottom, &rot))
		{
			return i;
		}
		bottom.y = (s16)CTR_MipsAddLo(bottom.y, VEH_SPAWN_DRIVER_BOTTOM_Y_OFFSET);
		VehSpawn_ProbeGround(&bottom, sps);
		centers[i] = sps->Union.QuadBlockColl.hitPos;
		if (i < retail)
		{
			continue;
		}
		if (!VehStartGrid_IsSafeGround(sps) || !VehStartGrid_HasFootprint(&bottom, &centers[i], sps))
		{
			return i;
		}
		for (int j = 0; j < i; j++)
		{
			int dx = centers[i].x - centers[j].x;
			int dz = centers[i].z - centers[j].z;
			if (abs(dx) < diameter && abs(dz) < diameter && dx * dx + dz * dz < diameter * diameter)
			{
				return i;
			}
		}
	}
	return requested;
}
