#include <common.h>

struct Thread *RB_GetThread_ClosestTracker(struct Driver *d)
{
	s32 smallestDist = 0x3fffffff;
	struct Thread *currThread = GAME_TRACKER->threadBuckets[TRACKING].thread;
	struct Instance *driverInst = d->instSelf;
	struct Thread *closestTh = NULL;

	for (; currThread != NULL; currThread = currThread->siblingThread)
	{
		struct TrackerWeapon *tw = currThread->object;
		struct Instance *currInst = currThread->inst;

		if (tw->driverTarget == d)
		{
			u32 distX = (u32)driverInst->matrix.t[0] - currInst->matrix.t[0];
			u32 distZ = (u32)driverInst->matrix.t[2] - currInst->matrix.t[2];
			s32 distance = distX * distX + distZ * distZ;

			// NOTE(aalhendi): Retail compares the wrapped 32-bit squared distance.
			// A tie retains the earlier tracker in the bucket.
			if (distance < smallestDist)
			{
				smallestDist = distance;
				closestTh = currThread;
			}
		}
	}
	return closestTh;
}
