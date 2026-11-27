#ifndef CTR_NATIVE_EXTERNAL_FILE_H
#define CTR_NATIVE_EXTERNAL_FILE_H

#include <stddef.h>
#include <stdio.h>

struct NativeExternalFile;
struct NativeExternalFile *NativeExternalFile_Open(const char *path);
size_t NativeExternalFile_Size(const struct NativeExternalFile *file);
size_t NativeExternalFile_SectorBytes(const struct NativeExternalFile *file);
int NativeExternalFile_Read(struct NativeExternalFile *file, void *destination, size_t capacity);
void NativeExternalFile_Close(struct NativeExternalFile *file);

// Creates a binary stream only if the path is absent; the caller closes it.
FILE *NativeExternalFile_CreateExclusive(const char *path);

#endif
