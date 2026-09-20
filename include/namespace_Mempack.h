#ifndef CTR_NATIVE_NAMESPACE_MEMPACK_H
#define CTR_NATIVE_NAMESPACE_MEMPACK_H

enum MempackConstants
{
	MEMPACK_BOOKMARK_COUNT = 0x10,

	MEMPACK_ALIGNMENT = 4,
	MEMPACK_ALIGNMENT_MASK = MEMPACK_ALIGNMENT - 1,
	MEMPACK_ALIGNMENT_CLEAR_MASK = -MEMPACK_ALIGNMENT,

	MEMPACK_PS1_RAM_ADDRESS_MASK = 0xffffff,
	MEMPACK_PS1_OVERLAY_ALIGNMENT = 0x800,
	MEMPACK_PS1_OVERLAY_ALIGNMENT_MASK = MEMPACK_PS1_OVERLAY_ALIGNMENT - 1,
	MEMPACK_PS1_END_GUARD_SIZE = 0x800,
	MEMPACK_PS1_END_OF_MEMORY = 0x80200000,
};

#define MEMPACK_ALIGN_SIZE(size) (((size) + MEMPACK_ALIGNMENT_MASK) & MEMPACK_ALIGNMENT_CLEAR_MASK)

#ifndef MEMPACK_ACTIVE
#define MEMPACK_ACTIVE sdata->PtrMempack
#endif

#ifndef MEMPACK_POOLS
#define MEMPACK_POOLS sdata->mempack
#endif

struct Mempack
{
	// 0x0
	s32 packSize; // allocator capacity in bytes

	// 0x4
	void *start; // low cursor reset point

	// 0x8
	void *lastFreeByte; // high allocations grow downward from here

	// 0xC
	void *endOfAllocator; // high cursor reset point; NewPack leaves it unchanged

	// 0x10
	void *endOfMemory; // backing limit; Init includes the reserved end-of-RAM sector

	// 0x14
	void *firstFreeByte; // low allocations grow upward from here

	// 0x18
	s32 sizeOfPrevAllocation; // rounded size of the last low or high allocation

	// 0x1C
	s32 numBookmarks; // amount of bookmarks used

	// 0x20
	void *bookmarks[MEMPACK_BOOKMARK_COUNT]; // address of each bookmark

	// 0x60 -- size of struct
};

CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, packSize) == 0x0);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, start) == 0x4);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, lastFreeByte) == 0x8);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, endOfAllocator) == 0xc);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, endOfMemory) == 0x10);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, firstFreeByte) == 0x14);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, sizeOfPrevAllocation) == 0x18);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, numBookmarks) == 0x1c);
CTR_STATIC_ASSERT(OFFSETOF(struct Mempack, bookmarks) == 0x20);
CTR_STATIC_ASSERT(sizeof(struct Mempack) == 0x60);

#endif
