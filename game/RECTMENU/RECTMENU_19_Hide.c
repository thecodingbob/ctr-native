#include <common.h>

void RECTMENU_Hide(struct RectMenu *m)
{
	m->state |= 0x1000;
}
