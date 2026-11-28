#ifndef CTR_NATIVE_CHARACTER_MODEL_H
#define CTR_NATIVE_CHARACTER_MODEL_H

#include <stddef.h>

struct NativeExternalFile;

/* Read a model into caller-owned, sector-rounded storage. The read
   checks the envelope but does not make nested model data safe to render. */
int NativeCharacterModel_Read(struct NativeExternalFile *file, void *destination, size_t capacity);

/* Check the outer model group and relocation table only. Nested mesh,
   animation and texture data still need validation before runtime loading. */
int NativeCharacterModel_CheckEnvelope(const void *data, size_t size);

#endif
