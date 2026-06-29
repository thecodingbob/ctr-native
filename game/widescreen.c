#include <common.h>

int Widescreen_GetFactor(void)
{
        // (4 * windowHeight * 1000) / (3 * windowWidth)
	if (!g_config.widescreen)
		return 1000; // 4:3
	return 750; // 16:9
}

int Widescreen_XShift(int width)
{
	if (!g_config.widescreen)
		return 0;

	const int factor = Widescreen_GetFactor();

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
