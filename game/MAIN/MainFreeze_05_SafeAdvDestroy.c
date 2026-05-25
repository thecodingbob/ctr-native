#include <common.h>

void MainFreeze_SafeAdvDestroy(void)
{
// byte budget
#if 0
	// If you're in Adventure Arena
	if((sdata->gGT->gameMode1 & ADVENTURE_ARENA) == 0) return;
#endif

	// check if Adv Hub is loaded
	if (LOAD_IsOpen_AdvHub() == 0)
		return;

	AH_Pause_Destroy();
	return;
}
