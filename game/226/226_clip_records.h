#ifndef DRAW_LEVEL_OVR1P_CLIP_RECORDS_H
#define DRAW_LEVEL_OVR1P_CLIP_RECORDS_H

// Private unity-build implementation, also included by the clip-record tests.
enum DrawLevelOvr1PClipRecordSize
{
	DRAW_LEVEL_OVR1P_CLIP_RECORD_GT3_SIZE = 0x30,
	DRAW_LEVEL_OVR1P_CLIP_RECORD_GT4_SIZE = 0x3c,
};

static u32 DrawLevelOvr1P_GetClipRecordSize(int count)
{
	return count == 4 ? DRAW_LEVEL_OVR1P_CLIP_RECORD_GT4_SIZE : DRAW_LEVEL_OVR1P_CLIP_RECORD_GT3_SIZE;
}

static int DrawLevelOvr1P_HasClipRecordSpace(u32 size)
{
	u8 *start = DrawLevelOvr1P_GetClipRecordStart();
	u8 *cursor = DrawLevelOvr1P_GetClipRecordCursor();
	u8 *end = DrawLevelOvr1P_GetClipRecordEnd();

	return start != NULL && cursor != NULL && end != NULL && cursor >= start && cursor <= end && size <= (u32)(end - cursor);
}

static u32 DrawLevelOvr1P_GetPreparedProjectedMaxDepthCount(const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count)
{
	u32 maxDepth = projected[indices[0]].depth;

	for (s32 vertexIndex = 1; vertexIndex < count; vertexIndex++)
	{
		if (maxDepth < projected[indices[vertexIndex]].depth)
		{
			maxDepth = projected[indices[vertexIndex]].depth;
		}
	}

	return maxDepth;
}

static void DrawLevelOvr1P_CopyClipRecordVertex(struct DrawLevelOvr1PClipRecordVertex *dst, const struct DrawLevelOvr1PScratchVertex *src)
{
	CTR_COPY_VEC3(dst->pos, src->pos);
	dst->flags = src->flags;
	DrawLevelOvr1P_CopyColorWord(dst->color_hi, src->color_hi);
	if (NATIVE_PGXP_ACTIVE())
	{
		NativePgxp_CopyPosition(&dst->pos[0], &src->pos[0], &src->pos[0]);
	}

	// Retail 0x800a4f78/0x800a3680 carry the near mask in the color/code word.
	dst->color_hi[3] |= src->clipHalfNear;
}

static u32 DrawLevelOvr1P_GetRenderedClipRecordHeader(const struct QuadBlock *block, int count)
{
	(void)block;
	return DrawLevelOvr1P_Scratch()->clipRecordHeader | (count == 4 ? 1u : 0u);
}

static int DrawLevelOvr1P_WriteClipRecord(const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                        u32 header, u32 *otEntry)
{
	u8 *cursor = DrawLevelOvr1P_GetClipRecordCursor();
	u32 recordSize = DrawLevelOvr1P_GetClipRecordSize(count);

	// Let the near clipper handle visibility: the retail source-position window
	// can reject visible faces, especially in widescreen.
	if (!DrawLevelOvr1P_HasClipRecordSpace(recordSize))
	{
		return 1;
	}

	struct DrawLevelOvr1PClipRecord *record = (struct DrawLevelOvr1PClipRecord *)cursor;
	record->header = header;
	record->otEntry = (u32)(u32)otEntry;
	record->tpage = DrawLevelOvr1P_Scratch()->uv.uv1.halves.high;
	record->clut = DrawLevelOvr1P_Scratch()->uv.uv0.halves.high;

	for (s32 vertexIndex = 0; vertexIndex < count; vertexIndex++)
	{
		DrawLevelOvr1P_CopyClipRecordVertex(&record->vertex[vertexIndex], &projected[indices[vertexIndex]]);
	}

	DrawLevelOvr1P_SetClipRecordCursor(cursor + recordSize);
	return 1;
}

static int DrawLevelOvr1P_WriteRenderedClippedRecordAtOt(struct PushBuffer *pb, const struct QuadBlock *block,
                                                       const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count, int faceIndex,
                                                       const struct TextureLayout *texture, int otIndexOverride)
{
	(void)texture;
	u32 maxDepth = DrawLevelOvr1P_GetPreparedProjectedMaxDepthCount(projected, indices, count);
	int otIndex = DrawLevelOvr1P_ResolveProjectedOtIndex(block, projected, maxDepth, faceIndex, otIndexOverride);
	return DrawLevelOvr1P_WriteClipRecord(projected, indices, count, DrawLevelOvr1P_GetRenderedClipRecordHeader(block, count), &pb->ptrOT[otIndex]);
}

static int DrawLevelOvr1P_WriteWaterRenderedClippedRecordAtOt(struct PushBuffer *pb, const struct QuadBlock *block,
                                                            const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count, int faceIndex,
                                                            int otIndexOverride)
{
	u32 maxDepth = DrawLevelOvr1P_GetPreparedProjectedMaxDepthCount(projected, indices, count);
	int otIndex = DrawLevelOvr1P_ResolveProjectedOtIndex(block, projected, maxDepth, faceIndex, otIndexOverride);
	// Retail water headers keep either NCLIP polarity; bit 0 selects GT4.
	return DrawLevelOvr1P_WriteClipRecord(projected, indices, count, count == 4 ? 0x80000001u : 0x80000000u, &pb->ptrOT[otIndex]);
}

static int Ovr226_800a34d4_WriteWaterRenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct DrawLevelOvr1PScratchVertex *projected,
                                                                 const int *indices, int count, u32 *otEntry)
{
	(void)pb;
	return DrawLevelOvr1P_WriteClipRecord(projected, indices, count, count == 4 ? 0x80000001u : 0x80000000u, otEntry);
}

static int DrawLevelOvr1P_WriteRenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct QuadBlock *block,
                                                                     const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                                                     u32 *otEntry)
{
	(void)pb;
	if (otEntry == NULL)
	{
		return 1;
	}
	return DrawLevelOvr1P_WriteClipRecord(projected, indices, count, DrawLevelOvr1P_GetRenderedClipRecordHeader(block, count), otEntry);
}

static int Ovr226_800a6d6c_WriteGround4x2RenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct QuadBlock *block,
                                                                     const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                                                     u32 *otEntry)
{
	return DrawLevelOvr1P_WriteRenderedClippedRecordAtOtEntry(pb, block, projected, indices, count, otEntry);
}

static int Ovr226_800a898c_WriteDynamicRenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct QuadBlock *block,
                                                                   const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                                                   u32 *otEntry)
{
	return DrawLevelOvr1P_WriteRenderedClippedRecordAtOtEntry(pb, block, projected, indices, count, otEntry);
}

static int Ovr226_800aa5ac_WriteQuad4x4RenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct QuadBlock *block,
                                                                   const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                                                   u32 *otEntry)
{
	return DrawLevelOvr1P_WriteRenderedClippedRecordAtOtEntry(pb, block, projected, indices, count, otEntry);
}

static int Ovr226_800a4dcc_WriteGround4x1RenderedClippedRecordAtOtEntry(struct PushBuffer *pb, const struct QuadBlock *block,
                                                                   const struct DrawLevelOvr1PScratchVertex *projected, const int *indices, int count,
                                                                   u32 *otEntry)
{
	return DrawLevelOvr1P_WriteRenderedClippedRecordAtOtEntry(pb, block, projected, indices, count, otEntry);
}

#endif
