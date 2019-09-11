/*
 * buffer.c
 *
 *  Created on: Sep 23, 2018
 *      Author: meir
 */

#include "buffer.h"
#include <string.h>
#include "memory/rlutils_memory.h"


RMUTILS_PRFX_Buffer* RMUTILS_PRFX_BufferNew(size_t initialCap){
    RMUTILS_PRFX_Buffer* ret = RMUTILS_PRFX_malloc(sizeof(*ret));
    ret->cap = initialCap;
    ret->size = 0;
    ret->buff = RMUTILS_PRFX_malloc(initialCap * sizeof(char));
    return ret;
}

void RMUTILS_PRFX_BufferFree(RMUTILS_PRFX_Buffer* buff){
    RMUTILS_PRFX_free(buff->buff);
    RMUTILS_PRFX_free(buff);
}

void RMUTILS_PRFX_BufferAdd(RMUTILS_PRFX_Buffer* buff, const char* data, size_t len){
    if (buff->size + len >= buff->cap){
        buff->cap = buff->size + len;
        buff->buff = RMUTILS_PRFX_realloc(buff->buff, buff->cap);
    }
    memcpy(buff->buff + buff->size, data, len);
    buff->size += len;
}

void RMUTILS_PRFX_BufferClear(RMUTILS_PRFX_Buffer* buff){
    buff->size = 0;
}

void RMUTILS_PRFX_BufferWriterInit(RMUTILS_PRFX_BufferWriter* bw, RMUTILS_PRFX_Buffer* buff){
    bw->buff = buff;
}

void RMUTILS_PRFX_BufferWriterWriteLong(RMUTILS_PRFX_BufferWriter* bw, long val){
    RMUTILS_PRFX_BufferAdd(bw->buff, (char*)&val, sizeof(long));
}

void RMUTILS_PRFX_BufferWriterWriteString(RMUTILS_PRFX_BufferWriter* bw, const char* str){
    RMUTILS_PRFX_BufferWriterWriteBuff(bw, str, strlen(str) + 1);
}

void RMUTILS_PRFX_BufferWriterWriteBuff(RMUTILS_PRFX_BufferWriter* bw, const char* buff, size_t len){
    RMUTILS_PRFX_BufferWriterWriteLong(bw, len);
    RMUTILS_PRFX_BufferAdd(bw->buff, buff, len);
}

void RMUTILS_PRFX_BufferReaderInit(RMUTILS_PRFX_BufferReader* br, RMUTILS_PRFX_Buffer* buff){
    br->buff = buff;
    br->location = 0;
}

long RMUTILS_PRFX_BufferReaderReadLong(RMUTILS_PRFX_BufferReader* br){
    long ret = *(long*)(&br->buff->buff[br->location]);
    br->location += sizeof(long);
    return ret;
}

char* RMUTILS_PRFX_BufferReaderReadBuff(RMUTILS_PRFX_BufferReader* br, size_t* len){
    *len = (size_t)RMUTILS_PRFX_BufferReaderReadLong(br);
    char* ret = br->buff->buff + br->location;
    br->location += *len;
    return ret;
}

char* RMUTILS_PRFX_BufferReaderReadString(RMUTILS_PRFX_BufferReader* br){
    size_t len;
    return RMUTILS_PRFX_BufferReaderReadBuff(br, &len);
}

