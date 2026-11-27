#ifndef CTR_NATIVE_DRAM_VALIDATE_H
#define CTR_NATIVE_DRAM_VALIDATE_H

#include <stddef.h>
#include <stdint.h>

/* Validate the relocation table before passing an external file to the game
   loader. This does not validate the format-specific objects in its payload. */
static inline uint32_t NativeDramValidate_Read32(const unsigned char *bytes)
{
	return (uint32_t)bytes[0] | (uint32_t)bytes[1] << 8 | (uint32_t)bytes[2] << 16 | (uint32_t)bytes[3] << 24;
}

static inline int NativeDramValidate_PointerMap(const void *data, size_t size, size_t minimumPayload)
{
	enum { WORD_BYTES = 4 };
	if (data == NULL || size < 2 * WORD_BYTES)
	{
		return 0;
	}
	const unsigned char *bytes = data;
	size_t payload = size - WORD_BYTES;
	uint32_t mapOffset = NativeDramValidate_Read32(bytes);
	if (mapOffset < minimumPayload || mapOffset > payload || payload - mapOffset < WORD_BYTES)
	{
		return 0;
	}
	const unsigned char *map = bytes + WORD_BYTES + mapOffset;
	uint32_t mapSize = NativeDramValidate_Read32(map);
	if (mapSize % WORD_BYTES != 0 || mapSize > payload - mapOffset - WORD_BYTES)
	{
		return 0;
	}
	for (size_t i = 0; i < mapSize; i += WORD_BYTES)
	{
		uint32_t slot = NativeDramValidate_Read32(map + WORD_BYTES + i) & ~UINT32_C(3);
		if (slot > mapOffset || mapOffset - slot < WORD_BYTES)
		{
			return 0;
		}
		if (NativeDramValidate_Read32(bytes + WORD_BYTES + slot) >= mapOffset)
		{
			return 0;
		}
	}
	return 1;
}

#endif
