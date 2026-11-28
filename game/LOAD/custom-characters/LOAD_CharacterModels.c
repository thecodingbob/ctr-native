#include <common.h>
#include <custom-characters/character_resources.h>
#include <platform/custom-characters/native_character_model.h>
#include <platform/native_external_file.h>
#include <platform/native_log.h>
#include <game/custom-characters/character_policy.h>

void LOAD_ResetCharacterModelCache(void)
{
	memset(sdata->customDriverModels, 0, sizeof(sdata->customDriverModels));
	memset(sdata->customPreviewModels, 0, sizeof(sdata->customPreviewModels));
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
	const char *modelPath;
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
		return request->modelPath != NULL && request->modelPath[0] != '\0' ? request->modelPath : NULL;
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
		struct CustomCharacterModelCache *caches = preview ? sdata->customPreviewModels : sdata->customDriverModels;
		int count = preview ? (int)len(sdata->customPreviewModels) : (int)len(sdata->customDriverModels);
		struct CustomCharacterModelCache *cached = NULL;
		for (int index = 0; index < count; index++)
		{
			if (strcmp(caches[index].identity, request->identity) == 0)
			{
				cached = &caches[index];
				break;
			}
			if (cached == NULL && caches[index].identity[0] == '\0') { cached = &caches[index]; }
		}
		if (cached != NULL && load && !cached->attempted)
		{
			memcpy(cached->identity, request->identity, strlen(request->identity) + 1);
			cached->attempted = true;
			cached->model = LOAD_ReadCustomCharacterModel(path, reservedBytes);
		}
		if (cached != NULL && strcmp(cached->identity, request->identity) == 0 && cached->model != NULL) { return cached->model; }
	}
	return request->context == CHARACTER_MODEL_PREVIEW ? LOAD_GetRetailPreviewModel(request->baseID) :
	       LOAD_GetRetailCharacterModel(GAME_CHARACTER_METADATA[request->baseID].name_Debug);
}

static struct CharacterModelRequest LOAD_DriverModelRequest(int driverID, int baseID)
{
	bool menu = (sdata->gGT->gameMode1 & MAIN_MENU) != 0;
	bool custom = !menu && CustomCharacter_IsSelected(driverID) && baseID == sdata->customCharacters[driverID].baseID;
	return (struct CharacterModelRequest){
		.baseID = baseID, .context = CHARACTER_MODEL_DRIVER,
		.identity = custom ? sdata->customCharacters[driverID].identity : NULL,
		.modelPath = custom ? sdata->customCharacters[driverID].modelPath : NULL,
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
	if ((gGT->gameMode1 & MAIN_MENU) == 0) { LOAD_FinalizeCharacterIcons(); }
	int reservedBytes = MainInit_GetPlayerModelMemoryReserve(gGT);
	for (int driverID = 0; driverID < (int)len(sdata->customCharacters); driverID++)
	{
		struct CharacterModelRequest request = LOAD_DriverModelRequest(driverID, data.characterIDs[driverID]);
		LOAD_ResolveCharacterModel(&request, true, reservedBytes);
	}
}
