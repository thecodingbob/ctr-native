#include <common.h>

void LOAD_StringToUpper(char *path)
{
	for (u8 *letter = (u8 *)path; *letter != 0; letter++)
	{
		// if lowercase letter
		if ((u32)(*letter - 0x61) < 0x1a)
		{
			// uppercase
			*letter -= 0x20;
		}
	}
}

#include <platform/native_cd.h>

int LOAD_InitCDvol(void)
{
	// NOTE(aalhendi): Retail CdInit calls this hook to reset SPU volume
	// state before howl_InitGlobals re-applies game defaults. Native has
	// no CdInit path (CDSYS_Init(0) skips it), so mirror the SPU CD
	// volume write through the native SPU API to keep parity with the
	// retail init ordering. Master volume defaults are already handled
	// by NativeAudio_SpuInit (0x3fff), matching retail's "only if zero"
	// guard.
	SpuSetCommonCDVolume(0x3fff, 0x3fff);

	return 0;
}

void LOAD_InitCD()
{
	NativeCD_Init();
	CDSYS_Init(0);
	// NOTE(aalhendi): Retail chains LOAD_InitCD -> CDSYS_Init(1) -> CdInit
	// -> LOAD_InitCDvol. Native skips CdInit (no disc), so call the volume
	// hook explicitly to preserve the same init ordering.
	LOAD_InitCDvol();
	return;

	CDSYS_Init(1);
}

void *LOAD_ReadDirectory(char *filename)
{
	CdlFILE cdlFile;
	u8 buf[8];

	CDSYS_SetMode_StreamData();

	if (CdSearchFile(&cdlFile, filename) == NULL)
	{
		return NULL;
	}

	struct BigHeader *bh = MEMPACK_AllocMem(LOAD_BIGFILE_HEADER_ALLOC_BYTES, NULL /* filename */);

	// Search for file on disc
	// Set Cd laser to file position
	// Read the bigfile header
	// Wait for read to end
	CdControl(CdlSetloc, (u8 *)&cdlFile, buf);
	if (CdRead(LOAD_BIGFILE_HEADER_SECTORS, (u32 *)bh, CdlModeSpeed) == 0)
	{
		return NULL;
	}

	if (CdReadSync(0, 0) != 0)
	{
		return NULL;
	}

	// Save position
	bh->cdpos = CdPosToInt(&cdlFile.pos);

	// undo header allocation, only use "needed" size
	MEMPACK_ReallocMem(sizeof(struct BigHeader) + sizeof(struct BigEntry) * bh->numEntry);

	sdata->ptrBigfileCdPos_2 = bh;
	return bh;
}

void LOAD_DramFileCallback(struct LoadQueueSlot *lqs)
{
	char *fileBuf = lqs->ptrDestination;
	void (*callback)(struct LoadQueueSlot *) = lqs->callbackFuncPtr;

	if (fileBuf != NULL)
	{
		int ptrMapOffset = *(int *)&fileBuf[0];
		char *realFileBuf = &fileBuf[4];

		if (ptrMapOffset >= 0)
		{
			struct DramPointerMap *dpm = (struct DramPointerMap *)&realFileBuf[ptrMapOffset];

			LOAD_RunPtrMap(realFileBuf, (int *)DRAM_GETOFFSETS(dpm), dpm->numBytes >> 2);

			if ((lqs->flags & LT_MEMPACK) != 0)
			{
				MEMPACK_ReallocMem(ptrMapOffset + 4);
			}
		}
		else
		{
			lqs->flags |= LT_GETADDR;
		}

		lqs->ptrDestination = &fileBuf[4];
	}

	// Host callback pointers and queue sentinels are preserved.
	if ((callback != NULL) && (callback != LOAD_DramFileCallback) && (callback != (void (*)(struct LoadQueueSlot *))-1) &&
	    (callback != LOAD_QUEUE_CALLBACK_SET_POINTER))
	{
		callback(lqs);
	}

	sdata->queueReady = 1;
}

void *LOAD_DramFile(void *bigfilePtr, int subfileIndex, void *ptrDestination, u32 *sizePtr, int callbackOrFlags)
{
	struct LoadQueueSlot lqs;
	void *loadedFile;

	if (callbackOrFlags == -1)
	{
		loadedFile = LOAD_ReadFile_ex(bigfilePtr, LT_GETADDR, subfileIndex, ptrDestination, sizePtr, NULL);

		lqs.ptrBigfileCdPos_UNUSED = bigfilePtr;
		lqs.flags = 0;
		lqs.type_UNUSED = LT_DRAM;
		lqs.subfileIndex = subfileIndex;
		lqs.ptrDestination = loadedFile;
		lqs.size_UNUSED = *sizePtr;
		lqs.callbackFuncPtr = NULL;

		LOAD_DramFileCallback(&lqs);

		return loadedFile;
	}

	if (callbackOrFlags == -2)
	{
		loadedFile = LOAD_ReadFile_ex(bigfilePtr, LT_GETADDR, subfileIndex, NULL, sizePtr, LOAD_DramFileCallback);
		data.currSlot.ptrDestination = loadedFile;
		*(void **)ptrDestination = loadedFile;
		return loadedFile;
	}

	return LOAD_ReadFile_ex(bigfilePtr, LT_GETADDR, subfileIndex, ptrDestination, sizePtr, LOAD_DramFileCallback);
}

void LOAD_VramFileCallback(struct LoadQueueSlot *lqs)
{
	int *vramBuf = lqs->ptrDestination;

	struct VramHeader *vh = (struct VramHeader *)vramBuf;

	// if just one TIM
	if ((vramBuf != NULL) && (vramBuf[0] != 0x20))
	{
		LoadImage(&vh->rect, VRAMHEADER_GETPIXLES(vh));
	}

	// if multiple TIMs are packed together
	if ((vramBuf != NULL) && (vramBuf[0] == 0x20))
	{
		int size;
		vramBuf++;

		size = vramBuf[0];
		vh = (struct VramHeader *)&vramBuf[1];

		while (size != 0)
		{
			LoadImage(&vh->rect, VRAMHEADER_GETPIXLES(vh));

			// goto next
			vramBuf = (int *)((u8 *)vh + (size & ~3));

			size = vramBuf[0];
			vh = (struct VramHeader *)&vramBuf[1];
		}
	}

	// LOAD_NextQueuedFile waits 3 vsync frames before releasing the queue.
	sdata->frameFinishedVRAM = sdata->gGT->frameTimer_VsyncCallback;
}

void *LOAD_VramFile(void *bigfilePtr, int subfileIndex, void *ptrDestination, u32 *sizePtr, int callbackOrFlags)
{
	struct LoadQueueSlot lqs;
	void *loadedFile;

	if (ptrDestination == NULL)
	{
		MEMPACK_PushState();
	}

	if (callbackOrFlags == -1)
	{
		loadedFile = LOAD_ReadFile_ex(bigfilePtr, LT_VRAM, subfileIndex, ptrDestination, sizePtr, NULL);

		lqs.ptrBigfileCdPos_UNUSED = bigfilePtr;
		lqs.flags = 0;
		lqs.type_UNUSED = LT_VRAM;
		lqs.subfileIndex = subfileIndex;
		lqs.ptrDestination = loadedFile;
		lqs.size_UNUSED = *sizePtr;
		lqs.callbackFuncPtr = NULL;

		LOAD_VramFileCallback(&lqs);

		VSync(2);
		sdata->frameFinishedVRAM = 0;

		if (ptrDestination == NULL)
		{
			MEMPACK_PopState();
		}

		return loadedFile;
	}

	if (callbackOrFlags == -2)
	{
		loadedFile = LOAD_ReadFile_ex(bigfilePtr, LT_VRAM, subfileIndex, NULL, sizePtr, LOAD_VramFileCallback);
		data.currSlot.ptrDestination = loadedFile;
		*(void **)ptrDestination = loadedFile;
		return loadedFile;
	}

	return LOAD_ReadFile_ex(bigfilePtr, LT_VRAM, subfileIndex, ptrDestination, sizePtr, LOAD_VramFileCallback);
}

void LOAD_ReadFileASyncCallback(u8 result, u8 *unk)
{
	(void)unk;
	CdReadCallback(0);
	result &= 0xff;

	struct LoadQueueSlot *lqs = &data.currSlot;

	if (result == CdlComplete)
	{
		if ((lqs->flags & LT_MEMPACK) != 0)
		{
			MEMPACK_ReallocMem(lqs->size_UNUSED);
		}

		if (sdata->callbackCdReadSuccess != NULL)
		{
			sdata->callbackCdReadSuccess(lqs);
		}
	}

	// CdlDiskError
	else
	{
		if ((lqs->flags & LT_MEMPACK) != 0)
		{
			// undo allocation, try again
			MEMPACK_PopState();
		}

		sdata->queueRetry = 1;
	}
}

void *LOAD_ReadFile_ex(struct BigHeader *bigfile, u32 loadType, int subfileIndex, void *ptrDst, u32 *sizePtr, void (*callback)(struct LoadQueueSlot *))
{
	int uVar5;
	CdlLOC cdLoc;
	u8 paramOutput[8];
	void *originalDst;
	int sectorSize;
	int sectorCount;
	int readComplete;

	(void)loadType;
	CDSYS_SetMode_StreamData();

	// Existing queues that pass 0 for the default bigfile are
	// preserved; retail callers are expected to pass the real pointer.
	if (bigfile == NULL)
	{
		bigfile = sdata->ptrBigfile1;
	}

	// get size and offset of subfile
	struct BigEntry *entry = BIG_GETENTRY(bigfile);
	int eSize = entry[subfileIndex].size;
	int eOffs = entry[subfileIndex].offset;

	*sizePtr = eSize;

	CdIntToPos(bigfile->cdpos + eOffs, &cdLoc);

	struct LoadQueueSlot *lqs = &data.currSlot;
	originalDst = ptrDst;
	sectorCount = (eSize + LOAD_CD_DATA_SECTOR_ROUND_MASK) >> LOAD_CD_DATA_SECTOR_SHIFT;
	readComplete = 1;

	// If no address given, then find one.
	if (ptrDst == NULL)
	{
		lqs->flags |= LT_MEMPACK;

		// allocate room for all sectors,
		// remove alignment before next Read
		sectorSize = sectorCount << LOAD_CD_DATA_SECTOR_SHIFT;
		ptrDst = (void *)MEMPACK_AllocMem(sectorSize, NULL); // "FILE"
		if (ptrDst == NULL)
		{
			return NULL;
		}
	}
	else
	{
		lqs->flags &= ~LT_MEMPACK;
	}

	// NOTE(aalhendi): native CD reads can call back before wrapper callers store
	// the returned pointer back into data.currSlot.
	lqs->ptrDestination = ptrDst;
	lqs->size_UNUSED = eSize;

	while (1)
	{
		uVar5 = CdControl(CdlSetloc, (u8 *)&cdLoc, &paramOutput[0]);

		if (callback != NULL)
		{
			sdata->callbackCdReadSuccess = callback;
			CdReadCallback(LOAD_ReadFileASyncCallback);
		}
		else
		{
			sdata->callbackCdReadSuccess = NULL;
			CdReadCallback(NULL);
		}

		uVar5 &= CdRead(sectorCount, ptrDst, CdlModeSpeed);

		if (callback == NULL)
		{
			// Wait for all sectors to finish
			readComplete = CdReadSync(0, (u8 *)0x0) == 0;
		}

		// If either command failed, or sync read did not finish, retry.
		if ((uVar5 != 0) && (readComplete != 0))
		{
			break;
		}
	}

	if ((callback == NULL) && (originalDst == NULL))
	{
		MEMPACK_ReallocMem(*sizePtr);
	}

	return ptrDst;
}

// Used for XNF and only the XNF
void *LOAD_XnfFile(char *filename, void *ptrDestination, int *size)
{
	CdlFILE cdlFile;

	LOAD_StringToUpper(filename);
	CDSYS_SetMode_StreamData();

	if (CdSearchFile(&cdlFile, filename) == 0)
	{
		return ptrDestination;
	}

	*size = cdlFile.size;

	int allocated = ptrDestination == NULL;
	if (allocated)
	{
		// allocate room for all sectors,
		// remove alignment before next Read
		int sectorSize = (cdlFile.size + LOAD_CD_DATA_SECTOR_ROUND_MASK) & LOAD_CD_DATA_SECTOR_ALIGN_MASK;
		ptrDestination = MEMPACK_AllocMem(sectorSize, NULL /* fileName */);
		if (ptrDestination == NULL)
		{
			return NULL;
		}
	}

	u8 buf[8];
	CdControl(CdlSetloc, (u8 *)&cdlFile, buf);

	if (CdRead((cdlFile.size + LOAD_CD_DATA_SECTOR_ROUND_MASK) >> LOAD_CD_DATA_SECTOR_SHIFT, ptrDestination, CdlModeSpeed) == 0)
	{
		return 0;
	}

	if (CdReadSync(0, 0))
	{
		return 0;
	}

	if (allocated)
	{
		MEMPACK_ReallocMem(cdlFile.size);
	}

	return ptrDestination;
}

int LOAD_FindFile(char *filename, CdlFILE *cdlFile)
{
	if (filename == 0)
	{
		return 0;
	}
	if (cdlFile == 0)
	{
		return 0;
	}

	CDSYS_SetMode_StreamData();
	LOAD_StringToUpper(filename);

	return (CdSearchFile(cdlFile, filename) != 0);
}
