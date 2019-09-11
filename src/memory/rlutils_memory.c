/*
 * rlutils_memory.c
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#include "redismodule.h"
#include "rlutils_memory.h"

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

void *RMUTILS_PRFX_malloc(size_t n){
    return ALLOC(n);
}

void *RMUTILS_PRFX_calloc(size_t nelem, size_t elemsz){
    return CALLOC(nelem, elemsz);
}

void *RMUTILS_PRFX_realloc(void *p, size_t n){
    return REALLOC(p, n);
}

void RMUTILS_PRFX_free(void *p){
    return FREE(p);
}

char *RMUTILS_PRFX_strdup(const char *s){
    return STRDUP(s);
}

char *RMUTILS_PRFX_strndup(const char *s, size_t n){
    char *ret = (char *)ALLOC(n + 1);

    if (ret) {
        ret[n] = '\0';
        memcpy(ret, s, n);
    }
    return ret;
}

int RMUTILS_PRFX_vasprintf(char **__restrict __ptr, const char *__restrict __fmt, va_list __arg) {
  va_list args_copy;
  va_copy(args_copy, __arg);

  size_t needed = vsnprintf(NULL, 0, __fmt, __arg) + 1;
  *__ptr = (char *)ALLOC(needed);

  int res = vsprintf(*__ptr, __fmt, args_copy);

  va_end(args_copy);

  return res;
}

int RMUTILS_PRFX_asprintf(char **__ptr, const char *__restrict __fmt, ...) {
  va_list ap;
  va_start(ap, __fmt);

  int res = RMUTILS_PRFX_vasprintf(__ptr, __fmt, ap);

  va_end(ap);

  return res;
}