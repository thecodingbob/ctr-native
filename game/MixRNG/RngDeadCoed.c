#include <common.h>

s32 RngDeadCoed(struct RngDeadCoedState *state)
{
	u32 state1 = state->state1;
	u32 state0 = state->state0;
	// NOTE(aalhendi): Retail reuses v1 for the byte carry and XOR salt;
	// t0 holds the feedback sum. Native ignores these register bindings.
	register u32 mix CTR_PSX_REGISTER("$3") = state1 << 24;
	register u32 feedback CTR_PSX_REGISTER("$8");

	// Shift the two-word state by one byte and feed the sum into its high byte.
	state1 >>= 8;
	feedback = state0 + state1;
	state0 = (state0 >> 8) | mix;
	feedback += state0 >> 8;
	state1 |= feedback << 24;
	mix = 0xdeadc0ed;
	state1 ^= mix;

	state->state1 = state1;
	state->state0 = state0;
	return state1;
}
