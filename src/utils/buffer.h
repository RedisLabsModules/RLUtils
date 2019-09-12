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

typedef struct RMUTILS_PRFX_Buffer{
    size_t cap;
    size_t size;
    char* buff;
}RMUTILS_PRFX_Buffer;

#define RMUTILS_PRFX_BufferCreate() RMUTILS_PRFX_BufferNew(DEFAULT_INITIAL_CAP)

RMUTILS_PRFX_Buffer* RMUTILS_PRFX_BufferNew(size_t initialCap);
void RMUTILS_PRFX_BufferFree(RMUTILS_PRFX_Buffer* buff);
void RMUTILS_PRFX_BufferAdd(RMUTILS_PRFX_Buffer* buff, const char* data, size_t len);
void RMUTILS_PRFX_BufferClear(RMUTILS_PRFX_Buffer* buff);

typedef struct RMUTILS_PRFX_BufferWriter{
    RMUTILS_PRFX_Buffer* buff;
}RMUTILS_PRFX_BufferWriter;

void RMUTILS_PRFX_BufferWriterInit(RMUTILS_PRFX_BufferWriter* bw, RMUTILS_PRFX_Buffer* buff);
void RMUTILS_PRFX_BufferWriterWriteLong(RMUTILS_PRFX_BufferWriter* bw, long val);
void RMUTILS_PRFX_BufferWriterWriteString(RMUTILS_PRFX_BufferWriter* bw, const char* str);
void RMUTILS_PRFX_BufferWriterWriteBuff(RMUTILS_PRFX_BufferWriter* bw, const char* buff, size_t len);

typedef struct RMUTILS_PRFX_BufferReader{
    RMUTILS_PRFX_Buffer* buff;
    size_t location;
}RMUTILS_PRFX_BufferReader;

void RMUTILS_PRFX_BufferReaderInit(RMUTILS_PRFX_BufferReader* br, RMUTILS_PRFX_Buffer* buff);
long RMUTILS_PRFX_BufferReaderReadLong(RMUTILS_PRFX_BufferReader* br);
char* RMUTILS_PRFX_BufferReaderReadBuff(RMUTILS_PRFX_BufferReader* br, size_t* len);
char* RMUTILS_PRFX_BufferReaderReadString(RMUTILS_PRFX_BufferReader* br);




#endif /* SRC_UTILS_BUFFER_H_ */
