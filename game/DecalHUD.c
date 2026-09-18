#include <common.h>
#include <widescreen.h>

enum
{
	DECAL_HUD_GPU_CODE_SEMI_TRANS = 0x02000000u,
	DECAL_HUD_GPU_CODE_POLY_GT4 = 0x3c000000u,
	DECAL_HUD_GPU_TAG_LENGTH_POLY_GT4 = 0x0c000000u,
	DECAL_HUD_TPAGE_TRANSPARENCY_MASK = 0xff9fffffu,
};


#if 0
void DecalHUD_DrawPolyFT4(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, u8 transparency, s16 scale)
{
	// NOTE(aalhendi): Keep the packed UV snapshots in their retail registers across blending.
	u32 uvTopRight;
	register u32 uvBottomLeft CTR_PSX_REGISTER("$10");
	register u32 uvTopLeft CTR_PSX_REGISTER("$6");
	s32 width, height;

	POLY_FT4 *p;

	if (!icon)
		return;
	uvTopRight = CTR_ReadU32LE(&icon->texLayout.u1);
	uvTopLeft = CTR_ReadU32LE(&icon->texLayout.u0);
	width = (s32)(((uvTopRight & 0xff) - (uvTopLeft & 0xff)) * scale) >> 12;
	height = (s32)((icon->texLayout.v2 - ((s32)uvTopLeft >> 8 & 0xff)) * scale) >> 12;
	// NOTE(aalhendi): Retail leaves X unmasked, so its upper bits can spill into packed Y.
	posY = (u32)posY << 16;
	uvBottomLeft = CTR_ReadU32LE(&icon->texLayout.u2);
	p = (POLY_FT4 *)primMem->cursor;
	if (transparency)
	{
		p->code = 0x2f;

/* upstream */
		CtrGpu_WritePackedUVWord(&p->u1, (uvTopRight & DECAL_HUD_TPAGE_TRANSPARENCY_MASK) | (((u32)transparency - 1) << 21));
/* fork */
	Widescreen_CompressFT4(p);

	primMem->cursor = p + 1;
}

void DecalHUD_DrawWeapon(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, u32 *ot, char transparency, s16 scale, char rot)
{
	if (!icon)
	{
		return;
	}

	POLY_FT4 *p = (POLY_FT4 *)primMem->cursor;
	addPolyFT4(ot, p);

	u32 width = icon->texLayout.u1 - icon->texLayout.u0;
	u32 height = icon->texLayout.v2 - icon->texLayout.v0;
	u32 rightX = posX + FP_Mult(width, scale);
	u32 bottomY = posY + FP_Mult(height, scale);
	u32 sidewaysX = posX + FP_Mult(height, scale);
	u32 sidewaysY = posY + FP_Mult(width, scale);

	// NOTE(aalhendi): Retail leaves X unmasked while packing XY, allowing its
	// upper bits to spill into Y for negative or overflowing coordinates.
	if (!(rot & 1))
	{
		if (rot == 0)
		{
			setXY4CompilerHack(p, posX, posY, rightX, posY, posX, bottomY, rightX, bottomY);
		}
		else
		{
			setXY4CompilerHack(p, rightX, bottomY, posX, bottomY, rightX, posY, posX, posY);
		}
/* end fork */
	}
	else
	{
		p->code = 0x2d;

		CtrGpu_WritePackedUVWord(&p->u1, uvTopRight);
		// NOTE(aalhendi): Keep the opaque UV store in this branch instead of merging both paths.
		CTR_PSX_OBSERVE_VALUE(uvTopRight);
	}

	CtrGpu_WritePackedUVWord(&p->u0, uvTopLeft);
	CtrGpu_WritePackedUV(&p->u2, uvBottomLeft);
	CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&icon->texLayout.u3));
	CtrGpu_WritePackedXY(&p->x0, posX | posY);
	CtrGpu_WritePackedXY(&p->x1, ((u32)posX + width) | posY);
	CtrGpu_WritePackedXY(&p->x2, posX | (posY + ((u32)height << 16)));
	CtrGpu_WritePackedXY(&p->x3, ((u32)posX + width) | (posY + ((u32)height << 16)));

/* upstream */
	p->tag = *ot | 0x09000000;
	*ot = CtrGpu_PrimToOTLink24(p);
/* fork */
	int len = Widescreen_XShift(p->x2 - p->x0);
	p->x0 += len;
	p->x1 += len;
	p->x2 -= len;
	p->x3 -= len;

/* end fork */
	primMem->cursor = p + 1;
}


void DecalHUD_DrawWeapon(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, u8 transparency, s16 scale, s16 rot)
{
	u32 uvTopRight;
	register u32 uvBottomLeft CTR_PSX_REGISTER("$11");
	u32 uvTopLeft;
	s32 width, height;
	u32 right, bottom, topLeft, topRight, bottomLeft, bottomRight;

	POLY_FT4 *p;

	if (!icon)
		return;
	uvTopRight = CTR_ReadU32LE(&icon->texLayout.u1);
	uvTopLeft = CTR_ReadU32LE(&icon->texLayout.u0);
	width = (s32)(((uvTopRight & 0xff) - (uvTopLeft & 0xff)) * scale) >> 12;
	height = (s32)((icon->texLayout.v2 - ((s32)uvTopLeft >> 8 & 0xff)) * scale) >> 12;

	posY = (u32)posY << 16;
	uvBottomLeft = CTR_ReadU32LE(&icon->texLayout.u2);
	p = (POLY_FT4 *)primMem->cursor;
	if (transparency)
	{
		p->code = 0x2f;

		CtrGpu_WritePackedUVWord(&p->u1, (uvTopRight & DECAL_HUD_TPAGE_TRANSPARENCY_MASK) | (((u32)transparency - 1) << 21));
	}
/* upstream and fork */

	Widescreen_CompressGT4(p);

	primMem->cursor = p + 1;
}


void DecalHUD_Arrow2D(struct Icon *icon, s16 posX, s16 posY, struct PrimMem *primMem, u32 *otMemPtr, u32 color1, u32 color2, u32 color3, u32 color4,
                      char transparency, int scale, u16 rot)
{
	u8 y2;
	u32 code;
	int bitshiftTopRightCorner;
	u32 topRightCornerAndPageXY;
	int bitshiftPosY;
	int iVar6;
	int iVar7;
	s16 sVar8;
	u32 bottomMargin;
	int iVar10;
	u32 topLeftCornerAndPaletteXY;
	int iVar12;
	int iVar13;

	POLY_GT4 *p;

	if (icon == 0)
	{
		return;
	}
	scale = (s16)scale;

	topRightCornerAndPageXY = CTR_ReadU32LE(&icon->texLayout.u1);
	topLeftCornerAndPaletteXY = CTR_ReadU32LE(&icon->texLayout.u0);
	y2 = icon->texLayout.v2;
	bottomMargin = CTR_ReadU32LE(&icon->texLayout.u2);

	p = (POLY_GT4 *)primMem->cursor;

	if (transparency == 0)
	{
		code = DECAL_HUD_GPU_CODE_POLY_GT4;
		CtrGpu_WritePackedUVWord(&p->u1, topRightCornerAndPageXY);
	}

/* end fork */
	else
	{
		p->code = 0x2d;

		CtrGpu_WritePackedUVWord(&p->u1, uvTopRight);
		CTR_PSX_OBSERVE_VALUE(uvTopRight);
	}

	CtrGpu_WritePackedUVWord(&p->u0, uvTopLeft);
	CtrGpu_WritePackedUV(&p->u2, uvBottomLeft);
	CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&icon->texLayout.u3));

	// Rotate the corner assignment in quarter turns, swapping width and height for odd turns.
	if (rot & 1)
	{
		right = (u32)posX + height;
		bottom = posY + ((u32)width << 16);
		topLeft = posX | posY;
		topRight = right | posY;
		bottomLeft = posX | bottom;
		bottomRight = right | bottom;
		if (rot == 1)
		{
			CtrGpu_WritePackedXY(&p->x1, topLeft);
			CtrGpu_WritePackedXY(&p->x3, topRight);
			CtrGpu_WritePackedXY(&p->x0, bottomLeft);
			CtrGpu_WritePackedXY(&p->x2, bottomRight);
		}
		else
		{
			CtrGpu_WritePackedXY(&p->x2, topLeft);
			CtrGpu_WritePackedXY(&p->x0, topRight);
			CtrGpu_WritePackedXY(&p->x3, bottomLeft);
			CtrGpu_WritePackedXY(&p->x1, bottomRight);
		}
	}
	else
	{
		right = (u32)posX + width;
		bottom = posY + ((u32)height << 16);
		topLeft = posX | posY;
		topRight = right | posY;
		bottomLeft = posX | bottom;
		bottomRight = right | bottom;
		if (rot == 0)
		{
			CtrGpu_WritePackedXY(&p->x0, topLeft);
			CtrGpu_WritePackedXY(&p->x1, topRight);
			CtrGpu_WritePackedXY(&p->x2, bottomLeft);
			CtrGpu_WritePackedXY(&p->x3, bottomRight);
		}
		else
		{
			CtrGpu_WritePackedXY(&p->x3, topLeft);
			CtrGpu_WritePackedXY(&p->x2, topRight);
			CtrGpu_WritePackedXY(&p->x1, bottomLeft);
			CtrGpu_WritePackedXY(&p->x0, bottomRight);
		}
	}
	p->tag = *ot | 0x09000000;
	*ot = CtrGpu_PrimToOTLink24(p);
	primMem->cursor = p + 1;
}


#endif

void DecalHUD_DrawPolyFT4(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, u8 transparency, s16 scale)
{
	POLY_FT4 *p;
	if (!icon)
		return;
	p = (POLY_FT4 *)primMem->cursor;
	addPolyFT4(ot, p);
	setXY4CompilerHack(p, posX, posY, posX + FP_Mult(icon->texLayout.u1 - icon->texLayout.u0, scale), posY,
	                  posX, posY + FP_Mult(icon->texLayout.v2 - icon->texLayout.v0, scale),
	                  posX + FP_Mult(icon->texLayout.u1 - icon->texLayout.u0, scale), posY + FP_Mult(icon->texLayout.v2 - icon->texLayout.v0, scale));
	setIconUV(p, icon);
	setShadeTex(p, true);
	if (transparency)
		setTransparency(p, transparency);
	Widescreen_CompressFT4(p);
	primMem->cursor = p + 1;
}

void DecalHUD_DrawWeapon(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, u8 transparency, s16 scale, s16 rot)
{
	POLY_FT4 *p;
	s32 width, height;
	if (!icon)
		return;
	p = (POLY_FT4 *)primMem->cursor;
	addPolyFT4(ot, p);
	width = FP_Mult(icon->texLayout.u1 - icon->texLayout.u0, scale);
	height = FP_Mult(icon->texLayout.v2 - icon->texLayout.v0, scale);
	setXY4CompilerHack(p, posX, posY, posX + width, posY, posX, posY + height, posX + width, posY + height);
	setIconUV(p, icon);
	setShadeTex(p, true);
	if (transparency)
		setTransparency(p, transparency);
	Widescreen_CompressFT4(p);
	primMem->cursor = p + 1;
}

void DecalHUD_DrawPolyGT4(struct Icon *icon, s32 posX, s32 inputY, struct PrimMem *primMem, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                          u8 transparency, s16 scale)
{
	u32 uv0, uv1, uv2;
	u32 posY = inputY;
	s32 width, height;
	POLY_GT4 *p;
	u32 tint, right, bottom, xy0, xy1, xy2, xy3;
	register u32 code CTR_PSX_REGISTER("$3");

	if (!icon)
		return;

	uv1 = CTR_ReadU32LE(&icon->texLayout.u1);
	uv0 = CTR_ReadU32LE(&icon->texLayout.u0);
	width = (s32)(((uv1 & 0xff) - (uv0 & 0xff)) * scale) >> 12;
	height = (s32)((icon->texLayout.v2 - ((s32)uv0 >> 8 & 0xff)) * scale) >> 12;
	posX &= 0xffff;
	posY <<= 16;
	uv2 = CTR_ReadU32LE(&icon->texLayout.u2);
	p = (POLY_GT4 *)primMem->cursor;
	if (transparency)
	{
		// NOTE(aalhendi): Retail reuses the blending register for the packet command.
		register u32 page CTR_PSX_REGISTER("$4");
		code = transparency;
		CTR_PSX_OBSERVE_VALUE(code);
		page = uv1 & DECAL_HUD_TPAGE_TRANSPARENCY_MASK;
		page |= (code - 1) << 21;
		tint = ColorCode_GetPacked(&color0);
		code = DECAL_HUD_GPU_CODE_POLY_GT4 | DECAL_HUD_GPU_CODE_SEMI_TRANS;
		CtrGpu_WritePackedUVWord(&p->u1, page);
	}
	else
	{
		tint = ColorCode_GetPacked(&color0);
		code = DECAL_HUD_GPU_CODE_POLY_GT4;
		CtrGpu_WritePackedUVWord(&p->u1, uv1);
		CTR_PSX_OBSERVE_VALUE(uv1);
	}
	CtrGpu_WriteColorCode(&p->r0, ((tint << 8) >> 8) | code);
	CtrGpu_WritePackedUVWord(&p->u0, uv0);
	CtrGpu_WritePackedUV(&p->u2, uv2);
	CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&icon->texLayout.u3));

	// NOTE(aalhendi): Only the initial X is masked; the scaled right edge can carry into Y.
	right = (u32)posX + width;
	bottom = posY + ((u32)height << 16);
	xy0 = posX | posY;
	xy1 = right | posY;
	xy2 = posX | bottom;
	xy3 = right | bottom;
	CtrGpu_WritePackedXY(&p->x0, xy0);
	CtrGpu_WritePackedXY(&p->x1, xy1);
	CtrGpu_WritePackedXY(&p->x2, xy2);
	CtrGpu_WritePackedXY(&p->x3, xy3);
	CtrGpu_WriteColorCode(&p->r1, ColorCode_GetPacked(&color1));
	CtrGpu_WriteColorCode(&p->r2, ColorCode_GetPacked(&color2));
	CtrGpu_WriteColorCode(&p->r3, ColorCode_GetPacked(&color3));
	Widescreen_CompressGT4(p);

	p->tag = *ot | DECAL_HUD_GPU_TAG_LENGTH_POLY_GT4;
	*ot = CtrGpu_PrimToOTLink24(p);
	primMem->cursor = p + 1;
}


void DecalHUD_Arrow2D(struct Icon *icon, s32 posX, s32 posY, struct PrimMem *primMem, u32 *ot, Color color0, Color color1, Color color2, Color color3,
                      u8 transparency, s16 scale, u16 rot)
{
	u32 uv1;
	u32 uv2;
	u32 uv0;
	u32 tint;
	register u32 code CTR_PSX_REGISTER("$3");
	s32 width;
	s32 height;
	s32 halfWidth, halfHeight;
	s32 cosine;
	register s32 sine CTR_PSX_REGISTER("$8");
	s32 sideX;
	s32 sideY;
	s32 leftCos, leftSin, rightCos, topCos, topSin;
	register s32 rightSin CTR_PSX_REGISTER("$24");
	s32 lowerLeftCos, lowerLeftSin, lowerRightCos, lowerRightSin, bottomCos, bottomSin;
	u32 xy0, xy1, xy2, xy3;
	POLY_GT4 *p;
	const struct TrigTable *table;
	if (!icon)
		return;

	uv1 = CTR_ReadU32LE(&icon->texLayout.u1);
	uv0 = CTR_ReadU32LE(&icon->texLayout.u0);
	width = (s32)(((uv1 & 0xff) - (uv0 & 0xff)) * scale) >> 12;
	height = (s32)((icon->texLayout.v2 - ((s32)uv0 >> 8 & 0xff)) * scale) >> 12;
	uv2 = CTR_ReadU32LE(&icon->texLayout.u2);
	p = (POLY_GT4 *)primMem->cursor;
	if (transparency)
	{
		register u32 page CTR_PSX_REGISTER("$4");
		code = transparency;
		CTR_PSX_OBSERVE_VALUE(code);
		page = uv1 & DECAL_HUD_TPAGE_TRANSPARENCY_MASK;
		page |= ((code - 1) << 21);
		tint = ColorCode_GetPacked(&color0);
		code = (DECAL_HUD_GPU_CODE_POLY_GT4 | DECAL_HUD_GPU_CODE_SEMI_TRANS);
		CtrGpu_WritePackedUVWord(&p->u1, page);
	}
	else
	{
		tint = ColorCode_GetPacked(&color0);
		code = DECAL_HUD_GPU_CODE_POLY_GT4;
		CtrGpu_WritePackedUVWord(&p->u1, uv1);
		CTR_PSX_OBSERVE_VALUE(uv1);
	}
	CtrGpu_WriteColorCode(&p->r0, ((tint << 8) >> 8) | code);

	// NOTE(aalhendi): Finish the command/color store before setting up the rotated vertices.
	CTR_PSX_OBSERVE_MEMORY(p->r0);
	posX &= 0xffff;
	table = data.trigApprox;
	CtrGpu_WritePackedUVWord(&p->u0, uv0);
	CtrGpu_WritePackedUV(&p->u2, uv2);
	halfHeight = height >> 1;
	CtrGpu_WritePackedUV(&p->u3, CTR_ReadU16LE(&icon->texLayout.u3));
	sine = CTR_ReadU32LE(&table[rot & 0x3ff]);
	// NOTE(aalhendi): Keep full width alive through the trig lookup, then halve it separately.
	CTR_PSX_OBSERVE_VALUE(width);
	halfWidth = width >> 1;
	// The table stores first-quadrant sine/cosine; other quadrants swap and negate them.
	if (rot & 0x400)
	{
		cosine = (s16)sine;
		sine >>= 16;
		if (!(rot & 0x800))
			cosine = -cosine;
		else
			sine = -sine;
	}
	else
	{
		cosine = sine >> 16;
		sine = (s16)sine;
		if (rot & 0x800)
		{
			cosine = -cosine;
			sine = -sine;
		}
	}
	// NOTE(aalhendi): The far edges include one extra pixel after scaling and halving.
	sideX = -halfWidth;
	leftCos = sideX * cosine;
	sideY = -halfHeight;
	topSin = sideY * sine;
	topCos = sideY * cosine;
	leftSin = sideX * sine;
	sideX = halfWidth + 1;
	rightCos = sideX * cosine;
	rightSin = sideX * sine;
	// NOTE(aalhendi): Retail recomputes horizontal products for the bottom pair of vertices.
	CTR_PSX_KEEP_VALUE_RELAXED(halfWidth);
	sideX = -halfWidth;
	lowerLeftCos = sideX * cosine;
	sideY = halfHeight + 1;
	bottomSin = sideY * sine;
	lowerLeftSin = sideX * sine;
	bottomCos = sideY * cosine;
	sideX = halfWidth + 1;
	lowerRightCos = sideX * cosine;
	lowerRightSin = sideX * sine;
	xy0 = ((posX + (leftCos >> 12) + (topSin >> 12)) & 0xffff) | ((u32)((u32)posY + (topCos >> 12) - (leftSin >> 12)) << 16);
	xy1 = ((posX + (rightCos >> 12) + (topSin >> 12)) & 0xffff) | ((u32)((u32)posY + (topCos >> 12) - (rightSin >> 12)) << 16);
	xy2 = ((posX + (lowerLeftCos >> 12) + (bottomSin >> 12)) & 0xffff) | ((u32)((u32)posY + (bottomCos >> 12) - (lowerLeftSin >> 12)) << 16);
	xy3 = ((posX + (lowerRightCos >> 12) + (bottomSin >> 12)) & 0xffff) | ((u32)((u32)posY + (bottomCos >> 12) - (lowerRightSin >> 12)) << 16);
	CtrGpu_WritePackedXY(&p->x0, xy0);
	CtrGpu_WritePackedXY(&p->x1, xy1);
	CtrGpu_WritePackedXY(&p->x2, xy2);
	CtrGpu_WritePackedXY(&p->x3, xy3);
	// NOTE(aalhendi): Retain raw products through the XY stores so their shifts use scratch registers.
	CTR_PSX_OBSERVE_VALUE(rightSin);
	{
		register s32 bottomRightSin CTR_PSX_REGISTER("$8") = lowerRightSin;
		CTR_PSX_OBSERVE_VALUE(bottomRightSin);
	}
	CtrGpu_WriteColorCode(&p->r1, ColorCode_GetPacked(&color1));
	CtrGpu_WriteColorCode(&p->r2, ColorCode_GetPacked(&color2));
	CtrGpu_WriteColorCode(&p->r3, ColorCode_GetPacked(&color3));
	p->tag = *ot | DECAL_HUD_GPU_TAG_LENGTH_POLY_GT4;
	*ot = CtrGpu_PrimToOTLink24(p);
	primMem->cursor = p + 1;
}
