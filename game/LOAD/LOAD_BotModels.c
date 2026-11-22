#include <common.h>

static void LOAD_ResetExtraCharacterModels(void)
{
	sdata->standaloneBotModels = NULL;
	sdata->standaloneBotModelCount = 0;
}

// Called once per driver batch, after the human requests and before the final pack.
static void LOAD_QueueExtraCharacterModels(struct BigHeader *bigfile, const s16 *characterIDs, int characterCount, int modelFileIndex)
{
	const int available = LOAD_QUEUE_SLOT_COUNT - sdata->queueLength - LOAD_QUEUE_RACER_PACK_BUDGET;
	if (characterCount > available)
	{
		characterCount = available;
	}
	if (characterCount <= 0)
	{
		return;
	}

	struct LoadBotModelSlot *slots = MEMPACK_AllocMem(characterCount * sizeof(*slots), "standalone bot models");
	sdata->standaloneBotModels = slots;
	sdata->standaloneBotModelCount = characterCount;
	for (int i = 0; i < characterCount; i++)
	{
		slots[i].asset.fileBase = NULL;
		slots[i].characterID = characterIDs[i];
		LOAD_AppendQueue(bigfile, LT_GETADDR, modelFileIndex + characterIDs[i], &slots[i].asset.fileBase,
		                 LOAD_QUEUE_CALLBACK_SET_POINTER);
	}
}

struct Model *LOAD_GetExtraCharacterModelByName(char *searchName)
{
	for (int i = 0; i < sdata->standaloneBotModelCount; i++)
	{
		const struct LoadBotModelSlot *slot = &sdata->standaloneBotModels[i];
		if (strcmp(GAME_CHARACTER_METADATA[slot->characterID].name_Debug, searchName) == 0)
		{
			return slot->asset.model;
		}
	}
	return NULL;
}

void LOAD_FinalizeExtraCharacterModels(void)
{
	for (int i = 0; i < sdata->standaloneBotModelCount; i++)
	{
		DriverModelExtraSlot *asset = &sdata->standaloneBotModels[i].asset;
		if (asset->fileBase != NULL)
		{
			asset->model = (struct Model *)((u8 *)asset->fileBase + LOAD_MODEL_FILE_HEADER_BYTES);
		}
	}
}
