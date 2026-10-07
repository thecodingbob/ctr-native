#include <common.h>

void MainDrawCb_DrawSync()
{
	struct GameTracker *gGT;
	gGT = sdata->gGT;

	if (gGT->bool_DrawOTag_InProgress == 1)
	{
		gGT->bool_DrawOTag_InProgress = 0;
	}

	return;
}

// Keeps host input/audio/VRAM bridge work.
void MainDrawCb_Vsync()
{
	struct GameTracker *gGT;

	gGT = sdata->gGT;
	gGT->frameTimer_VsyncCallback++;
	if ((gGT->gameMode1 & PAUSE_ALL) == 0)
	{
		gGT->frameTimer_Confetti++;
	}

	sdata->vsyncTillFlip--;
	gGT->vSync_between_drawSync++;

	// NOTE(aalhend): accumulate root-counter 1 units between VSync callbacks; Timer_GetTime_Total converts them through the retail 0x147e divisor.
	sdata->rcntTotalUnits += GetRCnt(0xf2000001);
	ResetRCnt(0xf2000001);

	// NOTE(aalhendi): Retail calls HOWL unconditionally. Native skips only while
	// channel lists are mid-edit.
	if (sdata->criticalSectionCount == 0)
	{
		howl_PlayAudio_Update();
	}

	// NOTE(aalhendi): Native owns host input and writes PSX-shaped pad
	// snapshots before retail GAMEPAD_PollVsync consumes them.
	Platform_PollInput();

	GAMEPAD_PollVsync(sdata->gGamepads);

	return;
}
