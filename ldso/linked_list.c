#include "ldso.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

void linked_list_init(linked_list_t *list) {
    list->head = list->tail = NULL;
    list->size = 0;
}

linked_node_t* linked_list_append(linked_list_t *list, data_t obj) {
    linked_node_t *node = malloc(sizeof(linked_node_t));
    if (!node) return NULL;
    node->data = obj;
    node->next = NULL;
    node->prev = list->tail;

    if (list->tail) list->tail->next = node;
    else list->head = node;

    list->tail = node;
    list->size++;
    return node;
}

linked_node_t* linked_list_prepend(linked_list_t *list, data_t obj) {
    linked_node_t *node = malloc(sizeof(linked_node_t));
    if (!node) return NULL;
    node->data = obj;
    node->prev = NULL;
    node->next = list->head;

    if (list->head) list->head->prev = node;
    else list->tail = node;

    list->head = node;
    list->size++;
    return node;
}

linked_node_t* linked_list_insert_after(linked_list_t *list, linked_node_t *node, data_t obj) {
    if (!node) return linked_list_append(list, obj);

    linked_node_t *new_node = malloc(sizeof(linked_node_t));
    if (!new_node) return NULL;
    new_node->data = obj;
    new_node->prev = node;
    new_node->next = node->next;

    if (node->next) node->next->prev = new_node;
    else list->tail = new_node;

    node->next = new_node;
    list->size++;
    return new_node;
}

linked_node_t* linked_list_insert_before(linked_list_t *list, linked_node_t *node, data_t obj) {
    if (!node) return linked_list_prepend(list, obj);

    linked_node_t *new_node = malloc(sizeof(linked_node_t));
    if (!new_node) return NULL;
    new_node->data = obj;
    new_node->next = node;
    new_node->prev = node->prev;

    if (node->prev) node->prev->next = new_node;
    else list->head = new_node;

    node->prev = new_node;
    list->size++;
    return new_node;
}

void linked_list_delete(linked_list_t *list, linked_node_t *node) {
    if (!node) return;

    if (node->prev) node->prev->next = node->next;
    else list->head = node->next;

    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;

    free(node);
    list->size--;
}

data_t linked_list_pop_front(linked_list_t *list) {
    if (!list->head) return (data_t){0};
    linked_node_t *node = list->head;
    data_t data = node->data;
    linked_list_delete(list, node);
    return data;
}

data_t linked_list_pop_back(linked_list_t *list) {
    if (!list->tail) return (data_t){0};
    linked_node_t *node = list->tail;
    data_t data = node->data;
    linked_list_delete(list, node);
    return data;
}

void linked_list_clear(linked_list_t *list) {
    linked_node_t *cur = list->head;
    while (cur) {
        linked_node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

void linked_list_free(linked_list_t *list) {
    linked_list_clear(list);
}

linked_node_t *linked_list_search(linked_list_t *list, data_t *needle, int (*test)(data_t *, data_t *)) {
    if (list == NULL)
        return NULL;
    linked_node_t *cur = list->head;
    while (cur) {
        if (test(needle, &cur->data)) {
            return cur;
        }
        cur = cur->next;
    }
    return NULL;
}