#include <platform/native_ini.h>
#include <platform/native_external_file.h>
#include <SDL3/SDL.h>

#include <ctype.h>
#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>

static char *NativeIni_Trim(char *text)
{
	while (isspace((unsigned char)*text))
	{
		text++;
	}
	char *end = text + strlen(text);
	while (end > text && isspace((unsigned char)end[-1]))
	{
		*--end = '\0';
	}
	return text;
}

enum NativeIniReadResult NativeIni_Read(FILE *file, char *buffer, size_t capacity, bool rejectLongLines,
                                      struct NativeIniLine *line)
{
	if (file == NULL || buffer == NULL || line == NULL || capacity < 2 || capacity > INT_MAX)
	{
		return NATIVE_INI_READ_ERROR;
	}
	while (fgets(buffer, (int)capacity, file) != NULL)
	{
		if (rejectLongLines && strchr(buffer, '\n') == NULL && !feof(file))
		{
			return NATIVE_INI_READ_TOO_LONG;
		}
		char *text = NativeIni_Trim(buffer);
		if (*text == '\0' || *text == ';' || *text == '#')
		{
			continue;
		}
		*line = (struct NativeIniLine){.type = NATIVE_INI_MALFORMED};
		if (*text == '[')
		{
			line->type = NATIVE_INI_SECTION;
			char *end = strchr(text + 1, ']');
			if (end != NULL)
			{
				*end = '\0';
				line->section = text + 1;
				line->trailingSectionText = *NativeIni_Trim(end + 1) != '\0';
			}
		}
		else
		{
			char *equals = strchr(text, '=');
			if (equals != NULL)
			{
				*equals = '\0';
				line->type = NATIVE_INI_VALUE;
				line->key = NativeIni_Trim(text);
				line->value = NativeIni_Trim(equals + 1);
			}
		}
		return NATIVE_INI_READ_RECORD;
	}
	return ferror(file) ? NATIVE_INI_READ_ERROR : NATIVE_INI_READ_END;
}

bool NativeIni_WriteSection(FILE *file, const char *section)
{
	return fprintf(file, "[%s]\n", section) >= 0;
}

bool NativeIni_WriteValue(FILE *file, const char *key, const char *value)
{
	return fprintf(file, "%s = %s\n", key, value) >= 0;
}

bool NativeIni_WriteInt(FILE *file, const char *key, int value)
{
	return fprintf(file, "%s = %d\n", key, value) >= 0;
}

bool NativeIni_Save(const char *path, NativeIniWriter write, const void *context)
{
	enum { TEMPORARY_SUFFIX_BYTES = 64, TEMPORARY_ATTEMPTS = 16 };
	if (path == NULL || path[0] == '\0' || write == NULL || strlen(path) > SIZE_MAX - TEMPORARY_SUFFIX_BYTES)
	{
		return false;
	}
	size_t capacity = strlen(path) + TEMPORARY_SUFFIX_BYTES;
	char *temporary = malloc(capacity);
	if (temporary == NULL)
	{
		return false;
	}
	FILE *file = NULL;
	unsigned long long stamp = (unsigned long long)SDL_GetTicksNS();
	for (int attempt = 0; attempt < TEMPORARY_ATTEMPTS; attempt++)
	{
		int length = snprintf(temporary, capacity, "%s.%llu.%d.tmp", path, stamp, attempt);
		if (length < 0 || (size_t)length >= capacity)
		{
			break;
		}
		file = NativeExternalFile_CreateExclusive(temporary);
		if (file != NULL || errno != EEXIST)
		{
			break;
		}
	}
	if (file == NULL)
	{
		free(temporary);
		return false;
	}

	bool ok = write(file, context);
	if (ferror(file))
	{
		ok = false;
	}
	if (fflush(file) != 0)
	{
		ok = false;
	}
	if (fclose(file) != 0)
	{
		ok = false;
	}
	if (ok)
	{
		ok = SDL_RenamePath(temporary, path);
	}
	if (!ok)
	{
		remove(temporary);
	}
	free(temporary);
	return ok;
}
