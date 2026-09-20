#include <common.h>

s32 MixRNG_Scramble(void)
{
	// Advance the 16-bit linear congruential generator.
	sdata->randomNumber = ((u32)sdata->randomNumber * 0x6255 + 0x3619) & 0xffff;
	// NOTE(aalhendi): Keep the state store before the return, not in its delay slot.
	CTR_PSX_OBSERVE_MEMORY(sdata->randomNumber);
	return sdata->randomNumber;
}

s32 MixRNG_Particles(s32 range)
{
	u32 random;

	random = RngDeadCoed(&GAME_TRACKER->deadcoed_struct);
	// Preserve the signed high half of the wrapping 32-bit product.
	return (s32)((random & 0xffff) * range) >> 16;
}

u32 MixRNG_GetValue(s32 seed)
{
	return ((u32)seed * 0x6255 + 0x3619) & 0xffff;
}
