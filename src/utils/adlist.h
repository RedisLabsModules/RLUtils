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

typedef struct RMUTILS_PRFX_listNode {
    struct RMUTILS_PRFX_listNode *prev;
    struct RMUTILS_PRFX_listNode *next;
    void *value;
} RMUTILS_PRFX_listNode;

typedef struct RMUTILS_PRFX_listIter {
    RMUTILS_PRFX_listNode *next;
    int direction;
} RMUTILS_PRFX_listIter;

typedef struct RMUTILS_PRFX_list {
    RMUTILS_PRFX_listNode *head;
    RMUTILS_PRFX_listNode *tail;
    void *(*dup)(void *ptr);
    void (*free)(void *ptr);
    int (*match)(void *ptr, void *key);
    unsigned long len;
} RMUTILS_PRFX_list;

/* Functions implemented as macros */
#define RMUTILS_PRFX_listLength(l) ((l)->len)
#define RMUTILS_PRFX_listFirst(l) ((l)->head)
#define RMUTILS_PRFX_listLast(l) ((l)->tail)
#define RMUTILS_PRFX_listPrevNode(n) ((n)->prev)
#define RMUTILS_PRFX_listNextNode(n) ((n)->next)
#define RMUTILS_PRFX_listNodeValue(n) ((n)->value)

#define RMUTILS_PRFX_listSetDupMethod(l,m) ((l)->dup = (m))
#define RMUTILS_PRFX_listSetFreeMethod(l,m) ((l)->free = (m))
#define RMUTILS_PRFX_listSetMatchMethod(l,m) ((l)->match = (m))

#define RMUTILS_PRFX_listGetDupMethod(l) ((l)->dup)
#define RMUTILS_PRFX_listGetFree(l) ((l)->free)
#define RMUTILS_PRFX_listGetMatchMethod(l) ((l)->match)

/* Prototypes */
RMUTILS_PRFX_list *RMUTILS_PRFX_listCreate(void);
void RMUTILS_PRFX_listRelease(RMUTILS_PRFX_list *list);
void RMUTILS_PRFX_listEmpty(RMUTILS_PRFX_list *list);
RMUTILS_PRFX_list *RMUTILS_PRFX_listAddNodeHead(RMUTILS_PRFX_list *list, void *value);
RMUTILS_PRFX_list *RMUTILS_PRFX_listAddNodeTail(RMUTILS_PRFX_list *list, void *value);
RMUTILS_PRFX_list *RMUTILS_PRFX_listInsertNode(RMUTILS_PRFX_list *list, RMUTILS_PRFX_listNode *old_node, void *value, int after);
void RMUTILS_PRFX_listDelNode(RMUTILS_PRFX_list *list, RMUTILS_PRFX_listNode *node);
RMUTILS_PRFX_listIter *RMUTILS_PRFX_listGetIterator(RMUTILS_PRFX_list *list, int direction);
RMUTILS_PRFX_listNode *RMUTILS_PRFX_listNext(RMUTILS_PRFX_listIter *iter);
void RMUTILS_PRFX_listReleaseIterator(RMUTILS_PRFX_listIter *iter);
RMUTILS_PRFX_list *RMUTILS_PRFX_listDup(RMUTILS_PRFX_list *orig);
RMUTILS_PRFX_listNode *RMUTILS_PRFX_listSearchKey(RMUTILS_PRFX_list *list, void *key);
RMUTILS_PRFX_listNode *RMUTILS_PRFX_listIndex(RMUTILS_PRFX_list *list, long index);
void RMUTILS_PRFX_listRewind(RMUTILS_PRFX_list *list, RMUTILS_PRFX_listIter *li);
void RMUTILS_PRFX_listRewindTail(RMUTILS_PRFX_list *list, RMUTILS_PRFX_listIter *li);
void RMUTILS_PRFX_listRotate(RMUTILS_PRFX_list *list);
void RMUTILS_PRFX_listJoin(RMUTILS_PRFX_list *l, RMUTILS_PRFX_list *o);

/* Directions for iterators */
#define AL_START_HEAD 0
#define AL_START_TAIL 1

#endif /* __ADLIST_H__ */
