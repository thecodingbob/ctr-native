#include <common.h>

void MM_JumpTo_Scrapbook(void)
{
	// go to scrapbook
	sdata->ptrActiveMenu = &D230.menuScrapbook;

	D230.menuScrapbook.state &= ~(ONLY_DRAW_TITLE);

	MM_Scrapbook_Init();
}
