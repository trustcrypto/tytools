/**
 * ty, command-line program to manage Teensy devices
 * http://github.com/Koromix/ty
 * Copyright (C) 2014 Niels Martignène
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#ifndef TY_LIST_H
#define TY_LIST_H

#include "common.h"

TY_C_BEGIN

typedef struct ty_list_head
{
    struct ty_list_head *prev;
    struct ty_list_head *next;
} ty_list_head;

#define TY_LIST_HEAD(head) \
    ty_list_head head = {&head, &head}

static inline void ty_list_init(ty_list_head *head)
{
    head->prev = head;
    head->next = head;
}

static inline void _ty_list_insert(ty_list_head *prev, ty_list_head *next, ty_list_head *head)
{
    assert(prev);
    assert(next);
    assert(head);

    prev->next = head;
    head->prev = prev;

    next->prev = head;
    head->next = next;
}

static inline void ty_list_append(ty_list_head *prev, ty_list_head *head)
{
    _ty_list_insert(prev, prev->next, head);
}

static inline void ty_list_prepend(ty_list_head *next, ty_list_head *head)
{
    _ty_list_insert(next->prev, next, head);
}

static inline void ty_list_remove(ty_list_head *head)
{
    assert(head);
    assert(head->prev);
    assert(head->next);

    head->prev->next = head->next;
    head->next->prev = head->prev;

    head->prev = NULL;
    head->next = NULL;
}

static inline bool ty_list_empty(ty_list_head *head)
{
    assert(head);

    return head->next == head;
}

#define ty_list_entry(head, type, member) \
    ((type *)((char *)(head) - (size_t)(&((type *)0)->member)))

#define ty_list_foreach(cur, head) \
    for (ty_list_head *cur = (head)->next, *TY_UNIQUE_ID(next) = cur->next; cur != (head); \
         cur = TY_UNIQUE_ID(next), TY_UNIQUE_ID(next) = cur->next)

TY_C_END

#endif
