#include <shared/ctr_test.h>
#include <common.h>

// Supply scratch/arena storage without the game, GTE or assets. The seven
// writer entry points and serialization below are the production implementation.
NativeConfig g_config;
struct sData sdata_static;
static struct DrawLevelOvr1PStableScratch s_scratch;
static union
{
	u32 alignment;
	u8 bytes[256];
} s_storage;
static u8 *s_start;
static u8 *s_cursor;
static u8 *s_end;
static u32 s_ot[16];
static struct PushBuffer s_pb;
static struct QuadBlock s_block;
static u32 s_resolvedDepth;
static int s_pgxpCopies;

static struct DrawLevelOvr1PStableScratch *DrawLevelOvr1P_Scratch(void) { return &s_scratch; }
static u8 *DrawLevelOvr1P_GetClipRecordStart(void) { return s_start; }
static u8 *DrawLevelOvr1P_GetClipRecordCursor(void) { return s_cursor; }
static u8 *DrawLevelOvr1P_GetClipRecordEnd(void) { return s_end; }
static void DrawLevelOvr1P_SetClipRecordCursor(u8 *cursor) { s_cursor = cursor; }

static void DrawLevelOvr1P_CopyColorWord(u8 *dst, const u8 *src)
{
	memcpy(dst, src, 4);
}

void NativePgxp_CopyPosition(const void *dstKey, const void *srcKey, const s16 *vector)
{
	(void)dstKey;
	(void)srcKey;
	(void)vector;
	s_pgxpCopies++;
}

static int DrawLevelOvr1P_ResolveProjectedOtIndex(const struct QuadBlock *block, const struct DrawLevelOvr1PScratchVertex *projected,
                                               u32 maxDepth, int faceIndex, int otIndexOverride)
{
	(void)block;
	(void)projected;
	(void)faceIndex;
	s_resolvedDepth = maxDepth;
	return otIndexOverride;
}

#include "game/226/226_clip_records.h"

enum TestWriter
{
	WRITER_GENERIC,
	WRITER_WATER,
	WRITER_WATER_INHERITED,
	WRITER_GROUND_4X1,
	WRITER_GROUND_4X2,
	WRITER_DYNAMIC,
	WRITER_QUAD_4X4,
	WRITER_COUNT,
};

static const char *const s_writerNames[] = {"generic", "water", "water inherited", "ground 4x1", "ground 4x2", "dynamic", "quad 4x4"};
static const int s_indices[] = {2, 0, 3, 1};

static int Write(enum TestWriter writer, const struct DrawLevelOvr1PScratchVertex *vertices, int count, u32 *otEntry)
{
	switch (writer)
	{
	case WRITER_GENERIC:
		return DrawLevelOvr1P_WriteRenderedClippedRecordAtOt(&s_pb, &s_block, vertices, s_indices, count, 0, NULL, 7);
	case WRITER_WATER:
		return DrawLevelOvr1P_WriteWaterRenderedClippedRecordAtOt(&s_pb, &s_block, vertices, s_indices, count, 0, 7);
	case WRITER_WATER_INHERITED:
		return Ovr226_800a34d4_WriteWaterRenderedClippedRecordAtOtEntry(&s_pb, vertices, s_indices, count, otEntry);
	case WRITER_GROUND_4X1:
		return Ovr226_800a4dcc_WriteGround4x1RenderedClippedRecordAtOtEntry(&s_pb, &s_block, vertices, s_indices, count, otEntry);
	case WRITER_GROUND_4X2:
		return Ovr226_800a6d6c_WriteGround4x2RenderedClippedRecordAtOtEntry(&s_pb, &s_block, vertices, s_indices, count, otEntry);
	case WRITER_DYNAMIC:
		return Ovr226_800a898c_WriteDynamicRenderedClippedRecordAtOtEntry(&s_pb, &s_block, vertices, s_indices, count, otEntry);
	case WRITER_QUAD_4X4:
		return Ovr226_800aa5ac_WriteQuad4x4RenderedClippedRecordAtOtEntry(&s_pb, &s_block, vertices, s_indices, count, otEntry);
	case WRITER_COUNT:
		CtrTest_Bail("unknown clip writer");
		return 0;
	}
	CtrTest_Bail("invalid clip writer");
	return 0;
}

static void Reset(size_t capacity)
{
	memset(&s_scratch, 0, sizeof(s_scratch));
	memset(&s_storage, 0xa5, sizeof(s_storage));
	s_start = s_storage.bytes + 16;
	s_cursor = s_start;
	s_end = s_start + capacity;
	s_pb.ptrOT = s_ot;
	s_scratch.clipRecordHeader = 0x40000000u;
	s_scratch.uv.uv0.halves.high = 0x1234;
	s_scratch.uv.uv1.halves.high = 0x5678;
	s_pgxpCopies = 0;
	s_resolvedDepth = 0;
}

static int GuardsIntact(size_t written)
{
	for (size_t i = 0; i < sizeof(s_storage.bytes); i++)
	{
		if ((i < 16 || i >= 16 + written) && s_storage.bytes[i] != 0xa5)
		{
			return 0;
		}
	}
	return 1;
}

static void MakeVertices(struct DrawLevelOvr1PScratchVertex *vertices, int captured)
{
	// First three positions and the camera centre are from the successful
	// Crash Cove clipwindow log. Every corner is outside the old Y extent.
	static const s16 capturedPositions[4][3] = {
	    {-832, 384, -3408}, {-606, 384, -3346}, {-855, 384, -3240}, {-632, 384, -3142},
	};
	// A face can also span the camera-centred box with no corner inside it.
	static const s16 spanningPositions[4][3] = {
	    {-600, 0, -600}, {600, 0, -600}, {-600, 0, 600}, {600, 0, 600},
	};
	memset(vertices, 0, 4 * sizeof(*vertices));
	for (int i = 0; i < 4; i++)
	{
		memcpy(vertices[i].pos, captured ? capturedPositions[i] : spanningPositions[i], sizeof(vertices[i].pos));
		vertices[i].flags = (u16)(0x1200 + i);
		vertices[i].color_hi[0] = (u8)(10 + i);
		vertices[i].color_hi[1] = (u8)(20 + i);
		vertices[i].color_hi[2] = (u8)(30 + i);
		vertices[i].color_hi[3] = 0x20;
		vertices[i].depth = (u16)(100 + 100 * i);
		vertices[i].clipHalfNear = i == 2 ? 0x80 : 0;
		vertices[i].clipNear = i == 2 ? 0x80 : 0;
	}
	if (captured)
	{
		s_scratch.projectedCenter.x = -608;
		s_scratch.projectedCenter.y = 858;
		s_scratch.projectedCenter.z = -4466;
	}
}

static void CheckWrittenRecord(enum TestWriter writer, const struct DrawLevelOvr1PScratchVertex *vertices, int count)
{
	struct DrawLevelOvr1PClipRecord expected = {0};
	const size_t bytes = count == 3 ? 48 : 60;
	expected.header = (writer == WRITER_WATER || writer == WRITER_WATER_INHERITED) ? 0x80000000u : 0x40000000u;
	expected.header |= count == 4 ? 1u : 0u;
	expected.otEntry = (u32)(uintptr_t)&s_ot[7];
	expected.tpage = 0x5678;
	expected.clut = 0x1234;
	for (int i = 0; i < count; i++)
	{
		const struct DrawLevelOvr1PScratchVertex *src = &vertices[s_indices[i]];
		memcpy(expected.vertex[i].pos, src->pos, sizeof(src->pos));
		expected.vertex[i].flags = src->flags;
		memcpy(expected.vertex[i].color_hi, src->color_hi, sizeof(src->color_hi));
		expected.vertex[i].color_hi[3] |= src->clipHalfNear;
	}
	CTR_TEST_CHECK(s_cursor == s_start + bytes, "an eligible face is queued for near clipping");
	CTR_TEST_CHECK(memcmp(s_start, &expected, bytes) == 0, "the clip record preserves vertex order, near masks, UV metadata and OT destination");
	CTR_TEST_CHECK(GuardsIntact(bytes), "serialization touches only the GT3 or GT4 record bytes");
}

static void TestOutsideWindow(enum TestWriter writer)
{
	for (int captured = 0; captured <= 1; captured++)
	{
		for (int count = 3; count <= 4; count++)
		{
			struct DrawLevelOvr1PScratchVertex vertices[4];
			struct DrawLevelOvr1PScratchVertex original[4];
			Reset(120);
			MakeVertices(vertices, captured);
			memcpy(original, vertices, sizeof(vertices));
			s_block.blockID = captured ? 1240 : 17;
			CTR_TEST_CHECK(Write(writer, vertices, count, &s_ot[7]) == 1, "queuing succeeds without terminating the render list");
			CheckWrittenRecord(writer, vertices, count);
			CTR_TEST_CHECK(memcmp(original, vertices, sizeof(vertices)) == 0, "queuing leaves source geometry unchanged");
			if (writer == WRITER_GENERIC || writer == WRITER_WATER)
			{
				CTR_TEST_CHECK(s_resolvedDepth == 400, "OT selection receives the maximum selected depth");
			}
		}
	}
}

static void TestCapacity(enum TestWriter writer)
{
	struct DrawLevelOvr1PScratchVertex vertices[4];
	for (int count = 3; count <= 4; count++)
	{
		const size_t bytes = count == 3 ? 48 : 60;
		Reset(bytes);
		MakeVertices(vertices, 0);
		for (int i = 0; i < 4; i++)
		{
			memset(vertices[i].pos, 0, sizeof(vertices[i].pos));
		}
		Write(writer, vertices, count, &s_ot[7]);
		CheckWrittenRecord(writer, vertices, count);
		u8 snapshot[sizeof(s_storage.bytes)];
		memcpy(snapshot, s_storage.bytes, sizeof(snapshot));
		Write(writer, vertices, count, &s_ot[7]);
		CTR_TEST_CHECK(s_cursor == s_end, "a full buffer cannot advance the cursor");
		CTR_TEST_CHECK(memcmp(snapshot, s_storage.bytes, sizeof(snapshot)) == 0, "a full buffer preserves existing records and guards");

		Reset(bytes - 1);
		Write(writer, vertices, count, &s_ot[7]);
		CTR_TEST_CHECK(s_cursor == s_start && GuardsIntact(0), "a record one byte too large is rejected without any write");
	}

	Reset(108);
	Write(writer, vertices, 3, &s_ot[7]);
	Write(writer, vertices, 4, &s_ot[7]);
	CTR_TEST_CHECK(s_cursor == s_end && GuardsIntact(108), "consecutive GT3 and GT4 records share the buffer without gaps or overruns");

	if (writer >= WRITER_GROUND_4X1)
	{
		Reset(120);
		Write(writer, vertices, 4, NULL);
		CTR_TEST_CHECK(s_cursor == s_start && GuardsIntact(0), "an absent inherited OT entry skips the non-water record");
	}
}

int main(void)
{
	for (enum TestWriter writer = WRITER_GENERIC; writer < WRITER_COUNT; writer++)
	{
		CTR_TEST_SECTION(s_writerNames[writer]);
		TestOutsideWindow(writer);
		TestCapacity(writer);
	}
	CTR_TEST_SECTION("PGXP position forwarding");
	struct DrawLevelOvr1PScratchVertex vertices[4];
	Reset(60);
	MakeVertices(vertices, 0);
	g_config.pgxpMode = PGXP_MODE_GEOMETRY;
	Write(WRITER_WATER, vertices, 4, &s_ot[7]);
	CTR_TEST_CHECK(s_pgxpCopies == 4, "PGXP forwards every queued vertex position");
	return CtrTest_Summary();
}
