#include <common.h>

int Widescreen_GetFactor(void)
{
	switch (g_config.aspectRatio)
	{
		case 1:  return 750;  // 16:9  → 1000 * (4/3) / (16/9)  = 750
		case 2:  return 833;  // 16:10 → 1000 * (4/3) / (16/10) ≈ 833
		case 3:  return 563;  // 21:9 (64:27) → 1000 * (4/3) / (64/27) ≈ 563
		default: return 1000; // 4:3 (vanilla)
	}
}

int Widescreen_XShift(int width)
{
	const int factor = Widescreen_GetFactor();

	if (factor == 1000)
		return 0;

	return (width * (1000 - factor)) / 2000;
}

void Widescreen_CompressFT4(POLY_FT4 *p)
{
	int len = Widescreen_XShift(p->x1 - p->x0);
	p->x0 += len;
	p->x2 += len;
	p->x1 -= len;
	p->x3 -= len;
}

void Widescreen_CompressGT4(POLY_GT4 *p)
{
	int len = Widescreen_XShift(p->x1 - p->x0);
	p->x0 += len;
	p->x2 += len;
	p->x1 -= len;
	p->x3 -= len;
}

void Widescreen_CompressNative(PolyFT4 *p)
{
	int w = p->v[1].pos.x - p->v[0].pos.x;
	int len = Widescreen_XShift(w);
	p->v[0].pos.x += len;
	p->v[2].pos.x += len;
	p->v[1].pos.x -= len;
	p->v[3].pos.x -= len;
}
