#ifndef RB_EFFECT_H
#define RB_EFFECT_H

#include <common.h>

// Advance a one-shot effect, releasing its owner's slot when the animation ends.
static inline void RB_Effect_UpdateSlot(struct Instance **slot)
{
	if (*slot == NULL)
	{
		return;
	}

	if ((*slot)->animFrame + 1 < INSTANCE_GetNumAnimFrames(*slot, 0))
	{
		(*slot)->animFrame++;
		return;
	}

	INSTANCE_Death(*slot);
	*slot = NULL;
}

#endif
