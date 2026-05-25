#include <common.h>

// NOTE(aalhendi): ASM-verified NTSC-U 926 0x8002c64c-0x8002c784
void howl_UnPauseChannel(struct ChannelStats *stats)
{
	int type;
	int soundID;
	struct ChannelAttr attr;

	type = stats->type;
	soundID = stats->soundID & 0xffff;

	// engineFX
	if (type == 0)
	{
		howl_InitChannelAttr_EngineFX(&sdata->howl_metaEngineFX[soundID], &attr, stats->vol, stats->LR, stats->distort);
	}

	// otherFX
	else if (type == 1)
	{
		howl_InitChannelAttr_OtherFX(&sdata->howl_metaOtherFX[soundID], &attr, stats->vol, stats->LR, stats->distort);
	}

	// music
	else if (type == 2)
	{
		howl_InitChannelAttr_Music(&sdata->songSeq[soundID], &attr, stats->drumIndex_pitchIndex, stats->vol);
	}
	else
	{
		return;
	}

	// enable all bits in ChannelUpdate flag
	sdata->ChannelUpdateFlags[stats->channelID] |= 0x7e;

	int *dest = (int *)&sdata->channelAttrNew[stats->channelID];
	int *src = (int *)&attr;
	dest[0] = src[0];
	dest[1] = src[1];
	dest[2] = src[2];
	dest[3] = src[3];
}
