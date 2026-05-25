#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x800ac214-0x800ac320
void CS_Instance_InitMatrix(void)
{
	if (OVR_233.cs_initMatrixBool != 0)
		return;

	OVR_233.cs_initMatrixBool = 1;

	MATRIX mat;
	MATRIX scale = {0};

	for (int i = 0; i < 4; i++)
	{
		char *data = (char *)OVR_233.cs_initMatrixTable[i].data;
		int count = OVR_233.cs_initMatrixTable[i].count;

		if (data == NULL || count <= 0)
			continue;

		for (int j = 0; j < count; j++)
		{
			char *entry = data + j * 0x20;

			ConvertRotToMatrix(&mat, (s16 *)(entry + 8));

			scale.m[0][0] = *(s16 *)(entry + 0x10);
			scale.m[1][1] = *(s16 *)(entry + 0x12);
			scale.m[2][2] = *(s16 *)(entry + 0x14);

			MatrixRotate((MATRIX *)(entry + 8), &scale, &mat);
		}
	}
}
