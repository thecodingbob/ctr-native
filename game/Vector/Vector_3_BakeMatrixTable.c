#include <common.h>

static s16 Vector_BakeMatrixTable_Div4TowardZero(s32 value)
{
	if (value < 0)
		value += 3;

	return (s16)(value >> 2);
}

static void Vector_BakeMatrixTable_PrepareBlastedFrames(void)
{
	char *entries = (char *)data.bakedGteMath[6].physEntry;
	int count = data.bakedGteMath[6].numEntries;

	if ((entries == NULL) || (count <= 0))
		return;

	for (int i = 0; i < count; i++)
	{
		char *entry = entries + (i * 0x20);
		s32 angle2000 = (i << 0xd) / count;
		s32 angle3000 = (i * 0x3000) / count;
		s32 sin2000 = MATH_Sin((u32)angle2000);

		*(s16 *)(void *)(entry + 0xc) = (s16)angle2000;
		*(s16 *)(void *)(entry + 0x8) = (s16)(-MATH_Sin((u32)angle3000) / 7);
		*(s16 *)(void *)(entry + 0x10) = Vector_BakeMatrixTable_Div4TowardZero(sin2000) + 0x1000;
		*(s16 *)(void *)(entry + 0x14) = (s16)(((sin2000 * 6) / 0x28) + 0x1000);
	}
}

static void Vector_BakeMatrixTable_BakeRotScaleEntries(void)
{
	MATRIX rot;
	MATRIX scale = {0};

	for (int i = 0; i < 0x14; i++)
	{
		char *entries = (char *)data.bakedGteMath[i].physEntry;
		int count = data.bakedGteMath[i].numEntries;

		if ((entries == NULL) || (count <= 0))
			continue;

		for (int j = 0; j < count; j++)
		{
			char *entry = entries + (j * 0x20);

			ConvertRotToMatrix(&rot, (s16 *)(void *)(entry + 8));

			scale.m[0][0] = *(s16 *)(void *)(entry + 0x10);
			scale.m[1][1] = *(s16 *)(void *)(entry + 0x12);
			scale.m[2][2] = *(s16 *)(void *)(entry + 0x14);

			MatrixRotate((MATRIX *)(void *)(entry + 8), &scale, &rot);
		}
	}
}

static void Vector_BakeMatrixTable_BakeBlastedOffsets(void)
{
	char *entries = (char *)data.bakedGteMath[6].physEntry;
	int count = data.bakedGteMath[6].numEntries;

	if ((entries == NULL) || (count <= 0))
		return;

	for (int i = 0; i < count; i++)
	{
		char *entry = entries + (i * 0x20);
		MATRIX *matrix = (MATRIX *)(void *)(entry + 8);
		s32 x = (matrix->m[0][1] * -0x2000) >> 12;
		s32 y = ((matrix->m[1][1] * -0x2000) >> 12) + 0x2000;
		s32 z = (matrix->m[2][1] * -0x2000) >> 12;

		*(s16 *)(void *)(entry + 0) = (s16)x;
		*(s16 *)(void *)(entry + 2) = (s16)y;
		*(s16 *)(void *)(entry + 4) = (s16)z;
	}
}

void Vector_BakeMatrixTable(void)
{
	// NOTE(aalhendi): Source-backed NTSC-U 926 0x80057884-0x80057c20.
	// Retail bakes authored rot/scale vehicle-animation entries in-place before
	// VehPhysForce_TranslateMatrix consumes entry+8 as a MATRIX.
	if (sdata->unk_UsedIn_FUN_80057884 != 0)
		return;

	sdata->unk_UsedIn_FUN_80057884 = 1;

	Vector_BakeMatrixTable_PrepareBlastedFrames();
	Vector_BakeMatrixTable_BakeRotScaleEntries();
	Vector_BakeMatrixTable_BakeBlastedOffsets();
}
