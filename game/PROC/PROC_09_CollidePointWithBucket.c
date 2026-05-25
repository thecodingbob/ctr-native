#include <common.h>

void PROC_CollidePointWithBucket(struct Thread *th, s16 *vec3_pos)
{
	// only used with drivers colliding
	// with other drivers, disabled online
	while (th != 0)
	{
		PROC_CollidePointWithSelf(th, (struct BucketSearchParams *)vec3_pos);

		// next
		th = th->siblingThread;
	}
}
