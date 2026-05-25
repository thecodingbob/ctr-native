#include <common.h>

// byte budget 580/632
void UI_TrackerBG(struct Icon *targetIcon, s16 centerX, s16 centerY, struct PrimMem *primMem, u_long *ot, s16 angleX, s16 angleY, int color)
{
	s16 rightX;
	s16 bottomY;
	s16 offsY;
	s16 tmpX;
	s16 tmpY;
	POLY_FT4 *p;
	int offsX;
	int quadIndex;
	s16 topY;
	s16 leftX;

	// wumpaShineTheta (given to sine)
	sdata->wumpaShineTheta += 0x100;

	offsX = ((targetIcon->texLayout.u1 - targetIcon->texLayout.u0) * angleX) >> 0xc;
	offsY = ((targetIcon->texLayout.v2 - targetIcon->texLayout.v0) * angleY) >> 0xc;

	rightX = centerX + offsX;
	angleX >>= 0xc;
	leftX = rightX - angleX;

	bottomY = centerY + offsY;
	angleY >>= 0xc;
	topY = bottomY - angleY;

	int altX0 = (centerX + (offsX * 2)) - angleX;
	int altY0 = (centerY + (offsY * 2)) - angleY;

	// loop 4 times
	for (quadIndex = 0; quadIndex < 4; quadIndex++)
	{
		p = primMem->curr;
		primMem->curr = (p + 1);

		*(int *)&p->r0 = *(int *)&color;
		*(int *)&p->u0 = *(int *)&targetIcon->texLayout.u0;
		*(int *)&p->u1 = *(int *)&targetIcon->texLayout.u1;
		*(s16 *)&p->u2 = *(s16 *)&targetIcon->texLayout.u2;
		*(s16 *)&p->u3 = *(s16 *)&targetIcon->texLayout.u3;

		setPolyFT4(p);

		p->tpage = (p->tpage & 0xff9f);
		p->code |= 2;

		// compiler optimization will remove this,
		// if not using widescreen hacks
		int len = 0;

		// quadIndex(0)
		p->x0 = centerX + len;
		p->x1 = rightX;
		p->y0 = centerY;
		p->y2 = bottomY;

		switch (quadIndex)
		{
		case 1:
			p->x0 = altX0 - len;
			p->x1 = leftX;
			break;

		case 2:
			p->y0 = altY0;
			p->y2 = topY;
			break;

		case 3:
			p->x0 = altX0 - len;
			p->y0 = altY0;

			p->x1 = leftX;
			p->y2 = topY;
		}

		p->x2 = p->x0;
		p->y1 = p->y0;

		p->x3 = p->x1;
		p->y3 = p->y2;

		AddPrim(ot, p);
	}
	return;
}
