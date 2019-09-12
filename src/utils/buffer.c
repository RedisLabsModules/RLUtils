/*
 * buffer.c
 *
 *  Created on: Sep 23, 2018
 *      Author: meir
 */

#include "buffer.h"
#include <string.h>
#include "../memory/rlutils_memory.h"


RLUTILS_PRFX_Buffer* RLUTILS_PRFX_BufferNew(size_t initialCap){
    RLUTILS_PRFX_Buffer* ret = RLUTILS_PRFX_malloc(sizeof(*ret));
    ret->cap = initialCap;
    ret->size = 0;
    ret->buff = RLUTILS_PRFX_malloc(initialCap * sizeof(char));
    return ret;
}

void RLUTILS_PRFX_BufferFree(RLUTILS_PRFX_Buffer* buff){
    RLUTILS_PRFX_free(buff->buff);
    RLUTILS_PRFX_free(buff);
}

void RLUTILS_PRFX_BufferAdd(RLUTILS_PRFX_Buffer* buff, const char* data, size_t len){
    if (buff->size + len >= buff->cap){
        buff->cap = buff->size + len;
        buff->buff = RLUTILS_PRFX_realloc(buff->buff, buff->cap);
    }
    memcpy(buff->buff + buff->size, data, len);
    buff->size += len;
}

void RLUTILS_PRFX_BufferClear(RLUTILS_PRFX_Buffer* buff){
    buff->size = 0;
}

void RLUTILS_PRFX_BufferWriterInit(RLUTILS_PRFX_BufferWriter* bw, RLUTILS_PRFX_Buffer* buff){
    bw->buff = buff;
}

void RLUTILS_PRFX_BufferWriterWriteLong(RLUTILS_PRFX_BufferWriter* bw, long val){
    RLUTILS_PRFX_BufferAdd(bw->buff, (char*)&val, sizeof(long));
}

void RLUTILS_PRFX_BufferWriterWriteString(RLUTILS_PRFX_BufferWriter* bw, const char* str){
    RLUTILS_PRFX_BufferWriterWriteBuff(bw, str, strlen(str) + 1);
}

void RLUTILS_PRFX_BufferWriterWriteBuff(RLUTILS_PRFX_BufferWriter* bw, const char* buff, size_t len){
    RLUTILS_PRFX_BufferWriterWriteLong(bw, len);
    RLUTILS_PRFX_BufferAdd(bw->buff, buff, len);
}

void RLUTILS_PRFX_BufferReaderInit(RLUTILS_PRFX_BufferReader* br, RLUTILS_PRFX_Buffer* buff){
    br->buff = buff;
    br->location = 0;
}

long RLUTILS_PRFX_BufferReaderReadLong(RLUTILS_PRFX_BufferReader* br){
    long ret = *(long*)(&br->buff->buff[br->location]);
    br->location += sizeof(long);
    return ret;
}

char* RLUTILS_PRFX_BufferReaderReadBuff(RLUTILS_PRFX_BufferReader* br, size_t* len){
    *len = (size_t)RLUTILS_PRFX_BufferReaderReadLong(br);
    char* ret = br->buff->buff + br->location;
    br->location += *len;
    return ret;
}

char* RLUTILS_PRFX_BufferReaderReadString(RLUTILS_PRFX_BufferReader* br){
    size_t len;
    return RLUTILS_PRFX_BufferReaderReadBuff(br, &len);
}

