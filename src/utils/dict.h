/* Hash Tables Implementation.
 *
 * This file implements in-memory hash tables with insert/del/replace/find/
 * get-random-element operations. Hash tables will auto-resize if needed
 * tables of power of two in size are used, collisions are handled by
 * chaining. See the source code for more information... :)
 *
 * Copyright (c) 2006-2012, Salvatore Sanfilippo <antirez at gmail dot com>
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *   * Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *   * Redistributions in binary form must reproduce the above copyright
 *     notice, this list of conditions and the following disclaimer in the
 *     documentation and/or other materials provided with the distribution.
 *   * Neither the name of Redis nor the names of its contributors may be used
 *     to endorse or promote products derived from this software without
 *     specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <stdint.h>
#include <stddef.h>

#ifndef __DICT_H
#define __DICT_H

#define DICT_OK 0
#define DICT_ERR 1

/* Unused arguments generate annoying warnings... */
#define DICT_NOTUSED(V) ((void) V)

typedef struct RMUTILS_PRFX_dictEntry {
    void *key;
    union {
        void *val;
        uint64_t u64;
        int64_t s64;
        double d;
    } v;
    struct RMUTILS_PRFX_dictEntry *next;
} RMUTILS_PRFX_dictEntry;

typedef struct RMUTILS_PRFX_dictType {
    uint64_t (*hashFunction)(const void *key);
    void *(*keyDup)(void *privdata, const void *key);
    void *(*valDup)(void *privdata, const void *obj);
    int (*keyCompare)(void *privdata, const void *key1, const void *key2);
    void (*keyDestructor)(void *privdata, void *key);
    void (*valDestructor)(void *privdata, void *obj);
} RMUTILS_PRFX_dictType;

/* This is our hash table structure. Every dictionary has two of this as we
 * implement incremental rehashing, for the old to the new table. */
typedef struct RMUTILS_PRFX_dictht {
    RMUTILS_PRFX_dictEntry **table;
    unsigned long size;
    unsigned long sizemask;
    unsigned long used;
} RMUTILS_PRFX_dictht;

typedef struct RMUTILS_PRFX_dict {
    RMUTILS_PRFX_dictType *type;
    void *privdata;
    RMUTILS_PRFX_dictht ht[2];
    long rehashidx; /* rehashing not in progress if rehashidx == -1 */
    unsigned long iterators; /* number of iterators currently running */
} RMUTILS_PRFX_dict;

/* If safe is set to 1 this is a safe iterator, that means, you can call
 * dictAdd, dictFind, and other functions against the dictionary even while
 * iterating. Otherwise it is a non safe iterator, and only dictNext()
 * should be called while iterating. */
typedef struct RMUTILS_PRFX_dictIterator {
    RMUTILS_PRFX_dict *d;
    long index;
    int table, safe;
    RMUTILS_PRFX_dictEntry *entry, *nextEntry;
    /* unsafe iterator fingerprint for misuse detection. */
    long long fingerprint;
} RMUTILS_PRFX_dictIterator;

typedef void (RMUTILS_PRFX_dictScanFunction)(void *privdata, const RMUTILS_PRFX_dictEntry *de);
typedef void (RMUTILS_PRFX_dictScanBucketFunction)(void *privdata, RMUTILS_PRFX_dictEntry **bucketref);

/* This is the initial size of every hash table */
#define DICT_HT_INITIAL_SIZE     4

/* ------------------------------- Macros ------------------------------------*/
#define RMUTILS_PRFX_dictFreeVal(d, entry) \
    if ((d)->type->valDestructor) \
        (d)->type->valDestructor((d)->privdata, (entry)->v.val)

#define RMUTILS_PRFX_dictSetVal(d, entry, _val_) do { \
    if ((d)->type->valDup) \
        (entry)->v.val = (d)->type->valDup((d)->privdata, _val_); \
    else \
        (entry)->v.val = (_val_); \
} while(0)

#define RMUTILS_PRFX_dictSetSignedIntegerVal(entry, _val_) \
    do { (entry)->v.s64 = _val_; } while(0)

#define RMUTILS_PRFX_dictSetUnsignedIntegerVal(entry, _val_) \
    do { (entry)->v.u64 = _val_; } while(0)

#define RMUTILS_PRFX_dictSetDoubleVal(entry, _val_) \
    do { (entry)->v.d = _val_; } while(0)

#define RMUTILS_PRFX_dictFreeKey(d, entry) \
    if ((d)->type->keyDestructor) \
        (d)->type->keyDestructor((d)->privdata, (entry)->key)

#define RMUTILS_PRFX_dictSetKey(d, entry, _key_) do { \
    if ((d)->type->keyDup) \
        (entry)->key = (d)->type->keyDup((d)->privdata, _key_); \
    else \
        (entry)->key = (_key_); \
} while(0)

#define RMUTILS_PRFX_dictCompareKeys(d, key1, key2) \
    (((d)->type->keyCompare) ? \
        (d)->type->keyCompare((d)->privdata, key1, key2) : \
        (key1) == (key2))

#define RMUTILS_PRFX_dictHashKey(d, key) (d)->type->hashFunction(key)
#define RMUTILS_PRFX_dictGetKey(he) ((he)->key)
#define RMUTILS_PRFX_dictGetVal(he) ((he)->v.val)
#define RMUTILS_PRFX_dictGetSignedIntegerVal(he) ((he)->v.s64)
#define RMUTILS_PRFX_dictGetUnsignedIntegerVal(he) ((he)->v.u64)
#define RMUTILS_PRFX_dictGetDoubleVal(he) ((he)->v.d)
#define RMUTILS_PRFX_dictSlots(d) ((d)->ht[0].size+(d)->ht[1].size)
#define RMUTILS_PRFX_dictSize(d) ((d)->ht[0].used+(d)->ht[1].used)
#define RMUTILS_PRFX_dictIsRehashing(d) ((d)->rehashidx != -1)

/* API */
RMUTILS_PRFX_dict *RMUTILS_PRFX_dictCreate(RMUTILS_PRFX_dictType *type, void *privDataPtr);
int RMUTILS_PRFX_dictExpand(RMUTILS_PRFX_dict *d, unsigned long size);
int RMUTILS_PRFX_dictAdd(RMUTILS_PRFX_dict *d, void *key, void *val);
RMUTILS_PRFX_dictEntry *RMUTILS_PRFX_dictAddRaw(RMUTILS_PRFX_dict *d, void *key, RMUTILS_PRFX_dictEntry **existing);
RMUTILS_PRFX_dictEntry *RMUTILS_PRFX_dictAddOrFind(RMUTILS_PRFX_dict *d, void *key);
int RMUTILS_PRFX_dictReplace(RMUTILS_PRFX_dict *d, void *key, void *val);
int RMUTILS_PRFX_dictDelete(RMUTILS_PRFX_dict *d, const void *key);
RMUTILS_PRFX_dictEntry *RMUTILS_PRFX_dictUnlink(RMUTILS_PRFX_dict *ht, const void *key);
void RMUTILS_PRFX_dictFreeUnlinkedEntry(RMUTILS_PRFX_dict *d, RMUTILS_PRFX_dictEntry *he);
void RMUTILS_PRFX_dictRelease(RMUTILS_PRFX_dict *d);
RMUTILS_PRFX_dictEntry * RMUTILS_PRFX_dictFind(RMUTILS_PRFX_dict *d, const void *key);
void *RMUTILS_PRFX_dictFetchValue(RMUTILS_PRFX_dict *d, const void *key);
int RMUTILS_PRFX_dictResize(RMUTILS_PRFX_dict *d);
RMUTILS_PRFX_dictIterator *RMUTILS_PRFX_dictGetIterator(RMUTILS_PRFX_dict *d);
RMUTILS_PRFX_dictIterator *RMUTILS_PRFX_dictGetSafeIterator(RMUTILS_PRFX_dict *d);
RMUTILS_PRFX_dictEntry *RMUTILS_PRFX_dictNext(RMUTILS_PRFX_dictIterator *iter);
void RMUTILS_PRFX_dictReleaseIterator(RMUTILS_PRFX_dictIterator *iter);
RMUTILS_PRFX_dictEntry *RMUTILS_PRFX_dictGetRandomKey(RMUTILS_PRFX_dict *d);
unsigned int RMUTILS_PRFX_dictGetSomeKeys(RMUTILS_PRFX_dict *d, RMUTILS_PRFX_dictEntry **des, unsigned int count);
void RMUTILS_PRFX_dictGetStats(char *buf, size_t bufsize, RMUTILS_PRFX_dict *d);
uint64_t RMUTILS_PRFX_dictGenHashFunction(const void *key, int len);
uint64_t RMUTILS_PRFX_dictGenCaseHashFunction(const unsigned char *buf, int len);
void RMUTILS_PRFX_dictEmpty(RMUTILS_PRFX_dict *d, void(callback)(void*));
void RMUTILS_PRFX_dictEnableResize(void);
void RMUTILS_PRFX_dictDisableResize(void);
int RMUTILS_PRFX_dictRehash(RMUTILS_PRFX_dict *d, int n);
int RMUTILS_PRFX_dictRehashMilliseconds(RMUTILS_PRFX_dict *d, int ms);
void RMUTILS_PRFX_dictSetHashFunctionSeed(uint8_t *seed);
uint8_t *RMUTILS_PRFX_dictGetHashFunctionSeed(void);
unsigned long RMUTILS_PRFX_dictScan(RMUTILS_PRFX_dict *d, unsigned long v, RMUTILS_PRFX_dictScanFunction *fn, RMUTILS_PRFX_dictScanBucketFunction *bucketfn, void *privdata);
uint64_t RMUTILS_PRFX_dictGetHash(RMUTILS_PRFX_dict *d, const void *key);
RMUTILS_PRFX_dictEntry **RMUTILS_PRFX_dictFindEntryRefByPtrAndHash(RMUTILS_PRFX_dict *d, const void *oldptr, uint64_t hash);

extern RMUTILS_PRFX_dictType RMUTILS_PRFX_dictTypeHeapStrings;

#endif /* __DICT_H */
