#ifndef CTR_NATIVE_INI_H
#define CTR_NATIVE_INI_H

#include <stdbool.h>
#include <stddef.h>
#include <stdio.h>

enum NativeIniLineType
{
	NATIVE_INI_SECTION,
	NATIVE_INI_VALUE,
	NATIVE_INI_MALFORMED,
};

struct NativeIniLine
{
	enum NativeIniLineType type;
	const char *section;
	const char *key;
	const char *value;
	bool trailingSectionText;
};

enum NativeIniReadResult
{
	NATIVE_INI_READ_RECORD,
	NATIVE_INI_READ_END,
	NATIVE_INI_READ_TOO_LONG,
	NATIVE_INI_READ_ERROR,
};

// Skips blank/comment lines. Strings borrow buffer until the next read.
// A section without ']' has a NULL name. Callers own section state and validation;
// compatible readers may keep accepting fixed-size chunks of overlong lines.
enum NativeIniReadResult NativeIni_Read(FILE *file, char *buffer, size_t capacity, bool rejectLongLines,
                                      struct NativeIniLine *line);
// Writes literal values with no quoting/escaping. Format owners validate text.
bool NativeIni_WriteSection(FILE *file, const char *section);
bool NativeIni_WriteValue(FILE *file, const char *key, const char *value);
bool NativeIni_WriteInt(FILE *file, const char *key, int value);

// The callback writes contents only: it must not close the stream. Save owns a
// uniquely created sibling temporary, checks writes/flush/close, then replaces
// the destination. Failure preserves the destination and removes the temporary.
typedef bool (*NativeIniWriter)(FILE *file, const void *context);
bool NativeIni_Save(const char *path, NativeIniWriter write, const void *context);

#endif
