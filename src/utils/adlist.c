/* adlist.c - A generic doubly linked list implementation
 *
 * Copyright (c) 2006-2010, Salvatore Sanfilippo <antirez at gmail dot com>
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


#include <stdlib.h>
#include "adlist.h"
#include "../memory/rlutils_memory.h"

/* Create a new list. The created list can be freed with
 * AlFreeList(), but private value of every node need to be freed
 * by the user before to call AlFreeList().
 *
 * On error, NULL is returned. Otherwise the pointer to the new list. */
RLUTILS_PRFX_list *RLUTILS_PRFX_listCreate(void)
{
    struct RLUTILS_PRFX_list *list;

    if ((list = RLUTILS_PRFX_malloc(sizeof(*list))) == NULL)
        return NULL;
    list->head = list->tail = NULL;
    list->len = 0;
    list->dup = NULL;
    list->free = NULL;
    list->match = NULL;
    return list;
}

/* Remove all the elements from the list without destroying the list itself. */
void RLUTILS_PRFX_listEmpty(RLUTILS_PRFX_list *list)
{
    unsigned long len;
    RLUTILS_PRFX_listNode *current, *next;

    current = list->head;
    len = list->len;
    while(len--) {
        next = current->next;
        if (list->free) list->free(current->value);
        RLUTILS_PRFX_free(current);
        current = next;
    }
    list->head = list->tail = NULL;
    list->len = 0;
}

/* Free the whole list.
 *
 * This function can't fail. */
void RLUTILS_PRFX_listRelease(RLUTILS_PRFX_list *list)
{
    RLUTILS_PRFX_listEmpty(list);
    RLUTILS_PRFX_free(list);
}

/* Add a new node to the list, to head, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
RLUTILS_PRFX_list *RLUTILS_PRFX_listAddNodeHead(RLUTILS_PRFX_list *list, void *value)
{
    RLUTILS_PRFX_listNode *node;

    if ((node = RLUTILS_PRFX_malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = NULL;
        node->next = list->head;
        list->head->prev = node;
        list->head = node;
    }
    list->len++;
    return list;
}

/* Add a new node to the list, to tail, containing the specified 'value'
 * pointer as value.
 *
 * On error, NULL is returned and no operation is performed (i.e. the
 * list remains unaltered).
 * On success the 'list' pointer you pass to the function is returned. */
RLUTILS_PRFX_list *RLUTILS_PRFX_listAddNodeTail(RLUTILS_PRFX_list *list, void *value)
{
    RLUTILS_PRFX_listNode *node;

    if ((node = RLUTILS_PRFX_malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (list->len == 0) {
        list->head = list->tail = node;
        node->prev = node->next = NULL;
    } else {
        node->prev = list->tail;
        node->next = NULL;
        list->tail->next = node;
        list->tail = node;
    }
    list->len++;
    return list;
}

RLUTILS_PRFX_list *RLUTILS_PRFX_listInsertNode(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listNode *old_node, void *value, int after) {
    RLUTILS_PRFX_listNode *node;

    if ((node = RLUTILS_PRFX_malloc(sizeof(*node))) == NULL)
        return NULL;
    node->value = value;
    if (after) {
        node->prev = old_node;
        node->next = old_node->next;
        if (list->tail == old_node) {
            list->tail = node;
        }
    } else {
        node->next = old_node;
        node->prev = old_node->prev;
        if (list->head == old_node) {
            list->head = node;
        }
    }
    if (node->prev != NULL) {
        node->prev->next = node;
    }
    if (node->next != NULL) {
        node->next->prev = node;
    }
    list->len++;
    return list;
}

/* Remove the specified node from the specified list.
 * It's up to the caller to free the private value of the node.
 *
 * This function can't fail. */
void RLUTILS_PRFX_listDelNode(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listNode *node)
{
    if (node->prev)
        node->prev->next = node->next;
    else
        list->head = node->next;
    if (node->next)
        node->next->prev = node->prev;
    else
        list->tail = node->prev;
    if (list->free) list->free(node->value);
    RLUTILS_PRFX_free(node);
    list->len--;
}

/* Returns a list iterator 'iter'. After the initialization every
 * call to listNext() will return the next element of the list.
 *
 * This function can't fail. */
RLUTILS_PRFX_listIter *RLUTILS_PRFX_listGetIterator(RLUTILS_PRFX_list *list, int direction)
{
    RLUTILS_PRFX_listIter *iter;

    if ((iter = RLUTILS_PRFX_malloc(sizeof(*iter))) == NULL) return NULL;
    if (direction == AL_START_HEAD)
        iter->next = list->head;
    else
        iter->next = list->tail;
    iter->direction = direction;
    return iter;
}

/* Release the iterator memory */
void RLUTILS_PRFX_listReleaseIterator(RLUTILS_PRFX_listIter *iter) {
    RLUTILS_PRFX_free(iter);
}

/* Create an iterator in the list private iterator structure */
void RLUTILS_PRFX_listRewind(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listIter *li) {
    li->next = list->head;
    li->direction = AL_START_HEAD;
}

void RLUTILS_PRFX_listRewindTail(RLUTILS_PRFX_list *list, RLUTILS_PRFX_listIter *li) {
    li->next = list->tail;
    li->direction = AL_START_TAIL;
}

/* Return the next element of an iterator.
 * It's valid to remove the currently returned element using
 * listDelNode(), but not to remove other elements.
 *
 * The function returns a pointer to the next element of the list,
 * or NULL if there are no more elements, so the classical usage patter
 * is:
 *
 * iter = listGetIterator(list,<direction>);
 * while ((node = listNext(iter)) != NULL) {
 *     doSomethingWith(listNodeValue(node));
 * }
 *
 * */
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listNext(RLUTILS_PRFX_listIter *iter)
{
    RLUTILS_PRFX_listNode *current = iter->next;

    if (current != NULL) {
        if (iter->direction == AL_START_HEAD)
            iter->next = current->next;
        else
            iter->next = current->prev;
    }
    return current;
}

/* Duplicate the whole list. On out of memory NULL is returned.
 * On success a copy of the original list is returned.
 *
 * The 'Dup' method set with listSetDupMethod() function is used
 * to copy the node value. Otherwise the same pointer value of
 * the original node is used as value of the copied node.
 *
 * The original list both on success or error is never modified. */
RLUTILS_PRFX_list *RLUTILS_PRFX_listDup(RLUTILS_PRFX_list *orig)
{
    RLUTILS_PRFX_list *copy;
    RLUTILS_PRFX_listIter iter;
    RLUTILS_PRFX_listNode *node;

    if ((copy = RLUTILS_PRFX_listCreate()) == NULL)
        return NULL;
    copy->dup = orig->dup;
    copy->free = orig->free;
    copy->match = orig->match;
    RLUTILS_PRFX_listRewind(orig, &iter);
    while((node = RLUTILS_PRFX_listNext(&iter)) != NULL) {
        void *value;

        if (copy->dup) {
            value = copy->dup(node->value);
            if (value == NULL) {
                RLUTILS_PRFX_listRelease(copy);
                return NULL;
            }
        } else
            value = node->value;
        if (RLUTILS_PRFX_listAddNodeTail(copy, value) == NULL) {
            RLUTILS_PRFX_listRelease(copy);
            return NULL;
        }
    }
    return copy;
}

/* Search the list for a node matching a given key.
 * The match is performed using the 'match' method
 * set with listSetMatchMethod(). If no 'match' method
 * is set, the 'value' pointer of every node is directly
 * compared with the 'key' pointer.
 *
 * On success the first matching node pointer is returned
 * (search starts from head). If no matching node exists
 * NULL is returned. */
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listSearchKey(RLUTILS_PRFX_list *list, void *key)
{
    RLUTILS_PRFX_listIter iter;
    RLUTILS_PRFX_listNode *node;

    RLUTILS_PRFX_listRewind(list, &iter);
    while((node = RLUTILS_PRFX_listNext(&iter)) != NULL) {
        if (list->match) {
            if (list->match(node->value, key)) {
                return node;
            }
        } else {
            if (key == node->value) {
                return node;
            }
        }
    }
    return NULL;
}

/* Return the element at the specified zero-based index
 * where 0 is the head, 1 is the element next to head
 * and so on. Negative integers are used in order to count
 * from the tail, -1 is the last element, -2 the penultimate
 * and so on. If the index is out of range NULL is returned. */
RLUTILS_PRFX_listNode *RLUTILS_PRFX_listIndex(RLUTILS_PRFX_list *list, long index) {
    RLUTILS_PRFX_listNode *n;

    if (index < 0) {
        index = (-index)-1;
        n = list->tail;
        while(index-- && n) n = n->prev;
    } else {
        n = list->head;
        while(index-- && n) n = n->next;
    }
    return n;
}

/* Rotate the list removing the tail node and inserting it to the head. */
void RLUTILS_PRFX_listRotate(RLUTILS_PRFX_list *list) {
    RLUTILS_PRFX_listNode *tail = list->tail;

    if (RLUTILS_PRFX_listLength(list) <= 1) return;

    /* Detach current tail */
    list->tail = tail->prev;
    list->tail->next = NULL;
    /* Move it as head */
    list->head->prev = tail;
    tail->prev = NULL;
    tail->next = list->head;
    list->head = tail;
}

/* Add all the elements of the list 'o' at the end of the
 * list 'l'. The list 'other' remains empty but otherwise valid. */
void RLUTILS_PRFX_listJoin(RLUTILS_PRFX_list *l, RLUTILS_PRFX_list *o) {
    if (o->head)
        o->head->prev = l->tail;

    if (l->tail)
        l->tail->next = o->head;
    else
        l->head = o->head;

    if (o->tail) l->tail = o->tail;
    l->len += o->len;

    /* Setup other as an empty list. */
    o->head = o->tail = NULL;
    o->len = 0;
}
