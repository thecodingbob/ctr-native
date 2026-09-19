#include <common.h>

#ifndef CTR_NATIVE

void MM_Video_DecDCToutCallbackFunc(void)
{
	u32 *ot;
	register u32 stCdIntrPage CTR_PSX_REGISTER("$16");
	register RECT *slice CTR_PSX_REGISTER("$17");
	if ((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_RGB24) != 0)
	{
		CTR_PSX_LOAD_SYMBOL_PAGE(stCdIntrPage, MM_VIDEO_ST_CD_INTERRUPT_ASM_NAME);
		if (CTR_PSX_PAGE_LVALUE(u32, stCdIntrPage, MM_VIDEO_ST_CD_INTERRUPT_PAGE_OFFSET, MM_VIDEO_ST_CD_INTERRUPT) != 0)
		{
			StCdInterrupt();

			CTR_PSX_PAGE_LVALUE(u32, stCdIntrPage, MM_VIDEO_ST_CD_INTERRUPT_PAGE_OFFSET, MM_VIDEO_ST_CD_INTERRUPT) = 0;
		}
	}

	ot = BreakDraw();

	slice = &MM_VIDEO_SLICE;
	LoadImage(slice, MM_VIDEO_OUT_BUFFERS[MM_VIDEO_DCT_OUT_BUFFER_INDEX]);

	/* update slice (rectangular strip) area to next one on the right */
	MM_VIDEO_SLICE.x += MM_VIDEO_SLICE.w;
	MM_VIDEO_DCT_OUT_BUFFER_INDEX ^= 1;

	if (MM_VIDEO_SLICE_INDEX == MM_VIDEO_FINAL_SLICE_INDEX)
	{
		MM_VIDEO_DCT_OUTPUT_DONE = 1;
	}
	else
	{
		MM_VIDEO_SLICE_INDEX++;
		DecDCTout(MM_VIDEO_OUT_BUFFERS[MM_VIDEO_DCT_OUT_BUFFER_INDEX], MM_VIDEO_DCT_OUT_SLICE_SIZE);
	}

	if (ot != 0)
	{
		DrawOTag(ot);
	}
}

void MM_Video_KickCD(CdlLOC *location)
{
	int cdlMode;
	u8 mode[4];
	int result;


	if ((location != NULL) && (MM_VIDEO_CD_LOCATION_PTR != &MM_VIDEO_CD_LOCATION_1))
	{
		MM_VIDEO_CD_LOCATION_PTR = location;
		MM_VIDEO_CD_KICK_STATE = 0;
	}

	switch (MM_VIDEO_CD_KICK_STATE)
	{
	case 0:
		result = CdControl(CdlSetloc, (u8 *)MM_VIDEO_CD_LOCATION_PTR, 0);
		if (result == 0)
			return;

		MM_VIDEO_CD_KICK_STATE = 1;

		// do NOT break,
		// original code never quit here

	case 1:
		mode[0] = CdlModeSpeed;

		result = CdControl(CdlSetmode, mode, 0);
		if (result == 0)
			return;

		MM_VIDEO_CD_KICK_STATE = 2;
		break;

	case 2:
		MM_VIDEO_CD_KICK_STATE = 3;
		break;

	case 3:
		cdlMode = CdlModeStream2 | CdlModeSpeed;

		// scrapbook
		// if video contains audio
		if ((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_HAS_XA_AUDIO) != 0)
		{
			cdlMode = CdlModeStream2 | CdlModeSpeed | CdlModeRT;
		}

		MM_VIDEO_CD_KICK_STATE = 0;

		result = CdRead2(cdlMode);
		if (result == 0)
			return;

		MM_VIDEO_CD_LOCATION_PTR = 0;
		break;
	}
}

void MM_Video_VLC_Decode(void)
{
	register s32 backloc CTR_PSX_REGISTER("$19");
	int result;
	u32 size;
	struct
	{
		s16 freeSectors;
		s16 overSectors;
		u32 *sectorData;
		StHEADER *sectorHeader[2];
	} ring;
	register s32 waitTime CTR_PSX_REGISTER("$17");
	register u32 **inBuffers CTR_PSX_REGISTER("$22");
	register s32 backlocNone CTR_PSX_REGISTER("$21");


	waitTime = MM_VIDEO_VLC_WAIT_FRAMES;

	// free sectors and over sectors
	StRingStatus(&ring.freeSectors, &ring.overSectors);

	backloc = StGetBackloc(&MM_VIDEO_CD_LOCATION_2);
	CTR_PSX_OBSERVE_VALUE(backloc);

	{
		register u16 oldDecodeState CTR_PSX_REGISTER("$18");
		register u32 decodeStatePage CTR_PSX_REGISTER("$20");

		CTR_PSX_LOAD_SYMBOL_PAGE(decodeStatePage, MM_VIDEO_DECODE_STATE_ASM_NAME);
		oldDecodeState = CTR_PSX_PAGE_LVALUE(u16, decodeStatePage, MM_VIDEO_DECODE_STATE_PAGE_OFFSET, MM_VIDEO_DECODE_STATE);
		if ((oldDecodeState == 1) && ((MM_VIDEO_RING_SECTOR_COUNT - (MM_VIDEO_RING_SECTOR_COUNT >> 2)) <= ring.freeSectors))
		{
			MM_VIDEO_STALL_RECOVERY_FRAME_COUNT++;

			if (MM_VIDEO_STALL_RECOVERY_FRAMES < MM_VIDEO_STALL_RECOVERY_FRAME_COUNT)
			{
				MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;
				StClearRing();
				MM_VIDEO_END_OF_STREAM = 0;
				MM_VIDEO_SECTOR_FRAME_COUNT = 0;
				MM_VIDEO_LAST_SECTOR_FRAME_COUNT = 0;
				MM_VIDEO_LAST_BACKLOC = 0;
				MM_VIDEO_LOOP_START_BACKLOC = MM_VIDEO_BACKLOC_NONE;
				MM_VIDEO_LOOP_END_BACKLOC = MM_VIDEO_BACKLOC_NONE;
				MM_VIDEO_LOOP_WRAP_PENDING = 0;
				MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT = 0;
				CTR_PSX_PAGE_LVALUE(u16, decodeStatePage, MM_VIDEO_DECODE_STATE_PAGE_OFFSET, MM_VIDEO_DECODE_STATE) = oldDecodeState;
				MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;
				MM_VIDEO_UNUSED_0X38 = MM_VIDEO_BACKLOC_NONE;

				MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_1);
			}

			MM_VIDEO_DRAW_NEXT_FRAME = 0;
			return;
		}
	}

	MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;

	// Scrapbook
	if (((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_SCRAPBOOK) == 0) && (ring.freeSectors < (MM_VIDEO_RING_SECTOR_COUNT >> 4)))
	{
		MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_2);
	}

	if (backloc == MM_VIDEO_LAST_BACKLOC)
	{
		MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT++;
		if ((u32)MM_VIDEO_STALLED_BACKLOC_FRAMES < MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT)
		{
			MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT = 0;
			MM_VIDEO_DRAW_NEXT_FRAME = 0;
			StClearRing();
			MM_VIDEO_LAST_SECTOR_FRAME_COUNT = 0;
			MM_VIDEO_LOOP_START_BACKLOC = MM_VIDEO_BACKLOC_NONE;
			MM_VIDEO_LOOP_END_BACKLOC = MM_VIDEO_BACKLOC_NONE;
			MM_VIDEO_LOOP_WRAP_PENDING = 0;
			MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;
			MM_VIDEO_LAST_BACKLOC = 0;
			MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT = 0;
			MM_VIDEO_UNUSED_0X38 = MM_VIDEO_BACKLOC_NONE;

			MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_3);
		}
	}
	else
	{
		MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT = 0;
	}

	MM_VIDEO_DECODE_STATE = 0;

	// if reached end of video,
	// choose to loop or not loop
	if ((MM_VIDEO_LOOP_START_BACKLOC < 0) &&

	    // length of video
	    ((MM_VIDEO_STREAM_FRAME_COUNT <= backloc ||

	      (backloc < MM_VIDEO_LAST_BACKLOC))))
	{
		if ((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_LOOP) == 0)
		{
			goto LAB_Video_PauseAtEnd;
		}

		// Track-select previews loop back to their first sector.
		{
			register s32 loopEndBackloc CTR_PSX_REGISTER("$2");
			register s32 *unusedAddress CTR_PSX_REGISTER("$3");

			unusedAddress = &MM_VIDEO_UNUSED_0X38;
			CTR_PSX_CLOBBER("$18");
			loopEndBackloc = MM_VIDEO_LOOP_END_BACKLOC;
			CTR_PSX_CLOBBER("$21");
			backlocNone = MM_VIDEO_BACKLOC_NONE;
			*unusedAddress = backlocNone;
			if (0 < loopEndBackloc)
			{
				if (backloc != MM_VIDEO_STREAM_FRAME_COUNT)
				{
					result = CdPosToInt(&MM_VIDEO_CD_LOCATION_2);
					if (MM_VIDEO_LOOP_END_BACKLOC < result)
					{
						MM_VIDEO_LOOP_START_BACKLOC = CdPosToInt(&MM_VIDEO_CD_LOCATION_2);
						MM_VIDEO_LOOP_START_BACKLOC--;
						MM_VIDEO_LOOP_WRAP_PENDING = 0;

						MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_1);
					}
					MM_VIDEO_LOOP_END_BACKLOC = backlocNone;
				}
			}
			else
			{
				MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_1);

				if (backloc != MM_VIDEO_STREAM_FRAME_COUNT)
				{
					MM_VIDEO_LOOP_START_BACKLOC = CdPosToInt(&MM_VIDEO_CD_LOCATION_2);
					MM_VIDEO_LOOP_START_BACKLOC--;
					MM_VIDEO_LOOP_WRAP_PENDING = 0;
				}
				else
				{
					MM_VIDEO_LOOP_END_BACKLOC = CdPosToInt(&MM_VIDEO_CD_LOCATION_2);
				}
			}
		}
		goto LAB_Video_AfterEnd;

	LAB_Video_PauseAtEnd:
		do
		{
			result = CdControl(CdlPause, 0, 0);
		} while (result == 0);
		MM_VIDEO_END_OF_STREAM = 1;
	}

LAB_Video_AfterEnd:
{
	register u32 loopStartPage CTR_PSX_REGISTER("$16");
	register u32 loopWrapPage CTR_PSX_REGISTER("$18");
	register CdlLOC *decodeLoc CTR_PSX_REGISTER("$20");
	register u32 symbolPage CTR_PSX_REGISTER("$2");

	CTR_PSX_LOAD_SYMBOL_PAGE(loopStartPage, MM_VIDEO_LOOP_START_BACKLOC_ASM_NAME);
	CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(loopWrapPage, MM_VIDEO_LOOP_WRAP_PENDING_ASM_NAME, loopStartPage);
	CTR_PSX_CLOBBER("$21");
	backlocNone = 1;
	CTR_PSX_LOAD_SYMBOL_PAGE(symbolPage, MM_VIDEO_CD_LOCATION_3_ASM_NAME);
	decodeLoc = (CdlLOC *)(symbolPage + MM_VIDEO_CD_LOCATION_3_PAGE_OFFSET);
	CTR_PSX_LOAD_SYMBOL_PAGE(symbolPage, MM_VIDEO_IN_BUFFERS_ASM_NAME);
	inBuffers = (u32 **)(symbolPage + MM_VIDEO_IN_BUFFERS_PAGE_OFFSET);
	do
	{
		MM_VIDEO_LAST_BACKLOC = backloc;
		MM_VIDEO_LAST_SECTOR_FRAME_COUNT = MM_VIDEO_SECTOR_FRAME_COUNT;

		// retrieve data with timeout (10 frames)
	LAB_Video_GetNext:
		do
		{
			result = StGetNext(&ring.sectorData, ring.sectorHeader);
			if (result == 0)
			{
				register StHEADER *sectorHeader CTR_PSX_REGISTER("$5");
				register s32 previousFrame CTR_PSX_REGISTER("$4");
				register s32 currentFrame CTR_PSX_REGISTER("$3");
				register u32 sectorFramePage CTR_PSX_REGISTER("$2");

				sectorHeader = ring.sectorHeader[0];
				previousFrame = MM_VIDEO_LAST_SECTOR_FRAME_COUNT;
				currentFrame = sectorHeader->frameCount;
				CTR_PSX_LOAD_SYMBOL_PAGE(sectorFramePage, MM_VIDEO_SECTOR_FRAME_COUNT_ASM_NAME);
				CTR_PSX_PAGE_LVALUE(s32, sectorFramePage, MM_VIDEO_SECTOR_FRAME_COUNT_PAGE_OFFSET, MM_VIDEO_SECTOR_FRAME_COUNT) = currentFrame;

				if (currentFrame == previousFrame)
				{
					CTR_PSX_ORDER_VALUES(previousFrame, currentFrame);
					StFreeRing(ring.sectorData);
					CTR_PSX_CLOBBER("$2");
					goto LAB_Video_GetNext;
				}

				if (0 < CTR_PSX_PAGE_LVALUE(s32, loopStartPage, MM_VIDEO_LOOP_START_BACKLOC_PAGE_OFFSET, MM_VIDEO_LOOP_START_BACKLOC))
				{
					result = CdPosToInt(&sectorHeader->loc);
					waitTime = MM_VIDEO_VLC_WAIT_FRAMES;

					if (CTR_PSX_PAGE_LVALUE(s32, loopStartPage, MM_VIDEO_LOOP_START_BACKLOC_PAGE_OFFSET, MM_VIDEO_LOOP_START_BACKLOC) <= result)
					{
						register u32 *sectorData CTR_PSX_REGISTER("$4");

						sectorData = ring.sectorData;
						CTR_PSX_PAGE_LVALUE(s32, loopWrapPage, MM_VIDEO_LOOP_WRAP_PENDING_PAGE_OFFSET, MM_VIDEO_LOOP_WRAP_PENDING) = backlocNone;
						StFreeRing(sectorData);
						goto LAB_Video_GetNext;
					}
					if (CTR_PSX_PAGE_LVALUE(s32, loopWrapPage, MM_VIDEO_LOOP_WRAP_PENDING_PAGE_OFFSET, MM_VIDEO_LOOP_WRAP_PENDING) == backlocNone)
					{
						CTR_PSX_PAGE_LVALUE(s32, loopStartPage, MM_VIDEO_LOOP_START_BACKLOC_PAGE_OFFSET, MM_VIDEO_LOOP_START_BACKLOC) = MM_VIDEO_BACKLOC_NONE;
						CTR_PSX_PAGE_LVALUE(s32, loopWrapPage, MM_VIDEO_LOOP_WRAP_PENDING_PAGE_OFFSET, MM_VIDEO_LOOP_WRAP_PENDING) = 0;
						MM_VIDEO_LAST_BACKLOC = backloc;
					}
				}

				size = DecDCTBufSize(ring.sectorData);

				if (MM_VIDEO_VLC_BUFFER_SIZE < size)
				{
					CTR_PSX_CLOBBER("$2");
					MM_VIDEO_DRAW_NEXT_FRAME = 0;
					StFreeRing(ring.sectorData);
					return;
				}

				{
					register u32 symbolPage CTR_PSX_REGISTER("$2");
					register u32 *sectorData CTR_PSX_REGISTER("$4");
					register u32 a1Value CTR_PSX_REGISTER("$5");
					register void *vlcTable CTR_PSX_REGISTER("$6");
					register u32 v1Value CTR_PSX_REGISTER("$3");

					sectorData = ring.sectorData;
					a1Value = (u32)ring.sectorHeader[0];
					vlcTable = MM_VIDEO_VLC_TABLE;
					v1Value = ((StHEADER *)a1Value)->loc.minute;
					CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(symbolPage, MM_VIDEO_CD_LOCATION_3_ASM_NAME, v1Value);
					CTR_PSX_PAGE_LVALUE(u8, symbolPage, MM_VIDEO_CD_LOCATION_3_PAGE_OFFSET, decodeLoc->minute) = v1Value;
					decodeLoc->second = ((StHEADER *)a1Value)->loc.second;
					v1Value = a1Value;
					CTR_PSX_OBSERVE_VALUE(v1Value);
					decodeLoc->sector = ((StHEADER *)v1Value)->loc.sector;
					symbolPage = (u32)ring.sectorHeader[0];
					CTR_PSX_LOAD_SYMBOL_PAGE_AFTER(v1Value, MM_VIDEO_VLC_BUFFER_INDEX_ASM_NAME, symbolPage);
					a1Value = ((StHEADER *)symbolPage)->loc.track;
					symbolPage = CTR_PSX_PAGE_LVALUE(u16, v1Value, MM_VIDEO_VLC_BUFFER_INDEX_PAGE_OFFSET, MM_VIDEO_VLC_BUFFER_INDEX);
					symbolPage = (symbolPage << 2) + (u32)inBuffers;
					decodeLoc->track = a1Value;
					a1Value = *(u32 *)symbolPage;

					// VLC Decode
					// last parameter is "VLC Table"
					DecDCTvlc2(sectorData, (u32 *)a1Value, vlcTable);
				}

				// ready to draw next frame
				MM_VIDEO_DRAW_NEXT_FRAME = backlocNone;

				StFreeRing(ring.sectorData);
				return;
			}
			waitTime--;
		} while (waitTime != 0);

		MM_VIDEO_DRAW_NEXT_FRAME = 0;
	} while (0);
}
}

void MM_Video_StartStream(s32 cdStartSector, s32 streamFrameCount)
{
	MM_VIDEO_CD_RETRY_STATE = 0;
	MM_VIDEO_END_OF_STREAM = 0;
	MM_VIDEO_DECODE_STATE = 1;

	MM_VIDEO_DCT_OUTPUT_DONE = 0;

	MM_VIDEO_SECTOR_FRAME_COUNT = 0;
	MM_VIDEO_LAST_SECTOR_FRAME_COUNT = 0;
	MM_VIDEO_LAST_BACKLOC = 0;
	MM_VIDEO_LOOP_START_BACKLOC = MM_VIDEO_BACKLOC_NONE;
	MM_VIDEO_LOOP_END_BACKLOC = MM_VIDEO_BACKLOC_NONE;
	MM_VIDEO_LOOP_WRAP_PENDING = 0;
	MM_VIDEO_STALLED_BACKLOC_FRAME_COUNT = 0;
	MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;
	MM_VIDEO_UNUSED_0X38 = MM_VIDEO_BACKLOC_NONE;
	MM_VIDEO_DRAW_NEXT_FRAME = 0;

	MM_VIDEO_STREAM_FRAME_COUNT = streamFrameCount;

	// start streaming video
	CdIntToPos(cdStartSector, &MM_VIDEO_CD_LOCATION_1);

	// next parameter (0) = START_FRAME
	StSetStream((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_RGB24), 0, MM_VIDEO_STREAM_END_FRAME_NONE, 0, 0);

	CDSYS_SetMode_StreamData();

	// 800b6814 = Ring_Buf (mempack)
	StSetRing(MM_VIDEO_RING_BUFFER, MM_VIDEO_RING_SECTOR_COUNT);

	StClearRing();

	MM_VIDEO_CD_LOCATION_PTR = &MM_VIDEO_CD_LOCATION_1;
	MM_VIDEO_CD_KICK_STATE = 0;
}

void MM_Video_StopStream(void)
{
	int cdReady = CdDiskReady(1);
	if (cdReady == CdlComplete)
	{
		do
		{
			cdReady = CdControl(CdlPause, 0, 0);

		} while (cdReady == 0);
	}

	StClearRing();

	StSetMask(1, 0, 0);

	CdDataCallback(0);

	CdReadyCallback(0);

	// Discontinue current decoding,
	// does not affect internal states (libref)
	DecDCTReset(1);

	MM_VIDEO_DRAW_NEXT_FRAME = 0;
}

void MM_Video_AllocMem(s32 width, s16 height, s32 flags, s32 ringSectorCount, s32 vlcBufferShift)
{
	s32 bytesPerPixel;
	register s32 videoWidth CTR_PSX_REGISTER("$19");
	register s16 videoHeight CTR_PSX_REGISTER("$20");
	register s32 pixelWidth CTR_PSX_REGISTER("$4");
	s32 pixelBytes;
	s32 vlcRowSize;
	s32 dctRowSize;
	u32 paddedHeight;

	videoWidth = width;
	videoHeight = height;
	MEMPACK_PushState();

	MM_VIDEO_FLAGS = flags;

	if (ringSectorCount > 0)
	{
		MM_VIDEO_RING_SECTOR_COUNT = ringSectorCount;
	}
	else
	{
		MM_VIDEO_RING_SECTOR_COUNT = MM_VIDEO_DEFAULT_RING_SECTORS;
	}

	if ((MM_VIDEO_FLAGS & MM_VIDEO_FLAG_RGB24) != 0)
	{
		MM_VIDEO_DCT_MODE = 1;
		bytesPerPixel = 3;
	}
	else
	{
		MM_VIDEO_DCT_MODE = 0;
		bytesPerPixel = 2;
	}

	CTR_PSX_FORGET_VALUE(bytesPerPixel);
	pixelBytes = (u16)bytesPerPixel;
	CTR_PSX_KEEP_VALUE(pixelBytes);
	pixelWidth = (u16)videoWidth;
	vlcRowSize = pixelWidth * pixelBytes;
	paddedHeight = ((((s32)(((u16)videoHeight) - 1) >> 4) + 1) * 0x10) & 0xffff;
	dctRowSize = (pixelBytes << 3) * paddedHeight;
	MM_VIDEO_FINAL_SLICE_INDEX = (((((pixelWidth)-1) >> 4) + 1) & 0xfff) - 1;
	MM_VIDEO_DCT_OUT_BUFFER_INDEX = 0;
	MM_VIDEO_VLC_BUFFER_INDEX = 0;
	MM_VIDEO_DCT_OUT_SLICE_SIZE = (s32)dctRowSize >> 1;
	MM_VIDEO_VLC_BUFFER_SIZE = (s32)((vlcRowSize >> 1) * paddedHeight) >> (vlcBufferShift + 1);

	MM_VIDEO_OUT_BUFFERS[0] = MM_VIDEO_ALLOC_MEM(MM_VIDEO_DCT_OUT_SLICE_SIZE << 3, MM_VIDEO_SLICE_BUFFER_NAME);
	MM_VIDEO_OUT_BUFFERS[1] = (u32 *)(((int)MM_VIDEO_OUT_BUFFERS[0]) + MM_VIDEO_DCT_OUT_SLICE_SIZE * 4);

	MM_VIDEO_IN_BUFFERS[0] = MM_VIDEO_ALLOC_MEM(MM_VIDEO_VLC_BUFFER_SIZE << 3, MM_VIDEO_VLC_BUFFER_NAME);
	MM_VIDEO_IN_BUFFERS[1] = (u32 *)(((int)MM_VIDEO_IN_BUFFERS[0]) + MM_VIDEO_VLC_BUFFER_SIZE * 4);

	MM_VIDEO_RING_BUFFER = MM_VIDEO_ALLOC_MEM(MM_VIDEO_RING_SECTOR_COUNT << LOAD_CD_DATA_SECTOR_SHIFT, MM_VIDEO_RING_BUFFER_NAME);

	MM_VIDEO_SLICE.x = 0;
	MM_VIDEO_SLICE.y = 0;
	MM_VIDEO_SLICE.w = (s16)(bytesPerPixel << 3);
	MM_VIDEO_SLICE.h = videoHeight;

	// reinitialize everything
	DecDCTReset(0);

	DecDCTvlcSize2(MM_VIDEO_VLC_BUFFER_SIZE);
	EnterCriticalSection();
	DecDCToutCallback(&MM_Video_DecDCToutCallbackFunc);
	ExitCriticalSection();
}
#endif


void MM_Video_ClearMem(void)
{
	MEMPACK_PopState();
}

#ifndef CTR_NATIVE

b32 MM_Video_DecodeFrame(s16 offsetX, s16 offsetY)
{
	u16 canDrawFrame;
	int cdReady = CdDiskReady(1);


	if (MM_VIDEO_CD_RETRY_STATE == 1)
	{
		if (cdReady == 2)
		{
			MM_VIDEO_CD_RETRY_STATE = 0;
			MM_VIDEO_DRAW_NEXT_FRAME = 0;
			MM_Video_KickCD(&MM_VIDEO_CD_LOCATION_3);
			return 0;
		}
	}
	else
	{
		if (cdReady == CdlStatShellOpen)
		{
			MM_VIDEO_DECODE_STATE = 1;
			MM_VIDEO_STALL_RECOVERY_FRAME_COUNT = 0;
			MM_VIDEO_CD_RETRY_STATE = 1;
			MM_VIDEO_LAST_BACKLOC = MM_VIDEO_SECTOR_FRAME_COUNT - 1;
			StClearRing();
		}
	}
	if (MM_VIDEO_CD_RETRY_STATE == 1)
	{
		MM_VIDEO_DRAW_NEXT_FRAME = 0;
		canDrawFrame = false;
	}
	else
	{
		if (MM_VIDEO_CD_LOCATION_PTR != 0)
		{
			MM_Video_KickCD(0);
		}

		MM_Video_VLC_Decode();

		// if value is zero, return zero,
		// not ready to draw
		canDrawFrame = MM_VIDEO_DRAW_NEXT_FRAME;

		if (MM_VIDEO_DRAW_NEXT_FRAME == 1)
		{
			MM_VIDEO_SLICE_INDEX = 0;

			MM_VIDEO_SLICE.x = offsetX;
			MM_VIDEO_SLICE.y = offsetY;

			// start decoding video
			DecDCTin(MM_VIDEO_IN_BUFFERS[MM_VIDEO_VLC_BUFFER_INDEX], MM_VIDEO_DCT_MODE);

			MM_VIDEO_VLC_BUFFER_INDEX ^= 1;

			// get result of decoding
			DecDCTout(MM_VIDEO_OUT_BUFFERS[MM_VIDEO_DCT_OUT_BUFFER_INDEX], MM_VIDEO_DCT_OUT_SLICE_SIZE);

			// return 1, ready to draw
			canDrawFrame = MM_VIDEO_DRAW_NEXT_FRAME;
		}
	}
	return canDrawFrame;
}

b32 MM_Video_CheckIfFinished(s32 pollCdReady)
{
	int cdReady;
	volatile s32 timeoutFrames;
	volatile s32 cdPollTimer;
	register b32 cdReadError CTR_PSX_REGISTER("$16");

	timeoutFrames = MM_VIDEO_DCT_OUTPUT_TIMEOUT_POLLS;
	cdPollTimer = MM_VIDEO_CD_READY_POLL_INTERVAL;

	if (MM_VIDEO_DRAW_NEXT_FRAME == 0)
	{
		return 0;
	}

	cdReadError = false;

	do
	{
		if (pollCdReady == 1)
		{
			cdPollTimer--;

			if (cdPollTimer == 0)
			{
				cdReady = CdDiskReady(1);

				if (cdReady == CdlStatShellOpen)
				{
					cdReadError = true;
					MM_VIDEO_DCT_OUTPUT_DONE = 1;
				}
				else
				{
					cdPollTimer = MM_VIDEO_CD_READY_POLL_INTERVAL;
				}
			}
		}

		timeoutFrames--;

		if (timeoutFrames == 0)
		{
			MM_VIDEO_DCT_OUTPUT_DONE = 1;
		}

	} while (!MM_VIDEO_DCT_OUTPUT_DONE);

	do
	{
		cdReady = IsIdleGPU(MM_VIDEO_GPU_IDLE_TIMEOUT);

	} while (cdReady != 0);

	MM_VIDEO_DCT_OUTPUT_DONE = 0;

	MM_VIDEO_DRAW_NEXT_FRAME = 0;

	if ((!cdReadError) && (MM_VIDEO_SLICE_INDEX != MM_VIDEO_FINAL_SLICE_INDEX))
	{
		// Discontinue current decoding,
		// does not affect internal states (libref)
		DecDCTReset(1);
	}

	return (u16)MM_VIDEO_END_OF_STREAM;
}
#endif
