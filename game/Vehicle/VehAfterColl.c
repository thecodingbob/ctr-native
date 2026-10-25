#include <common.h>

enum
{
	VEH_AFTER_COLL_SCRUB_COUNT = len(data.MetaDataScrub),
	VEH_AFTER_COLL_TERRAIN_COUNT = len(data.MetaDataTerrain),
};

CTR_STATIC_ASSERT(VEH_AFTER_COLL_TERRAIN_COUNT == TERRAIN_SLOWDIRT + 1);

struct Scrub *VehAfterColl_GetSurface(u32 scrubId)
{
	if (scrubId >= VEH_AFTER_COLL_SCRUB_COUNT)
	{
		scrubId = 0;
	}

	return &data.MetaDataScrub[scrubId];
}

struct Terrain *VehAfterColl_GetTerrain(u32 terrainType)
{
	// if terrain is valid, max 20
	if (terrainType >= VEH_AFTER_COLL_TERRAIN_COUNT)
	{
		terrainType = 0;
	}

	return &data.MetaDataTerrain[terrainType];
}
