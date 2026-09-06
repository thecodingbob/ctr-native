#include <common.h>

void GTE_AudioLR_Inst(MATRIX *matrix, s32 *vec)
{
	SVECTOR input;

	input.vx = (s16)vec[0];
	input.vy = (s16)vec[1];
	input.vz = (s16)vec[2];
	input.pad = 0;

	SetRotMatrix(matrix);
	CTR_GteLoadSV0(&input);
	gte_rtv0();
	CTR_GteStoreMAC(vec);
}

void GTE_AudioLR_Driver(MATRIX *matrix, struct Driver *driver, s32 *out)
{
	SVECTOR input;

	input.vx = (s16)((u32)driver->posCurr.x >> 8) - (s16)matrix->t[0];
	input.vy = (s16)((u32)driver->posCurr.y >> 8) - (s16)matrix->t[1];
	input.vz = (s16)((u32)driver->posCurr.z >> 8) - (s16)matrix->t[2];
	input.pad = 0;

	SetRotMatrix(matrix);
	CTR_GteLoadSV0(&input);
	gte_rtv0();
	CTR_GteStoreMAC(out);
}

int GTE_GetSquaredLength(s32 *vec)
{
	MTC2(vec[0], 9);
	MTC2(vec[1], 10);
	MTC2(vec[2], 11);
	gte_sqr0();

	s32 x2 = MFC2(25);
	s32 y2 = MFC2(26);
	s32 z2 = MFC2(27);

	return CTR_MipsAddLo(CTR_MipsAddLo(x2, y2), z2);
}
