/*
 * buffer.h
 *
 *  Created on: Sep 23, 2018
 *      Author: meir
 */

#ifndef SRC_UTILS_BUFFER_H_
#define SRC_UTILS_BUFFER_H_

#include <stddef.h>

#define DEFAULT_INITIAL_CAP 50

typedef struct RLUTILS_PRFX_Buffer{
    size_t cap;
    size_t size;
    char* buff;
}RLUTILS_PRFX_Buffer;

#define RLUTILS_PRFX_BufferCreate() RLUTILS_PRFX_BufferNew(DEFAULT_INITIAL_CAP)

RLUTILS_PRFX_Buffer* RLUTILS_PRFX_BufferNew(size_t initialCap);
void RLUTILS_PRFX_BufferFree(RLUTILS_PRFX_Buffer* buff);
void RLUTILS_PRFX_BufferAdd(RLUTILS_PRFX_Buffer* buff, const char* data, size_t len);
void RLUTILS_PRFX_BufferClear(RLUTILS_PRFX_Buffer* buff);

typedef struct RLUTILS_PRFX_BufferWriter{
    RLUTILS_PRFX_Buffer* buff;
}RLUTILS_PRFX_BufferWriter;

void RLUTILS_PRFX_BufferWriterInit(RLUTILS_PRFX_BufferWriter* bw, RLUTILS_PRFX_Buffer* buff);
void RLUTILS_PRFX_BufferWriterWriteLong(RLUTILS_PRFX_BufferWriter* bw, long val);
void RLUTILS_PRFX_BufferWriterWriteString(RLUTILS_PRFX_BufferWriter* bw, const char* str);
void RLUTILS_PRFX_BufferWriterWriteBuff(RLUTILS_PRFX_BufferWriter* bw, const char* buff, size_t len);

typedef struct RLUTILS_PRFX_BufferReader{
    RLUTILS_PRFX_Buffer* buff;
    size_t location;
}RLUTILS_PRFX_BufferReader;

void RLUTILS_PRFX_BufferReaderInit(RLUTILS_PRFX_BufferReader* br, RLUTILS_PRFX_Buffer* buff);
long RLUTILS_PRFX_BufferReaderReadLong(RLUTILS_PRFX_BufferReader* br);
char* RLUTILS_PRFX_BufferReaderReadBuff(RLUTILS_PRFX_BufferReader* br, size_t* len);
char* RLUTILS_PRFX_BufferReaderReadString(RLUTILS_PRFX_BufferReader* br);




#endif /* SRC_UTILS_BUFFER_H_ */
