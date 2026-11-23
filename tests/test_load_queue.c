#include <shared/ctr_test.h>
#include <common.h>
#include <race_field.h>
#include <game/LOAD/LOAD_BotModels.c>
#include <game/LOAD/LOAD_Queue.c>

struct sData sdata_static;
struct Data data;

static struct LoadBotModelSlot s_botModelArena[RACE_FIELD_CONFIG_MAX_RACERS];
static s32 s_botAllocationSize;

// The queue test exercises dispatch order, not disc, track or arena I/O.
const struct NativeTrackRegistry *NativeTrackLibrary_Current(void) { return NULL; }
struct NativeTrackFile *NativeTrackFile_Open(const struct NativeTrackRegistry *registry, int levelID, enum NativeTrackFileRole role)
{
	(void)registry; (void)levelID; (void)role; return NULL;
}
size_t NativeTrackFile_Size(const struct NativeTrackFile *file) { (void)file; return 0; }
size_t NativeTrackFile_SectorBytes(const struct NativeTrackFile *file) { (void)file; return 0; }
int NativeTrackFile_Read(struct NativeTrackFile *file, void *destination, size_t capacity)
{
	(void)file; (void)destination; (void)capacity; return 0;
}
void NativeTrackFile_Close(struct NativeTrackFile *file) { (void)file; }
s32 MEMPACK_GetFreeBytes(void) { return 0; }
s32 MEMPACK_PushState(void) { return 0; }
void MEMPACK_PopState(void) {}
void *MEMPACK_AllocMem(s32 size, const char *name)
{
	(void)name;
	s_botAllocationSize = size;
	if (size <= 0 || (size_t)size > sizeof(s_botModelArena))
	{
		CtrTest_Bail("bot model request must fit the test arena");
	}
	return s_botModelArena;
}
void *MEMPACK_ReallocMem(s32 size) { (void)size; return NULL; }
void LOAD_DramFileCallback(struct LoadQueueSlot *slot) { (void)slot; }
void LOAD_VramFileCallback(struct LoadQueueSlot *slot) { (void)slot; }
void MainRaceTrack_RequestLoad(s16 levelID) { (void)levelID; }
void *LOAD_ReadFile_ex(struct BigHeader *bigfile, u32 type, int index, void *destination, u32 *size,
                       void (*callback)(struct LoadQueueSlot *))
{
	(void)bigfile; (void)type; (void)index; (void)size; (void)callback; return destination;
}
void *LOAD_DramFile(void *bigfile, int index, void *destination, u32 *size, int callback)
{
	(void)bigfile; (void)index; (void)size; (void)callback; return destination;
}
void *LOAD_VramFile(void *bigfile, int index, void *destination, u32 *size, int callback)
{
	(void)bigfile; (void)index; (void)size; (void)callback; return destination;
}

static void QueueCallback(struct LoadQueueSlot *slot)
{
	(void)slot;
}

static void TestStandaloneBotModels(void)
{
	CTR_TEST_SECTION("standalone bots allocate only queued destinations and leave room for the shared pack");
	s16 ids[RACE_FIELD_CONFIG_MAX_RACERS - 1];
	for (int i = 0; i < (int)len(ids); i++)
	{
		ids[i] = i + 1;
	}
	sdata->queueLength = 0;
	LOAD_QueueExtraCharacterModels(NULL, ids, 0, BI_RACERMODELHI);
	CTR_TEST_CHECK(s_botAllocationSize == 0 && sdata->standaloneBotModels == NULL, "empty batches do not allocate");

	const int prefix = LOAD_QUEUE_EXISTING_BATCH_BUDGET + 1;
	for (int i = 0; i < prefix; i++)
	{
		LOAD_AppendQueue(NULL, LT_GETADDR, i, NULL, QueueCallback);
	}
	CTR_TEST_CHECK_EQ(LOAD_QueueExtraCharacterModels(NULL, ids, len(ids), BI_RACERMODELHI), len(ids),
	                  "the caller can admit all requested bots when the batch fits");
	CTR_TEST_CHECK_EQ(s_botAllocationSize, sizeof(*s_botModelArena) * len(ids), "metadata allocation tracks the number of requested files");
	CTR_TEST_CHECK_EQ(sdata->standaloneBotModelCount, len(ids), "every bot fits after pending requests and one human model");
	for (int i = 0; i < (int)len(ids); i++)
	{
		const struct LoadQueueSlot *request = &sdata->queueSlots[prefix + i];
		CTR_TEST_CHECK(request->subfileIndex == (u32)(BI_RACERMODELHI + ids[i]) &&
		                   request->ptrDestination == &s_botModelArena[i].asset.fileBase &&
		                   request->callbackFuncPtr == LOAD_QUEUE_CALLBACK_SET_POINTER,
		               "each bot request writes to its own MEMPACK slot");
	}
	LOAD_AppendQueue(NULL, LT_GETADDR, BI_1PARCADEPACK, NULL, QueueCallback);
	CTR_TEST_CHECK_EQ(sdata->queueSlots[LOAD_QUEUE_SLOT_COUNT - 1].subfileIndex, BI_1PARCADEPACK, "the shared pack is queued last");

	char modelName[] = "last bot";
	GAME_CHARACTER_METADATA[ids[len(ids) - 1]].name_Debug = modelName;
	for (int i = 0; i < (int)len(ids) - 1; i++)
	{
		GAME_CHARACTER_METADATA[ids[i]].name_Debug = "other bot";
	}
	_Alignas(struct Model) u8 file[LOAD_MODEL_FILE_HEADER_BYTES + sizeof(struct Model)];
	void **lastDestination = sdata->queueSlots[prefix + len(ids) - 1].ptrDestination;
	*lastDestination = file;
	LOAD_FinalizeExtraCharacterModels();
	CTR_TEST_CHECK((void *)LOAD_GetExtraCharacterModelByName(modelName) == file + LOAD_MODEL_FILE_HEADER_BYTES,
	               "model lookup finds a finalized slot beyond the old eight-slot limit");
	CTR_TEST_CHECK(LOAD_GetExtraCharacterModelByName("other bot") == NULL, "an unloaded slot remains empty");
	LOAD_ResetExtraCharacterModels();
	CTR_TEST_CHECK(sdata->standaloneBotModels == NULL && sdata->standaloneBotModelCount == 0 &&
	                   LOAD_GetExtraCharacterModelByName(modelName) == NULL,
	               "the next level cannot see the old allocation");

	sdata->queueLength = LOAD_QUEUE_SLOT_COUNT - 2;
	CTR_TEST_CHECK_EQ(LOAD_QueueExtraCharacterModels(NULL, ids, len(ids), BI_RACERMODELHI), 1,
	                  "the caller is told how many requests survived queue pressure");
	CTR_TEST_CHECK_EQ(s_botAllocationSize, sizeof(*s_botModelArena), "a crowded queue allocates only the slot it can use");
	CTR_TEST_CHECK_EQ(sdata->queueLength, LOAD_QUEUE_SLOT_COUNT - 1, "the pack retains its place even when bot requests have to be capped");
}

int main(void)
{
	int destinations[LOAD_QUEUE_SLOT_COUNT];

	CTR_TEST_SECTION("extended queue preserves order and request fields");
	CTR_TEST_CHECK((int)LOAD_QUEUE_RACER_MODEL_BUDGET >= (int)RACE_FIELD_CONFIG_MAX_RACERS,
	               "the model budget covers every configured racer");
	CTR_TEST_CHECK(LOAD_QUEUE_SLOT_COUNT >= RACE_FIELD_CONFIG_MAX_RACERS + LOAD_QUEUE_RACER_PACK_BUDGET + LOAD_QUEUE_EXISTING_BATCH_BUDGET,
	               "the queue fits sixteen models, their pack and a pending podium-sized batch");
	for (int i = 0; i < LOAD_QUEUE_SLOT_COUNT; i++)
	{
		LOAD_AppendQueue(NULL, LT_GETADDR, i, &destinations[i], QueueCallback);
	}
	CTR_TEST_CHECK_EQ(sdata->queueLength, LOAD_QUEUE_SLOT_COUNT, "all queue slots accept requests");
	for (int i = 0; i < LOAD_QUEUE_SLOT_COUNT; i++)
	{
		struct LoadQueueSlot *slot = &sdata->queueSlots[i];
		CTR_TEST_CHECK_EQ(slot->subfileIndex, (u32)i, "each request stays in insertion order");
		CTR_TEST_CHECK(slot->ptrDestination == &destinations[i] && slot->callbackFuncPtr == QueueCallback,
		               "each request retains its destination and callback");
	}
	LOAD_AppendQueue(NULL, LT_GETADDR, 999, NULL, NULL);
	CTR_TEST_CHECK_EQ(sdata->queueLength, LOAD_QUEUE_SLOT_COUNT, "a full queue does not overflow");
	CTR_TEST_CHECK_EQ(sdata->queueSlots[LOAD_QUEUE_SLOT_COUNT - 1].subfileIndex, LOAD_QUEUE_SLOT_COUNT - 1,
	                  "a rejected request does not overwrite the last slot");

	CTR_TEST_SECTION("pending requests shift across the retail boundary");
	for (int removed = 0; removed < LOAD_QUEUE_SLOT_COUNT; removed++)
	{
		CTR_TEST_CHECK_EQ(sdata->queueSlots[0].subfileIndex, (u32)removed, "the next request is the oldest pending request");
		sdata->queueReady = 1;
		LOAD_NextQueuedFile();
		CTR_TEST_CHECK_EQ(data.currSlot.subfileIndex, (u32)removed, "dispatch selects the oldest request");
	}
	CTR_TEST_CHECK_EQ(sdata->queueLength, 0, "all requests can be removed");
	LOAD_AppendQueue(NULL, LT_GETADDR, 123, NULL, QueueCallback);
	CTR_TEST_CHECK_EQ(sdata->queueSlots[0].subfileIndex, 123, "a drained queue can be reused");
	TestStandaloneBotModels();
	return CtrTest_Summary();
}
