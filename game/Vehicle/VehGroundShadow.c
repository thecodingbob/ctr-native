#include "VehCommon.h"

enum
{
	VEH_GROUND_SHADOW_MAX_DRIVERS = 8,
	VEH_GROUND_SHADOW_NUM_QUADS = 4,

	VEH_GROUND_SHADOW_STATE_CULLED = -1,
	VEH_GROUND_SHADOW_STATE_PENDING = 0,
	VEH_GROUND_SHADOW_STATE_READY = 1,

	VEH_GROUND_SHADOW_PRIM_GUARD_WORDS = 0x140,
	VEH_GROUND_SHADOW_OT_MAX = 0x3ff,
	VEH_GROUND_SHADOW_GPU_TAG_POLY_FT4 = 0x09000000u,

	VEH_GROUND_SHADOW_COLOR_PREFIX = 0x2e000000u,
	VEH_GROUND_SHADOW_LARGE_COLOR = 0x2e1f1f1f,
	VEH_GROUND_SHADOW_ICON_RIGHT = 1,
};

typedef u32 VehGroundShadowWord CTR_MAY_ALIAS;
typedef u16 VehGroundShadowHalfword CTR_MAY_ALIAS;
typedef s16 VehGroundShadowSignedHalfword CTR_MAY_ALIAS;

#define VEH_GROUND_SHADOW_WORD(base, offset)        (*(VehGroundShadowWord *)((u8 *)(base) + (offset)))
#define VEH_GROUND_SHADOW_HALF(base, offset)        (*(VehGroundShadowHalfword *)((u8 *)(base) + (offset)))
#define VEH_GROUND_SHADOW_SIGNED_HALF(base, offset) (*(VehGroundShadowSignedHalfword *)((u8 *)(base) + (offset)))
#define VEH_GROUND_SHADOW_BYTE(base, offset)        (*(u8 *)((u8 *)(base) + (offset)))
#define VEH_GROUND_SHADOW_SIGNED_BYTE(base, offset) (*(s8 *)((u8 *)(base) + (offset)))

#ifdef CTR_NATIVE
#define VehGroundShadow_LoadTransMatrix(vector)          gte_SetTransVector(vector)
#define VehGroundShadow_LoadRotMatrix(matrix)            VehGroundShadow_LoadGteRotMatrix(matrix)
#define VehGroundShadow_SetLocalVector(vector, scratch)  ((vector) = (scratch) + 0x48)
#define VehGroundShadow_SetGeomOffset(x, y)              gte_SetGeomOffset(x, y)
#define VehGroundShadow_AddPointer(result, base, offset) ((result) = (base) + (offset))
#define VehGroundShadow_SetCounterZero(counter)          ((counter) = 0)
#define VehGroundShadow_SetFirstTexture(texture)         ((texture) = (size_t)CTR_SCRATCHPAD_PTR(struct TextureLayout, 0x224))
#define VehGroundShadow_SetIconIndex(index)              ((index) = NULL)
#else
// NOTE(aalhendi): These matching-only loads intentionally expose only their
// base register; broader memory constraints perturb retail's instruction order.
#define VehGroundShadow_LoadTransMatrix(vector) \
	__asm__ volatile("lw $12,0(%0)\n\tlw $13,4(%0)\n\tlw $14,8(%0)\n\tctc2 $12,$5\n\tctc2 $13,$6\n\tctc2 $14,$7" : : "r"(vector) : "$12", "$13", "$14")
#define VehGroundShadow_LoadRotMatrix(matrix)                                                                                                               \
	__asm__ volatile("lw $12,0(%0)\n\tlw $13,4(%0)\n\tctc2 $12,$0\n\tctc2 $13,$1\n\tlw $12,8(%0)\n\tlw $13,12(%0)\n\tlw $14,16(%0)\n\tctc2 $12,$2\n\tctc2 " \
	                 "$13,$3\n\tctc2 $14,$4"                                                                                                                \
	                 :                                                                                                                                      \
	                 : "r"(matrix)                                                                                                                          \
	                 : "$12", "$13", "$14")
#define VehGroundShadow_SetLocalVector(vector, scratch) __asm__("addiu %0,%1,72" : "=r"(vector) : "r"(scratch))
#define VehGroundShadow_SetGeomOffset(x, y) __asm__ volatile("sll $12,%0,16\n\tsll $13,%1,16\n\tctc2 $12,$24\n\tctc2 $13,$25" : : "r"(x), "r"(y) : "$12", "$13")
#define VehGroundShadow_AddPointer(result, base, offset) __asm__("addiu %0,%1,%2" : "=r"(result) : "r"(base), "I"(offset))
#define VehGroundShadow_SetCounterZero(counter)          __asm__ volatile("move %0,$0" : "=r"(counter))
#define VehGroundShadow_SetFirstTexture(texture)         __asm__("lui %0,0x1f80\n\tori %0,%0,0x224" : "=r"(texture))
#define VehGroundShadow_SetIconIndex(index)              __asm__("move %0,$0" : "=r"(index))
#endif

/// @brief Copies texture layout data from icon to arbitrary mem address. Particularly used to copy kart shadow textures to scratchpad.
/// @param dst - destination texture layout
/// @param iconIndex - icon index to take data from
b32 VehGroundShadow_Subset1(struct TextureLayout *pDst, int iconIndex)
{
	// get pointer to icon
	struct Icon *pIcon = GAME_TRACKER->ptrIcons[iconIndex];
	register VehGroundShadowWord textureWord CTR_PSX_REGISTER("$2");

	// validate icon pointer
	if (!pIcon)
	{
		return false;
	}

	((VehGroundShadowWord *)pDst)[0] = ((VehGroundShadowWord *)&pIcon->texLayout)[0];
	textureWord = ((VehGroundShadowWord *)&pIcon->texLayout)[1];
	textureWord &= 0xff9fffff;
	textureWord |= 0x00400000;
	((VehGroundShadowWord *)pDst)[1] = textureWord;
	((VehGroundShadowHalfword *)pDst)[4] = ((VehGroundShadowHalfword *)&pIcon->texLayout)[4];
	((VehGroundShadowHalfword *)pDst)[5] = ((VehGroundShadowHalfword *)&pIcon->texLayout)[5];

	return true;
}

static inline u32 VehGroundShadow_ReadWord(const void *base, u32 offset)
{
	u32 value;
	memcpy(&value, (const char *)base + offset, sizeof(value));
	return value;
}

static inline void VehGroundShadow_LoadGteRotMatrix(const MATRIX *m)
{
	CTC2(VehGroundShadow_ReadWord(m, 0x00), 0);
	CTC2(VehGroundShadow_ReadWord(m, 0x04), 1);
	CTC2(VehGroundShadow_ReadWord(m, 0x08), 2);
	CTC2(VehGroundShadow_ReadWord(m, 0x0c), 3);
	CTC2(VehGroundShadow_ReadWord(m, 0x10), 4);
}

void VehGroundShadow_Main(void)
{
	register u8 *scratch CTR_PSX_REGISTER("$19");
	u32 *prim;
	register s32 playerIndex CTR_PSX_REGISTER("$23");
	register u8 *entryBase CTR_PSX_REGISTER("$21");
	register u8 *pushBufferCursor CTR_PSX_REGISTER("$22");
	register u32 colorOrDistance CTR_PSX_REGISTER("$20");
	register u8 *entryCursor CTR_PSX_REGISTER("$18");
	register s32 height CTR_PSX_REGISTER("$17");
	register MATRIX *axisMatrix CTR_PSX_REGISTER("$16");
	register size_t x CTR_PSX_REGISTER("$8");
	register size_t y CTR_PSX_REGISTER("$6");
	register size_t z CTR_PSX_REGISTER("$4");
	register s32 culledState CTR_PSX_REGISTER("$11");
	register s32 readyState CTR_PSX_REGISTER("$15");
	register u8 *localVector CTR_PSX_REGISTER("$24");
	register size_t v0Value CTR_PSX_REGISTER("$2");
	register size_t v1Value CTR_PSX_REGISTER("$3");
	register size_t gteWord0 CTR_PSX_REGISTER("$12");
	register size_t gteWord1 CTR_PSX_REGISTER("$13");
	register size_t gteWord2 CTR_PSX_REGISTER("$14");
	struct Driver *driver;
	register u8 *localOutput CTR_PSX_REGISTER("$5");
	s16 *axisVector;
	s32 axisAngle;
	u32 *texture;
	u32 *ot;
	register s32 entryPlayerIndex CTR_PSX_REGISTER("$7");
	s32 isSmallScreen;
	s32 quadIndex;
	s32 depthIndex;
	register s32 localZ0 CTR_PSX_REGISTER("$10");
	register s32 localZ1 CTR_PSX_REGISTER("$9");
	s32 packetAddress;
	s32 geomX;
	s32 geomY;
	u32 retailCallState[3];

	VehGroundShadow_SetFirstTexture(z);
	VehGroundShadow_SetIconIndex(localOutput);
	scratch = CTR_SCRATCHPAD_PTR(u8, 0);
	if (!VehGroundShadow_Subset1((struct TextureLayout *)z, (int)(size_t)localOutput))
		return;
	CTR_PSX_MEMORY_BARRIER();
	if (!VehGroundShadow_Subset1((struct TextureLayout *)(scratch + 0x230), VEH_GROUND_SHADOW_ICON_RIGHT))
		return;

	{
		struct GameTracker *tracker = GAME_TRACKER;
		struct DB *backBuffer = tracker->backBuffer;

		prim = (u32 *)backBuffer->primMem.cursor;
		CTR_PSX_KEEP_VALUE(prim);
		if (prim + VEH_GROUND_SHADOW_PRIM_GUARD_WORDS >= (u32 *)backBuffer->primMem.guardEnd)
			return;
	}

	VEH_GROUND_SHADOW_WORD(scratch, 0x90) = 0;
	VEH_GROUND_SHADOW_WORD(scratch, 0x94) = 0;
	VEH_GROUND_SHADOW_WORD(scratch, 0x98) = 0;
	VehGroundShadow_LoadTransMatrix((VECTOR *)(scratch + 0x90));

	playerIndex = 0;
	entryBase = scratch + 0xa4;
	y = (size_t)(scratch + 0xb6);
	do
	{
		struct GameTracker *tracker = GAME_TRACKER;

		driver = tracker->drivers[playerIndex];
		if (driver != NULL)
		{
			*(struct Driver **)((u8 *)y + 2) = driver;
			x = (size_t)driver->instSelf;
			*(struct Instance **)((u8 *)y + 6) = (struct Instance *)x;
			VEH_GROUND_SHADOW_HALF((u8 *)y, 0x14) = (u16)((struct Instance *)x)->flags;
			v0Value = tracker->numPlyrCurrGame;
			entryPlayerIndex = (s32)v0Value - 1;
			if (entryPlayerIndex >= 0)
			{
				v0Value = entryPlayerIndex << 4;
				v0Value += entryPlayerIndex;
				v0Value <<= 3;
				z = v0Value + x;
				do
				{
					v1Value = VEH_GROUND_SHADOW_BYTE((u8 *)z, 0xb8);
					z -= sizeof(struct InstDrawPerPlayer);
					v0Value = (size_t)entryBase + entryPlayerIndex;
					entryPlayerIndex--;
					VEH_GROUND_SHADOW_BYTE((u8 *)v0Value, 0x1c) = (u8)v1Value;
				} while (entryPlayerIndex >= 0);
			}
			VEH_GROUND_SHADOW_HALF((u8 *)y, 0x0e) = (u16)CTR_MipsSra(driver->posCurr.x, 8);
			VEH_GROUND_SHADOW_HALF((u8 *)y, 0x10) = (u16)CTR_MipsAddLo(CTR_MipsSra(driver->quadBlockHeight, 8), 3);
			VEH_GROUND_SHADOW_HALF((u8 *)y, 0x12) = (u16)CTR_MipsSra(driver->posCurr.z, 8);
			if ((VEH_GROUND_SHADOW_HALF((u8 *)y, 0x14) & SPLIT_LINE) != 0)
				VEH_GROUND_SHADOW_BYTE((u8 *)y, 1) = (u8)(((struct Instance *)x)->depthBiasSecondary + 1);
			else
				VEH_GROUND_SHADOW_BYTE((u8 *)y, 1) = (u8)(((struct Instance *)x)->depthBiasNormal + 1);
		}
		else
		{
			*(struct Driver **)((u8 *)y + 2) = NULL;
		}
		VEH_GROUND_SHADOW_BYTE((u8 *)y, 0) = 0;
		playerIndex++;
		y += 0x28;
		entryBase += 0x28;
	} while (playerIndex < VEH_GROUND_SHADOW_MAX_DRIVERS);

	{
		struct GameTracker *tracker = GAME_TRACKER;

		*(struct Driver **)(entryBase + 0x14) = NULL;
		v0Value = tracker->numPlyrCurrGame;
		playerIndex = (s32)v0Value - 1;
		v0Value = playerIndex << 4;
		v0Value += playerIndex;
		v0Value <<= 4;
		v0Value += 0x168;
		v0Value = (size_t)tracker + v0Value;
	}
	if (playerIndex < 0)
		goto write_cursor;
	culledState = VEH_GROUND_SHADOW_STATE_CULLED;
	VehGroundShadow_SetLocalVector(localVector, scratch);
	readyState = VEH_GROUND_SHADOW_STATE_READY;
	pushBufferCursor = (u8 *)v0Value + 0x38;

	do
	{
		VEH_GROUND_SHADOW_WORD(scratch, 0x240) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, -0x20);
		VEH_GROUND_SHADOW_WORD(scratch, 0x23c) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, 0xbc);
		geomX = (s16)VEH_GROUND_SHADOW_HALF(pushBufferCursor, -0x18) >> 1;
		geomY = (s16)VEH_GROUND_SHADOW_HALF(pushBufferCursor, -0x16) >> 1;
		VehGroundShadow_SetGeomOffset(geomX, geomY);
		CTC2(VEH_GROUND_SHADOW_WORD(scratch, 0x240), 26);
		VEH_GROUND_SHADOW_WORD(scratch, 0x90) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, 0x44);
		VEH_GROUND_SHADOW_WORD(scratch, 0x94) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, 0x48);
		VEH_GROUND_SHADOW_WORD(scratch, 0x98) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, 0x4c);
		VEH_GROUND_SHADOW_WORD(scratch, 0x50) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, -0x10);
		VEH_GROUND_SHADOW_WORD(scratch, 0x54) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, -0x0c);
		VEH_GROUND_SHADOW_WORD(scratch, 0x58) = VEH_GROUND_SHADOW_WORD(pushBufferCursor, -0x08);
		v0Value = VEH_GROUND_SHADOW_HALF(pushBufferCursor, 0x00);
		v1Value = VEH_GROUND_SHADOW_WORD(pushBufferCursor, -0x04);
		VEH_GROUND_SHADOW_HALF(scratch, 0x60) = (u16)v0Value;
		VEH_GROUND_SHADOW_WORD(scratch, 0x5c) = (u32)v1Value;
		VehGroundShadow_LoadRotMatrix((MATRIX *)(scratch + 0x50));

		VehGroundShadow_AddPointer(entryBase, scratch, 0xa4);
		driver = *(struct Driver **)(entryBase + 0x14);
		if (driver == NULL)
			goto drivers_done;
		CTR_PSX_MEMORY_BARRIER();
		VehGroundShadow_AddPointer(entryCursor, scratch, 0xb8);
		do
		{
			if (VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) == culledState)
				goto next_driver;
			if ((VEH_GROUND_SHADOW_HALF(entryCursor, 0x12) & HIDE_MODEL) != 0)
			{
				VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) = (s8)culledState;
				goto next_driver;
			}
			v0Value = (size_t)entryBase + playerIndex;
			if ((VEH_GROUND_SHADOW_BYTE((u8 *)v0Value, 0x1c) & DRAW_SUCCESSFUL) == 0)
				goto next_driver;

			v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, 0x0c);
			v1Value -= VEH_GROUND_SHADOW_HALF(scratch, 0x90);
			VEH_GROUND_SHADOW_HALF(scratch, 0x9c) = (u16)(v1Value << 2);
			x = (s16)(v1Value << 2);
			z = VEH_GROUND_SHADOW_HALF(entryCursor, 0x0e);
			z -= VEH_GROUND_SHADOW_HALF(scratch, 0x94);
			VEH_GROUND_SHADOW_HALF(scratch, 0x9e) = (u16)(z << 2);
			y = (s16)(z << 2);
			v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, 0x10);
			v1Value -= VEH_GROUND_SHADOW_HALF(scratch, 0x98);
			v0Value = v1Value << 18;
			z = (s32)v0Value >> 16;
			v0Value = VEH_GROUND_SHADOW_WORD(scratch, 0x240);
			v1Value <<= 2;
			VEH_GROUND_SHADOW_HALF(scratch, 0xa0) = (u16)v1Value;
			isSmallScreen = (s32)v0Value < 0x101;
			if (isSmallScreen && ((s32)x >= 0x1771 || (s32)y >= 0x1771 || (s32)z >= 0x1771 || (s32)x < -0x1770 || (s32)y < -0x1770 || (s32)z < -0x1770))
				goto next_driver;

			VehGteLoadV0((SVECTOR *)(scratch + 0x9c));
			CTR_PSX_GTE_PIPELINE_DELAY();
			gte_rtv0_b();
			colorOrDistance = MFC2(27);
			CTR_PSX_GTE_READ_DELAY();
			colorOrDistance = (s32)colorOrDistance >> 2;
			if ((s32)colorOrDistance < -0x34)
				goto next_driver;
			CTR_PSX_FORGET_VALUE(scratch);

			if (isSmallScreen)
			{
				if ((s32)colorOrDistance < 0x180)
					colorOrDistance = 0x1f;
				else
				{
					v0Value = 0x200 - colorOrDistance;
					v1Value = v0Value << 5;
					v0Value = v1Value - v0Value;
					colorOrDistance = (s32)v0Value >> 7;
					if ((s32)v0Value < 0)
					{
						v0Value += 0x7f;
						colorOrDistance = (s32)v0Value >> 7;
					}
					if ((s32)colorOrDistance < 1)
						goto next_driver;
				}
				v0Value = colorOrDistance << 8;
				v1Value = VEH_GROUND_SHADOW_COLOR_PREFIX;
				v0Value |= v1Value;
				v0Value = colorOrDistance | v0Value;
				v1Value = colorOrDistance << 16;
				colorOrDistance = v0Value | v1Value;
			}
			else
				colorOrDistance = VEH_GROUND_SHADOW_LARGE_COLOR;

			if (VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) == VEH_GROUND_SHADOW_STATE_PENDING)
			{
				v0Value = driver->posCurr.y;
				v1Value = driver->quadBlockHeight;
				v0Value -= v1Value;
				v0Value = (s32)v0Value >> 8;
				v1Value = 0x100;
				height = (s32)v1Value - (s32)v0Value;
				if (height < 1)
				{
					VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) = (s8)culledState;
					goto next_driver;
				}
				if (height >= 0x10a)
				{
					VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) = (s8)culledState;
					goto next_driver;
				}
				if (height > 0x100)
					height = 0x100;
				axisMatrix = (MATRIX *)(scratch + 0x70);
				axisVector = CTR_VECTOR_DATA(&driver->AxisAngle3_normalVec);
				axisAngle = driver->rotCurr.y;
				retailCallState[0] = (u32)culledState;
				retailCallState[1] = (u32)readyState;
				retailCallState[2] = (u32)localVector;
				VehPhysForce_RotAxisAngle(axisMatrix, axisVector, axisAngle);
				CTR_PSX_FORGET_VALUE(axisMatrix);
				gteWord0 = VEH_GROUND_SHADOW_WORD(axisMatrix, 0x00);
				gteWord1 = VEH_GROUND_SHADOW_WORD(axisMatrix, 0x04);
				CTC2(gteWord0, 8);
				CTC2(gteWord1, 9);
				gteWord0 = VEH_GROUND_SHADOW_WORD(axisMatrix, 0x08);
				gteWord1 = VEH_GROUND_SHADOW_WORD(axisMatrix, 0x0c);
				gteWord2 = VEH_GROUND_SHADOW_WORD(axisMatrix, 0x10);
				CTC2(gteWord0, 10);
				CTC2(gteWord1, 11);
				CTC2(gteWord2, 12);
				VehGroundShadow_SetCounterZero(entryPlayerIndex);
				v1Value = height << 2;
				v1Value += height;
				v1Value <<= 3;
				v0Value = v1Value + height;
				localZ0 = (s32)v0Value >> 6;
				v0Value = height << 1;
				v0Value += height;
				v0Value <<= 2;
				v0Value += height;
				v0Value <<= 2;
				localZ1 = (s32)v0Value >> 6;
				localOutput = entryBase;
				v1Value = (s32)v1Value >> 6;
				VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x48) = (s16)v1Value;
				VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x4a) = 0;
				VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x4c) = 0;
				localVector = (u8 *)(size_t)*(volatile u32 *)&retailCallState[2];
				readyState = (s32) * (volatile u32 *)&retailCallState[1];
				culledState = (s32) * (volatile u32 *)&retailCallState[0];
				do
				{
					VehGteLoadV0((SVECTOR *)localVector);
					CTR_PSX_GTE_PIPELINE_DELAY();
					gte_llv0_b();
					if (entryPlayerIndex == 0)
						goto first_axis_point;
					if (entryPlayerIndex == readyState)
						goto second_axis_point;
					goto axis_point_ready;

				first_axis_point:
					VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x48) = 0;
					VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x4c) = (s16)localZ0;
					goto axis_point_ready;

				second_axis_point:
					VEH_GROUND_SHADOW_SIGNED_HALF(scratch, 0x4c) = (s16)localZ1;

				axis_point_ready:
					x = MFC2(25);
					y = MFC2(26);
					z = MFC2(27);
					VEH_GROUND_SHADOW_HALF(localOutput, 0) = (u16)x;
					VEH_GROUND_SHADOW_HALF(localOutput, 2) = (u16)y;
					VEH_GROUND_SHADOW_HALF(localOutput, 4) = (u16)z;
					CTR_PSX_MEMORY_BARRIER();
					entryPlayerIndex++;
					localOutput += 6;
				} while (entryPlayerIndex < 3);
				VehGroundShadow_LoadRotMatrix((MATRIX *)(scratch + 0x50));
				v0Value = VEH_GROUND_SHADOW_HALF(scratch, 0x9c);
				v1Value = VEH_GROUND_SHADOW_HALF(scratch, 0x9e);
				z = VEH_GROUND_SHADOW_HALF(scratch, 0xa0);
				VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -2) = (s8)readyState;
				v0Value <<= 16;
				x = (s32)v0Value >> 16;
				v1Value <<= 16;
				y = (s32)v1Value >> 16;
				z <<= 16;
				z = (s32)z >> 16;
			}

			VEH_GROUND_SHADOW_HALF(scratch, 0x20c) = (u16)x;
			VEH_GROUND_SHADOW_HALF(scratch, 0x20e) = (u16)y;
			VEH_GROUND_SHADOW_HALF(scratch, 0x210) = (u16)z;
			VEH_GROUND_SHADOW_HALF(scratch, 0x214) = (u16)(x - VEH_GROUND_SHADOW_HALF(entryBase, 0) - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0e));
			VEH_GROUND_SHADOW_HALF(scratch, 0x216) = (u16)(y - VEH_GROUND_SHADOW_HALF(entryCursor, -0x12) - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0c));
			VEH_GROUND_SHADOW_HALF(scratch, 0x218) = (u16)(z - VEH_GROUND_SHADOW_HALF(entryCursor, -0x10) - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0a));
			v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x0e);
			VehGroundShadow_SetCounterZero(entryPlayerIndex);
			VEH_GROUND_SHADOW_HALF(scratch, 0x21c) = (u16)(x - v0Value);
			v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x0c);
			localZ1 = 0x24;
			VEH_GROUND_SHADOW_HALF(scratch, 0x21e) = (u16)(y - v0Value);
			v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x0a);
			localOutput = scratch;
			VEH_GROUND_SHADOW_HALF(scratch, 0x220) = (u16)(z - v0Value);
			do
			{
				VehGteLoadV3((SVECTOR *)(scratch + 0x20c));
				CTR_PSX_GTE_PIPELINE_DELAY();
				gte_rtpt_b();
				v0Value = 3;
				if (entryPlayerIndex == 0)
				{
					VEH_GROUND_SHADOW_HALF(scratch, 0x20c) = VEH_GROUND_SHADOW_HALF(entryBase, 0x00) + x - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0e);
					VEH_GROUND_SHADOW_HALF(scratch, 0x20e) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x12) + y - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0c);
					VEH_GROUND_SHADOW_HALF(scratch, 0x210) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x10) + z - VEH_GROUND_SHADOW_HALF(entryCursor, -0x0a);
					VEH_GROUND_SHADOW_HALF(scratch, 0x214) = VEH_GROUND_SHADOW_HALF(entryBase, 0x00) + x;
					VEH_GROUND_SHADOW_HALF(scratch, 0x216) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x12) + y;
					VEH_GROUND_SHADOW_HALF(scratch, 0x218) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x10) + z;
					v0Value = VEH_GROUND_SHADOW_HALF(entryBase, 0x00);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x08);
					v0Value += x;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x21c) = (u16)v1Value;
					v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x12);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x06);
					v0Value += y;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x21e) = (u16)v1Value;
					v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x10);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x04);
					v0Value += z;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x220) = (u16)v1Value;
				}
				else if (entryPlayerIndex == (s32)v0Value)
				{
					VEH_GROUND_SHADOW_HALF(scratch, 0x20c) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x08) + x;
					VEH_GROUND_SHADOW_HALF(scratch, 0x20e) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x06) + y;
					VEH_GROUND_SHADOW_HALF(scratch, 0x210) = VEH_GROUND_SHADOW_HALF(entryCursor, -0x04) + z;
					v0Value = VEH_GROUND_SHADOW_HALF(entryBase, 0x00);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x08);
					v0Value = x - v0Value;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x214) = (u16)v1Value;
					v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x12);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x06);
					v0Value = y - v0Value;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x216) = (u16)v1Value;
					v0Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x10);
					v1Value = VEH_GROUND_SHADOW_HALF(entryCursor, -0x04);
					v0Value = z - v0Value;
					v1Value += v0Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x218) = (u16)v1Value;
					VEH_GROUND_SHADOW_HALF(scratch, 0x21c) = x - VEH_GROUND_SHADOW_HALF(entryBase, 0x00);
					VEH_GROUND_SHADOW_HALF(scratch, 0x21e) = y - VEH_GROUND_SHADOW_HALF(entryCursor, -0x12);
					VEH_GROUND_SHADOW_HALF(scratch, 0x220) = z - VEH_GROUND_SHADOW_HALF(entryCursor, -0x10);
				}
				VehGteStoreSxy3((u32 *)localOutput);
				VehGteStoreSz3((u32 *)(scratch + localZ1));
				localZ1 += 0x0c;
				entryPlayerIndex += 3;
				localOutput += 0x0c;
			} while (entryPlayerIndex < 9);

			quadIndex = 0;
			y = (size_t)(prim + 8);
			do
			{
				texture = (u32 *)(scratch + 0x224);
				if ((quadIndex & 1) != 0)
					texture = (u32 *)(scratch + 0x230);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x1c) = colorOrDistance;
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x14) = texture[0];
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x0c) = texture[1];
				VEH_GROUND_SHADOW_HALF((u8 *)y, -0x04) = VEH_GROUND_SHADOW_HALF(texture, 8);
				VEH_GROUND_SHADOW_HALF((u8 *)y, 0x04) = VEH_GROUND_SHADOW_HALF(texture, 10);
				if (quadIndex == readyState)
					goto quad_case_one;
				if (quadIndex >= 2)
					goto quad_at_least_two;
				if (quadIndex == 0)
					goto quad_case_zero;
				goto quad_points_ready;

			quad_at_least_two:
				if (quadIndex == 2)
					goto quad_case_two;
				if (quadIndex == 3)
					goto quad_case_three;
				goto quad_points_ready;

			quad_case_zero:
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x18) = VEH_GROUND_SHADOW_WORD(scratch, 0x20);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x10) = VEH_GROUND_SHADOW_WORD(scratch, 0x00);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x08) = VEH_GROUND_SHADOW_WORD(scratch, 0x04);
				VEH_GROUND_SHADOW_WORD((u8 *)y, 0x00) = VEH_GROUND_SHADOW_WORD(scratch, 0x08);
				goto quad_points_ready;

			quad_case_one:
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x18) = VEH_GROUND_SHADOW_WORD(scratch, 0x20);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x10) = VEH_GROUND_SHADOW_WORD(scratch, 0x00);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x08) = VEH_GROUND_SHADOW_WORD(scratch, 0x1c);
				VEH_GROUND_SHADOW_WORD((u8 *)y, 0x00) = VEH_GROUND_SHADOW_WORD(scratch, 0x18);
				goto quad_points_ready;

			quad_case_two:
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x18) = VEH_GROUND_SHADOW_WORD(scratch, 0x10);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x10) = VEH_GROUND_SHADOW_WORD(scratch, 0x00);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x08) = VEH_GROUND_SHADOW_WORD(scratch, 0x0c);
				VEH_GROUND_SHADOW_WORD((u8 *)y, 0x00) = VEH_GROUND_SHADOW_WORD(scratch, 0x08);
				goto quad_points_ready;

			quad_case_three:
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x18) = VEH_GROUND_SHADOW_WORD(scratch, 0x10);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x10) = VEH_GROUND_SHADOW_WORD(scratch, 0x00);
				VEH_GROUND_SHADOW_WORD((u8 *)y, -0x08) = VEH_GROUND_SHADOW_WORD(scratch, 0x14);
				VEH_GROUND_SHADOW_WORD((u8 *)y, 0x00) = VEH_GROUND_SHADOW_WORD(scratch, 0x18);

			quad_points_ready:
				depthIndex = ((s32)VEH_GROUND_SHADOW_WORD(scratch, 0x24) >> 8) + VEH_GROUND_SHADOW_SIGNED_BYTE(entryCursor, -1);
				if (depthIndex < 0)
					depthIndex = 0;
				if (depthIndex > VEH_GROUND_SHADOW_OT_MAX)
					depthIndex = VEH_GROUND_SHADOW_OT_MAX;
				packetAddress = (s32)CtrGpu_PrimToOTLink24(prim);
				ot = (u32 *)VEH_GROUND_SHADOW_WORD(scratch, 0x23c) + depthIndex;
				prim[0] = *ot | VEH_GROUND_SHADOW_GPU_TAG_POLY_FT4;
				prim += 10;
				*ot = (u32)packetAddress;
				y += 0x28;
				quadIndex++;
			} while (quadIndex < VEH_GROUND_SHADOW_NUM_QUADS);

		next_driver:
			entryCursor += 0x28;
			driver = *(struct Driver **)entryCursor;
			entryBase += 0x28;
		} while (driver != NULL);

	drivers_done:
		playerIndex--;
		pushBufferCursor -= sizeof(struct PushBuffer);
	} while (playerIndex >= 0);

write_cursor:
{
	struct GameTracker *tracker = GAME_TRACKER;
	tracker->backBuffer->primMem.cursor = prim;
}
}
