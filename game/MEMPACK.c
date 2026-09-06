#include <common.h>


void MEMPACK_Init(int ramSize)
{
	(void)ramSize;
	u32 startPtr;
	s32 packSize;

#if defined(CTR_NATIVE)

	const struct PlatformMempackArena *arena = Platform_InitMempackArena();

	startPtr = (u32)arena->start;
	packSize = arena->size;

	printf("[CTR] MEMPACK native backing: base=%08x\n", (u32)arena->base);

	MEMPACK_NewPack((void *)startPtr, packSize);
	sdata->PtrMempack->endOfAllocator = (void *)(startPtr + packSize);
	sdata->PtrMempack->endOfMemory = arena->endOfMemory;

	printf("[CTR] MEMPACK native arena: start=%08x size=%08x end=%08x\n", startPtr, packSize, (u32)sdata->PtrMempack->endOfAllocator);

#else

	maxOverlayEnd = (u32)AH_EndOfFile;
	if (maxOverlayEnd < (u32)RB_EndOfFile)
		maxOverlayEnd = (u32)RB_EndOfFile;
	if (maxOverlayEnd < (u32)MM_EndOfFile)
		maxOverlayEnd = (u32)MM_EndOfFile;
	if (maxOverlayEnd < (u32)CS_EndOfFile)
		maxOverlayEnd = (u32)CS_EndOfFile;

	startPtr = (u32)OVR_Region3 + (((maxOverlayEnd - (u32)OVR_Region3) + MEMPACK_PS1_OVERLAY_ALIGNMENT_MASK) & ~MEMPACK_PS1_OVERLAY_ALIGNMENT_MASK);
	packSize = ramSize - (int)(startPtr & MEMPACK_PS1_RAM_ADDRESS_MASK) - MEMPACK_PS1_END_GUARD_SIZE;

	ptrMempack = sdata->PtrMempack;
	ptrMempack->start = (void *)startPtr;
	ptrMempack->endOfAllocator = (void *)(startPtr + packSize);
	ptrMempack->lastFreeByte = (void *)(startPtr + packSize);
	ptrMempack->packSize = packSize;
	ptrMempack->numBookmarks = 0;
	ptrMempack->endOfMemory = (void *)MEMPACK_PS1_END_OF_MEMORY;
	ptrMempack->firstFreeByte = (void *)startPtr;
#endif
}


void MEMPACK_SwapPacks(int index)
{
	sdata->PtrMempack = &sdata->mempack[index];
}


void MEMPACK_NewPack(void *start, int size)
{
	struct Mempack *ptrMempack = sdata->PtrMempack;
	void *end = (void *)((u32)start + size);

	ptrMempack->packSize = size;
	ptrMempack->start = start;
	ptrMempack->lastFreeByte = end;
	ptrMempack->endOfMemory = end;
	ptrMempack->firstFreeByte = start;
	ptrMempack->numBookmarks = 0;
}


int MEMPACK_GetFreeBytes()
{
	struct Mempack *ptrMempack = sdata->PtrMempack;

	return (u32)ptrMempack->lastFreeByte - (u32)ptrMempack->firstFreeByte;
}


void *MEMPACK_AllocMem(int allocSize)
{
	struct Mempack *ptrMempack = sdata->PtrMempack;

	if (MEMPACK_GetFreeBytes() < allocSize)
	{
		CTR_ErrorScreen(0xFF, 0, 0);
		for (;;)
		{
		}
	}

	s32 newAllocSize = MEMPACK_ALIGN_SIZE(allocSize);
	ptrMempack->sizeOfPrevAllocation = newAllocSize;

	s32 firstFreeByte = (s32)ptrMempack->firstFreeByte;
	ptrMempack->firstFreeByte = (void *)(firstFreeByte + newAllocSize);

	return (void *)firstFreeByte;
}


void *MEMPACK_AllocHighMem(int allocSize)
{
	while (MEMPACK_GetFreeBytes() < allocSize)
	{
	}

	allocSize = MEMPACK_ALIGN_SIZE(allocSize);
	sdata->PtrMempack->sizeOfPrevAllocation = allocSize;

	s32 newLastFreeByte = (s32)sdata->PtrMempack->lastFreeByte - allocSize;
	sdata->PtrMempack->lastFreeByte = (void *)newLastFreeByte;

	return (void *)newLastFreeByte;
}


void MEMPACK_ClearHighMem()
{
	sdata->PtrMempack->lastFreeByte = sdata->PtrMempack->endOfAllocator;
}


void *MEMPACK_ReallocMem(int allocSize)
{
	struct Mempack *ptrMempack = sdata->PtrMempack;

	s32 newAllocSize = MEMPACK_ALIGN_SIZE(allocSize);
	ptrMempack->firstFreeByte = (void *)((s32)ptrMempack->firstFreeByte - ptrMempack->sizeOfPrevAllocation + newAllocSize);
	ptrMempack->sizeOfPrevAllocation = newAllocSize;

	return ptrMempack->firstFreeByte;
}


int MEMPACK_PushState()
{
	struct Mempack *ptrMempack = sdata->PtrMempack;
	s32 numBookmarks = ptrMempack->numBookmarks;
	if (numBookmarks < MEMPACK_BOOKMARK_COUNT)
	{
		ptrMempack->bookmarks[numBookmarks] = ptrMempack->firstFreeByte;
		ptrMempack->numBookmarks++;
	}

	return numBookmarks;
}


void MEMPACK_ClearLowMem()
{
	struct Mempack *ptrMempack = sdata->PtrMempack;

	ptrMempack->numBookmarks = 0;
	ptrMempack->firstFreeByte = ptrMempack->start;
}


void MEMPACK_PopState()
{
	struct Mempack *ptrMempack = sdata->PtrMempack;
	s32 numBookmarks = ptrMempack->numBookmarks;
	if (numBookmarks > 0)
	{
		numBookmarks--;
		ptrMempack->firstFreeByte = ptrMempack->bookmarks[numBookmarks];
		ptrMempack->numBookmarks = numBookmarks;
	}
}


void MEMPACK_PopToState(int id)
{
	struct Mempack *ptrMempack = sdata->PtrMempack;

	ptrMempack->numBookmarks = id;
	ptrMempack->firstFreeByte = ptrMempack->bookmarks[id];
}
