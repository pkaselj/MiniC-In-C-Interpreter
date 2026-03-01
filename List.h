#pragma once

#include "Common.h"

typedef void (*ListNodeDestructor)(void*);
typedef struct ListNode ListNode;
typedef struct List List;

typedef struct ListConstIterator ListConstIterator;

List* list_create(void);
void list_free(TRANSFER List* list);

// Pops the first token. NULL on empty
ListNode* list_pop(List* list);
// Pushed to the end.
void list_push(List* list, TRANSFER ListNode* node);
// Create list node, destructor can be NULL
ListNode* list_create_node(void* data, ListNodeDestructor destructor);
void list_free_node(TRANSFER ListNode* node);

ListConstIterator* list_create_iterator(List* list);
void list_free_iterator(ListConstIterator* iter);
NONOWNING ListNode* list_iterator_advance(ListConstIterator* iter);

NONOWNING void* list_node_data(ListNode* node);

