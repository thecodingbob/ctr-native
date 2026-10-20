#ifndef CTR_NATIVE_CHAOS_RUNTIME_H
#define CTR_NATIVE_CHAOS_RUNTIME_H

#include <chaos_effect.h>

enum ChaosRuntimeConstants
{
	CHAOS_MAX_ACTIVATIONS = 128,
	CHAOS_MILLISECONDS_PER_SECOND = 1000,
	CHAOS_ACTIVATION_NOTICE_MS = 3000,
};

struct ChaosActivation
{
	ChaosEffectId effect;
	s32 remainingMS;
};

// Race-only state, captured in SDATA; array order is activation order.
struct ChaosRuntime
{
	struct ChaosActivation active[CHAOS_MAX_ACTIVATIONS];
	int numActive;
	s32 intervalRemainingMS;
	ChaosEffectId noticeEffect;
	s32 noticeRemainingMS;
	bool raceInitialized;
};

#endif
