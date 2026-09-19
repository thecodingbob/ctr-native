#include <common.h>

void CTR_Box_DrawWirePrims(s16 x1, s16 y1, s16 x2, s16 y2, s32 r, s32 g, s32 b, u32 *ot, struct PrimMem *primMem)
{
	LineF2 *p = primMem->cursor;
	const PrimCode primCode = {.kind.line = {.renderCode = RenderCode_Line}};
	if (p == NULL || p > (LineF2 *)primMem->guardEnd)
	{
		return;
	}
	primMem->cursor = p + 1;

	p->tag.bits.size = (sizeof(*p) - sizeof(p->tag)) / sizeof(u32);
	p->colorCode = MakeColor(r, g, b);
	p->colorCode.code = primCode;
	p->v[0].pos = MakePoint(x1, y1);
	p->v[1].pos = MakePoint(x2, y2);

	AddPrimitive(p, ot);
}

void CTR_Box_DrawWireBox(RECT *r, const Color *color, void *ot, struct PrimMem *primMem)
{
	LineF3 *p = primMem->cursor;
	if (p > (LineF3 *)primMem->guardEnd)
	{
		return;
	}
	primMem->cursor = p + 1;

	const PrimCode primCode = {.kind.line = {.renderCode = RenderCode_Line, .polyline = 1}};
	p->tag.bits.size = (sizeof(*p) - sizeof(p->tag)) / sizeof(u32);
	p->colorCode = *color;
	p->colorCode.code = primCode;

	s16 topX = r->x;
	s16 topY = r->y;
	s16 bottomX = r->x + r->w;
	s16 bottomY = r->y + r->h;
	p->v[0].pos.x = topX;
	p->v[0].pos.y = topY;
	p->v[1].pos.x = bottomX;
	p->v[1].pos.y = topY;
	p->v[2].pos.x = bottomX;
	p->v[2].pos.y = bottomY;
	p->end = 0x55555555;

	AddPrimitive(p, ot);
	p = primMem->cursor;
	if (p > (LineF3 *)primMem->guardEnd)
	{
		return;
	}
	primMem->cursor = p + 1;

	p->tag.bits.size = (sizeof(*p) - sizeof(p->tag)) / sizeof(u32);
	p->colorCode = *color;
	p->colorCode.code = primCode;
	p->v[0].pos.x = topX;
	p->v[0].pos.y = topY;
	p->v[1].pos.x = topX;
	p->v[1].pos.y = bottomY;
	p->v[2].pos.x = bottomX;
	p->v[2].pos.y = bottomY;
	p->end = 0x55555555;

	AddPrimitive(p, ot);
}

void CTR_Box_DrawClearBox(const RECT *r, const Color *color, s32 transparency, u32 *ot, struct PrimMem *primMem)
{
	typedef struct TPage_PolyF4
	{
		TPage t;
		PolyF4 p;
	} TPage_PolyF4;

	TPage_PolyF4 *p = primMem->cursor;
	if (p > (TPage_PolyF4 *)primMem->guardEnd)
	{
		return;
	}
	primMem->cursor = p + 1;
	p->t.tag.bits.size = (sizeof(*p) - sizeof(p->t.tag)) / sizeof(u32);

	p->t.texpage = (Texpage){.bits = {.code = 0xE1, .semiTransparency = transparency, .dither = 1, .y_VRAM_EXP = 1}};
	p->p.tag.self = 0;

	const PrimCode primCode = {.kind.poly = {.renderCode = RenderCode_Polygon, .quad = 1, .semiTransparency = 1}};
	Color primColor = *color;
	primColor.code = primCode;
	p->p.colorCode = primColor;

	s16 topX = r->x;
	s16 topY = r->y;
	s16 bottomX = r->x + r->w;
	s16 bottomY = r->y + r->h;
	p->p.v[0].pos.x = topX;
	p->p.v[0].pos.y = topY;
	p->p.v[1].pos.x = bottomX;
	p->p.v[1].pos.y = topY;
	p->p.v[2].pos.x = topX;
	p->p.v[2].pos.y = bottomY;
	p->p.v[3].pos.x = bottomX;
	p->p.v[3].pos.y = bottomY;

// NOTE(aalhendi): CTR_NATIVE keeps PsyCross display-area drawing enabled.
#ifdef CTR_NATIVE
	((TPage *)p)->texpage.bits.drawDisplayArea = 1;
#endif

	AddPrimitive(p, ot);
}

void CTR_Box_DrawSolidBox(RECT *r, const Color *color, u32 *ot, struct PrimMem *primMem)
{
	PolyF4 *p = primMem->cursor;
	if (p > (PolyF4 *)primMem->guardEnd)
	{
		return;
	}
	primMem->cursor = p + 1;
	p->tag.bits.size = (sizeof(*p) - sizeof(p->tag)) / sizeof(u32);

	const PrimCode primCode = {.kind.poly = {.renderCode = RenderCode_Polygon, .quad = 1}};
	p->colorCode = *color;
	p->colorCode.code = primCode;

	s16 topX = r->x;
	s16 topY = r->y;
	s16 bottomX = r->x + r->w;
	s16 bottomY = r->y + r->h;
	p->v[0].pos.x = topX;
	p->v[0].pos.y = topY;
	p->v[1].pos.x = bottomX;
	p->v[1].pos.y = topY;
	p->v[2].pos.x = topX;
	p->v[2].pos.y = bottomY;
	p->v[3].pos.x = bottomX;
	p->v[3].pos.y = bottomY;

	AddPrimitive(p, ot);
}
