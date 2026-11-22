#include <shared/ctr_test.h>
#include <common.h>
#include <race_field.h>
#include <game/LOAD/LOAD_Queue.c>

struct sData sdata_static;
struct Data data;

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
void *MEMPACK_AllocMem(s32 size, const char *name) { (void)size; (void)name; return NULL; }
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
	return CtrTest_Summary();
}
