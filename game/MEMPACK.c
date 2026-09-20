#include <common.h>


void MEMPACK_Init(s32 ramSize)
{
	u32 startPtr;

#if defined(CTR_NATIVE)
	// NOTE(aalhendi): Native uses host-backed RAM; PSX reserves the arena after the largest overlay.
	s32 packSize;
	const struct PlatformMempackArena *arena = Platform_InitMempackArena();
	(void)ramSize;

	startPtr = (u32)arena->start;
	packSize = arena->size;

	printf("[CTR] MEMPACK native backing: base=%08x\n", (u32)arena->base);

	MEMPACK_NewPack((void *)startPtr, packSize);
	MEMPACK_ACTIVE->endOfAllocator = (void *)(startPtr + packSize);
	MEMPACK_ACTIVE->endOfMemory = arena->endOfMemory;

	printf("[CTR] MEMPACK native arena: start=%08x size=%08x end=%08x\n", startPtr, packSize, (u32)MEMPACK_ACTIVE->endOfAllocator);

#else
	u32 maxOverlayEnd;
	struct Mempack *ptrMempack;
	register u32 addressMask CTR_PSX_REGISTER("$3");
	register u32 overlayBase CTR_PSX_REGISTER("$4");

	// NOTE(aalhendi): Keep the two candidate lifetimes through the join so GCC retains retail's max-selection branches.
	register u32 overlayEndA CTR_PSX_REGISTER("$4") = (u32)AH_EndOfFile;
	register u32 overlayEndB CTR_PSX_REGISTER("$3") = (u32)RB_EndOfFile;
	if (overlayEndA < overlayEndB)
	{
		overlayEndA = (u32)MM_EndOfFile;
		if (overlayEndA >= overlayEndB)
			maxOverlayEnd = overlayEndA;
		else
			maxOverlayEnd = overlayEndB;
	}
	else
	{
		overlayEndB = (u32)MM_EndOfFile;
		CTR_PSX_KEEP_VALUE_RELAXED(overlayEndB);
		if (overlayEndB >= overlayEndA)
			maxOverlayEnd = overlayEndB;
		else
			maxOverlayEnd = overlayEndA;
	}
	CTR_PSX_OBSERVE_VALUE(overlayEndA);
	CTR_PSX_OBSERVE_VALUE(overlayEndB);
	overlayEndB = (u32)CS_EndOfFile;
	if (overlayEndB >= maxOverlayEnd)
		maxOverlayEnd = overlayEndB;
	CTR_PSX_OBSERVE_VALUE(overlayEndB);

	// Round the overlay footprint up to a CD sector, relative to its load address.
	// Leave the final sector of RAM outside the allocator.
	addressMask = MEMPACK_PS1_RAM_ADDRESS_MASK;
	overlayBase = (u32)OVR_Region3;
	startPtr = overlayBase + ((((maxOverlayEnd - overlayBase) + MEMPACK_PS1_OVERLAY_ALIGNMENT_MASK) >> 11) << 11);
	addressMask &= startPtr;
	ramSize -= addressMask;
	ramSize -= MEMPACK_PS1_END_GUARD_SIZE;

	ptrMempack = MEMPACK_ACTIVE;
	ptrMempack->start = (void *)startPtr;
	ptrMempack->endOfAllocator = (void *)(startPtr + ramSize);
	ptrMempack->lastFreeByte = (void *)(startPtr + ramSize);
	ptrMempack->packSize = ramSize;
	ptrMempack->numBookmarks = 0;
	ptrMempack->endOfMemory = (void *)MEMPACK_PS1_END_OF_MEMORY;
	ptrMempack->firstFreeByte = ptrMempack->start;
#endif
}


void MEMPACK_SwapPacks(s32 index)
{
	MEMPACK_ACTIVE = &MEMPACK_POOLS[index];
	// NOTE(aalhendi): Retail publishes the active pack before the return delay slot.
	CTR_PSX_OBSERVE_MEMORY(MEMPACK_ACTIVE);
}


void MEMPACK_NewPack(void *start, s32 size)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;
	ptrMempack->start = start;
	// NOTE(aalhendi): Preserve retail's start-pointer readback instead of forwarding the argument.
	CTR_PSX_RELOAD(ptrMempack->start);
	start = (u8 *)start + size;
	ptrMempack->lastFreeByte = start;
	ptrMempack->packSize = size;
	ptrMempack->numBookmarks = 0;
	ptrMempack->endOfMemory = start;
	ptrMempack->firstFreeByte = ptrMempack->start;
}


inline s32 MEMPACK_GetFreeBytes(void)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;

	return (u32)ptrMempack->lastFreeByte - (u32)ptrMempack->firstFreeByte;
}


void *MEMPACK_AllocMem(s32 allocSize, const char *name)
{
	struct Mempack *ptrMempack;
	register s32 newAllocSize CTR_PSX_REGISTER("$5");
	void *firstFreeByte;
	u8 *cursor;
	(void)name;

	if (MEMPACK_GetFreeBytes() < allocSize)
	{
		CTR_ErrorScreen(0xFF, 0, 0);
		for (;;)
		{
			CTR_TRAP();
		}
	}

	// NOTE(aalhendi): Keep rounding in a1 while the old cursor remains available for the return value.
	newAllocSize = allocSize + MEMPACK_ALIGNMENT_MASK;
	CTR_PSX_KEEP_VALUE_RELAXED(newAllocSize);
	newAllocSize &= MEMPACK_ALIGNMENT_CLEAR_MASK;
	ptrMempack = MEMPACK_ACTIVE;
	ptrMempack->sizeOfPrevAllocation = newAllocSize;

	cursor = ptrMempack->firstFreeByte;
	firstFreeByte = (void *)cursor;
	cursor += newAllocSize;
	ptrMempack->firstFreeByte = (void *)cursor;

	return (void *)firstFreeByte;
}


void *MEMPACK_AllocHighMem(s32 allocSize, const char *name)
{
	u8 *newLastFreeByte;
	s32 newAllocSize;
	struct Mempack *ptrMempack;
	(void)name;

	if (MEMPACK_GetFreeBytes() < allocSize)
	{
		for (;;)
		{
			CTR_TRAP();
		}
	}

	newAllocSize = MEMPACK_ALIGN_SIZE(allocSize);
	ptrMempack = MEMPACK_ACTIVE;
	ptrMempack->sizeOfPrevAllocation = newAllocSize;

	newLastFreeByte = (u8 *)ptrMempack->lastFreeByte - newAllocSize;
	ptrMempack->lastFreeByte = (void *)newLastFreeByte;

	return (void *)newLastFreeByte;
}


void MEMPACK_ClearHighMem(void)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;
	ptrMempack->lastFreeByte = ptrMempack->endOfAllocator;
}


void *MEMPACK_ReallocMem(s32 allocSize)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;

	// Resize the last low allocation in place; the return value is its new end, not its start.
	s32 newAllocSize = MEMPACK_ALIGN_SIZE(allocSize);
	ptrMempack->firstFreeByte = (void *)((u8 *)ptrMempack->firstFreeByte - ptrMempack->sizeOfPrevAllocation + newAllocSize);
	ptrMempack->sizeOfPrevAllocation = newAllocSize;

	return ptrMempack->firstFreeByte;
}


s32 MEMPACK_PushState(void)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;
	s32 numBookmarks = ptrMempack->numBookmarks;
	if (numBookmarks < MEMPACK_BOOKMARK_COUNT)
	{
		ptrMempack->bookmarks[numBookmarks] = ptrMempack->firstFreeByte;
		ptrMempack->numBookmarks = numBookmarks + 1;
		return numBookmarks;
	}

	return numBookmarks;
}


void MEMPACK_ClearLowMem(void)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;

	ptrMempack->numBookmarks = 0;
	ptrMempack->firstFreeByte = ptrMempack->start;
}


void MEMPACK_PopState(void)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;
	s32 numBookmarks = ptrMempack->numBookmarks;
	if (numBookmarks > 0)
	{
		numBookmarks--;
		ptrMempack->numBookmarks = numBookmarks;
		ptrMempack->firstFreeByte = ptrMempack->bookmarks[numBookmarks];
	}
}


void MEMPACK_PopToState(s32 id)
{
	struct Mempack *ptrMempack = MEMPACK_ACTIVE;

	ptrMempack->numBookmarks = id;
	ptrMempack->firstFreeByte = ptrMempack->bookmarks[id];
}
