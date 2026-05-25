#include <common.h>

void RECTMENU_Show(struct RectMenu *m)
{
	sdata->ptrActiveMenu = m;

	m->state &= ~NEEDS_TO_CLOSE;

	RECTMENU_ClearInput();
}
