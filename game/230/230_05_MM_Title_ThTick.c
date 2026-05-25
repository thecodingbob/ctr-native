#include <common.h>

void MM_Title_ThTick(struct Thread *title)
{
	s16 animFram;
	struct GameTracker *gGT;
	struct Instance *titleInst;
	s16 in_zero;
	s16 in_at;
	s16 framIndex;
	int i;
	int timer;
	struct Title *ptrTitle;
	int cops[6];
	s16 direction[3];
	s16 rot[3];

	// frame counters
	timer = D230.timerInTitle;

	// If you press Cross, Circle, Triangle, or Square
	if ((sdata->buttonTapPerPlayer[0] & 0x40070) != 0)
	{
		// clear gamepad input (for menus)
		RECTMENU_ClearInput();

		// set frame to 1000, skip the animation
		D230.timerInTitle = 1000;
	}

	// cap at 230
	if (timer > 230)
		timer = 230;

	// play 8 sounds, one on each frame
	for (i = 0; i < 8; i++)
	{
		if (D230.titleSounds[i].frameToPlay == timer)
		{
			// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac3e8-0x800ac400 for title queued SFX.
			OtherFX_Play(D230.titleSounds[i].soundID, 1);
		}
	}

	// copy pointer to title object
	ptrTitle = (struct Title *)title->object;

	// loop 6 times
	for (i = 0; i < 6; i++)
	{
		// current instance
		titleInst = ptrTitle->i[i];

		// make visible
		titleInst->flags &= 0xffffff7f;

		// the frame of title screen that each instance should start animation
		animFram = D230.titleInstances[i].frameIndex_startMoving;

		// set all instances to first animation
		titleInst->animIndex = 0;

		// set animation frame, based on what frame each instance should start
		titleInst->animFrame = (timer - animFram);

		// if instance has not started animation
		if (((timer - animFram) * 0x10000) < 0)
		{
			// skip the trophy instance
			if (i != 2)
			{
				// make invisible
				titleInst->flags |= 0x80;
			}

			// set animFrame to zero
			titleInst->animFrame = 0;
		}

		if ((D230.titleInstances[i].boolTrophy) != 0)
		{
			// if frame is anywhere in the two seconds
			// that the trophy is in the air
			if ((u32)(timer - 138) < 62)
			{
				// make invisible
				titleInst->flags |= 0x80;
			}

			// otherwise
			else if (200 <= timer)
			{
				// play frame index, based on total animation frame
				titleInst->animFrame = timer - 200;

				// set animation to 1
				titleInst->animIndex = 1;
			}

			// TODO(aalhendi): Retail runs a GTE/specular-light block here; port it before stamping this function.
		}
	}

	MM_Title_CameraMove(ptrTitle, timer);

	// increment frame counter
	timer = D230.timerInTitle + 1;

	if (245 < D230.timerInTitle)
	{
		// animation is over
		D230.menuMainMenu.state &= ~(DISABLE_INPUT_ALLOW_FUNCPTRS);
		D230.menuMainMenu.state |= EXECUTE_FUNCPTR;

		// dont increment index
		timer = D230.timerInTitle;
	}

	// write to index
	D230.timerInTitle = timer;
}
