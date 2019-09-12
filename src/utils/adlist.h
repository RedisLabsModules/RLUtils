/* adlist.h - A generic doubly linked list implementation
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

#ifndef __ADLIST_H__
#define __ADLIST_H__

/* Node, List, and Iterator are the only data structures used currently. */

typedef struct RLUTILS_PRFX_listNode {
    struct RLUTILS_PRFX_listNode *prev;
    struct RLUTILS_PRFX_listNode *next;
    void *value;
} RLUTILS_PRFX_listNode;

typedef struct RLUTILS_PRFX_listIter {
    RLUTILS_PRFX_listNode *next;
    int direction;
} RLUTILS_PRFX_listIter;

typedef struct RLUTILS_PRFX_list {
    RLUTILS_PRFX_listNode *head;
    RLUTILS_PRFX_listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
} RLUTILS_PRFX_list;

/* Functions implemented as macros */
#define RLUTILS_PRFX_listLength(l) ((l)->len)
#define RLUTILS_PRFX_listFirst(l) ((l)->head)
#define RLUTILS_PRFX_listLast(l) ((l)->tail)
#define RLUTILS_PRFX_listPrevNode(n) ((n)->prev)
#define RLUTILS_PRFX_listNextNode(n) ((n)->next)
#define RLUTILS_PRFX_listNodeValue(n) ((n)->value)

#define RLUTILS_PRFX_listSetDupMethod(l,m) ((l)->dup = (m))
#define RLUTILS_PRFX_listSetFreeMethod(l,m) ((l)->free = (m))
#define RLUTILS_PRFX_listSetMatchMethod(l,m) ((l)->match = (m))

#define RLUTILS_PRFX_listGetDupMethod(l) ((l)->dup)
#define RLUTILS_PRFX_listGetFree(l) ((l)->free)
#define RLUTILS_PRFX_listGetMatchMethod(l) ((l)->match)

/* Prototypes */
RLUTILS_PRFX_list *RLUTILS_PRFX_listCreate(void);
void RLUTILS_PRFX_listRelease(RLUTILS_PRFX_list *list);
void RLUTILS_PRFX_listEmpty(RLUTILS_PRFX_list *list);
RLUTILS_PRFX_list *RLUTILS_PRFX_listAddNodeHead(RLUTILS_PRFX_list *list, void *value);
RLUTILS_PRFX_list *RLUTILS_PRFX_listAddNodeTail(RLUTILS_PRFX_list *list, void *value);
RLUTILS_PRFX_list *RLUTILS_PRFX_listInsertNode(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listNode *old_node, void *value, int after);
void RLUTILS_PRFX_listDelNode(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listNode *node);
RLUTILS_PRFX_listIter *RLUTILS_PRFX_listGetIterator(RLUTILS_PRFX_list *list, int direction);
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listNext(RLUTILS_PRFX_listIter *iter);
void RLUTILS_PRFX_listReleaseIterator(RLUTILS_PRFX_listIter *iter);
RLUTILS_PRFX_list *RLUTILS_PRFX_listDup(RLUTILS_PRFX_list *orig);
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listSearchKey(RLUTILS_PRFX_list *list, void *key);
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listIndex(RLUTILS_PRFX_list *list, long index);
void RLUTILS_PRFX_listRewind(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listIter *li);
void RLUTILS_PRFX_listRewindTail(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listIter *li);
void RLUTILS_PRFX_listRotate(RLUTILS_PRFX_list *list);
void RLUTILS_PRFX_listJoin(RLUTILS_PRFX_list *l, RLUTILS_PRFX_list *o);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
