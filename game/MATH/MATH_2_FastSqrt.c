#include <common.h>

s32 MATH_FastSqrt(u32 value, s32 shift)
{
	s32 root = 0;
	u32 remainder = (u32)root;
	s32 iterations = (shift >> 1) + 0xf;
	s32 keepGoing;

	do
	{
		u32 test;

		remainder = (remainder << 2) | (value >> 30);
		value <<= 2;

		root *= 2;
		test = root * 2 + 1;

		if (test <= remainder)
		{
			remainder -= test;
			root++;
		}

		keepGoing = iterations;
		iterations--;
	} while (keepGoing != 0);

	return root;
}
