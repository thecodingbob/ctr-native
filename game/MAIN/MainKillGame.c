#include <common.h>

void MainKillGame_StopCTR(void)
{
	EnterCriticalSection();
	DrawSyncCallback((void (*)(void))sdata->MainDrawCb_DrawSyncPtr);
	ExitCriticalSection();
	StopCallback();
	PadStopCom();
	ResetGraph(3);
	VSyncCallback(0);

	Timer_Destroy();
}

void MainKillGame_LaunchSpyro2(void)
{
	CTR_ErrorScreen(0, 0, 0);

	Music_Stop();

	// clear backup, destroy music, destroy all fx
	howl_StopAudio(1, 1, 1);

	Bank_DestroyAll();

	howl_Disable();

	VSync(0x1e);

	MainKillGame_StopCTR();

	// NOTE(aalhendi): Native cannot chain-load the Spyro executable.

	while (1)
	{
	}

}
