#include <common.h>

void RB_Player_KillPlayer(struct Driver *attacker, struct Driver *victim)
{
	u32 gameMode = GAME_TRACKER->gameMode1;

	if ((gameMode & BATTLE_MODE) == 0)
	{
		return;
	}

	if (attacker == NULL)
	{
		return;
	}

	if (victim == NULL)
	{
		return;
	}

	if ((gameMode & POINT_LIMIT) != 0)
	{
		s32 victimTeam = victim->BattleHUD.teamID;
		s32 attackerTeam = attacker->BattleHUD.teamID;

		if (victimTeam != attackerTeam)
		{
			s32 score = GAME_TRACKER->battleSetup.pointsPerTeam[attackerTeam] + 1;
			s32 i;

			if (score < 100)
			{
				GAME_TRACKER->battleSetup.pointsPerTeam[attackerTeam] = score;
			}
			if (GAME_TRACKER->battleSetup.pointsPerTeam[attacker->BattleHUD.teamID] != GAME_TRACKER->battleSetup.killLimit)
			{
				return;
			}
			if ((GAME_TRACKER->gameMode1 & TIME_LIMIT) != 0)
			{
				return;
			}
			for (i = 0; i < GAME_TRACKER->numPlyrCurrGame; i++)
			{
				GAME_TRACKER->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
			}
		}
		else
		{
			s32 score = GAME_TRACKER->battleSetup.pointsPerTeam[victimTeam] - 1;
			if (score >= -9)
			{
				GAME_TRACKER->battleSetup.pointsPerTeam[victimTeam] = score;
			}
			return;
		}
	}
	else
	{
		s32 lives;
		s32 i;
		s16 isTeamAlive[4];
		s32 deadPlayers;
		s16 teamsAlive;
		s32 victimTeam;
		if ((gameMode & LIFE_LIMIT) == 0)
		{
			return;
		}

		lives = victim->BattleHUD.numLives - 1;

		if (lives > 0)
		{
			victim->BattleHUD.numLives = lives;
			return;
		}


		deadPlayers = 0;
		memset(isTeamAlive, deadPlayers, sizeof(isTeamAlive));
		// NOTE(aalhendi): Seed both tallies and the first scan from the reset count before counting starts.
		teamsAlive = deadPlayers;
		i = deadPlayers;

		victim->funcPtrs[DRIVER_FUNC_INIT] = VehStuckProc_RIP_Init;
		victim->BattleHUD.numLives = 0;
		victim->actionsFlagSet |= ACTION_RACE_FINISHED;

		for (; i < GAME_TRACKER->numPlyrCurrGame; i++)
		{
			struct Driver *driver = GAME_TRACKER->drivers[i];

			if ((driver->actionsFlagSet & ACTION_RACE_FINISHED) != 0)
			{
				deadPlayers++;
			}
			else
			{
				isTeamAlive[driver->BattleHUD.teamID] = 1;
			}
		}

		victimTeam = victim->BattleHUD.teamID;

		if (((GAME_TRACKER->battleSetup.teamFlags & (1 << victimTeam)) != 0) && (isTeamAlive[victimTeam] == 0))
		{
			s32 remainingPlayers = GAME_TRACKER->numPlyrCurrGame - deadPlayers;

			if (remainingPlayers < 3)
			{
				GAME_TRACKER->standingsPoints[victimTeam * 3 + remainingPlayers]++;
			}

			GAME_TRACKER->battleSetup.finishedRankOfEachTeam[victim->BattleHUD.teamID] = GAME_TRACKER->numPlyrCurrGame - deadPlayers;
		}

		for (i = 0; i < 4; i++)
		{
			if (((GAME_TRACKER->battleSetup.teamFlags & (1 << i)) != 0) && (isTeamAlive[i] != 0))
			{
				teamsAlive++;
			}
		}

		if (teamsAlive != 1)
		{
			return;
		}

		for (i = 0; i < GAME_TRACKER->numPlyrCurrGame; i++)
		{
			GAME_TRACKER->drivers[i]->actionsFlagSet |= ACTION_RACE_FINISHED;
		}
	}

	MainGameEnd_Initialize();
}

void RB_Player_ModifyWumpa(struct Driver *driver, int wumpaDelta)
{
	s32 numWumpaOriginal = driver->numWumpas;

	// if using unlimited wumpa, quit
	if ((GAME_TRACKER->gameMode2 & CHEAT_WUMPA) != 0)
	{
		return;
	}

	if (
	    // if wumpa is being subtracted
	    (wumpaDelta < 0) &&

	    // using mask weapon that protects from damage
	    ((driver->actionsFlagSet & ACTION_MASK_WEAPON) != 0) &&

	    g_config.maskProtectsFromDamage)
	{
		// quit, dont lose wumpa
		return;
	}

	if (
	    // wumpa increasing
	    (wumpaDelta > 0) &&

	    // driver is not an AI
	    ((driver->actionsFlagSet & ACTION_BOT) == 0))
	{
		// for end-of-race comments
		driver->numTimesWumpa += wumpaDelta;
	}

	// works for positive and negative delta
	driver->numWumpas += wumpaDelta;

	// dont allow negatives
	if (driver->numWumpas < 0)
	{
		driver->numWumpas = 0;
	}

	// cap at max wumpa count
	if (driver->numWumpas > DRIVER_WUMPA_MAX_COUNT)
	{
		driver->numWumpas = DRIVER_WUMPA_MAX_COUNT;
	}

	if (
	    // if did not have juiced wumpa before
	    (numWumpaOriginal < DRIVER_WUMPA_JUICED_COUNT) &&

	    // if have juiced wumpa now
	    (driver->numWumpas == DRIVER_WUMPA_JUICED_COUNT))
	{
		// Play "juiced up" sound
		OtherFX_Play(0x41, 1);

		driver->BattleHUD.juicedUpCooldown = DRIVER_WUMPA_JUICED_HUD_COOLDOWN_FRAMES;
	}
}

void RB_Player_ToggleInvisible(void)
{
	struct GameTracker *gGT = GAME_TRACKER;
	struct Driver *d;
	struct Thread *t;

	// loop through player threads
	for (t = gGT->threadBuckets[PLAYER].thread; t != NULL; t = t->siblingThread)
	{
		// driver object
		d = t->object;

		// if driver is invisible
		if (d->invisibleTimer != 0)
		{
			s32 i;
			for (i = 0; i < GAME_TRACKER->numPlyrCurrGame; i++)
			{
				// Keep the driver's own viewport visible.
				if (i != d->driverID)
				{
					d->instSelf->idpp[i].instFlags &= 0xffffffbf;
				}
			}
		}
	}
	return;
}

void RB_Player_ToggleFlicker(void)
{
	struct GameTracker *gGT = GAME_TRACKER;
	struct Thread *t;
	struct Driver *d;

	for (t = gGT->threadBuckets[PLAYER].thread; t != NULL; t = t->siblingThread)
	{
		// driver object
		d = t->object;

		if (
		    // invincible timer
		    (0x2a0 < d->invincibleTimer) &&

		    // odd number frames
		    ((GAME_TRACKER->timer & 1) != 0))
		{
			s32 i;
			// Flicker on every viewport, including the driver's own.
			for (i = 0; i < GAME_TRACKER->numPlyrCurrGame; i++)
			{
				d->instSelf->idpp[i].instFlags &= 0xffffffbf;
			}
		}
	}
	return;
}
