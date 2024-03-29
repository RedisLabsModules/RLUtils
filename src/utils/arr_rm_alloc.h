#ifndef ARR_RM_ALLOC_H_
#define ARR_RM_ALLOC_H_

/* A wrapper for arr.h that sets the allocation functions to those of the RedisModule_Alloc &
 * friends. This file should not be included alongside arr.h, and should not be included from .h
 * files in general */

#include "../memory/rlutils_memory.h"

/* Define the allcation functions before including arr.h */
#define array_alloc_fn RLUTILS_PRFX_malloc
#define array_realloc_fn RLUTILS_PRFX_realloc
#define array_free_fn RLUTILS_PRFX_free

#include "arr.h"

#endif
