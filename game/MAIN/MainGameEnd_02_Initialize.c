#include <common.h>

// TODO(aalhendi): Source-backed bridge for NTSC-U 926 0x8003a3fc-0x8003aee8.
// Keep unstamped until the battle ranking loops and reward callouts are checked
// instruction-by-instruction against retail.

static void MainGameEnd_AddBattleWinner(struct GameTracker *gGT, struct Driver *driver)
{
	if (driver == NULL)
		return;

	u32 winnerIndex = gGT->numWinners;

	if (winnerIndex >= 4)
		return;

	u8 driverID = driver->driverID;
	gGT->winnerIndex[winnerIndex] = driverID;

	struct PushBuffer *pb = &gGT->pushBuffer[driverID];
	pb->fadeFromBlack_currentValue = 0x1fff;
	pb->fadeFromBlack_desiredResult = 0x1000;
	pb->fade_step = 0xff78;

	BOTS_Driver_Convert(driver);
	gGT->numWinners++;

	gGT->confetti.numParticles_max = 0xfa;
	gGT->confetti.unk2 = 0xfa;
}

static void MainGameEnd_UpdateAdventureLosses(struct GameTracker *gGT, struct Driver *player)
{
	if ((gGT->gameMode1 & (ADVENTURE_CUP | RELIC_RACE | ADVENTURE_MODE)) != ADVENTURE_MODE)
		return;

	if (player == NULL)
		return;

	if (player->driverRank == 0)
	{
		if ((s32)gGT->gameMode1 < 0)
			sdata->advProgress.timesLostBossRace[gGT->bossID] = 0;
		else
			sdata->advProgress.timesLostRacePerLev[gGT->levelID] = 0;

		return;
	}

	char *lossCounter;

	if ((s32)gGT->gameMode1 < 0)
		lossCounter = &sdata->advProgress.timesLostBossRace[gGT->bossID];
	else
		lossCounter = &sdata->advProgress.timesLostRacePerLev[gGT->levelID];

	if (*lossCounter < 10)
		(*lossCounter)++;
}

static void MainGameEnd_RecordNonBattleStandings(struct GameTracker *gGT)
{
	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if (driver == NULL)
			continue;

		int rank = driver->driverRank;

		if (rank < 3)
			gGT->standingsPoints[i * 3 + rank]++;
	}
}

static int MainGameEnd_BattleTeamActive(struct GameTracker *gGT, int team)
{
	if ((gGT->gameMode1 & BATTLE_MODE) == 0)
		return 1;

	return (gGT->battleSetup.teamFlags & (1u << team)) != 0;
}

static void MainGameEnd_RankBattleTeams(struct GameTracker *gGT, int scores[4])
{
	u8 usedTeams = 0;

	for (int rank = 0; rank < 4; rank++)
	{
		int bestScore = -400;
		int tiedTeams[4];
		int numTies = 0;

		for (int team = 3; team >= 0; team--)
		{
			if ((usedTeams & (1u << team)) != 0)
				continue;

			if (!MainGameEnd_BattleTeamActive(gGT, team))
				continue;

			if (scores[team] < bestScore)
				continue;

			if (scores[team] > bestScore)
			{
				bestScore = scores[team];
				numTies = 0;
			}

			tiedTeams[numTies++] = team;
		}

		if (numTies == 0)
			break;

		for (int i = 0; i < numTies; i++)
		{
			int team = tiedTeams[i];

			usedTeams |= 1u << team;
			gGT->battleSetup.finishedRankOfEachTeam[team] = rank;

			if (rank < 3)
				gGT->standingsPoints[team * 3 + rank]++;
		}

		rank += numTies - 1;
	}
}

static void MainGameEnd_UpdateBattleWinners(struct GameTracker *gGT)
{
	for (int i = 0; i < 4; i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if (driver == NULL)
			continue;

		if (gGT->battleSetup.finishedRankOfEachTeam[driver->BattleHUD.teamID] != 0)
			continue;

		MainGameEnd_AddBattleWinner(gGT, driver);
	}
}

static void MainGameEnd_UpdateBattlePointLimit(struct GameTracker *gGT)
{
	int scores[4];

	for (int team = 0; team < 4; team++)
		scores[team] = gGT->battleSetup.pointsPerTeam[team];

	MainGameEnd_RankBattleTeams(gGT, scores);
	MainGameEnd_UpdateBattleWinners(gGT);
}

static void MainGameEnd_UpdateBattleLifeLimit(struct GameTracker *gGT)
{
	int scores[4] = {-400, -400, -400, -400};

	for (int team = 0; team < 4; team++)
	{
		if (!MainGameEnd_BattleTeamActive(gGT, team))
			continue;

		scores[team] = 0;
	}

	for (int i = 0; i < gGT->numPlyrCurrGame; i++)
	{
		struct Driver *driver = gGT->drivers[i];

		if (driver == NULL)
			continue;

		int team = driver->BattleHUD.teamID;

		if (!MainGameEnd_BattleTeamActive(gGT, team))
			continue;

		scores[team] += driver->BattleHUD.numLives;
	}

	MainGameEnd_RankBattleTeams(gGT, scores);
	MainGameEnd_UpdateBattleWinners(gGT);
}

static void MainGameEnd_UpdateStandingsOrder(struct GameTracker *gGT)
{
	for (int team = 0; team < 4; team++)
	{
		int score = 0;

		for (int rank = 0; rank < 3; rank++)
			score += gGT->standingsPoints[team * 3 + rank] * (3 - rank);

		gGT->battleSetup.unk_afterTeams[team] = score;
	}

	u8 usedTeams = 0;

	for (int rank = 0; rank < 4; rank++)
	{
		int bestScore = 0;
		int bestTeam = -1;

		for (int team = 3; team >= 0; team--)
		{
			if ((usedTeams & (1u << team)) != 0)
				continue;

			if (!MainGameEnd_BattleTeamActive(gGT, team))
				continue;

			if (gGT->battleSetup.unk_afterTeams[team] < bestScore)
				continue;

			bestScore = gGT->battleSetup.unk_afterTeams[team];
			bestTeam = team;
		}

		gGT->battleSetup.unk1dc8[rank] = bestTeam;

		if (bestTeam >= 0)
			usedTeams |= 1u << bestTeam;
	}
}

static void MainGameEnd_FinalizeDriverClocks(struct GameTracker *gGT)
{
	struct Thread *thread = gGT->threadBuckets[PLAYER].thread;

	while (thread != NULL)
	{
		UI_RaceEnd_GetDriverClock(thread->object);
		thread = thread->siblingThread;
	}
}

static void MainGameEnd_CheckTimeTrialGhost(struct GameTracker *gGT, struct Driver *player)
{
	if ((gGT->gameMode1 & RELIC_RACE) != 0)
	{
		// TODO(aalhendi): Retail jumps to the active relic overlay entry at
		// 0x8009f71c. Native currently calls the compiled 223 helper directly.
		DECOMP_RR_EndEvent_UnlockAward();
		return;
	}

	if ((sdata->boolReplayHumanGhost != 0) &&
	    ((sdata->boolGhostTooBigToSave != 0) || (player->timeElapsedInRace >= sdata->ptrGhostTapePlaying->timeElapsedInRace)))
		return;

	GhostTape_End();
	gGT->gameModeEnd |= PLAYER_GHOST_BEAT;
}

void MainGameEnd_Initialize(void)
{
	struct GameTracker *gGT = sdata->gGT;
	struct Driver *player = gGT->drivers[0];

	if ((gGT->gameMode1 & END_OF_RACE) == 0)
	{
		for (int i = 0; i < 8; i++)
		{
			struct Driver *driver = gGT->drivers[i];

			if (driver != NULL)
				sdata->kartSpawnOrderArray[i] = driver->driverRank;
		}

		gGT->newHighScoreIndex = -1;

		if ((gGT->gameMode1 & ROLLING_ITEM) != 0)
		{
			OtherFX_Stop2(0x5d);
			gGT->gameMode1 &= ~ROLLING_ITEM;
		}

		MainGameEnd_UpdateAdventureLosses(gGT, player);

		gGT->gameMode1 |= END_OF_RACE;
		gGT->gameModeEnd = gGT->gameMode1 & 0x3e0020;

		if ((gGT->gameMode1 & BATTLE_MODE) == 0)
		{
			if (gGT->numPlyrCurrGame != 0)
				MainGameEnd_RecordNonBattleStandings(gGT);
		}
		else if ((gGT->gameMode1 & POINT_LIMIT) != 0)
		{
			MainGameEnd_UpdateBattlePointLimit(gGT);
		}
		else if ((gGT->gameMode1 & TIME_LIMIT) != 0)
		{
			MainGameEnd_UpdateBattleLifeLimit(gGT);
		}

		MainGameEnd_UpdateStandingsOrder(gGT);
		MainGameEnd_FinalizeDriverClocks(gGT);

		UI_VsQuipAssignAll();
		SubmitName_RestoreName(2);

		if ((gGT->gameMode1 & (RELIC_RACE | TIME_TRIAL)) != 0)
		{
			MainGameEnd_SoloRaceGetReward(1);
			MainGameEnd_CheckTimeTrialGhost(gGT, player);
		}
	}

	Podium_InitModels(gGT);
}

void DECOMP_MainGameEnd_Initialize(void)
{
	MainGameEnd_Initialize();
}
