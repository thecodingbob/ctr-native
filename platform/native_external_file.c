#if !defined(_WIN32) && !defined(_POSIX_C_SOURCE)
#define _POSIX_C_SOURCE 200809L
#endif

#include <platform/native_external_file.h>
#include <errno.h>
#include <fcntl.h>
#include <limits.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined(_WIN32)
#include <io.h>
#include <sys/stat.h>
#else
#include <unistd.h>
#endif

enum { NATIVE_EXTERNAL_FILE_SECTOR_SIZE = 2048 };

struct NativeExternalFile
{
	FILE *stream;
	size_t size;
	size_t sectorBytes;
};

struct NativeExternalFile *NativeExternalFile_Open(const char *path)
{
	if (path == NULL)
	{
		return NULL;
	}
	FILE *stream = fopen(path, "rb");
	if (stream == NULL)
	{
		return NULL;
	}
	if (fseek(stream, 0, SEEK_END) != 0)
	{
		fclose(stream);
		return NULL;
	}
	long length = ftell(stream);
	if (length <= 0 || length > INT_MAX - (NATIVE_EXTERNAL_FILE_SECTOR_SIZE - 1) || fseek(stream, 0, SEEK_SET) != 0)
	{
		fclose(stream);
		return NULL;
	}
	struct NativeExternalFile *file = malloc(sizeof(*file));
	if (file == NULL)
	{
		fclose(stream);
		return NULL;
	}
	file->stream = stream;
	file->size = (size_t)length;
	file->sectorBytes = (file->size + NATIVE_EXTERNAL_FILE_SECTOR_SIZE - 1) / NATIVE_EXTERNAL_FILE_SECTOR_SIZE * NATIVE_EXTERNAL_FILE_SECTOR_SIZE;
	return file;
}

size_t NativeExternalFile_Size(const struct NativeExternalFile *file)
{
	return file != NULL ? file->size : 0;
}

size_t NativeExternalFile_SectorBytes(const struct NativeExternalFile *file)
{
	return file != NULL ? file->sectorBytes : 0;
}

int NativeExternalFile_Read(struct NativeExternalFile *file, void *destination, size_t capacity)
{
	if (file == NULL || destination == NULL || capacity < file->sectorBytes || fseek(file->stream, 0, SEEK_SET) != 0)
	{
		return 0;
	}
	if (fread(destination, 1, file->size, file->stream) != file->size || fgetc(file->stream) != EOF || ferror(file->stream))
	{
		return 0;
	}
	memset((unsigned char *)destination + file->size, 0, file->sectorBytes - file->size);
	return 1;
}

void NativeExternalFile_Close(struct NativeExternalFile *file)
{
	if (file != NULL)
	{
		fclose(file->stream);
		free(file);
	}
}

FILE *NativeExternalFile_CreateExclusive(const char *path)
{
	// Windows' C runtime does not implement fopen's C11 "x" mode.
#if defined(_WIN32)
	int descriptor = _open(path, _O_WRONLY | _O_CREAT | _O_EXCL | _O_BINARY, _S_IREAD | _S_IWRITE);
	FILE *file = descriptor < 0 ? NULL : _fdopen(descriptor, "wb");
#else
	int descriptor = open(path, O_WRONLY | O_CREAT | O_EXCL, 0666);
	FILE *file = descriptor < 0 ? NULL : fdopen(descriptor, "wb");
#endif
	if (descriptor >= 0 && file == NULL)
	{
		int error = errno;
#if defined(_WIN32)
		_close(descriptor);
#else
		close(descriptor);
#endif
		remove(path);
		errno = error;
	}
	return file;
}
