#include <common.h>

void RECTMENU_DrawOuterRect_LowLevel(RECT *p, s16 xOffset, u16 yOffset, Color color, s16 param_5, u_long *otMem)
{
	int iVar1;
	RECT r;

	r.x = p->x;
	iVar1 = (int)param_5;
	r.y = p->y;
	r.w = p->w;
	r.h = yOffset;
	RECTMENU_DrawOuterRect_Edge(&r, color, iVar1, otMem);

	r.y += (p->h - yOffset);
	RECTMENU_DrawOuterRect_Edge(&r, color, iVar1, otMem);

	r.y = p->y + yOffset;
	r.h = p->h - (s16)((int)((u32)yOffset << 0x10) >> 0xf);
	r.w = xOffset;
	RECTMENU_DrawOuterRect_Edge(&r, color, iVar1, otMem);

	r.x += (p->w - xOffset);
	RECTMENU_DrawOuterRect_Edge(&r, color, iVar1, otMem);
	return;
}
