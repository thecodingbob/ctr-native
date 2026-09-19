#ifndef CTR_MATCHING_NAMESPACE_MEMPACK_RETAIL_SYMBOLS_H
#define CTR_MATCHING_NAMESPACE_MEMPACK_RETAIL_SYMBOLS_H

#define MEMPACK_POOLS  mempackPools
#define MEMPACK_ACTIVE mempackActive
#include <common.h>

// NOTE(aalhendi): Retail addresses the pool array absolutely and the active pointer
// as an individual small-data global. The latter also preserves GCC's alias analysis.
extern struct Mempack mempackPools[4] asm("sdata_static+49528");
extern struct Mempack *mempackActive;

#endif
