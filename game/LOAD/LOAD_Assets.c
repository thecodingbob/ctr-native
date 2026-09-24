#include <common.h>

#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
#include <platform/native_checkpoint.h>
#include <platform/native_config.h>
#endif

enum
{
	LOAD_EXTRA_CHARACTER_MODEL_CAPACITY = LOAD_CHARACTER_ID_COUNT - 1,
	LOAD_DEFAULT_CHARACTER_COUNT = 8,
	LOAD_CHARACTER_COUNT = len(data.MetaDataCharacters),
};

static DriverModelExtraSlot s_extraCharacterModels[LOAD_EXTRA_CHARACTER_MODEL_CAPACITY];
static s16 s_extraCharacterModelIDs[LOAD_EXTRA_CHARACTER_MODEL_CAPACITY];
static int s_extraCharacterModelCount;

static b32 LOAD_IsRandomBotSelectionEnabled(void)
{
	int mode = g_config.botSelectionMode;
	return mode == BOT_SELECTION_RANDOM_UNLOCKED || mode == BOT_SELECTION_RANDOM_ALL;
}

static b32 LOAD_IsRandomBotRace(void)
{
	u32 gameMode = sdata->gGT->gameMode1;

	if (!LOAD_IsRandomBotSelectionEnabled())
	{
		return false;
	}

	if ((gameMode & (ARCADE_MODE | ADVENTURE_MODE)) == 0)
	{
		return false;
	}

	if ((gameMode & (GAME_CUTSCENE | ADVENTURE_ARENA | MAIN_MENU | BATTLE_MODE | RELIC_RACE | TIME_TRIAL | ADVENTURE_BOSS)) != 0)
	{
		return false;
	}

	return !((gameMode & ADVENTURE_CUP) != 0 && sdata->gGT->cup.cupID == CUP_ID_PURPLE_GEM);
}

static void LOAD_ShuffleCharacterIDs(s16 *characterIDs)
{
	for (int i = LOAD_CHARACTER_COUNT - 1; i > 0; i--)
	{
		int swapIndex = (u32)RngDeadCoed(&sdata->advRng) % (i + 1);
		s16 characterID = characterIDs[i];
		characterIDs[i] = characterIDs[swapIndex];
		characterIDs[swapIndex] = characterID;
	}
}

static b32 LOAD_IsCharacterUsedByPlayer(s16 characterID)
{
	for (int playerIndex = 0; playerIndex < sdata->gGT->numPlyrCurrGame; playerIndex++)
	{
		if (data.characterIDs[playerIndex] == characterID)
		{
			return true;
		}
	}

	return false;
}

static int LOAD_GetRandomBotCount(void)
{
	int playerCount = sdata->gGT->numPlyrCurrGame;

	if (playerCount == 1 || g_config.extendedArcadeMultiplayer)
	{
		return LOAD_CHARACTER_ID_COUNT - playerCount;
	}

	return 6 - playerCount;
}

static void LOAD_ResetExtraCharacterModels(void)
{
	for (int i = 0; i < LOAD_EXTRA_CHARACTER_MODEL_CAPACITY; i++)
	{
		s_extraCharacterModels[i].fileBase = NULL;
		s_extraCharacterModelIDs[i] = -1;
	}

	s_extraCharacterModelCount = 0;
}

static void LOAD_QueueExtraCharacterModels(struct BigHeader *bigfile, const s16 *characterIDs, int characterCount, int modelFileIndex)
{
	for (int characterIndex = 0; characterIndex < characterCount; characterIndex++)
	{
		if (s_extraCharacterModelCount >= LOAD_EXTRA_CHARACTER_MODEL_CAPACITY)
		{
			return;
		}

		int modelIndex = s_extraCharacterModelCount++;
		s_extraCharacterModelIDs[modelIndex] = characterIDs[characterIndex];
		LOAD_AppendQueue(bigfile, LT_GETADDR, modelFileIndex + characterIDs[characterIndex], &s_extraCharacterModels[modelIndex].fileBase,
		                 LOAD_QUEUE_CALLBACK_SET_POINTER);
	}
}

static void LOAD_SelectRandomBots(struct BigHeader *bigfile)
{
	s16 shuffledCharacterIDs[LOAD_CHARACTER_COUNT];
	s16 standaloneCharacterIDs[LOAD_EXTRA_CHARACTER_MODEL_CAPACITY];
	int botCount = LOAD_GetRandomBotCount();
	int botIndex = 0;
	int standaloneCharacterCount = 0;
	int modelFileIndex = sdata->gGT->numPlyrCurrGame == 2 && !g_config.extendedArcadeMultiplayer ? BI_RACERMODELMED : BI_RACERMODELHI;

	for (int characterID = 0; characterID < LOAD_CHARACTER_COUNT; characterID++)
	{
		shuffledCharacterIDs[characterID] = (s16)characterID;
	}

	LOAD_ShuffleCharacterIDs(shuffledCharacterIDs);

	for (int shuffledIndex = 0; shuffledIndex < LOAD_CHARACTER_COUNT && botIndex < botCount; shuffledIndex++)
	{
		s16 characterID = shuffledCharacterIDs[shuffledIndex];

		if (LOAD_IsCharacterUsedByPlayer(characterID))
		{
			continue;
		}

		if (g_config.botSelectionMode == BOT_SELECTION_RANDOM_UNLOCKED && !MM_Characters_IsCharacterUnlocked(characterID))
		{
			continue;
		}

		data.characterIDs[sdata->gGT->numPlyrCurrGame + botIndex] = characterID;

		// The 1P arcade pack provides the original eight racers. The regular
		// 2P pack has only its predefined AI set, so every randomized 2P bot
		// needs a standalone model.
		if (characterID >= LOAD_DEFAULT_CHARACTER_COUNT || (sdata->gGT->numPlyrCurrGame == 2 && !g_config.extendedArcadeMultiplayer))
		{
			standaloneCharacterIDs[standaloneCharacterCount++] = characterID;
		}

		botIndex++;
	}

	LOAD_QueueExtraCharacterModels(bigfile, standaloneCharacterIDs, standaloneCharacterCount, modelFileIndex);
}

struct Model *LOAD_GetExtraCharacterModelByName(char *searchName)
{
	for (int i = 0; i < s_extraCharacterModelCount; i++)
	{
		if (strcmp(GAME_CHARACTER_METADATA[s_extraCharacterModelIDs[i]].name_Debug, searchName) == 0)
		{
			return s_extraCharacterModels[i].model;
		}
	}

	return NULL;
}

void LOAD_FinalizeExtraCharacterModels(void)
{
	for (int i = 0; i < s_extraCharacterModelCount; i++)
	{
		if (s_extraCharacterModels[i].fileBase != NULL)
		{
			s_extraCharacterModels[i].model = (struct Model *)((u8 *)s_extraCharacterModels[i].fileBase + LOAD_MODEL_FILE_HEADER_BYTES);
		}
	}
}

void LOAD_RunPtrMap(char *origin, int *patchArr, int numPtrs)
{
	int *ptrCurrOffset = patchArr;

	for (ptrCurrOffset = &patchArr[0]; ptrCurrOffset < &patchArr[numPtrs]; ptrCurrOffset++)
	{
		int offset = (*ptrCurrOffset >> 2) << 2;
		*(int *)&origin[offset] = *(int *)&origin[offset] + (int)origin;
#if defined(CTR_NATIVE) && defined(CTR_INTERNAL)
		NativeCheckpoint_RegisterPointerSlot(&origin[offset]);
#endif
	}
}

static int LOAD_SelectRobots2P(int p1, int p2)
{
	int setIndex;
	u8 *robotSet;
	b32 boolFoundRepeat = false;

	// 8 sets, but only check 7 because the last is the Gem Cups pack (4 bosses).
	for (setIndex = 0; setIndex < LOAD_2P_AI_SET_COUNT; setIndex++)
	{
		robotSet = data.characterIDs_2P_AIs[setIndex];

		boolFoundRepeat = false;
		for (int racerIndex = 0; racerIndex < LOAD_2P_AI_SET_RACER_COUNT; racerIndex++)
		{
			if ((robotSet[racerIndex] == p1) || (robotSet[racerIndex] == p2))
			{
				boolFoundRepeat = true;
				break;
			}
		}

		if (!boolFoundRepeat)
		{
			break;
		}
	}

	if (setIndex >= LOAD_2P_AI_SET_COUNT)
	{
		return -1;
	}

	data.characterIDs[2] = robotSet[0];
	data.characterIDs[3] = robotSet[1];
	data.characterIDs[4] = robotSet[2];
	data.characterIDs[5] = robotSet[3];
	return setIndex;
}

void LOAD_Robots2P(struct BigHeader *bigfile, int p1, int p2, void (*callback)(struct LoadQueueSlot *))
{
	int setIndex = LOAD_SelectRobots2P(p1, p2);
	if (setIndex < 0)
	{
		return;
	}

	LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK + setIndex, NULL, callback);
}

void LOAD_Robots1P(int characterID)
{
	struct GameTracker *gGT = sdata->gGT;
	int nextCharacterID = 0;

	data.characterIDs[0] = characterID;

	for (int driverID = gGT->numPlyrCurrGame; driverID < LOAD_CHARACTER_ID_COUNT; driverID++)
	{
		while (LOAD_IsCharacterUsedByPlayer(nextCharacterID))
		{
			nextCharacterID++;
		}

		data.characterIDs[driverID] = nextCharacterID++;
	}
}

static void (*const LOAD_DriverMPK_SetPointer)(struct LoadQueueSlot *) = LOAD_QUEUE_CALLBACK_SET_POINTER;

int LOAD_DriverMPK(struct BigHeader *bigfile, int levelLOD, void (*callback)(struct LoadQueueSlot *))
{
	int i;
	int gameMode1;
	b32 extendedArcadeMultiplayer;
	b32 randomBotRace;

	struct GameTracker *gGT = sdata->gGT;
	gameMode1 = gGT->gameMode1;
	extendedArcadeMultiplayer = (gameMode1 & ARCADE_MODE) != 0 && g_config.extendedArcadeMultiplayer;

	randomBotRace = LOAD_IsRandomBotRace();
	LOAD_ResetExtraCharacterModels();
	if (randomBotRace)
	{
		LOAD_SelectRandomBots(bigfile);
	}

	if (extendedArcadeMultiplayer && gGT->numPlyrCurrGame > 1)
	{
		// The 1P arcade pack contains the full racer roster for AI opponents.
		if (!randomBotRace)
		{
			LOAD_Robots1P(data.characterIDs[0]);
		}
	}

	int lastFileIndexMPK;
	if (sdata->highDetailSplitScreenLevel)
	{
		// The 1P arcade pack contains P1 at player quality. Use the standalone
		// slots for P2-P4 instead of redundantly loading P1 again.
		for (i = 1; (i < gGT->numPlyrCurrGame) && (i <= LOAD_DRIVER_MODEL_EXTRA_COUNT); i++)
		{
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[i], &data.driverModelExtras[i - 1].fileBase, LOAD_DriverMPK_SetPointer);
		}

		if (!randomBotRace && !extendedArcadeMultiplayer && (gGT->numPlyrCurrGame == 2) && (LOAD_SelectRobots2P(data.characterIDs[0], data.characterIDs[1]) < 0))
		{
			return sdata->ptrMPK;
		}

		lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
		goto QueueLastPack;
	}

	// 3P/4P
	if ((u32)(levelLOD - LOAD_LEVEL_LOD_3P) < LOAD_LEVEL_LOD_3P4P_COUNT)
	{
		if ((gameMode1 & ARCADE_MODE) != 0 && g_config.extendedArcadeMultiplayer)
		{
			// P1 and all AI models come from the arcade pack. The standalone slots
			// provide the remaining human selections at multiplayer LOD.
			for (i = 1; i < gGT->numPlyrCurrGame; i++)
			{
				// low lod CTR model
				LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELLOW + data.characterIDs[i], &data.driverModelExtras[i - 1].fileBase,
				                 LOAD_DriverMPK_SetPointer);
			}

			lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
		}
		else
		{
			for (i = 0; i < LOAD_DRIVER_MODEL_EXTRA_COUNT; i++)
			{
				LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELLOW + data.characterIDs[i], &data.driverModelExtras[i].fileBase,
				                 LOAD_DriverMPK_SetPointer);
			}
			// load 4P MPK of fourth player
			lastFileIndexMPK = BI_4PARCADEPACK + data.characterIDs[3];
		}
	}

	else if (levelLOD == LOAD_LEVEL_LOD_1P)
	{
		if ((gameMode1 & (TIME_TRIAL | MAIN_MENU)) == TIME_TRIAL)
		{
			goto LoadHighAndPack;
		}

		if (
		    // adv/cutscene mpk when we just need text from MPK
		    ((gameMode1 & (GAME_CUTSCENE | ADVENTURE_ARENA)) != 0) ||

		    // credits
		    ((gGT->gameMode2 & CREDITS) != 0) ||

		    // adventure character select
		    (gGT->levelID == ADVENTURE_GARAGE))
		{
			lastFileIndexMPK = BI_ADVENTUREPACK + data.characterIDs[0];
			goto QueueLastPack;
		}

		if ((gameMode1 & ADVENTURE_BOSS) != 0)
		{
			goto LoadHighAndPack;
		}

		if (
		    // If you are in Adventure cup
		    ((gameMode1 & ADVENTURE_CUP) != 0) &&

		    // purple gem cup
		    (gGT->cup.cupID == CUP_ID_PURPLE_GEM))
		{
			// high lod model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0].fileBase, LOAD_DriverMPK_SetPointer);

			// pack of four AIs with bosses
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK + LOAD_PURPLE_GEM_CUP_AI_SET_INDEX, NULL, callback);

			data.characterIDs[1] = RIPPER_ROO;
			data.characterIDs[2] = PAPU_PAPU;
			data.characterIDs[3] = KOMODO_JOE;
			data.characterIDs[4] = PINSTRIPE;

			return sdata->ptrMPK;
		}

		if (!randomBotRace && ((gameMode1 & (TIME_TRIAL | MAIN_MENU)) != MAIN_MENU))
		{
			LOAD_Robots1P(data.characterIDs[0]);
		}

		// arcade mpk
		lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
	}

	else if ((levelLOD == LOAD_LEVEL_LOD_RELIC) || ((gameMode1 & TIME_TRIAL) != 0))
	{
	LoadHighAndPack:
		// Do NOT switch the order to optimize Relic,
		// if HI+IDs[1] and PACK+IDs[0] is loaded,
		// then mask-grab breaks for all characters
		// on Hot Air Skyway (except Crash Bandicoot)

		// Load Player 1 [0]
		LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELHI + data.characterIDs[0], &data.driverModelExtras[0].fileBase, LOAD_DriverMPK_SetPointer);

		// Load boss or ghost [1]
		lastFileIndexMPK = BI_TIMETRIALPACK + data.characterIDs[1];
	}

	// else if (levelLOD == LOAD_LEVEL_LOD_2P)
	else
	{
		if (extendedArcadeMultiplayer)
		{
			// P1 and every AI model come from the 1P arcade pack. P2 keeps the
			// normal 2P medium-detail model.
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELMED + data.characterIDs[1], &data.driverModelExtras[0].fileBase,
			                 LOAD_DriverMPK_SetPointer);
			lastFileIndexMPK = BI_1PARCADEPACK + data.characterIDs[0];
			goto QueueLastPack;
		}

		// med models
		for (i = 0; i < LOAD_MED_LOD_DRIVER_MODEL_EXTRA_COUNT; i++)
		{
			// med lod CTR model
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_RACERMODELMED + data.characterIDs[i], &data.driverModelExtras[i].fileBase, LOAD_DriverMPK_SetPointer);
		}

		if (randomBotRace)
		{
			// This pack supplies the shared 2P arcade assets; bot models above
			// are loaded individually and do not need its predefined roster.
			LOAD_AppendQueue(bigfile, LT_GETADDR, BI_2PARCADEPACK, NULL, callback);
		}
		else
		{
			LOAD_Robots2P(bigfile, data.characterIDs[0], data.characterIDs[1], callback);
		}
		return sdata->ptrMPK;
	}

QueueLastPack:
	LOAD_AppendQueue(bigfile, LT_GETADDR, lastFileIndexMPK, NULL, callback);
	return sdata->ptrMPK;
}

struct LngFile
{
	int numStrings;
	int offsetToPtrArr;
	char strings[1];
};

// param_1 - Pointer to "cd position of bigfile"
// param_2 - language index - 0 ja, 1 en, 2 en2, 3 fr, 4 de, 5 it, 6 es, 7 ne
void LOAD_LangFile(int bigfilePtr, int lang)
{
	struct LngFile *lngFile;
	u32 size;

	int i;
	int numStrings;
	char **strArray;


	if (sdata->lngFile == 0)
	{
		sdata->lngFile = MEMPACK_AllocMem(sdata->langBufferSize, NULL /* "lang buffer" */);
	}

	lngFile = sdata->lngFile;

	lngFile = LOAD_ReadFile_ex((struct BigHeader *)bigfilePtr, LT_SETADDR, BI_LANGUAGEFILE + lang, lngFile, &size, NULL);
	if (lngFile == NULL)
	{
		return;
	}

	numStrings = lngFile->numStrings;
	strArray = (char **)((u32)lngFile + lngFile->offsetToPtrArr);

	sdata->numLngStrings = numStrings;
	sdata->lngStrings = strArray;

	for (i = 0; i < numStrings; i++)
	{
		strArray[i] = (char *)((u32)strArray[i] + (u32)lngFile);
	}
}

int LOAD_GetBigfileIndex(u32 levelID, int lod, int fileIndexInGroup)
{
	if (levelID < NITRO_COURT)
	{
		return BI_ARCADETRACKS + levelID * LOAD_TRACK_FILES_PER_LOD_GROUP + sdata->levBigLodIndex[lod - 1] + fileIndexInGroup;
	}

	if ((u32)(levelID - NITRO_COURT) < LOAD_BATTLE_TRACK_COUNT)
	{
		return BI_BATTLETRACKS + (levelID - NITRO_COURT) * LOAD_TRACK_FILES_PER_LOD_GROUP + sdata->levBigLodIndex[lod - 1] + fileIndexInGroup;
	}

	if ((u32)(levelID - INTRO_RACE_TODAY) < LOAD_INTRO_CUTSCENE_COUNT)
	{
		return BI_CUTSCENES_INTRO + (levelID - INTRO_RACE_TODAY) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if ((u32)(levelID - OXIDE_ENDING) < LOAD_OUTRO_CUTSCENE_COUNT)
	{
		return BI_CUTSCENES_OUTRO + (levelID - OXIDE_ENDING) * LOAD_OUTRO_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if (levelID == ADVENTURE_GARAGE)
	{
		return BI_MAINMENUFILE + LOAD_MAIN_MENU_GARAGE_FILE_OFFSET + fileIndexInGroup;
	}

	if (levelID == NAUGHTY_DOG_CRATE)
	{
		return BI_NDBOX + fileIndexInGroup;
	}

	if ((u32)(levelID - CREDITS_CRASH) < LOAD_CREDIT_LEVEL_COUNT)
	{
		return BI_CREDITS + (levelID - CREDITS_CRASH) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
	}

	if (levelID == MAIN_MENU_LEVEL)
	{
		return BI_MAINMENUFILE + fileIndexInGroup;
	}

	if (levelID == SCRAPBOOK)
	{
		return BI_SCRAPBOOK + fileIndexInGroup;
	}

	return BI_ADVENTUREHUB + (levelID - GEM_STONE_VALLEY) * LOAD_CUTSCENE_FILES_PER_LEVEL + fileIndexInGroup;
}
