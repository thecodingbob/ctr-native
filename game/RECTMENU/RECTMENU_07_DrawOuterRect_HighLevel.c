#include <common.h>

void RECTMENU_DrawOuterRect_HighLevel(RECT *r, Color color, s16 param_3, u_long *otMem)
{
	RECTMENU_DrawOuterRect_LowLevel(r, 3, 2, color, param_3, otMem);
	return;
}
