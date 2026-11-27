#include <common.h>
#include <custom-characters/character_resources.h>
#include <platform/native_cd.h>
#include <platform/native_spu_memory.h>
#include <platform/text/native_language_disc.h>
#include <platform/custom-characters/native_character_library.h>
#include <game/custom-characters/character_policy.h>

enum { HOWL_SAMPLE_SPU_UNIT = 8 };

enum { LOCAL_VOICE_FIRST = 28, LOCAL_VOICE_COUNT = 32, LOCAL_VOICE_FIRST_LANGUAGE = NATIVE_LANGUAGE_FRENCH, LOCAL_VOICE_LANGUAGES = NATIVE_LANGUAGE_DUTCH - NATIVE_LANGUAGE_FRENCH + 1 };

static bool Bank_CacheCanAllocate(void)
{
	if (sdata->bankLoadStage != 4 || sdata->songLoadStage != 3 || sdata->numAudioBanks == 0)
	{
		return false;
	}
	return sdata->bank[sdata->numAudioBanks - 1].bankID == HOWL_BANK_LEVEL_CACHE ||
	       sdata->numAudioBanks < (int)len(sdata->bank);
}

static void Bank_CacheCommit(u32 allocation)
{
	// The cache bank shares the existing level-teardown lifetime.
	if (sdata->bank[sdata->numAudioBanks - 1].bankID != HOWL_BANK_LEVEL_CACHE)
	{
		sdata->bank[sdata->numAudioBanks++] = (struct Bank){
			.bankID = HOWL_BANK_LEVEL_CACHE, .flags = 2, .min = sdata->audioAllocPtr,
		};
	}
	sdata->audioAllocPtr += CTR_SPU_UNITS(allocation);
	sdata->bank[sdata->numAudioBanks - 1].max += CTR_SPU_UNITS(allocation);
}

struct BankCachedVoice
{
	struct OtherFX meta;
	u16 addr;
};

static struct BankCachedVoice s_bankLocalizedVoices[LOCAL_VOICE_LANGUAGES][LOCAL_VOICE_COUNT];
static struct BankCachedVoice s_bankCustomVoices[NATIVE_CHARACTER_SHORT_VOICES];
static char s_bankCustomVoiceDirectory[NATIVE_CHARACTER_PATH_BYTES];

static u16 Bank_UploadVoiceSample(const unsigned char *bytes, size_t size)
{
	if (!Bank_CacheCanAllocate()) { return 0; }
	u32 destination = CTR_SPU_BYTES_AT((u32)sdata->audioAllocPtr);
	u32 allocation = (u32)((size + (1u << CTR_SPU_ADDR_SHIFT) - 1) & ~((1u << CTR_SPU_ADDR_SHIFT) - 1));
	if (size == 0 || size > CTR_SPU_CEILING || size % HOWL_SAMPLE_SPU_UNIT != 0 || destination + allocation >= CTR_SPU_CEILING) { return 0; }
	SpuSetTransferStartAddr(destination);
	if (SpuWrite((u8 *)bytes, (u32)size) != size) { return 0; }
	while (!SpuIsTransferCompleted(SPU_TRANSFER_PEEK)) {}
	u16 address = sdata->audioAllocPtr;
	Bank_CacheCommit(allocation);
	return address;
}

static bool Bank_CacheVoiceSample(struct BankCachedVoice *cached, unsigned char *bytes, size_t size, struct OtherFX metadata)
{
	cached->addr = Bank_UploadVoiceSample(bytes, size);
	free(bytes);
	if (cached->addr == 0)
	{
		return false;
	}
	cached->meta = metadata;
	return true;
}

void Bank_InvalidateLocalizedVoices(void)
{
	memset(s_bankLocalizedVoices, 0, sizeof(s_bankLocalizedVoices));
	memset(s_bankCustomVoices, 0, sizeof(s_bankCustomVoices));
	s_bankCustomVoiceDirectory[0] = '\0';
}

void Bank_ClearLocalizedVoices(u16 min, u16 size)
{
	u32 end = (u32)min + size;
	for (size_t i = 0; i < len(s_bankCustomVoices); i++)
	{
		if (s_bankCustomVoices[i].addr >= min && s_bankCustomVoices[i].addr < end) { s_bankCustomVoices[i].addr = 0; }
	}
	for (int language = 0; language < LOCAL_VOICE_LANGUAGES; language++)
	{
		for (int i = 0; i < LOCAL_VOICE_COUNT; i++)
		{
			struct BankCachedVoice *voice = &s_bankLocalizedVoices[language][i];
			if (voice->addr >= min && voice->addr < end)
			{
				voice->addr = 0;
			}
		}
	}
}

bool Bank_LocalizedVoice(u16 soundID, struct OtherFX *meta, u16 *spuAddr)
{
	int language = g_config.voiceLanguage - LOCAL_VOICE_FIRST_LANGUAGE;
	int index = (int)soundID - LOCAL_VOICE_FIRST;
	if (language < 0 || language >= LOCAL_VOICE_LANGUAGES || index < 0 || index >= LOCAL_VOICE_COUNT ||
	    meta == NULL || spuAddr == NULL || !sdata->boolAudioEnabled)
	{
		return false;
	}
	struct BankCachedVoice *cached = &s_bankLocalizedVoices[language][index];
	if (cached->addr == 0)
	{
		if (!Bank_CacheCanAllocate())
		{
			return false;
		}
		struct NativeLanguageVoiceSample sample;
		if (!NativeLanguageDisc_ReadVoiceSample(g_config.voiceLanguage, soundID, &sample))
		{
			return false;
		}
		if (!Bank_CacheVoiceSample(cached, sample.data, sample.size,
		                          (struct OtherFX){.flags = sample.flags, .volume = sample.volume, .pitch = sample.pitch, .duration = sample.duration}))
		{
			return false;
		}
	}
	*meta = cached->meta;
	*spuAddr = cached->addr;
	return true;
}

bool Bank_CustomCharacterVoice(u16 soundID, struct OtherFX *meta, u16 *spuAddr)
{
	int index = CustomCharacter_GetShortVoiceIndex(soundID);
	if (index < 0 || meta == NULL || spuAddr == NULL || !sdata->boolAudioEnabled)
	{
		return false;
	}
	const char *directory = CustomCharacter_GetAudioDirectory();
	if (directory == NULL)
	{
		return false;
	}
	if (strcmp(s_bankCustomVoiceDirectory, directory) != 0)
	{
		memset(s_bankCustomVoices, 0, sizeof(s_bankCustomVoices));
		memcpy(s_bankCustomVoiceDirectory, directory, sizeof(s_bankCustomVoiceDirectory));
	}
	struct BankCachedVoice *cached = &s_bankCustomVoices[index];
	if (cached->addr == 0)
	{
		struct NativeCharacterVoiceSample sample;
		if (!Bank_CacheCanAllocate() || !NativeCharacterLibrary_ReadVoice(s_bankCustomVoiceDirectory, index, &sample))
		{
			return false;
		}
		if (!Bank_CacheVoiceSample(cached, sample.data, sample.size,
		                          (struct OtherFX){.flags = sample.flags, .volume = sample.volume, .pitch = sample.pitch, .duration = sample.duration}))
		{
			return false;
		}
	}
	*meta = cached->meta;
	*spuAddr = cached->addr;
	return true;
}

static bool Bank_FindSample(u16 sampleIndex, u32 *sampleOffset)
{
	u16 sector[LOAD_CD_DATA_SECTOR_SIZE / sizeof(u16)];
	for (int bank = 0; bank < sdata->ptrHowlHeader->numBanks; bank++)
	{
		const u32 firstSector = sdata->howl_bankOffsets[bank];
		if (!NativeCD_ReadFileSectorsAt(&sdata->KartHWL_CdFile, firstSector, 1, sector))
		{
			return false;
		}
		const u16 count = sector[0];
		if (count > sizeof(sector) / sizeof(sector[0]) - 1)
		{
			return false;
		}
		u32 offset = (firstSector + 1) * LOAD_CD_DATA_SECTOR_SIZE;
		for (int i = 0; i < count; i++)
		{
			const u16 index = sector[i + 1];
			if (index >= sdata->ptrHowlHeader->numSpuAddrs)
			{
				return false;
			}
			if (index == sampleIndex)
			{
				*sampleOffset = offset;
				return true;
			}
			offset += sdata->howl_spuAddrs[index].spuSize * HOWL_SAMPLE_SPU_UNIT;
		}
	}
	return false;
}

static bool Bank_UploadSample(u32 source, u32 destination, u32 size)
{
	u8 sector[LOAD_CD_DATA_SECTOR_SIZE];
	while (size != 0)
	{
		const u32 skip = source % LOAD_CD_DATA_SECTOR_SIZE;
		u32 count = LOAD_CD_DATA_SECTOR_SIZE - skip;
		if (count > size)
		{
			count = size;
		}
		if (!NativeCD_ReadFileSectorsAt(&sdata->KartHWL_CdFile, source / LOAD_CD_DATA_SECTOR_SIZE, 1, sector))
		{
			return false;
		}
		SpuSetTransferStartAddr(destination);
		if (SpuWrite(sector + skip, count) != count)
		{
			return false;
		}
		while (!SpuIsTransferCompleted(SPU_TRANSFER_PEEK))
		{
		}
		source += count;
		destination += count;
		size -= count;
	}
	return true;
}

bool Bank_EnsureOtherFX(u16 soundID)
{
	if (!sdata->boolAudioEnabled || sdata->ptrHowlHeader == NULL ||
	    soundID >= sdata->ptrHowlHeader->numOtherFX)
	{
		return false;
	}
	const u16 index = sdata->howl_metaOtherFX[soundID].spuIndex;
	if (index >= sdata->ptrHowlHeader->numSpuAddrs)
	{
		return false;
	}
	struct SpuAddrEntry *sample = &sdata->howl_spuAddrs[index];
	if (sample->spuAddr != 0)
	{
		return true;
	}
	if (!Bank_CacheCanAllocate())
	{
		return false;
	}

	const u32 size = sample->spuSize * HOWL_SAMPLE_SPU_UNIT;
	const u32 destination = CTR_SPU_BYTES_AT((u32)sdata->audioAllocPtr);
	if (size == 0 || size % (1 << CTR_SPU_ADDR_SHIFT) != 0 || destination + size >= CTR_SPU_CEILING)
	{
		return false;
	}
	u32 source;
	if (!Bank_FindSample(index, &source) || !Bank_UploadSample(source, destination, size))
	{
		return false;
	}
	sample->spuAddr = sdata->audioAllocPtr;
	Bank_CacheCommit(size);
	return true;
}
