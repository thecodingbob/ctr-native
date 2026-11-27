#ifndef CTR_NATIVE_CHARACTER_LIBRARY_H
#define CTR_NATIVE_CHARACTER_LIBRARY_H

#include <stddef.h>

enum { NATIVE_CHARACTER_TEXT_BYTES = 256, NATIVE_CHARACTER_PATH_BYTES = 1024,
       NATIVE_CHARACTER_SHORT_VOICES = 2, NATIVE_CHARACTER_XA_VOICES = 19 };

struct NativeCharacterMetadata
{
	char identity[NATIVE_CHARACTER_TEXT_BYTES];
	char name[NATIVE_CHARACTER_TEXT_BYTES];
	char shortName[NATIVE_CHARACTER_TEXT_BYTES];
	char author[NATIVE_CHARACTER_TEXT_BYTES];
	char base[NATIVE_CHARACTER_TEXT_BYTES];
	char modelHi[NATIVE_CHARACTER_PATH_BYTES];
	char iconPixels[NATIVE_CHARACTER_PATH_BYTES];
	char iconClut[NATIVE_CHARACTER_PATH_BYTES];
	char directory[NATIVE_CHARACTER_PATH_BYTES];
	char shortVoices[NATIVE_CHARACTER_SHORT_VOICES][NATIVE_CHARACTER_PATH_BYTES];
	char xaVoices[NATIVE_CHARACTER_XA_VOICES][NATIVE_CHARACTER_PATH_BYTES];
};

struct NativeCharacterVoiceSample
{
	unsigned char *data;
	size_t size;
	unsigned char flags, volume;
	unsigned short pitch, duration;
};

/* Replaces the scan only on success. Results remain valid until the next scan. */
int NativeCharacterLibrary_Refresh(const char *directory);
size_t NativeCharacterLibrary_Count(void);
const struct NativeCharacterMetadata *NativeCharacterLibrary_Get(size_t index);
void NativeCharacterLibrary_Shutdown(void);
int NativeCharacterLibrary_ReadIcon(const struct NativeCharacterMetadata *metadata, unsigned short *pixels);
/* Audio indices are character-local, not retail sound IDs. Returned paths live until refresh. */
int NativeCharacterLibrary_ReadVoice(const char *directory, int index, struct NativeCharacterVoiceSample *sample);
const char *NativeCharacterLibrary_GetVoicePath(const char *directory, int index, int xa);

#endif
