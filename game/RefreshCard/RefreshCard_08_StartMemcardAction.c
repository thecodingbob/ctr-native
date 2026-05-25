#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x80047198-0x800471ac.
void RefreshCard_StartMemcardAction(int action)
{
	sdata->mcStart = action;
	sdata->unk8008d964 = 0;
	sdata->boolError = 0;
}
