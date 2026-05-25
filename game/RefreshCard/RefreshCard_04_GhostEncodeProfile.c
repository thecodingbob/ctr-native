#include <common.h>

static int RefreshCard_GhostProfileNameExists(char *profileName)
{
	int i;

	for (i = 0; i < sdata->numGhostProfilesSaved; i++)
	{
		if (strcmp(sdata->ghostProfile_memcard[i].profile_name, profileName) == 0)
			return 1;
	}

	return 0;
}

static void RefreshCard_CopyBounded(char *dst, char *src, int len)
{
	int i;

	for (i = 0; i < len; i++)
	{
		dst[i] = src[i];
		if (src[i] == '\0')
			break;
	}

	for (; i < len; i++)
		dst[i] = '\0';
}

// TODO(aalhendi): Port the CTR_ScrambleGhostString side effect for the memcard icon header.
void RefreshCard_GhostEncodeProfile(u32 slotIndex, u16 characterID, u16 levelID, int time, char *name)
{
	struct GhostProfile *profile;
	char candidateName[0x15];
	u32 packedLow;
	u32 packedMid;
	u32 packedHigh;
	int i;
	int attempts;

	if (time > 0x8c9ff)
		time = 0x8c9ff;

	slotIndex &= 7;

	for (attempts = 0; attempts < 8; attempts++)
	{
		packedLow = (characterID & 0xffff) | ((u32)levelID << 4);
		packedMid = packedLow | ((u32)time << 9);
		packedHigh = packedMid | ((u32)slotIndex << 29);

		RefreshCard_CopyBounded(candidateName, data.s_BASCUS_94426G_Question, sizeof(candidateName));
		candidateName[13] = RefreshCard_GhostEncodeByte(packedLow & 0x3f);
		candidateName[14] = RefreshCard_GhostEncodeByte((packedMid & 0xfc0) >> 6);
		candidateName[15] = RefreshCard_GhostEncodeByte((packedMid & 0x3f000) >> 0xc);
		candidateName[16] = RefreshCard_GhostEncodeByte((packedMid & 0xfc0000) >> 0x12);
		candidateName[17] = RefreshCard_GhostEncodeByte((packedHigh >> 0x18) & 0x3f);
		candidateName[18] = RefreshCard_GhostEncodeByte(packedHigh >> 0x1e);
		candidateName[19] = '\0';

		if (RefreshCard_GhostProfileNameExists(candidateName) == 0)
			break;

		slotIndex = (slotIndex + 1) & 7;
	}

	profile = &sdata->ghostProfile_current;
	RefreshCard_CopyBounded(profile->profile_name, candidateName, sizeof(profile->profile_name));

	for (i = 0; i < sizeof(profile->SubmitName_name); i++)
		profile->SubmitName_name[i] = name[i];
	profile->SubmitName_name[sizeof(profile->SubmitName_name) - 1] = '\0';

	profile->alwaysOne = 1;
	profile->trackID = levelID;
	profile->characterID = characterID;
	profile->memcardProfileIndex = slotIndex;
	profile->trackTime = time;
}
