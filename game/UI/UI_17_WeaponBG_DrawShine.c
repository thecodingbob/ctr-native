#include <common.h>

void UI_WeaponBG_DrawShine(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, u_long *ot, char param_6, s16 param_7, s16 param_8, int param9)
{
	s16 sVar1;
	s16 sVar2;
	s16 sVar3;
	s16 sVar4;
	s16 sVar5;
	s16 sVar6;
	POLY_GT4 *p;
	int i;
	s16 sVar11;
	s16 sVar12;

	u32 *wumpaShine = (u32 *)&sdata->wumpaShineColor1[0][0];

	if (param_6 == 3)
	{
		wumpaShine = (u32 *)&sdata->wumpaShineColor2[0][0];
	}

	sVar3 = (s16)(((icon->texLayout.u1 - icon->texLayout.u0) * (int)param_7) >> 0xc);
	sVar1 = posX + sVar3;
	param_7 = param_7 >> 0xc;
	sVar12 = sVar1 - param_7;

	sVar4 = (s16)(((icon->texLayout.v2 - icon->texLayout.v0) * (int)param_8) >> 0xc);
	sVar2 = posY + sVar4;
	param_8 = param_8 >> 0xc;
	sVar11 = sVar2 - param_8;

	// loop 4 times
	for (i = 0; i < 4; i++)
	{
		p = primMem->curr;
		*(int *)&p->u0 = *(int *)&icon->texLayout.u0;
		*(int *)&p->u1 = *(int *)&icon->texLayout.u1;
		*(int *)&p->u2 = *(int *)&icon->texLayout.u2;
		*(s16 *)&p->u3 = *(s16 *)&icon->texLayout.u3;

		switch (i)
		{
		// top left
		case 0:
			// xy0
			p->x0 = posX;
			p->y0 = posY;

			p->x1 = sVar1;
			p->y1 = posY;
			p->x2 = posX;
			p->y2 = sVar2;
			p->x3 = sVar1;
			p->y3 = sVar2;
			break;

		// top right
		case 1:
			// xy0
			sVar5 = (posX + sVar3 * 2) - param_7;
			p->x0 = sVar5;
			p->y0 = posY;

			p->x1 = sVar12;
			p->y1 = posY;
			p->x2 = sVar5;
			p->y2 = sVar2;
			p->x3 = sVar12;
			p->y3 = sVar2;

			break;

		case 2:
			// xy0
			sVar5 = (posY + sVar4 * 2) - param_8;
			p->x0 = posX;
			p->y0 = sVar5;

			p->x1 = sVar1;
			p->y1 = sVar5;
			p->x2 = posX;
			p->y2 = sVar11;
			p->x3 = sVar1;
			p->y3 = sVar11;

			break;

		case 3:
			// xy0
			sVar5 = (posX + sVar3 * 2) - param_7;
			sVar6 = (posY + sVar4 * 2) - param_8;
			p->x0 = sVar5;
			p->y0 = sVar6;

			p->x1 = sVar12;
			p->y1 = sVar6;
			p->x2 = sVar5;
			p->y2 = sVar11;
			p->x3 = sVar12;
			p->y3 = sVar11;

			break;
		}

		// color RGB
		*(u32 *)&p->r0 = wumpaShine[2];
		*(u32 *)&p->r1 = wumpaShine[1];
		*(u32 *)&p->r2 = wumpaShine[1];
		*(u32 *)&p->r3 = wumpaShine[0];

		setPolyGT4(p);

// always true
#if 0
        if (param_6 != 0)
#endif
		{
			p->tpage = (p->tpage & ~(0x60)) | (((u16)param_6 - 1) * 0x20);
			p->code |= 2;
		}

		AddPrim(ot, p);

		// increment primMem
		primMem->curr = p + 1;
	}
}
