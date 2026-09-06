#include <common.h>

s32 MATH_VectorNormalize(SVec3 *vector)
{
	s32 length = MATH_VectorLength(vector);

	if (length == 0)
	{
		return length;
	}

	vector->x = (vector->x << 0xc) / length;
	vector->y = (vector->y << 0xc) / length;
	vector->z = (vector->z << 0xc) / length;

	return length;
}
