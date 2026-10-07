#ifndef CTR_GTE_TRANSFER_H
#define CTR_GTE_TRANSFER_H

#include <ctr_gte.h>

// NOTE(aalhendi): Keep the read delay inside the transfer when writing directly
// to a signed C destination. The compiler owns the register and final store.
#define CTR_GteReadDataDelayed(dst, reg) ((dst) = MFC2_S(reg))

// The MAC2 read fills MAC1's CPU load delay. A caller using MAC2 must delay too.
#define CTR_GteReadMAC12(mac1, mac2) \
	do                               \
	{                                \
		(mac1) = MFC2_S(25);         \
		(mac2) = MFC2_S(26);         \
	} while (0)

// NOTE(aalhendi): Three word-aligned SDK vectors; native does not read VZ padding.
#define CTR_GteLoadPositionsV0V1V2(first, second, third) CTR_GteLoadSV3(first, second, third)
#define CTR_GteStorePositionsXY(first, second, third)    CTR_GteStoreSXY3(first, second, third)
#define CTR_GteSetGeomOffset(x, y)                       gte_SetGeomOffset(x, y)

// Word-aligned vectors. LH also sign-extends the row's Z into R21's upper half.
static inline void CTR_GteLoadDotProduct(const SVec3 *row, const SVec3 *vector)
{
	CTC2(CTR_PackS16Pair(row->x, row->y), 0);
	CTC2((u32)(s32)row->z, 1);
	MTC2(CTR_PackS16Pair(vector->x, vector->y), 0);
	MTC2((u32)(s32)vector->z, 1);
}

// NOTE(aalhendi): These transfers retain the PsyQ instruction schedule;
// native builds use the equivalent GTE interface.
static inline void CTR_GteSetRotMatrix(const MATRIX *matrix)
{
	gte_SetRotMatrix(matrix);
}

static inline void CTR_GteLoadPositionV0(const SVec4 *worldPosition)
{
	MTC2(CTR_PackS16Pair(worldPosition->x, worldPosition->y), 0);
	MTC2(CTR_PackS16Pair(worldPosition->z, 0), 1);
}

static inline void CTR_GteStorePositionXY(s16 *screenPosition)
{
	CTR_PSX_STORE_COP2_WORD(screenPosition, 14);
}

// Three packed words: V0 XY, V1 XY, and their shared signed Z coordinate.
#define CTR_GteLoadLineV0V1(line)                       \
	do                                                  \
	{                                                   \
		MTC2(CTR_ReadU32LE((const u8 *)(line)), 0);     \
		MTC2(CTR_ReadU32LE((const u8 *)(line) + 8), 1); \
		MTC2(CTR_ReadU32LE((const u8 *)(line) + 4), 2); \
		MTC2(CTR_ReadU32LE((const u8 *)(line) + 8), 3); \
	} while (0)

static inline void CTR_GteStoreLineXY(void *screenPositions)
{
	CTR_WriteU32LE(screenPositions, (u32)MFC2(12));
	CTR_WriteU32LE((u8 *)screenPositions + 4, (u32)MFC2(13));
}

// Word-aligned XYZ vectors; the GTE ignores the upper half of each Z word.
#define CTR_GteLoadPositionsV0V1(first, second) \
	do                                          \
	{                                           \
		CTR_GteLoadSVec3V0(first);              \
		CTR_GteLoadSVec3V1(second);             \
	} while (0)

static inline s32 CTR_GteReadDepthZ1(void)
{
	return (s32)MFC2(17);
}

#define CTR_GteSetTransMatrix(matrix) gte_SetTransMatrix(matrix)

#endif
