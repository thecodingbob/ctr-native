#ifndef CTR_NATIVE_CHAOS_EVENT_H
#define CTR_NATIVE_CHAOS_EVENT_H

// Setup supplies a prediction; runtime supplies the number of spawned bots.
static inline bool ChaosEvent_EffectFits(ChaosEffectId effect, int gameMode1, int players, int bots)
{
	const ChaosEffectDef *def = ChaosEffect_Def(effect);
	if (def == NULL)
	{
		return false;
	}
	if (effect == CHAOS_EFFECT_IDENTITY_CRISIS)
	{
		return players + bots < RACE_FIELD_CONFIG_MAX_RACERS;
	}
	if (effect == CHAOS_EFFECT_YARDRAT_TRAINING || effect == CHAOS_EFFECT_DEFECTIVE_TELEPORT)
	{
		return (gameMode1 & (BATTLE_MODE | CRYSTAL_CHALLENGE)) == 0;
	}
	return def->scope != CHAOS_SCOPE_BOTS || bots > 0;
}

#endif
