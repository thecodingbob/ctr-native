#include "VehCommon.h"

enum
{
	VEH_GROUND_SKIDS_ICON_TIREMARK = 0x2f,
	VEH_GROUND_SKIDS_ALT_TPAGE_FLAG = 0x1,
	VEH_GROUND_SKIDS_TPAGE_BLEND_ALT = 0x00600000,
	VEH_GROUND_SKIDS_TPAGE_BLEND_NORMAL = 0x00400000,
	VEH_GROUND_SKIDS_GPU_TAG_POLY_GT4 = 0x0c000000u,
	VEH_GROUND_SKIDS_OT_DEPTH_SHIFT = 6,
	VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT = 2,
	VEH_GROUND_SKIDS_CULL_ABS_MAX = 0x1771,
	VEH_GROUND_SKIDS_DEPTH_SHIFT = 2,
	VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH = 0x180,
	VEH_GROUND_SKIDS_FULL_INTENSITY = 0x7f,
	VEH_GROUND_SKIDS_LZCR_SHIFT_BASE = 0x1a,
	VEH_GROUND_SKIDS_MIN_INTENSITY = 0x10,
	VEH_GROUND_SKIDS_COLOR_PREFIX = 0x3e000000u,
	VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH = 0x20,
	VEH_GROUND_SKIDS_COLOR_FADE_SHIFT = 1,
};

#define VEH_GROUND_SKIDS_TPAGE_BLEND_MASK 0xff9fffffu
#define VEH_GROUND_SKIDS_COLOR_SENTINEL   0xffffffffu

#ifdef CTR_NATIVE
#define VehGroundSkids_LoadOriginTransform()                                                        \
	do                                                                                              \
	{                                                                                               \
		const VehGroundSkidsWord *originWords = CTR_SCRATCHPAD_PTR(const VehGroundSkidsWord, 0xb8); \
		CTC2(originWords[0], 5);                                                                    \
		CTC2(originWords[1], 6);                                                                    \
		CTC2(originWords[2], 7);                                                                    \
	} while (0)
#else
#define VehGroundSkids_LoadOriginTransform()                                                                                                        \
	__asm__ volatile("lui $8,0x1f80\n\tori $8,$8,0xb8\n\tlw $12,0($8)\n\tlw $13,4($8)\n\tlw $14,8($8)\n\tctc2 $12,$5\n\tctc2 $13,$6\n\tctc2 $14,$7" \
	                 :                                                                                                                              \
	                 :                                                                                                                              \
	                 : "memory")
#endif

// NOTE(aalhendi): This dependency keeps the fourth segment's a3 setup after
// its flag-byte load. Native builds only need the equivalent pointer assignment.
#ifdef CTR_NATIVE
#define VehGroundSkids_SetCallScratch(result, source, dependency) \
	do                                                            \
	{                                                             \
		(void)(dependency);                                       \
		(result) = (source);                                      \
	} while (0)
#else
#define VehGroundSkids_SetCallScratch(result, source, dependency) __asm__("addu %0,%1,$0" : "=r"(result) : "r"(source), "r"(dependency))
#endif

typedef u32 VehGroundSkidsWord CTR_MAY_ALIAS;

void VehGroundSkids_Subset1(u32 *currXY, u32 *prevXY, int depth, struct VehGroundSkidsScratch *scratch)
{
	register struct GameTracker *gGT CTR_PSX_REGISTER("$9");
	register u32 *previousXY CTR_PSX_REGISTER("$10");
	struct DB *backBuffer;
	register CtrPackedU32 *polyCursor CTR_PSX_REGISTER("$5");
	register CtrPackedU32 *nextPrim CTR_PSX_REGISTER("$8");
	register CtrPackedU32 *poly CTR_PSX_REGISTER("$8");
	struct GameTracker *uvGameTracker;
	register u32 tpage CTR_PSX_REGISTER("$2");
	u32 colorNear;
	register u32 packetAddress CTR_PSX_REGISTER("$5");
	struct PushBuffer *pb;
	u32 *ot;

	VEH_LOAD_GAME_TRACKER(gGT);
	backBuffer = gGT->backBuffer;
	previousXY = prevXY;
	CTR_PSX_KEEP_VALUE(previousXY);
	polyCursor = (CtrPackedU32 *)backBuffer->primMem.cursor;
	nextPrim = polyCursor + (sizeof(POLY_GT4) / sizeof(*polyCursor));
	if ((CtrPackedU32 *)backBuffer->primMem.guardEnd < nextPrim)
	{
		return;
	}

	backBuffer->primMem.cursor = (void *)nextPrim;
	colorNear = scratch->colorNear;
	poly = polyCursor;
	poly[1] = colorNear;
	poly[4] = scratch->colorNear;
	poly[7] = scratch->colorFar;
	poly[10] = scratch->colorFar;
	poly[2] = currXY[0];
	poly[5] = currXY[1];
	poly[8] = previousXY[0];
	poly[11] = previousXY[1];

	poly[3] = *(const CtrPackedU32 *)&gGT->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK]->texLayout.u0;
	if ((scratch->segment.segmentFlags & VEH_GROUND_SKIDS_ALT_TPAGE_FLAG) != 0)
	{
		tpage = *(const CtrPackedU32 *)&gGT->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK]->texLayout.u1;
		tpage &= VEH_GROUND_SKIDS_TPAGE_BLEND_MASK;
		CTR_PSX_KEEP_VALUE(tpage);
		tpage |= VEH_GROUND_SKIDS_TPAGE_BLEND_ALT;
	}
	else
	{
		tpage = *(const CtrPackedU32 *)&gGT->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK]->texLayout.u1;
		tpage &= VEH_GROUND_SKIDS_TPAGE_BLEND_MASK;
		CTR_PSX_KEEP_VALUE(tpage);
		tpage |= VEH_GROUND_SKIDS_TPAGE_BLEND_NORMAL;
	}
	poly[6] = tpage;
	CTR_PSX_MEMORY_BARRIER();
	uvGameTracker = GAME_TRACKER;
	CtrGpu_WritePackedUV((u8 *)&poly[9], CTR_ReadU16LE(&uvGameTracker->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK]->texLayout.u2));
	packetAddress = CtrGpu_PrimToOTLink24(poly);
	CtrGpu_WritePackedUV((u8 *)&poly[12], CTR_ReadU16LE(&uvGameTracker->ptrIcons[VEH_GROUND_SKIDS_ICON_TIREMARK]->texLayout.u3));

	pb = scratch->pushBuffer;
	ot = pb->ptrOT + ((s32)depth >> VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
	poly[0] = *ot | VEH_GROUND_SKIDS_GPU_TAG_POLY_GT4;
	*ot = packetAddress;
}

void VehGroundSkids_Subset2(struct VehGroundSkidsScratch *scratch, const SVECTOR *v1, const SVECTOR *v2, const SVECTOR *v3)
{
	// NOTE(aalhendi): Retail deliberately subtracts the low halfwords before
	// projection; keep the 16-bit wraparound visible to both targets.
	scratch->projected[0].vx = (s16)(u16)(((u32)(u16)v1->vx - (u32)(u16)scratch->origin.x) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[0].vy = (s16)(u16)(((u32)(u16)v1->vy - (u32)(u16)scratch->origin.y) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[0].vz = (s16)(u16)(((u32)(u16)v1->vz - (u32)(u16)scratch->origin.z) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);

	scratch->projected[1].vx = (s16)(u16)(((u32)(u16)v2->vx - (u32)(u16)scratch->origin.x) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[1].vy = (s16)(u16)(((u32)(u16)v2->vy - (u32)(u16)scratch->origin.y) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[1].vz = (s16)(u16)(((u32)(u16)v2->vz - (u32)(u16)scratch->origin.z) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);

	scratch->projected[2].vx = (s16)(u16)(((u32)(u16)v3->vx - (u32)(u16)scratch->origin.x) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[2].vy = (s16)(u16)(((u32)(u16)v3->vy - (u32)(u16)scratch->origin.y) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
	scratch->projected[2].vz = (s16)(u16)(((u32)(u16)v3->vz - (u32)(u16)scratch->origin.z) << VEH_GROUND_SKIDS_PROJECT_SCALE_SHIFT);
}

void VehGroundSkids_Main(struct Thread *thread, struct PushBuffer *pb)
{
	register struct VehGroundSkidsScratch *scratch CTR_PSX_REGISTER("$18");
	register u32 *currXY CTR_PSX_REGISTER("$20");
	register u32 *prevXY CTR_PSX_REGISTER("$23");
	register s32 *currDepth CTR_PSX_REGISTER("$17");
	register s32 *prevDepth CTR_PSX_REGISTER("$19");
	register u32 flags CTR_PSX_REGISTER("$21");
	register u32 prevFlags;
	register int frameOffset CTR_PSX_REGISTER("$22");
	register union VehEmitterSkidmark *frame CTR_PSX_REGISTER("$16");
	register s32 value CTR_PSX_REGISTER("$4");
	register s32 geomX CTR_PSX_REGISTER("$3");
	register s32 geomY CTR_PSX_REGISTER("$2");
	register union VehEmitterSkidmark *initialFrame CTR_PSX_REGISTER("$5");
	// NOTE(aalhendi): Volatile stack views below preserve retail reloads while
	// this cursor reuses t0 for thread, driver, frame, and mark addresses.
	register size_t cursor CTR_PSX_REGISTER("$8");
	struct Driver *volatile savedDriver;
	int frameIndex;
	int depth;
	s32 absDelta;
	register s32 coordinate CTR_PSX_REGISTER("$2");
	register s32 origin CTR_PSX_REGISTER("$3");
	register u8 *markBytes CTR_PSX_REGISTER("$3");
	const VehGroundSkidsWord *viewWords;
	register u32 gteWord0 CTR_PSX_REGISTER("$12");
	register u32 gteWord1 CTR_PSX_REGISTER("$13");
	register u32 gteWord2 CTR_PSX_REGISTER("$14");

	geomX = (s16)pb->rect.w >> 1;
	geomY = (s16)pb->rect.h >> 1;
	CTR_PSX_KEEP_VALUE(geomX);
	CTR_PSX_KEEP_VALUE(geomY);
	gteWord0 = (u32)geomX << 16;
	gteWord1 = (u32)geomY << 16;
	CTC2(gteWord0, 24);
	CTC2(gteWord1, 25);
	cursor = (size_t)pb->distanceToScreen_PREV;
	gte_SetGeomScreen((s32)cursor);

	scratch = CTR_SCRATCHPAD_PTR(struct VehGroundSkidsScratch, 0x0);
	CTR_PSX_KEEP_VALUE(scratch);
	scratch->pushBuffer = pb;
	scratch->origin.x = 0;
	scratch->origin.y = 0;
	scratch->origin.z = 0;

	viewWords = (const VehGroundSkidsWord *)(const void *)&pb->matrix_ViewProj;
	CTR_PSX_KEEP_VALUE(viewWords);
	gteWord0 = viewWords[0];
	gteWord1 = viewWords[1];
	CTC2(gteWord0, 0);
	CTC2(gteWord1, 1);
	gteWord0 = viewWords[2];
	gteWord1 = viewWords[3];
	gteWord2 = viewWords[4];
	CTC2(gteWord0, 2);
	CTC2(gteWord1, 3);
	CTC2(gteWord2, 4);
	VehGroundSkids_LoadOriginTransform();

	scratch->origin.x = pb->matrix_Camera.t[0];
	scratch->origin.y = pb->matrix_Camera.t[1];
	scratch->origin.z = pb->matrix_Camera.t[2];
	CTR_PSX_MEMORY_BARRIER();

	cursor = (size_t)*(struct Thread *volatile *)&thread;
	while (cursor != 0)
	{
		cursor = (size_t)*(struct Thread *volatile *)&thread;
		cursor = (size_t)((struct Thread *)cursor)->object;
		savedDriver = (struct Driver *)cursor;
		CTR_PSX_MEMORY_BARRIER();
		flags = ((struct Driver *)cursor)->skidmarkEnableFlags;

		if (flags > DRIVER_SKIDMARK_CURRENT_FRAME_MASK)
		{
			prevFlags = 0;
			currXY = scratch->currXY;
			currDepth = scratch->currDepth;
			coordinate = (u8)((struct Driver *)cursor)->skidmarkFrameIndex;
			prevXY = scratch->prevXY;
			frameIndex = (coordinate - 1) & DRIVER_SKIDMARK_FRAME_INDEX_MASK;
			CTR_PSX_OBSERVE_MEMORY(frameIndex);
			initialFrame = (union VehEmitterSkidmark *)(cursor + frameIndex * sizeof(savedDriver->skidmarks[0]));

			coordinate = *(s16 *)((char *)initialFrame + offsetof(struct Driver, skidmarks));
			CTR_PSX_MEMORY_BARRIER();
			origin = scratch->origin.x;
			value = (coordinate - origin) * 4;
			absDelta = value;
			if (value < 0)
			{
				absDelta = -absDelta;
			}
			prevDepth = scratch->prevDepth;

			if (absDelta < VEH_GROUND_SKIDS_CULL_ABS_MAX)
			{
				scratch->projected[0].vx = (s16)value;
				coordinate = *(s16 *)((char *)initialFrame + offsetof(struct Driver, skidmarks) + 2);
				CTR_PSX_MEMORY_BARRIER();
				origin = scratch->origin.y;
				value = (coordinate - origin) * 4;
				absDelta = value;
				if (value < 0)
				{
					absDelta = -absDelta;
				}

				if (absDelta < VEH_GROUND_SKIDS_CULL_ABS_MAX)
				{
					scratch->projected[0].vy = (s16)value;
					coordinate = *(s16 *)((char *)initialFrame + offsetof(struct Driver, skidmarks) + 4);
					CTR_PSX_MEMORY_BARRIER();
					origin = scratch->origin.z;
					value = (coordinate - origin) * 4;
					absDelta = value;
					if (value < 0)
					{
						absDelta = -absDelta;
					}

					if (absDelta < VEH_GROUND_SKIDS_CULL_ABS_MAX)
					{
						scratch->projected[0].vz = (s16)value;
						VehGteLoadV0(&scratch->projected[0]);
						CTR_PSX_GTE_PIPELINE_DELAY();
						gte_rtv0();
						value = MFC2_S(27);
						CTR_PSX_GTE_READ_DELAY();
						value >>= VEH_GROUND_SKIDS_DEPTH_SHIFT;

						if (value < VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH)
						{
							value = VEH_GROUND_SKIDS_FULL_INTENSITY;
							// NOTE(aalhendi): Retail skips the minimum-intensity check on this path.
							goto intensity_ready;
						}
						else
						{
							coordinate = value - VEH_GROUND_SKIDS_FULL_INTENSITY_DEPTH;
							MTC2(coordinate, 30);
							CTR_PSX_GTE_PIPELINE_DELAY();
							value = MFC2(31);
							value = VEH_GROUND_SKIDS_LZCR_SHIFT_BASE - value;
							if (value < 0)
							{
								value = 0;
							}
							value = VEH_GROUND_SKIDS_FULL_INTENSITY >> value;
						}

						if (value >= VEH_GROUND_SKIDS_MIN_INTENSITY)
						{
						intensity_ready:
							CTR_PSX_MEMORY_BARRIER();
							coordinate = (u32)value << 8;
							origin = VEH_GROUND_SKIDS_COLOR_PREFIX;
							coordinate |= origin;
							coordinate = value | coordinate;
							origin = (u32)value << 16;
							coordinate |= origin;
							scratch->colorNear = (u32)coordinate;
							scratch->colorFar = VEH_GROUND_SKIDS_COLOR_SENTINEL;

							while (flags != 0)
							{
								register struct VehGroundSkidsScratch *subsetScratch CTR_PSX_REGISTER("$4");

								subsetScratch = scratch;
								if ((flags & DRIVER_SKIDMARK_CURRENT_FRAME_MASK) != 0)
								{
									cursor = (size_t)*(volatile int *)&frameIndex;
									frameOffset = cursor * sizeof(savedDriver->skidmarks[0]);
									cursor = (size_t)savedDriver;
									frame = (union VehEmitterSkidmark *)(frameOffset + offsetof(struct Driver, skidmarks));
									frame = (union VehEmitterSkidmark *)(cursor + (size_t)frame);

									VehGroundSkids_Subset2(subsetScratch, &frame[0].edge[0], &frame[0].edge[1], &frame[1].edge[0]);
									VehGteLoadV3(scratch->projected);
									CTR_PSX_GTE_PIPELINE_DELAY();
									gte_rtpt();

									VehGroundSkids_Subset2(scratch, &frame[1].edge[1], &frame[2].edge[0], &frame[2].edge[1]);
									VehGteStoreSxy3(&currXY[0]);
									VehGteStoreSz3(&currDepth[0]);
									VehGteLoadV3(scratch->projected);
									CTR_PSX_GTE_PIPELINE_DELAY();
									gte_rtpt();

									VehGroundSkids_Subset2(scratch, &frame[3].edge[0], &frame[3].edge[1], &frame[0].edge[0]);
									VehGteStoreSxy3(&currXY[3]);
									VehGteStoreSz3(&currDepth[3]);
									VehGteLoadV3(scratch->projected);
									CTR_PSX_GTE_PIPELINE_DELAY();
									gte_rtpt();
									VehGteStoreSxy3(&currXY[6]);
									VehGteStoreSz3(&currDepth[6]);

									if ((flags & prevFlags & DRIVER_SKIDMARK_BACK_LEFT) != 0 && currDepth[0] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    currDepth[1] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH && prevDepth[0] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    prevDepth[1] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH)
									{
										cursor = (size_t)savedDriver;
										markBytes = (u8 *)(cursor + frameOffset);
										scratch->segment.segmentFlags = markBytes[offsetof(struct Driver, skidmarks) + 7];
										depth = (currDepth[0] >> VEH_GROUND_SKIDS_DEPTH_SHIFT) +
										        (markBytes[offsetof(struct Driver, skidmarks) + 6] << VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
										VehGroundSkids_Subset1(&currXY[0], &prevXY[0], depth, scratch);
									}

									if ((flags & prevFlags & DRIVER_SKIDMARK_BACK_RIGHT) != 0 && currDepth[2] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    currDepth[3] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH && prevDepth[2] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    prevDepth[3] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH)
									{
										cursor = (size_t)*(volatile int *)&frameIndex;
										markBytes = (u8 *)(cursor * sizeof(savedDriver->skidmarks[0]));
										cursor = (size_t)savedDriver;
										markBytes = (u8 *)(cursor + (size_t)markBytes);
										scratch->segment.segmentFlags = markBytes[offsetof(struct Driver, skidmarks) + sizeof(union VehEmitterSkidmark) + 7];
										depth = (currDepth[2] >> VEH_GROUND_SKIDS_DEPTH_SHIFT) +
										        (markBytes[offsetof(struct Driver, skidmarks) + sizeof(union VehEmitterSkidmark) + 6]
										         << VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
										VehGroundSkids_Subset1(&currXY[2], &prevXY[2], depth, scratch);
									}

									if ((flags & prevFlags & DRIVER_SKIDMARK_FRONT_LEFT) != 0 && currDepth[4] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    currDepth[5] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH && prevDepth[4] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    prevDepth[5] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH)
									{
										cursor = (size_t)*(volatile int *)&frameIndex;
										markBytes = (u8 *)(cursor * sizeof(savedDriver->skidmarks[0]));
										cursor = (size_t)savedDriver;
										markBytes = (u8 *)(cursor + (size_t)markBytes);
										scratch->segment.segmentFlags =
										    markBytes[offsetof(struct Driver, skidmarks) + 2 * sizeof(union VehEmitterSkidmark) + 7];
										depth = (currDepth[4] >> VEH_GROUND_SKIDS_DEPTH_SHIFT) +
										        (markBytes[offsetof(struct Driver, skidmarks) + 2 * sizeof(union VehEmitterSkidmark) + 6]
										         << VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
										VehGroundSkids_Subset1(&currXY[4], &prevXY[4], depth, scratch);
									}

									// NOTE(aalhendi): Computing the first call argument in the final
									// predicate preserves retail's branch-delay-slot scheduling.
									if ((flags & prevFlags & DRIVER_SKIDMARK_FRONT_RIGHT) != 0 && currDepth[6] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    currDepth[7] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH && prevDepth[6] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH &&
									    (value = (s32)(size_t)&currXY[6], prevDepth[7] > VEH_GROUND_SKIDS_MIN_VISIBLE_DEPTH))
									{
										register struct VehGroundSkidsScratch *callScratch CTR_PSX_REGISTER("$7");

										cursor = (size_t)*(volatile int *)&frameIndex;
										markBytes = (u8 *)(cursor * sizeof(savedDriver->skidmarks[0]));
										cursor = (size_t)savedDriver;
										initialFrame = (union VehEmitterSkidmark *)&prevXY[6];
										markBytes = (u8 *)(cursor + (size_t)markBytes);
										coordinate = markBytes[offsetof(struct Driver, skidmarks) + 3 * sizeof(union VehEmitterSkidmark) + 7];
										VehGroundSkids_SetCallScratch(callScratch, scratch, coordinate);
										scratch->segment.segmentFlags = (u32)coordinate;
										depth = (currDepth[6] >> VEH_GROUND_SKIDS_DEPTH_SHIFT) +
										        (markBytes[offsetof(struct Driver, skidmarks) + 3 * sizeof(union VehEmitterSkidmark) + 6]
										         << VEH_GROUND_SKIDS_OT_DEPTH_SHIFT);
										VehGroundSkids_Subset1((u32 *)(size_t)(u32)value, (u32 *)initialFrame, depth, callScratch);
									}
								}

								currXY = (u32 *)((size_t)currXY ^ (size_t)prevXY);
								prevXY = (u32 *)((size_t)prevXY ^ (size_t)currXY);
								currXY = (u32 *)((size_t)currXY ^ (size_t)prevXY);
								currDepth = (s32 *)((size_t)currDepth ^ (size_t)prevDepth);
								prevDepth = (s32 *)((size_t)prevDepth ^ (size_t)currDepth);
								currDepth = (s32 *)((size_t)currDepth ^ (size_t)prevDepth);
								cursor = (size_t)*(volatile int *)&frameIndex;
								coordinate = (s32)cursor + 1;
								coordinate &= DRIVER_SKIDMARK_FRAME_INDEX_MASK;
								frameIndex = coordinate;

								if (scratch->colorFar == VEH_GROUND_SKIDS_COLOR_SENTINEL)
								{
									scratch->colorFar = scratch->colorNear;
									prevFlags = DRIVER_SKIDMARK_CURRENT_FRAME_MASK;
								}
								else
								{
									prevFlags = flags;
									flags >>= DRIVER_SKIDMARK_HISTORY_SHIFT;
									coordinate = scratch->colorNear;
									CTR_PSX_MEMORY_BARRIER();
									origin = scratch->colorNear;
									coordinate &= 0xff;
									CTR_PSX_KEEP_VALUE(coordinate);
									value = coordinate >> VEH_GROUND_SKIDS_COLOR_FADE_SHIFT;
									coordinate = (u32)value << 8;
									scratch->colorFar = (u32)origin;
									origin = VEH_GROUND_SKIDS_COLOR_PREFIX;
									coordinate |= origin;
									coordinate = value | coordinate;
									origin = (u32)value << 16;
									coordinate |= origin;
									scratch->colorNear = (u32)coordinate;
								}

								if (scratch->colorFar == 0)
								{
									break;
								}
							}
						}
					}
				}
			}
		}

		cursor = (size_t)*(struct Thread *volatile *)&thread;
		cursor = (size_t)((struct Thread *)cursor)->siblingThread;
		thread = (struct Thread *)cursor;
	}
}
