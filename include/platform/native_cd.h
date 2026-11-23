#ifndef NATIVE_CD_H
#define NATIVE_CD_H

#include <psx/libcd.h>

// NOTE(aalhendi): Native Cd* facade for extracted assets and disc-image fallback.
int NativeCD_Init(void);
// Positional read preserving the Cd* cursor and callbacks.
int NativeCD_ReadFileSectorsAt(const CdlFILE *file, unsigned int sector, unsigned int count, void *destination);
// Read relative to an opened CD position without changing the active stream.
int NativeCD_ReadSectorsAt(const CdlLOC *base, unsigned int offset, unsigned int count, void *destination);

#endif
