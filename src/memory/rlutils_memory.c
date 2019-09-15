#include "../redismodule.h"
#include "../utils/arr_rm_alloc.h"
#include "rlutils_memory.h"
#include <string.h>
#include <stdarg.h>

#ifdef VALGRIND
#define ALLOC malloc
#define CALLOC calloc
#define REALLOC realloc
#define FREE free
#define STRDUP strdup
#else
#define ALLOC RedisModule_Alloc
#define CALLOC RedisModule_Calloc
#define REALLOC RedisModule_Realloc
#define FREE RedisModule_Free
#define STRDUP RedisModule_Strdup
#endif

void *RLUTILS_PRFX_malloc(size_t n){
    return ALLOC(n);
}

void *RLUTILS_PRFX_calloc(size_t nelem, size_t elemsz){
    return CALLOC(nelem, elemsz);
}

void *RLUTILS_PRFX_realloc(void *p, size_t n){
    return REALLOC(p, n);
}

void RLUTILS_PRFX_free(void *p){
    return FREE(p);
}

char *RLUTILS_PRFX_strdup(const char *s){
    return STRDUP(s);
}

char *RLUTILS_PRFX_strndup(const char *s, size_t n){
    char *ret = (char *)ALLOC(n + 1);

    if (ret) {
        ret[n] = '\0';
        memcpy(ret, s, n);
    }
    return ret;
}

int RLUTILS_PRFX_vasprintf(char **__restrict __ptr, const char *__restrict __fmt, va_list __arg) {
  va_list args_copy;
  va_copy(args_copy, __arg);

  size_t needed = vsnprintf(NULL, 0, __fmt, __arg) + 1;
  *__ptr = (char *)ALLOC(needed);

  int res = vsprintf(*__ptr, __fmt, args_copy);

  va_end(args_copy);

  return res;
}

int RLUTILS_PRFX_asprintf(char **__ptr, const char *__restrict __fmt, ...) {
  va_list ap;
  va_start(ap, __fmt);

  int res = RLUTILS_PRFX_vasprintf(__ptr, __fmt, ap);

  va_end(ap);

  return res;
}

void RLUTILS_PRFX_MemoryGuardrInit(RLUTILS_PRFX_MemoryGuard* mg){
#define INITIAL_CAP 10
    mg->units = array_new(RLUTILS_PRFX_GuardUnit, INITIAL_CAP);
}

void RLUTILS_PRFX_MemoryGuardFreeUnits(RLUTILS_PRFX_MemoryGuard* mg){
    for(size_t i = 0 ; i < array_len(mg->units) ; ++i){
        RLUTILS_PRFX_GuardUnit* unit = mg->units + i;
        unit->freeFunc(unit->ptr);
    }
}

void RLUTILS_PRFX_MemoryGuardFreeStructure(RLUTILS_PRFX_MemoryGuard* mg){
    array_free(mg->units);
}

void RLUTILS_PRFX_MemoryGuardFree(RLUTILS_PRFX_MemoryGuard* mg){
    RLUTILS_PRFX_MemoryGuardFreeUnits(mg);
    RLUTILS_PRFX_MemoryGuardFreeStructure(mg);
}

void RLUTILS_PRFX_MemoryGuardAddUnit(RLUTILS_PRFX_MemoryGuard* mg, RLUTILS_PRFX_GuardUnit unit){
    mg->units = array_append(mg->units, unit);
}

void RLUTILS_PRFX_MemoryGuardAddPtr(RLUTILS_PRFX_MemoryGuard* mg, void* ptr){
    RLUTILS_PRFX_MemoryGuardAddUnit(mg, (RLUTILS_PRFX_GuardUnit){
            .ptr = ptr,
            .freeFunc = free,
    });

}

static void FreeRedisModuleString(void* ptr){
    RedisModule_FreeString(NULL, ptr);
}

void RLUTILS_PRFX_MemoryGuardAddRedisString(RLUTILS_PRFX_MemoryGuard* mg, RedisModuleString* ptr){
    RLUTILS_PRFX_MemoryGuardAddUnit(mg, (RLUTILS_PRFX_GuardUnit){
            .ptr = ptr,
            .freeFunc = FreeRedisModuleString,
    });
}
