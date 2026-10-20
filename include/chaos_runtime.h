#ifndef CTR_NATIVE_CHAOS_RUNTIME_H
#define CTR_NATIVE_CHAOS_RUNTIME_H

#include <chaos_effect.h>

enum ChaosRuntimeConstants
{
	CHAOS_MAX_ACTIVATIONS = 4,
	CHAOS_MILLISECONDS_PER_SECOND = 1000,
	CHAOS_ACTIVATION_NOTICE_MS = 3000,
	CHAOS_ACTIVATION_DOCK_MS = 320,
};

struct ChaosActivation
{
	ChaosEffectId effect;
	s32 remainingMS;
	s32 noticeRemainingMS;
};

// Race-only state, captured in SDATA; array order is activation order.
struct ChaosRuntime
{
	struct ChaosActivation active[CHAOS_MAX_ACTIVATIONS];
	int numActive;
	s32 intervalRemainingMS;
	bool raceInitialized;
};

#endif
