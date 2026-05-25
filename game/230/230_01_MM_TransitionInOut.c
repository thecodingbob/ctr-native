#include <common.h>

// NOTE(aalhendi): ASM-verified against retail 230 0x800abaf0-0x800abcac.
u8 MM_TransitionInOut(struct TransitionMeta *meta, int framesPassed, int numFrames)
{
	u8 bool_Transitioning;
	int transitionIndex;
	s16 start;
	s16 framesLeft;

	bool_Transitioning = 1;
	transitionIndex = 0;

	// last member of array is null-terminated with 0xFFFF
	for (/**/; meta->headStart > -1; meta++, transitionIndex++)
	{
		start = meta->headStart;
		framesLeft = ((s16)framesPassed - start);

		if ((framesLeft == 4) && (transitionIndex == 0))
		{
			// Play "swoosh" sound for menu transition
			OtherFX_Play(0x65, 0);
		}

		if (framesLeft < 1)
		{
			bool_Transitioning = 0;
			meta->currX = 0;
			meta->currY = 0;
			continue;
		}

		// else if
		if (framesLeft < (s16)numFrames)
		{
			bool_Transitioning = 0;
			meta->currX = framesLeft * meta->distX / (s16)numFrames;
			meta->currY = framesLeft * meta->distY / (s16)numFrames;
			continue;
		}

		// else
		meta->currX = meta->distX;
		meta->currY = meta->distY;
	}
	return bool_Transitioning;
}
