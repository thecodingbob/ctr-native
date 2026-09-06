#include <common.h>

int MixRNG_Scramble()
{
	// random algorithm for seemingly-random numbers
	sdata->randomNumber = (sdata->randomNumber * 0x6255 + 0x3619) & 0xffff;
	return sdata->randomNumber;
}

int MixRNG_Particles(int param_1)
{
	u32 uVar1;

	uVar1 = RngDeadCoed(&sdata->gGT->deadcoed_struct);
	return (int)((uVar1 & 0xffff) * param_1) >> 0x10;
}

u32 MixRNG_GetValue(int param_1)
{
	return (param_1 * 0x6255 + 0x3619U) & 0xffff;
}
