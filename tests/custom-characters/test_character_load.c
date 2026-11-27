#include <shared/ctr_test_scratch.h>
#include <common.h>
#include <custom-characters/character_resources.h>
#include <platform/custom-characters/native_character_library.h>

struct sData sdata_static;
struct Data data;
static u32 s_buffer[1024];
static int s_freeBytes;
static int s_allocations;
static int s_relocations;
static int s_warnings;
static int s_reserve;
unsigned char oxideModel[128];
static struct NativeCharacterMetadata s_character;
static struct Model s_retailModel;

size_t NativeCharacterLibrary_Count(void) { return 1; }

const struct NativeCharacterMetadata *NativeCharacterLibrary_Get(size_t index)
{
	return index == 0 ? &s_character : NULL;
}

struct Model *LOAD_GetExtraCharacterModelByName(char *name)
{
	(void)name;
	return &s_retailModel;
}

s32 MEMPACK_GetFreeBytes(void) { return s_freeBytes; }

void *MEMPACK_AllocMem(s32 size, const char *name)
{
	(void)name;
	int aligned = MEMPACK_ALIGN_SIZE(size);
	if (aligned > s_freeBytes || aligned > (int)sizeof(s_buffer))
	{
		CtrTest_Bail("the loader called the trapping allocator without enough memory");
	}
	s_allocations++;
	s_freeBytes -= aligned;
	return s_buffer;
}

void LOAD_RunPtrMap(char *origin, int *offsets, int count)
{
	s_relocations++;
	for (int i = 0; i < count; i++)
	{
		u32 *slot = (u32 *)(origin + (offsets[i] & ~3));
		*slot += (u32)origin;
	}
}

int MainInit_GetPlayerModelMemoryReserve(struct GameTracker *tracker)
{
	(void)tracker;
	return s_reserve;
}

void Platform_LogWarn(const char *format, ...)
{
	(void)format;
	s_warnings++;
}

#include <game/LOAD/LOAD_CharacterModels.c>

static void Write32(unsigned char *bytes, u32 value)
{
	for (int i = 0; i < 4; i++)
	{
		bytes[i] = (unsigned char)(value >> (i * 8));
	}
}

static void WriteModel(const char *path, bool valid)
{
	// 101 source bytes require 104 arena bytes, not the 2048-byte host read buffer.
	unsigned char bytes[101] = {0};
	Write32(bytes, 88);
	bytes[4 + 18] = valid ? 1 : 0;
	Write32(bytes + 4 + 20, 24);
	Write32(bytes + 92, 4);
	Write32(bytes + 96, 20);
	FILE *file = fopen(path, "wb");
	if (file == NULL || fwrite(bytes, 1, sizeof(bytes), file) != sizeof(bytes) || fclose(file) != 0)
	{
		CtrTest_Bail("could not write the model fixture");
	}
}

int main(void)
{
	char directory[1024], path[1200];
	CtrTestScratch_Open(directory, sizeof(directory), "test-character-load", "model.ctr");
	snprintf(path, sizeof(path), "%smodel.ctr", directory);
	WriteModel(path, true);

	CTR_TEST_SECTION("custom model memory reserve");
	s_freeBytes = 1024 + 104;
	struct Model *model = LOAD_ReadCustomCharacterModel(path, 1024);
	CTR_TEST_CHECK(model == (struct Model *)((char *)s_buffer + 4), "a model fitting exactly beside the reserve is loaded");
	CTR_TEST_CHECK(s_freeBytes == 1024 && s_allocations == 1 && s_relocations == 1, "the aligned allocation retains the complete reserve");
	CTR_TEST_CHECK(model != NULL && model->headers == (struct ModelHeader *)((char *)model + 24), "accepted models are relocated before use");

	s_freeBytes = 1024 + 103;
	CTR_TEST_CHECK(LOAD_ReadCustomCharacterModel(path, 1024) == NULL, "alignment cannot consume the last bytes of the reserve");
	CTR_TEST_CHECK(s_freeBytes == 1127 && s_allocations == 1 && s_relocations == 1, "a rejected model never reaches allocation or relocation");
	s_freeBytes = 1023;
	CTR_TEST_CHECK(LOAD_ReadCustomCharacterModel(path, 1024) == NULL, "a reserve larger than free memory cannot wrap into available space");
	CTR_TEST_CHECK(LOAD_ReadCustomCharacterModel(path, -1) == NULL, "a negative reserve is rejected");
	s_freeBytes = 104;
	CTR_TEST_CHECK(LOAD_ReadCustomCharacterModel(path, 0) != NULL && s_freeBytes == 0, "a menu preview only needs the actual aligned model allocation");

	CTR_TEST_SECTION("race admission and fallback");
	static struct GameTracker tracker;
	sdata->gGT = &tracker;
	tracker.gameMode1 = ARCADE_MODE;
	tracker.numPlyrCurrGame = 1;
	data.characterIDs[0] = PAPU_PAPU;
	sdata->customPlayerBaseID = PAPU_PAPU;
	data.MetaDataCharacters[PAPU_PAPU].name_Debug = "papu";
	strcpy(sdata->customPlayerIdentity, "fixture");
	if (strlen(path) >= sizeof(sdata->customPlayerModelPath))
	{
		CtrTest_Bail("scratch path does not fit in the model path field");
	}
	memcpy(sdata->customPlayerModelPath, path, strlen(path) + 1);
	s_reserve = 1024;
	s_freeBytes = s_reserve + 104;
	LOAD_FinalizeCharacterModels();
	CTR_TEST_CHECK(sdata->customPlayerModel != NULL && s_freeBytes == s_reserve, "race loading uses the remaining level initialization budget");
	int preparedAllocations = s_allocations;
	CTR_TEST_CHECK(LOAD_GetDriverModel(0, PAPU_PAPU) == sdata->customPlayerModel && s_allocations == preparedAllocations,
	               "spawning resolves the prepared custom model without allocating");
	CTR_TEST_CHECK(LOAD_GetDriverModel(1, PAPU_PAPU) == &s_retailModel,
	               "another driver with the same base keeps the retail model");
	LOAD_ResetCharacterModelCache();
	s_freeBytes = s_reserve + 103;
	LOAD_FinalizeCharacterModels();
	CTR_TEST_CHECK(sdata->customPlayerModel == NULL && s_freeBytes == s_reserve + 103, "an oversized model leaves the retail fallback and the arena intact");
	CTR_TEST_CHECK(LOAD_GetDriverModel(0, PAPU_PAPU) == &s_retailModel,
	               "the same driver resolver falls back after a rejected custom load");

	CTR_TEST_SECTION("preview requests share resolution but retain a separate cache");
	strcpy(s_character.identity, "fixture");
	memcpy(s_character.modelHi, path, strlen(path) + 1);
	strcpy(s_retailModel.name, "papu");
	struct Model *previewModels[] = {&s_retailModel, NULL};
	struct Level menu = {.ptrModelsPtrArray = previewModels};
	tracker.level1 = &menu;
	s_freeBytes = 104;
	struct Model *preview = LOAD_GetCharacterPreviewModel(0, PAPU_PAPU, "fixture");
	CTR_TEST_CHECK(preview != NULL && preview != &s_retailModel && s_freeBytes == 0,
	               "a preview uses the same custom reader without a race reserve");
	preparedAllocations = s_allocations;
	CTR_TEST_CHECK(LOAD_GetCharacterPreviewModel(0, PAPU_PAPU, "fixture") == preview && s_allocations == preparedAllocations,
	               "repeated preview requests reuse the identity's model");
	CTR_TEST_CHECK(LOAD_GetCharacterPreviewModel(0, PAPU_PAPU, NULL) == &s_retailModel,
	               "retail preview identity does not inherit the custom model");
	CTR_TEST_CHECK(LOAD_GetCharacterPreviewModel(0, PAPU_PAPU, "missing") == &s_retailModel,
	               "an unavailable custom preview uses the menu's retail model");
	CTR_TEST_CHECK(LOAD_GetCharacterPreviewModel(-1, PAPU_PAPU, "fixture") == NULL &&
	               LOAD_GetCharacterPreviewModel(4, PAPU_PAPU, "fixture") == NULL,
	               "preview requests reject player indices outside the viewport slots");
	sdata->customPlayerModel = preview;
	strcpy(sdata->customPlayerModelIdentity, "fixture");
	LOAD_ResetCharacterModelCache();
	CTR_TEST_CHECK(sdata->customPlayerModel == NULL && sdata->customPreviewModel == NULL &&
	               !sdata->customPreviewAttempted && sdata->customPlayerModelIdentity[0] == '\0' &&
	               sdata->customPreviewIdentity[0] == '\0',
	               "the model owner resets both cache contexts and their identities");
	CTR_TEST_CHECK(strcmp(sdata->customPlayerIdentity, "fixture") == 0 && sdata->customPlayerBaseID == PAPU_PAPU &&
	               strcmp(sdata->customPlayerModelPath, path) == 0,
	               "level cache teardown preserves the selected character snapshot");

	WriteModel(path, false);
	s_freeBytes = 4096;
	int allocations = s_allocations;
	CTR_TEST_CHECK(LOAD_ReadCustomCharacterModel(path, 1024) == NULL && s_allocations == allocations, "invalid models are rejected before committing arena memory");
	CTR_TEST_CHECK(s_warnings == 5, "each rejected model emits one fallback warning");
	CtrTestScratch_Remove("test-character-load", "model.ctr");
	return CtrTest_Summary();
}
