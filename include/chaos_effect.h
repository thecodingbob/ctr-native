#ifndef CHAOS_EFFECT_H
#define CHAOS_EFFECT_H

// The Chaos effect catalogue.
//
// Three consumers have to agree on this list: the enabled-effect array
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

	// Tier A: driving constants and input, evaluated every frame. Undoing one
	// is implicit, because the game stops overriding the value.
	CHAOS_EFFECT_INVERTED_STEERING = 0,
	CHAOS_EFFECT_NO_HOPPING,
	CHAOS_EFFECT_WOBBLE_STEERING,
	CHAOS_EFFECT_LOW_GRAVITY,
	CHAOS_EFFECT_HEAVY_KARTS,
	CHAOS_EFFECT_SPEED_CAPPED,
	CHAOS_EFFECT_SUPER_SPEED,
	CHAOS_EFFECT_NO_RESERVES,

	// Tier B: items, wumpa, the economy.
	CHAOS_EFFECT_NO_ITEMS,
	CHAOS_EFFECT_PUNISHING_ITEMS,
	CHAOS_EFFECT_WUMPA_DRAIN,
	CHAOS_EFFECT_CLOCK_CURSE,
	CHAOS_EFFECT_CLEAR_BOOSTS,

	// Tier C: things the game does to the driver, and control takeovers.
	CHAOS_EFFECT_ANXIOUS_MASK,
	CHAOS_EFFECT_HARD_CRATES,
	CHAOS_EFFECT_BOT_TAKES_THE_WHEEL,

	// Derived, so adding an effect above is the only edit the list needs.
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
	{ "inverted-steering", "Inverted Steering", CHAOS_SCOPE_HUMANS },
	{ "no-hopping", "No Hopping", CHAOS_SCOPE_HUMANS },
	{ "wobble-steering", "Wobble Steering", CHAOS_SCOPE_HUMANS },
	{ "low-gravity", "Low Gravity", CHAOS_SCOPE_HUMANS },
	{ "heavy-karts", "Heavy Karts", CHAOS_SCOPE_HUMANS },
	{ "speed-capped", "Speed Capped", CHAOS_SCOPE_HUMANS },
	{ "super-speed", "Super Speed", CHAOS_SCOPE_HUMANS },
	{ "no-reserves", "No Reserves", CHAOS_SCOPE_HUMANS },
	{ "no-items", "No Items", CHAOS_SCOPE_HUMANS },
	{ "punishing-items", "Punishing Items", CHAOS_SCOPE_HUMANS },
	{ "wumpa-drain", "Wumpa Drain", CHAOS_SCOPE_HUMANS },
	{ "clock-curse", "Clock Curse", CHAOS_SCOPE_HUMANS },
	{ "clear-boosts", "Clear Boosts", CHAOS_SCOPE_HUMANS },
	{ "anxious-mask", "Anxious Mask", CHAOS_SCOPE_HUMANS },
	{ "hard-crates", "Hard Crates", CHAOS_SCOPE_HUMANS },
	{ "bot-takes-wheel", "Bot Takes Wheel", CHAOS_SCOPE_HUMANS },
};

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
static inline u32 ChaosEffect_NextRandom(u32 *state)
{
	*state = *state * 1103515245u + 12345u;
	return (*state >> 16) & 0x7fff;
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
