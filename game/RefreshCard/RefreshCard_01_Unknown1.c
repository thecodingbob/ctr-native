#include <common.h>

void RefreshCard_Unknown1(void)
{
	sdata->memcardUnk1 = sdata->memcardUnk1 & 0xfffffff7 | 6;
}
