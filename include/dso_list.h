#ifndef DSO_LIST_H
#define DSO_LIST_H

#include <stddef.h>
#include "ldso.h"

/* Node in the linked list */
typedef struct dso_node {
    dso_t data;
    struct dso_node *next;
    struct dso_node *prev;
} dso_node_t;

/* List head */
typedef struct {
    dso_node_t *head;
    dso_node_t *tail;
    size_t size;
} dso_list_t;

/* Initialize list */
void dso_list_init(dso_list_t *list);

/* Append or prepend */
dso_node_t* dso_list_append(dso_list_t *list, dso_t obj);
dso_node_t* dso_list_prepend(dso_list_t *list, dso_t obj);

/* Insert relative to a node */
dso_node_t* dso_list_insert_after(dso_list_t *list, dso_node_t *node, dso_t obj);
dso_node_t* dso_list_insert_before(dso_list_t *list, dso_node_t *node, dso_t obj);

/* Remove nodes */
void dso_list_delete(dso_list_t *list, dso_node_t *node);
dso_t dso_list_pop_front(dso_list_t *list);
dso_t dso_list_pop_back(dso_list_t *list);

/* Iterate macros */
#define dso_list_for_each(list, iter) \
    for (dso_node_t *iter = (list)->head; iter != NULL; iter = iter->next)

#define dso_list_for_each_reverse(list, iter) \
    for (dso_node_t *iter = (list)->tail; iter != NULL; iter = iter->prev)

/* Clear / free */
void dso_list_clear(dso_list_t *list);
void dso_list_free(dso_list_t *list);

/* Get size */
static inline size_t dso_list_size(dso_list_t *list) { return list->size; }

#endif /* DSO_LIST_H */
