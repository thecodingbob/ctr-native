#ifndef CTR_NATIVE_CHAOS_RUNTIME_H
#define CTR_NATIVE_CHAOS_RUNTIME_H

#include <chaos_effect.h>

enum ChaosRuntimeConstants
{
	CHAOS_MAX_ACTIVATIONS = 4,
	CHAOS_MAX_INSTANT_NOTICES = 4,
	CHAOS_MILLISECONDS_PER_SECOND = 1000,
	CHAOS_ACTIVATION_NOTICE_MS = 3000,
	CHAOS_ACTIVATION_DOCK_MS = 320,
	CHAOS_WOBBLE_PERIOD_MS = 2000,
	CHAOS_WUMPA_DRAIN_PERIOD_MS = 1000,
	CHAOS_RESERVES_MAX = 0x7fff,
	CHAOS_INJECTION_RESERVES = 10 * 960,
};

struct ChaosActivation
{
	ChaosEffectId effect;
	s32 remainingMS;
	s32 noticeRemainingMS;
	s32 elapsedMS;
};

// Race-only state, captured in SDATA; unique effects in activation order.
struct ChaosRuntime
{
	struct ChaosActivation active[CHAOS_MAX_ACTIVATIONS];
	int numActive;
	struct ChaosActivation instantNotices[CHAOS_MAX_INSTANT_NOTICES];
	int numInstantNotices;
	s32 intervalRemainingMS;
	bool raceInitialized;
	bool touchingTurboPad[ADV_MULTIPLAYER_NUM_PLAYERS_MAX];
};

#endif
