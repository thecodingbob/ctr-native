#include <common.h>

// NOTE(aalhendi): ctr-native stubs host-unsupported card directory ops here;
// the retail implementations live in MEMCARD_16/18/21-25 and are not included.
void MEMCARD_GetFreeBytes(int slotIdx)
{
	(void)slotIdx;
	sdata->memoryCard_SizeRemaining = 0x1e000;
}

u8 MEMCARD_GetInfo(int slotIdx)
{
	(void)slotIdx;
	return MC_RETURN_IOE;
}

u8 MEMCARD_Format(int slotIdx)
{
	(void)slotIdx;
	return MC_RETURN_IOE;
}

int MEMCARD_IsFile(int slotIdx, char *save_name)
{
	(void)slotIdx;
	(void)save_name;
	return MC_RETURN_NODATA;
}

char *MEMCARD_FindFirstGhost(int slotIdx, char *srcString)
{
	(void)slotIdx;
	(void)srcString;
	return NULL;
}

char *MEMCARD_FindNextGhost(void)
{
	return NULL;
}

u8 MEMCARD_EraseFile(int slotIdx, char *srcString)
{
	(void)slotIdx;
	(void)srcString;
	return MC_RETURN_IOE;
}
