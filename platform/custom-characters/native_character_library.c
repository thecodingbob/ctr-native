#include <platform/custom-characters/native_character_library.h>
#include <SDL3/SDL.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct NativeCharacterScan
{
	struct NativeCharacterMetadata *items;
	size_t count;
	int failed;
};

static struct NativeCharacterScan s_nativeCharacterLibrary;

enum NativeCharacterSection
{
	NATIVE_CHARACTER_SECTION_UNKNOWN,
	NATIVE_CHARACTER_SECTION_CHARACTER,
	NATIVE_CHARACTER_SECTION_ASSETS,
	NATIVE_CHARACTER_SECTION_AUDIO,
};

static char *NativeCharacterLibrary_Trim(char *text)
{
	while (isspace((unsigned char)*text)) { text++; }
	char *end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1])) { *--end = '\0'; }
	return text;
}

static int NativeCharacterLibrary_Copy(char *dst, size_t size, const char *value)
{
	size_t length = strlen(value);
	if (length >= size) { return 0; }
	memcpy(dst, value, length + 1);
	return 1;
}

static int NativeCharacterLibrary_Join(char *dst, size_t size, const char *directory, const char *name)
{
	size_t directoryLength = strlen(directory), nameLength = strlen(name);
	if (directoryLength >= size || size - directoryLength < 2 || nameLength >= size - directoryLength - 1) { return 0; }
	memcpy(dst, directory, directoryLength);
	dst[directoryLength] = '/';
	memcpy(dst + directoryLength + 1, name, nameLength + 1);
	return 1;
}

static int NativeCharacterLibrary_AssetPath(char *dst, size_t size, const char *directory, const char *name)
{
	/* Only relative paths with ordinary components belong to a character. */
	if (name[0] == '\0' || name[0] == '/' || strpbrk(name, "\\:") != NULL) { return 0; }
	const char *component = name;
	for (const char *p = name; ; p++)
	{
		if (*p == '/' || *p == '\0')
		{
			size_t length = (size_t)(p - component);
			if (length == 0 || (length == 1 && component[0] == '.') ||
			    (length == 2 && component[0] == '.' && component[1] == '.')) { return 0; }
			if (*p == '\0') { break; }
			component = p + 1;
		}
	}
	return NativeCharacterLibrary_Join(dst, size, directory, name);
}

static enum NativeCharacterSection NativeCharacterLibrary_Section(const char *text)
{
	if (strcmp(text, "[character]") == 0)
	{
		return NATIVE_CHARACTER_SECTION_CHARACTER;
	}
	if (strcmp(text, "[assets]") == 0)
	{
		return NATIVE_CHARACTER_SECTION_ASSETS;
	}
	if (strcmp(text, "[audio]") == 0)
	{
		return NATIVE_CHARACTER_SECTION_AUDIO;
	}
	return NATIVE_CHARACTER_SECTION_UNKNOWN;
}

static int NativeCharacterLibrary_MetadataValue(struct NativeCharacterMetadata *metadata, const char *key, const char *value)
{
	const struct
	{
		const char *key;
		char *destination;
	} fields[] = {
		{"id", metadata->identity},
		{"name", metadata->name},
		{"short_name", metadata->shortName},
		{"author", metadata->author},
		{"base", metadata->base},
	};
	for (size_t i = 0; i < sizeof(fields) / sizeof(fields[0]); i++)
	{
		if (strcmp(key, fields[i].key) == 0)
		{
			return NativeCharacterLibrary_Copy(fields[i].destination, NATIVE_CHARACTER_TEXT_BYTES, value);
		}
	}
	return 1;
}

static int NativeCharacterLibrary_AudioValue(struct NativeCharacterMetadata *metadata, const char *key, const char *value)
{
	const char *indexText;
	char (*paths)[NATIVE_CHARACTER_PATH_BYTES];
	int count;
	if (strncmp(key, "short_", 6) == 0)
	{
		indexText = key + 6;
		paths = metadata->shortVoices;
		count = NATIVE_CHARACTER_SHORT_VOICES;
	}
	else if (strncmp(key, "line_", 5) == 0)
	{
		indexText = key + 5;
		paths = metadata->xaVoices;
		count = NATIVE_CHARACTER_XA_VOICES;
	}
	else
	{
		return 1;
	}

	// Unknown keys are ignored, including indices outside the supported range.
	if (strlen(indexText) != 2 || !isdigit((unsigned char)indexText[0]) || !isdigit((unsigned char)indexText[1]))
	{
		return 1;
	}
	int index = (indexText[0] - '0') * 10 + indexText[1] - '0';
	if (index >= count)
	{
		return 1;
	}
	return NativeCharacterLibrary_AssetPath(paths[index], sizeof(paths[index]), metadata->directory, value);
}

static int NativeCharacterLibrary_Read(const char *directory, struct NativeCharacterMetadata *metadata)
{
	char path[NATIVE_CHARACTER_PATH_BYTES];
	char model[NATIVE_CHARACTER_TEXT_BYTES] = "hi.ctr";
	char icon[NATIVE_CHARACTER_TEXT_BYTES] = "icon-4bpp.bin";
	char clut[NATIVE_CHARACTER_TEXT_BYTES] = "icon-clut.bin";
	if (!NativeCharacterLibrary_Join(path, sizeof(path), directory, "character.ini")) { return 0; }
	FILE *file = fopen(path, "rb");
	if (file == NULL) { return 0; }
	char line[512];
	enum NativeCharacterSection section = NATIVE_CHARACTER_SECTION_UNKNOWN;
	int valid = 1;
	memset(metadata, 0, sizeof(*metadata));
	if (!NativeCharacterLibrary_Copy(metadata->directory, sizeof(metadata->directory), directory))
	{
		fclose(file);
		return 0;
	}
	while (valid && fgets(line, sizeof(line), file) != NULL)
	{
		if (strchr(line, '\n') == NULL && !feof(file)) { valid = 0; break; }
		char *text = NativeCharacterLibrary_Trim(line);
		if (*text == '\0' || *text == ';' || *text == '#') { continue; }
		if (*text == '[')
		{
			section = NativeCharacterLibrary_Section(text);
			continue;
		}
		char *equals = strchr(text, '=');
		if (equals == NULL) { continue; }
		*equals = '\0';
		char *key = NativeCharacterLibrary_Trim(text);
		char *value = NativeCharacterLibrary_Trim(equals + 1);
		switch (section)
		{
		case NATIVE_CHARACTER_SECTION_CHARACTER:
			valid = NativeCharacterLibrary_MetadataValue(metadata, key, value);
			break;
		case NATIVE_CHARACTER_SECTION_ASSETS:
			if (strcmp(key, "hi") == 0)
			{
				valid = NativeCharacterLibrary_Copy(model, sizeof(model), value);
			}
			else if (strcmp(key, "icon_pixels") == 0)
			{
				valid = NativeCharacterLibrary_Copy(icon, sizeof(icon), value);
			}
			else if (strcmp(key, "icon_clut") == 0)
			{
				valid = NativeCharacterLibrary_Copy(clut, sizeof(clut), value);
			}
			break;
		case NATIVE_CHARACTER_SECTION_AUDIO:
			valid = NativeCharacterLibrary_AudioValue(metadata, key, value);
			break;
		case NATIVE_CHARACTER_SECTION_UNKNOWN:
			break;
		}
	}
	if (ferror(file)) { valid = 0; }
	fclose(file);
	if (!valid || !metadata->identity[0] || !metadata->name[0] || !metadata->base[0] || !model[0] ||
	    !NativeCharacterLibrary_AssetPath(metadata->modelHi, sizeof(metadata->modelHi), directory, model))
	{
		return 0;
	}
	SDL_PathInfo info;
	if (icon[0] && clut[0])
	{
		if (!NativeCharacterLibrary_AssetPath(metadata->iconPixels, sizeof(metadata->iconPixels), directory, icon) ||
		    !NativeCharacterLibrary_AssetPath(metadata->iconClut, sizeof(metadata->iconClut), directory, clut)) { return 0; }
	}
	return SDL_GetPathInfo(metadata->modelHi, &info) && info.type == SDL_PATHTYPE_FILE;
}

static int NativeCharacterLibrary_ReadExact(const char *path, unsigned char *bytes, size_t size)
{
	FILE *file = fopen(path, "rb");
	if (file == NULL) { return 0; }
	int ok = fread(bytes, 1, size, file) == size && fgetc(file) == EOF && !ferror(file);
	fclose(file);
	return ok;
}

int NativeCharacterLibrary_ReadIcon(const struct NativeCharacterMetadata *metadata, unsigned short *pixels)
{
	unsigned char packed[44 * 26 / 2], clut[32];
	if (metadata == NULL || pixels == NULL || !NativeCharacterLibrary_ReadExact(metadata->iconPixels, packed, sizeof(packed)) ||
	    !NativeCharacterLibrary_ReadExact(metadata->iconClut, clut, sizeof(clut))) { return 0; }
	for (size_t i = 0; i < 44 * 26; i++)
	{
		unsigned int index = (packed[i / 2] >> ((i % 2) * 4)) & 15;
		pixels[i] = (unsigned short)((unsigned int)clut[index * 2] | (unsigned int)clut[index * 2 + 1] << 8);
	}
	return 1;
}

const char *NativeCharacterLibrary_GetVoicePath(const char *directory, int index, int xa)
{
	if (directory == NULL || index < 0 || index >= (xa ? NATIVE_CHARACTER_XA_VOICES : NATIVE_CHARACTER_SHORT_VOICES)) { return NULL; }
	for (size_t i = 0; i < s_nativeCharacterLibrary.count; i++)
	{
		const struct NativeCharacterMetadata *item = &s_nativeCharacterLibrary.items[i];
		if (strcmp(item->directory, directory) == 0)
		{
			const char *path = xa ? item->xaVoices[index] : item->shortVoices[index];
			return path[0] != '\0' ? path : NULL;
		}
	}
	return NULL;
}

int NativeCharacterLibrary_ReadVoice(const char *directory, int index, struct NativeCharacterVoiceSample *sample)
{
	if (sample == NULL) { return 0; }
	memset(sample, 0, sizeof(*sample));
	const char *path = NativeCharacterLibrary_GetVoicePath(directory, index, 0);
	if (path == NULL) { return 0; }
	SDL_PathInfo info;
	if (!SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_FILE || info.size <= 8 || info.size > 512 * 1024 || (info.size - 8) % 16 != 0) { return 0; }
	size_t size = 0;
	unsigned char *bytes = SDL_LoadFile(path, &size);
	if (bytes == NULL || size != info.size) { SDL_free(bytes); return 0; }
	sample->flags = bytes[0];
	sample->volume = bytes[1];
	sample->pitch = (unsigned short)((unsigned int)bytes[2] | (unsigned int)bytes[3] << 8);
	sample->duration = (unsigned short)((unsigned int)bytes[6] | (unsigned int)bytes[7] << 8);
	sample->size = size - 8;
	sample->data = malloc(sample->size);
	if (sample->data != NULL) { memcpy(sample->data, bytes + 8, sample->size); }
	SDL_free(bytes);
	return sample->data != NULL;
}

static SDL_EnumerationResult SDLCALL NativeCharacterLibrary_Visit(void *userdata, const char *directory, const char *name)
{
	struct NativeCharacterScan *scan = userdata;
	char path[NATIVE_CHARACTER_PATH_BYTES];
	SDL_PathInfo info;
	if (!NativeCharacterLibrary_Join(path, sizeof(path), directory, name) || !SDL_GetPathInfo(path, &info) || info.type != SDL_PATHTYPE_DIRECTORY)
	{
		return SDL_ENUM_CONTINUE;
	}
	struct NativeCharacterMetadata item;
	if (!NativeCharacterLibrary_Read(path, &item)) { return SDL_ENUM_CONTINUE; }
	for (size_t i = 0; i < scan->count; i++)
	{
		if (strcmp(scan->items[i].identity, item.identity) == 0) { return SDL_ENUM_CONTINUE; }
	}
	if (scan->count >= SIZE_MAX / sizeof(*scan->items) - 1) { scan->failed = 1; return SDL_ENUM_FAILURE; }
	void *next = realloc(scan->items, (scan->count + 1) * sizeof(*scan->items));
	if (next == NULL) { scan->failed = 1; return SDL_ENUM_FAILURE; }
	scan->items = next;
	scan->items[scan->count++] = item;
	return SDL_ENUM_CONTINUE;
}

int NativeCharacterLibrary_Refresh(const char *directory)
{
	struct NativeCharacterScan scan = {0};
	if (directory == NULL || !SDL_EnumerateDirectory(directory, NativeCharacterLibrary_Visit, &scan) || scan.failed)
	{
		free(scan.items);
		return 0;
	}
	free(s_nativeCharacterLibrary.items);
	s_nativeCharacterLibrary = scan;
	return 1;
}

size_t NativeCharacterLibrary_Count(void) { return s_nativeCharacterLibrary.count; }

const struct NativeCharacterMetadata *NativeCharacterLibrary_Get(size_t index)
{
	return index < s_nativeCharacterLibrary.count ? &s_nativeCharacterLibrary.items[index] : NULL;
}

void NativeCharacterLibrary_Shutdown(void)
{
	free(s_nativeCharacterLibrary.items);
	memset(&s_nativeCharacterLibrary, 0, sizeof(s_nativeCharacterLibrary));
}
