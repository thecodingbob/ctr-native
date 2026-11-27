#include <common.h>
#include <custom-characters/character_resources.h>
#include <platform/custom-characters/native_character_model.h>
#include <platform/native_external_file.h>
#include <platform/native_log.h>
#include <game/custom-characters/character_policy.h>

void LOAD_ResetCharacterModelCache(void)
{
	sdata->customPlayerModel = NULL;
	sdata->customPreviewModel = NULL;
	sdata->customPreviewAttempted = false;
	sdata->customPlayerModelIdentity[0] = '\0';
	sdata->customPreviewIdentity[0] = '\0';
}

static struct Model *LOAD_ReadCustomCharacterModel(const char *path, int reservedBytes)
{
	struct NativeExternalFile *file = NativeExternalFile_Open(path);
	if (file == NULL)
	{
		Platform_LogWarn("[Custom Characters] Cannot open %s; using the retail base\n", path);
		return NULL;
	}
	size_t capacity = NativeExternalFile_SectorBytes(file);
	size_t size = NativeExternalFile_Size(file);
	size_t allocation = MEMPACK_ALIGN_SIZE(size);
	int freeBytes = MEMPACK_GetFreeBytes();
	if (reservedBytes < 0 || freeBytes < reservedBytes || allocation > (size_t)(freeBytes - reservedBytes))
	{
		NativeExternalFile_Close(file);
		Platform_LogWarn("[Custom Characters] Not enough memory for %s; using the retail base\n", path);
		return NULL;
	}
	void *temporary = malloc(capacity);
	int valid = temporary != NULL && NativeCharacterModel_Read(file, temporary, capacity);
	NativeExternalFile_Close(file);
	struct Model *model = NULL;
	if (valid)
	{
		char *buffer = MEMPACK_AllocMem((int)size, "custom player model");
		memcpy(buffer, temporary, size);
		char *payload = buffer + sizeof(u32);
		struct DramPointerMap *map = (struct DramPointerMap *)(payload + *(u32 *)buffer);
		LOAD_RunPtrMap(payload, (int *)DRAM_GETOFFSETS(map), map->numBytes / sizeof(u32));
		model = (struct Model *)payload;
	}
	free(temporary);
	if (model == NULL)
	{
		Platform_LogWarn("[Custom Characters] Cannot load %s; using the retail base\n", path);
	}
	return model;
}

enum CharacterModelContext
{
	CHARACTER_MODEL_DRIVER,
	CHARACTER_MODEL_PREVIEW,
};

struct CharacterModelRequest
{
	int baseID;
	const char *identity;
	enum CharacterModelContext context;
};

extern unsigned char oxideModel[];

static struct Model *LOAD_GetOxidePreview(void)
{
	static bool initialized;
	struct Model *model = (struct Model *)&oxideModel[sizeof(u32)];
	if (!initialized)
	{
		u32 mapOffset;
		u32 pointerBytes;
		memcpy(&mapOffset, oxideModel, sizeof(mapOffset));
		u8 *map = (u8 *)model + mapOffset;
		memcpy(&pointerBytes, map, sizeof(pointerBytes));
		LOAD_RunPtrMap((char *)model, (int *)(map + sizeof(pointerBytes)), pointerBytes / sizeof(u32));
		initialized = true;
	}
	return model;
}

static struct Model *LOAD_FindCharacterModel(struct Model **models, const char *name)
{
	if (models != NULL)
	{
		for (; *models != NULL; models++)
		{
			if (strncmp((*models)->name, name, sizeof((*models)->name)) == 0) { return *models; }
		}
	}
	return NULL;
}

struct Model *LOAD_GetRetailCharacterModel(const char *name)
{
	for (size_t i = 0; i < len(data.driverModelExtras); i++)
	{
		struct Model *model = data.driverModelExtras[i].model;
		if (model != NULL && strncmp(model->name, name, sizeof(model->name)) == 0) { return model; }
	}
	struct Model *model = LOAD_FindCharacterModel((struct Model **)sdata->PLYROBJECTLIST, name);
	return model != NULL ? model : LOAD_GetExtraCharacterModelByName((char *)name);
}

static struct Model *LOAD_GetRetailPreviewModel(int baseID)
{
	const char *name = GAME_CHARACTER_METADATA[baseID].name_Debug;
	struct Level *level = sdata->gGT->level1;
	struct Model *model = LOAD_FindCharacterModel(level != NULL ? level->ptrModelsPtrArray : NULL, name);
	return model == NULL && baseID == NITROS_OXIDE ? LOAD_GetOxidePreview() : model;
}

static const char *LOAD_GetCustomModelPath(const struct CharacterModelRequest *request)
{
	if (request->identity == NULL || request->identity[0] == '\0') { return NULL; }
	if (request->context == CHARACTER_MODEL_DRIVER)
	{
		return sdata->customPlayerModelPath[0] != '\0' ? sdata->customPlayerModelPath : NULL;
	}
	for (size_t i = 0; i < NativeCharacterLibrary_Count(); i++)
	{
		const struct NativeCharacterMetadata *entry = NativeCharacterLibrary_Get(i);
		if (strcmp(entry->identity, request->identity) == 0) { return entry->modelHi; }
	}
	return NULL;
}

static struct Model *LOAD_ResolveCharacterModel(const struct CharacterModelRequest *request, bool load, int reservedBytes)
{
	if (request->baseID < 0 || request->baseID >= GAME_CHARACTER_COUNT) { return NULL; }
	const char *path = LOAD_GetCustomModelPath(request);
	if (path != NULL)
	{
		bool preview = request->context == CHARACTER_MODEL_PREVIEW;
		struct Model **cached = preview ? &sdata->customPreviewModel : &sdata->customPlayerModel;
		char *identity = preview ? sdata->customPreviewIdentity : sdata->customPlayerModelIdentity;
		if (strcmp(identity, request->identity) != 0)
		{
			*cached = NULL;
			identity[0] = '\0';
			if (preview) { sdata->customPreviewAttempted = false; }
		}
		if (load && *cached == NULL && (!preview || !sdata->customPreviewAttempted))
		{
			memcpy(identity, request->identity, strlen(request->identity) + 1);
			if (preview) { sdata->customPreviewAttempted = true; }
			*cached = LOAD_ReadCustomCharacterModel(path, reservedBytes);
		}
		if (*cached != NULL) { return *cached; }
	}
	return request->context == CHARACTER_MODEL_PREVIEW ? LOAD_GetRetailPreviewModel(request->baseID) :
	       LOAD_GetRetailCharacterModel(GAME_CHARACTER_METADATA[request->baseID].name_Debug);
}

static struct CharacterModelRequest LOAD_DriverModelRequest(int driverID, int baseID)
{
	bool menu = (sdata->gGT->gameMode1 & MAIN_MENU) != 0;
	bool custom = !menu && driverID == 0 && CustomCharacter_IsSelected() && baseID == sdata->customPlayerBaseID;
	return (struct CharacterModelRequest){
		.baseID = baseID, .context = CHARACTER_MODEL_DRIVER,
		.identity = custom ? sdata->customPlayerIdentity : NULL,
	};
}

struct Model *LOAD_GetDriverModel(int driverID, int baseID)
{
	struct CharacterModelRequest request = LOAD_DriverModelRequest(driverID, baseID);
	// Spawning only resolves prepared assets; it never allocates from the race pools' bookmark.
	return LOAD_ResolveCharacterModel(&request, false, 0);
}

struct Model *LOAD_GetCharacterPreviewModel(int driverID, int baseID, const char *identity)
{
	if (driverID < 0 || driverID >= (int)len(sdata->gGT->pushBuffer))
	{
		return NULL;
	}
	const struct CharacterModelRequest request = {
		.baseID = baseID, .identity = identity, .context = CHARACTER_MODEL_PREVIEW,
	};
	return LOAD_ResolveCharacterModel(&request, true, 0);
}

void LOAD_FinalizeCharacterModels(void)
{
	struct GameTracker *gGT = sdata->gGT;
	int reservedBytes = MainInit_GetPlayerModelMemoryReserve(gGT);
	for (int driverID = 0; driverID < gGT->numPlyrCurrGame; driverID++)
	{
		struct CharacterModelRequest request = LOAD_DriverModelRequest(driverID, data.characterIDs[driverID]);
		LOAD_ResolveCharacterModel(&request, true, reservedBytes);
	}
}
