#include <common.h>

void MM_Battle_DrawIcon_Weapon(struct Icon *icon, u32 posX, int posY, struct PrimMem *primMem, u32 *ot, char transparency, s16 param_7, u16 param_8, u32 *color)
{
	if (!icon)
		return;

	POLY_FT4 *p = (POLY_FT4 *)primMem->curr;

	u32 code = 0x2e000000;
	u32 u0 = *(int *)&icon->texLayout.u0;
	u32 u1 = *(int *)&icon->texLayout.u1;
	u32 u2 = *(int *)&icon->texLayout.u2;

	int iVar1 = 0x2f;
	int iVar2 = 0x1f;

	posY *= 0x10000;

	*(u32 *)&p->r0 = (*color & 0xffffff) | code;
	*(u32 *)&p->u0 = u0;
	*(u32 *)&p->u1 = (u1 & 0xff9fffff);
	*(u32 *)&p->u2 = u2;
	*(s16 *)&p->u3 = *(s16 *)&icon->texLayout.u3;

	*(u32 *)&p->x1 = (posX) | (posY);
	*(u32 *)&p->x3 = (posX + iVar1) | (posY);
	*(u32 *)&p->x0 = (posX) | (posY + iVar2 * 0x10000);
	*(u32 *)&p->x2 = (posX + iVar1) | (posY + iVar2 * 0x10000);

	*(int *)p = *(int *)ot | 0x9000000;
	*(int *)ot = (int)p & 0xffffff;

	primMem->curr = p + 1;
}
