#include <common.h>

void MM_Scrapbook_Init(void)
{
	D230.scrapbookState = 0;

	// change checkered flag
	RaceFlag_SetDrawOrder(1);

	// clear gamepad input (for menus)
	RECTMENU_ClearInput();
}
