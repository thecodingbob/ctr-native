#include <platform/custom-characters/native_character_model.h>
#include <platform/native_dram_validate.h>
#include <platform/native_external_file.h>

#include <stdint.h>

enum
{
	CHARACTER_MODEL_FILE_PREFIX_BYTES = 4,
	CHARACTER_MODEL_GROUP_BYTES = 24,
	CHARACTER_MODEL_HEADER_BYTES = 64,
	CHARACTER_MODEL_COUNT_OFFSET = 18,
	CHARACTER_MODEL_HEADERS_OFFSET = 20,
};

int NativeCharacterModel_CheckEnvelope(const void *data, size_t size)
{
	if (!NativeDramValidate_PointerMap(data, size, CHARACTER_MODEL_GROUP_BYTES))
	{
		return 0;
	}
	const unsigned char *bytes = data;
	const unsigned char *group = bytes + CHARACTER_MODEL_FILE_PREFIX_BYTES;
	uint32_t payloadSize = NativeDramValidate_Read32(bytes);
	unsigned int count = (unsigned int)group[CHARACTER_MODEL_COUNT_OFFSET] |
	                     (unsigned int)group[CHARACTER_MODEL_COUNT_OFFSET + 1] << 8;
	uint32_t headers = NativeDramValidate_Read32(group + CHARACTER_MODEL_HEADERS_OFFSET);
	if (count == 0 || count > INT16_MAX || headers < CHARACTER_MODEL_GROUP_BYTES || headers % 4 != 0 ||
	    headers > payloadSize || count > (payloadSize - headers) / CHARACTER_MODEL_HEADER_BYTES)
	{
		return 0;
	}

	/* LOAD_RunPtrMap must relocate the group header pointer before use. */
	const unsigned char *map = group + payloadSize;
	uint32_t mapSize = NativeDramValidate_Read32(map);
	for (size_t i = 0; i < mapSize; i += 4)
	{
		if ((NativeDramValidate_Read32(map + 4 + i) & ~UINT32_C(3)) == CHARACTER_MODEL_HEADERS_OFFSET)
		{
			return 1;
		}
	}
	return 0;
}

int NativeCharacterModel_Read(struct NativeExternalFile *file, void *destination, size_t capacity)
{
	return NativeExternalFile_Read(file, destination, capacity) &&
	       NativeCharacterModel_CheckEnvelope(destination, NativeExternalFile_Size(file));
}
