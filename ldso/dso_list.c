#include "dso_list.h"
#include "stdlib.h"
#include "string.h"
#include "stdio.h"

void dso_list_init(dso_list_t *list) {
    list->head = list->tail = NULL;
    list->size = 0;
}

dso_node_t* dso_list_append(dso_list_t *list, dso_t obj) {
    dso_node_t *node = malloc(sizeof(dso_node_t));
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

dso_node_t* dso_list_prepend(dso_list_t *list, dso_t obj) {
    dso_node_t *node = malloc(sizeof(dso_node_t));
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

dso_node_t* dso_list_insert_after(dso_list_t *list, dso_node_t *node, dso_t obj) {
    if (!node) return dso_list_append(list, obj);

    dso_node_t *new_node = malloc(sizeof(dso_node_t));
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

dso_node_t* dso_list_insert_before(dso_list_t *list, dso_node_t *node, dso_t obj) {
    if (!node) return dso_list_prepend(list, obj);

    dso_node_t *new_node = malloc(sizeof(dso_node_t));
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

void dso_list_delete(dso_list_t *list, dso_node_t *node) {
    if (!node) return;

    if (node->prev) node->prev->next = node->next;
    else list->head = node->next;

    if (node->next) node->next->prev = node->prev;
    else list->tail = node->prev;

    free(node);
    list->size--;
}

dso_t dso_list_pop_front(dso_list_t *list) {
    if (!list->head) return (dso_t){0};
    dso_node_t *node = list->head;
    dso_t data = node->data;
    dso_list_delete(list, node);
    return data;
}

dso_t dso_list_pop_back(dso_list_t *list) {
    if (!list->tail) return (dso_t){0};
    dso_node_t *node = list->tail;
    dso_t data = node->data;
    dso_list_delete(list, node);
    return data;
}

void dso_list_clear(dso_list_t *list) {
    dso_node_t *cur = list->head;
    while (cur) {
        dso_node_t *next = cur->next;
        free(cur);
        cur = next;
    }
    list->head = list->tail = NULL;
    list->size = 0;
}

void dso_list_free(dso_list_t *list) {
    dso_list_clear(list);
}
