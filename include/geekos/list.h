/*
 * GeekOS - generic intrusive list macros
 * Copyright (C) 2001-2008, David H. Hovemeyer <david.hovemeyer@gmail.com>
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *   
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *  
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 */

#ifndef GEEKOS_LIST_H
#define GEEKOS_LIST_H

#include <geekos/types.h>
#include <geekos/kassert.h>

/*
 * Declare a list structure and prototypes for the list accessor
 * functions.  This will most likely be in a header file
 * (if the list accessor functions are intended to be public).
 */
#define DECLARE_LIST(list_type, node_type) \
struct node_type; \
struct list_type { \
	struct node_type *head, *tail; \
}; \
bool list_type##_is_empty(struct list_type *list); \
void list_type##_clear(struct list_type *list); \
void list_type##_append(struct list_type *list, struct node_type *node); \
void list_type##_append_all(struct list_type *list, struct list_type *to_append); \
void list_type##_prepend(struct list_type *list, struct node_type *node); \
struct node_type *list_type##_get_last(struct list_type *list); \
struct node_type *list_type##_get_first(struct list_type *list); \
struct node_type *list_type##_remove_last(struct list_type *list); \
struct node_type *list_type##_remove_first(struct list_type *list); \
void list_type##_remove(struct list_type *list, struct node_type *node); \
struct node_type *list_type##_next(struct node_type *node); \
struct node_type *list_type##_prev(struct node_type *node)

/*
 * Define the link fields for given list/node type.
 * This should appear in the node type struct.
 */
#define DEFINE_LINK(list_type, node_type) \
	struct node_type *list_type##_prev; \
	struct node_type *list_type##_next

/* Define implementation of list_type##_is_empty function */
#define IMPLEMENT_LIST_IS_EMPTY(list_type, node_type) \
bool list_type##_is_empty(struct list_type *list) \
{ \
	return list->head == 0; \
}

/* Define implementation of list_type##_clear function */
#define IMPLEMENT_LIST_CLEAR(list_type, node_type) \
void list_type##_clear(struct list_type *list) \
{ \
	list->head = list->tail = 0; \
}

/* Define implementation of list_type##_append function */
#define IMPLEMENT_LIST_APPEND(list_type, node_type) \
void list_type##_append(struct list_type *list, struct node_type *node) \
{ \
	node->list_type##_next = node->list_type##_prev = 0; \
	if (list->head == 0) { \
		list->head = list->tail = node; \
	} else { \
		node->list_type##_prev = list->tail; \
		list->tail->list_type##_next = node; \
		list->tail = node; \
	} \
}

/* Define implementation of list_type##_append_all function */
#define IMPLEMENT_LIST_APPEND_ALL(list_type, node_type) \
void list_type##_append_all(struct list_type *list, struct list_type *to_append) \
{ \
	if (to_append->head == 0) { \
		return; \
	} \
	if (list->head == 0) { \
		list->head = to_append->head; \
		list->tail = to_append->tail; \
	} else { \
		list->tail->list_type##_next = to_append->head; \
		to_append->head->list_type##_prev = list->tail; \
		list->tail = to_append->tail; \
	} \
	list_type##_clear(to_append); \
}

/* Define implementation of list_type##_prepend function */
#define IMPLEMENT_LIST_PREPEND(list_type, node_type) \
void list_type##_prepend(struct list_type *list, struct node_type *node) \
{ \
	node->list_type##_next = node->list_type##_prev = 0; \
	if (list->head == 0) { \
		list->head = list->tail = node; \
	} else { \
		node->list_type##_next = list->head; \
		list->head->list_type##_prev = node; \
		list->head = node; \
	} \
}

/* Define implementation of list_type##_get_last function */
#define IMPLEMENT_LIST_GET_LAST(list_type, node_type) \
struct node_type *list_type##_get_last(struct list_type *list) \
{ \
	return list->tail; \
}

/* Define implementation of list_type##_get_first function */
#define IMPLEMENT_LIST_GET_FIRST(list_type, node_type) \
struct node_type *list_type##_get_first(struct list_type *list) \
{ \
	return list->head; \
}

/* Define implementation of list_type##_remove_last function */
#define IMPLEMENT_LIST_REMOVE_LAST(list_type, node_type) \
struct node_type *list_type##_remove_last(struct list_type *list) \
{ \
	struct node_type *result; \
	KASSERT(list->head != 0); \
	if (list->head == list->tail) { \
		result = list->head; \
		list->head = list->tail = 0; \
	} else { \
		result = list->tail; \
		list->tail = list->tail->list_type##_prev; \
		list->tail->list_type##_next = 0; \
	} \
	return result; \
}

/* Define implementation of list_type##_remove_first function */
#define IMPLEMENT_LIST_REMOVE_FIRST(list_type, node_type) \
struct node_type *list_type##_remove_first(struct list_type *list) \
{ \
	struct node_type *result; \
	KASSERT(list->head != 0); \
	if (list->head == list->tail) { \
		result = list->head; \
		list->head = list->tail = 0; \
	} else { \
		result = list->head; \
		list->head = list->head->list_type##_next; \
		list->head->list_type##_prev = 0; \
	} \
	return result; \
}

/* Define implementation of list_type##_remove function */
#define IMPLEMENT_LIST_REMOVE(list_type, node_type) \
void list_type##_remove(struct list_type *list, struct node_type *node) \
{ \
	if (node->list_type##_prev != 0) { \
		node->list_type##_prev->list_type##_next = node->list_type##_next; \
	} else { \
		list->head = node->list_type##_next; \
	} \
	if (node->list_type##_next != 0) { \
		node->list_type##_next->list_type##_prev = node->list_type##_prev; \
	} else { \
		list->tail = node->list_type##_prev; \
	} \
}

/* Define implementation of list_type##_next function */
#define IMPLEMENT_LIST_NEXT(list_type, node_type) \
struct node_type *list_type##_next(struct node_type *node) \
{ \
	return node->list_type##_next; \
}

/* Define implementation of list_type##_prev function */
#define IMPLEMENT_LIST_PREV(list_type, node_type) \
struct node_type *list_type##_prev(struct node_type *node) \
{ \
	return node->list_type##_prev; \
}

/* Define implementations of all list accessor functions. */
#define IMPLEMENT_LIST(list_type, node_type) \
IMPLEMENT_LIST_IS_EMPTY(list_type, node_type) \
IMPLEMENT_LIST_CLEAR(list_type, node_type) \
IMPLEMENT_LIST_APPEND(list_type, node_type) \
IMPLEMENT_LIST_APPEND_ALL(list_type, node_type) \
IMPLEMENT_LIST_PREPEND(list_type, node_type) \
IMPLEMENT_LIST_GET_LAST(list_type, node_type) \
IMPLEMENT_LIST_GET_FIRST(list_type, node_type) \
IMPLEMENT_LIST_REMOVE_LAST(list_type, node_type) \
IMPLEMENT_LIST_REMOVE_FIRST(list_type, node_type) \
IMPLEMENT_LIST_REMOVE(list_type, node_type) \
IMPLEMENT_LIST_NEXT(list_type, node_type) \
IMPLEMENT_LIST_PREV(list_type, node_type)

#endif /* GEEKOS_LIST_H */
