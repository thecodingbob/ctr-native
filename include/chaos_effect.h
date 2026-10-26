#ifndef CHAOS_EFFECT_H
#define CHAOS_EFFECT_H

// The Chaos effect catalogue.
//
// Three consumers have to agree on this list: the enabled-effect bits
// persisted per Adventure slot (include/namespace_Memcard.h), the settings menu
// that draws one row per effect, and the external provider protocol, which
// names effects by slug so a provider survives the list growing.
//
// Every function here is pure. The enabled array arrives as a plain pointer and
// no game state is touched, so the whole catalogue is testable without the
// game. Nothing in this header may include a game header; namespace_Memcard.h
// pulls it in for CHAOS_EFFECT_COUNT, below the point where the resident
// layouts are declared.

typedef enum {
	CHAOS_EFFECT_NONE = -1,

	// Tier A: continuous driving/input modifiers. Restore Chaos-owned baseline
	// constants once per driver before applying active overrides in order.
	CHAOS_EFFECT_INVERTED_STEERING = 0,
	CHAOS_EFFECT_NO_HOPPING = 1,
	CHAOS_EFFECT_WOBBLE_STEERING = 2,
	CHAOS_EFFECT_LOW_GRAVITY = 3,
	CHAOS_EFFECT_HEAVY_KARTS = 4,
	CHAOS_EFFECT_SPEED_CAPPED = 5,
	CHAOS_EFFECT_SUPER_SPEED = 6,
	CHAOS_EFFECT_NO_RESERVES = 7,

	// Tier B: items, wumpa, the economy.
	CHAOS_EFFECT_NO_ITEMS = 8,
	CHAOS_EFFECT_EXPLOSIVE_CRATES = 9,
	CHAOS_EFFECT_WUMPA_DRAIN = 10,
	CHAOS_EFFECT_CLOCK_CURSE = 11,
	CHAOS_EFFECT_DEFECTIVE_BOOSTS = 12,

	// Tier C: driver and collision interventions.
	CHAOS_EFFECT_ANXIOUS_MASK = 13,
	CHAOS_EFFECT_HARD_CRATES = 14,
	CHAOS_EFFECT_GIANT_KARTS = 15,
	CHAOS_EFFECT_USE_THIS = 16,
	CHAOS_EFFECT_JUICED_UP = 17,
	CHAOS_EFFECT_IT_IS_I = 18,
	CHAOS_EFFECT_PROVIDENCE = 19,
	CHAOS_EFFECT_RESERVE_INJECTION = 20,
	CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL = 21,
	CHAOS_EFFECT_SABOTAGE = 22,
	CHAOS_EFFECT_THIS_IS_FIRE = 23,
	CHAOS_EFFECT_HYDRAULICS = 24,
	CHAOS_EFFECT_MY_PLANET_NEEDS_ME = 25,
	CHAOS_EFFECT_WHO_BOUND_THIS = 26,
	CHAOS_EFFECT_TRY_SOMETHING_NEW = 27,

	// IDs are saved bit positions: append new IDs, never renumber or reuse one.
	// Growth within the saved bitset capacity does not change its layout.
	CHAOS_EFFECT_COUNT,
} ChaosEffectId;

// Who an effect touches. Humans-only is every effect in the first cut;
// CHAOS_SCOPE_ALL is where the bot effects earn this field.
enum ChaosEffectScope
{
	CHAOS_SCOPE_HUMANS = 0,
	CHAOS_SCOPE_ALL,
};

typedef struct
{
	const char *slug; // stable across releases; the IPC protocol names effects by it
	const char *label; // what the settings menu draws
	u8 scope;
} ChaosEffectDef;

static const ChaosEffectDef g_chaosEffectDefs[CHAOS_EFFECT_COUNT] =
{
	[CHAOS_EFFECT_INVERTED_STEERING] = { "inverted-steering", "Inverted Steering", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_NO_HOPPING] = { "no-hopping", "No Hopping", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_WOBBLE_STEERING] = { "wobble-steering", "Wobble Steering", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_LOW_GRAVITY] = { "low-gravity", "Low Gravity", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_HEAVY_KARTS] = { "heavy-karts", "Heavy Karts", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_SPEED_CAPPED] = { "speed-capped", "Speed Capped", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_SUPER_SPEED] = { "super-speed", "Super Speed", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_NO_RESERVES] = { "no-reserves", "No Reserves", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_NO_ITEMS] = { "no-items", "No Items", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_EXPLOSIVE_CRATES] = { "explosive-crates", "Explosive Crates", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_WUMPA_DRAIN] = { "wumpa-drain", "Wumpa Drain", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_CLOCK_CURSE] = { "clock-curse", "Clock Curse", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_DEFECTIVE_BOOSTS] = { "defective-boosts", "Defective Boosts", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_ANXIOUS_MASK] = { "anxious-mask", "Anxious Mask", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_HARD_CRATES] = { "hard-crates", "Hard Crates", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_GIANT_KARTS] = { "giant-karts", "Giant Karts", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_USE_THIS] = { "use-this", "Use This!", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_JUICED_UP] = { "juiced-up", "Juiced Up", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_IT_IS_I] = { "it-is-i", "It is I", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_PROVIDENCE] = { "providence", "Providence", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_RESERVE_INJECTION] = { "reserve-injection", "Reserve Injection", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_TOO_BOOSTED_TO_FAIL] = { "too-boosted-to-fail", "Too Boosted TO Fail", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_SABOTAGE] = { "sabotage", "Sabotage", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_THIS_IS_FIRE] = { "this-is-fire", "This Is Fire", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_HYDRAULICS] = { "hydraulics", "Hydraulics", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_MY_PLANET_NEEDS_ME] = { "my-planet-needs-me", "My Planet Needs Me", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_WHO_BOUND_THIS] = { "who-bound-this", "Who Bound This?", CHAOS_SCOPE_HUMANS },
	[CHAOS_EFFECT_TRY_SOMETHING_NEW] = { "try-something-new", "Try Something New", CHAOS_SCOPE_HUMANS },
};

static inline bool ChaosEffect_IsInstant(ChaosEffectId id)
{
	return id == CHAOS_EFFECT_USE_THIS || id == CHAOS_EFFECT_JUICED_UP ||
	       id == CHAOS_EFFECT_IT_IS_I || id == CHAOS_EFFECT_PROVIDENCE ||
	       id == CHAOS_EFFECT_RESERVE_INJECTION || id == CHAOS_EFFECT_MY_PLANET_NEEDS_ME ||
	       id == CHAOS_EFFECT_TRY_SOMETHING_NEW;
}

static inline const ChaosEffectDef *ChaosEffect_Def(ChaosEffectId id)
{
	if ((id < 0) || (id >= CHAOS_EFFECT_COUNT))
	{
		return NULL;
	}
	return &g_chaosEffectDefs[id];
}

static inline ChaosEffectId ChaosEffect_FromIndex(int index)
{
	if ((index < 0) || (index >= CHAOS_EFFECT_COUNT))
	{
		return CHAOS_EFFECT_NONE;
	}
	return (ChaosEffectId)index;
}

static inline ChaosEffectId ChaosEffect_FromSlug(const char *slug)
{
	if (slug == NULL)
	{
		return CHAOS_EFFECT_NONE;
	}

	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		if (strcmp(g_chaosEffectDefs[i].slug, slug) == 0)
		{
			return (ChaosEffectId)i;
		}
	}
	return CHAOS_EFFECT_NONE;
}

// Reads the entry at the effect's own index. A caller passing the settings
// array gets settings->effects[id] and nothing else.
static inline b32 ChaosEffect_IsEnabled(const bool *enabledEffects, ChaosEffectId id)
{
	if ((id < 0) || (id >= CHAOS_EFFECT_COUNT))
	{
		return false;
	}
	return enabledEffects[id];
}

// How many effects are on. A pick is uniform over this number rather than over
// the array length, so it has to be right, and it only changes when a toggle
// does — when the settings menu flips one, when a save is loaded, or when the
// defaults are restored. So the caller keeps the result in its settings and
// hands it to ChaosEffect_Pick, which must not walk the array to recover it.
static inline int ChaosEffect_CountEnabled(const bool *enabledEffects)
{
	int count = 0;

	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		if (enabledEffects[i])
		{
			count++;
		}
	}
	return count;
}

// The chaos system's own generator, kept separate from MixRNG_Scramble and
// RngDeadCoed because both of those are compared every recorded frame by the
// replay verifier. Its state lives in sdata->chaosRng.
enum ChaosRandomConstants
{
	CHAOS_RANDOM_SAMPLE_MAX = 0x7fff,
};

static inline u32 ChaosEffect_NextRandom(u32 *state)
{
	*state = *state * 1103515245u + 12345u;
	return (*state >> 16) & CHAOS_RANDOM_SAMPLE_MAX;
}

// Flips one entry and keeps the cached count in step with it. Every write to
// the array goes through here, or through ChaosEffect_CountEnabled on load, so
// the two cannot drift apart. Returns the new state of the entry.
static inline bool ChaosEffect_Toggle(bool *enabledEffects, u8 *enabledCount, ChaosEffectId id)
{
	if ((id < 0) || (id >= CHAOS_EFFECT_COUNT))
	{
		return false;
	}

	const bool enabled = !enabledEffects[id];

	enabledEffects[id] = enabled;
	*enabledCount = (u8)(*enabledCount + (enabled ? 1 : -1));
	return enabled;
}

static inline ChaosEffectId ChaosEffect_Pick(const bool *enabledEffects, int enabledCount, u32 *rngState)
{
	// Guards the modulus below, and stops a stale count from walking past the
	// end of the pool: an enabledCount larger than the true one simply runs the
	// selection loop out and returns nothing.
	if ((enabledCount <= 0) || (enabledCount > CHAOS_EFFECT_COUNT))
	{
		return CHAOS_EFFECT_NONE;
	}

	int pick = (int)(ChaosEffect_NextRandom(rngState) % (u32)enabledCount);

	for (int i = 0; i < CHAOS_EFFECT_COUNT; i++)
	{
		if (!enabledEffects[i])
		{
			continue;
		}
		if (pick == 0)
		{
			return (ChaosEffectId)i;
		}
		pick--;
	}
	return CHAOS_EFFECT_NONE;
}

// Bots are numbered above the human slots, the convention LOAD_Assets.c and
// AH_WarpPad.c already rely on, so driverID below the human count is the whole
// test. Kept as a pure function of two integers so the rule is testable.
static inline b32 ChaosEffect_AppliesToDriver(enum ChaosEffectScope scope, int driverID, int numHumanPlayers)
{
	if ((driverID < 0) || (numHumanPlayers <= 0))
	{
		return false;
	}
	if (scope == CHAOS_SCOPE_ALL)
	{
		return true;
	}
	return driverID < numHumanPlayers;
}

#endif
