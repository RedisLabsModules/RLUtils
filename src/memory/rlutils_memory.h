/*
 * rlutils_memory.h
 *
 *  Created on: Sep 11, 2019
 *      Author: root
 */

#ifndef MEMORY_RLUTILS_MEMORY_H_
#define MEMORY_RLUTILS_MEMORY_H_

#include <stddef.h>
#include <stdio.h>

void *RMUTILS_PRFX_malloc(size_t n);
void *RMUTILS_PRFX_calloc(size_t nelem, size_t elemsz);
void *RMUTILS_PRFX_realloc(void *p, size_t n);
void RMUTILS_PRFX_free(void *p);
char *RMUTILS_PRFX_strdup(const char *s);
char *RMUTILS_PRFX_strndup(const char *s, size_t n);
int RMUTILS_PRFX_vasprintf(char **__restrict __ptr, const char *__restrict __fmt, va_list __arg);
int RMUTILS_PRFX_asprintf(char **__ptr, const char *__restrict __fmt, ...);

#endif /* MEMORY_RLUTILS_MEMORY_H_ */
