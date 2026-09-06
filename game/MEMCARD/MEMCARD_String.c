#include <common.h>

char *MEMCARD_StringInit(int slotIndex, char *dstString)
{
	if (!dstString)
	{
		dstString = &sdata->s_memcardDirHeader[0];
	}

	dstString[0] = 'b';
	dstString[1] = 'u';
	dstString[2] = '0' + ((slotIndex >> 4) & 1);
	dstString[3] = '0' + (slotIndex & 3);
	dstString[4] = ':';
	dstString[5] = '\0';
	return dstString;
}

void MEMCARD_StringSet(char *dstString, int slotIdx, char *srcString)

{
	int i;
	MEMCARD_StringInit(slotIdx, dstString);

	// fast strlen
	for (i = 0; dstString[i] != '\0'; i++)
	{
	}

	// copy string from src to dst
	for (int j = 0; (srcString[j] != '\0' && i < 63); j++)
	{
		dstString[i] = srcString[j];
		i++;
	}

	// nullptr
	dstString[i] = '\0';
	return;
}
