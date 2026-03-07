#include <List.h>

#include <stddef.h>
#include <stdlib.h>
#include <assert.h>

struct ListNode
{
	struct ListNode* next;
	struct ListNode* previous;
	void* data;
	ListNodeDestructor destructor;
};

struct List
{
	struct ListNode* first;
	struct ListNode* last;
};

List* list_create(void)
{
	List* list = (List*)malloc(sizeof(List));
	if (list)
	{
		list->first = NULL;
		list->last = NULL;
	}
	return list;
}

void list_free(TRANSFER List* list)
{
	if (!list)
		return;

	while (list->first)
	{
		ListNode* node = list_pop(list);
		list_free_node(node);
	}

	list->first = NULL;
	list->last = NULL;

	free(list);
}

ListNode* list_pop(List* list)
{
	if (!list)
		return NULL;

	if (!list->first)
		return NULL;

	ListNode* node = list->first;

	if (list->first == list->last)
	{
		list->first = NULL;
		list->last = NULL;
	}
	else
	{
		ListNode* next = list->first->next;

		list->first = next;
		list->first->previous = NULL;
	}

	node->next = NULL;
	node->previous = NULL;

	return node;
}

ListNode* list_peek(List* list)
{
	if (!list)
		return NULL;

	return list->first;
}

void list_push(List* list, TRANSFER ListNode* node)
{
	if (!list || !node)
		return;

	if (!list->first)
	{
		assert(list->last == NULL);

		list->first = list->last = node;
		node->next = node->previous = NULL;
	}
	else
	{
		list->last->next = node;
		node->previous = list->last;
		list->last = node;

		node->next = NULL;
	}
}

ListNode* list_create_node(void* data, ListNodeDestructor destructor)
{
	ListNode* node = (ListNode*)malloc(sizeof(ListNode));
	assert(node);

	node->next = NULL;
	node->previous = NULL;

	node->data = data;
	node->destructor = destructor;

	return node;
}

void list_free_node(TRANSFER ListNode* node)
{
	// Assert node is detached
	assert(!node->next);
	assert(!node->previous);

	if (node->destructor)
	{
		node->destructor(node->data);
	}
	node->data = NULL;

	free(node);
}

// ---

struct ListConstIterator
{
	ListNode* current;
	size_t index;
};

ListConstIterator* list_create_iterator(List* list)
{
	if (!list)
	{
		return NULL;
	}

	ListConstIterator* iter = (ListConstIterator*)malloc(sizeof(ListConstIterator));
	iter->current = list->first;
	iter->index = 0;
	return iter;
}

void list_free_iterator(ListConstIterator* iter)
{
	if (!iter)
	{
		return;
	}

	iter->current = NULL;
	iter->index = 0;

	free(iter);
}

ListNode* list_iterator_current(ListConstIterator* iter)
{
	if (!iter)
	{
		return NULL;
	}

	return iter->current;
}

NONOWNING ListNode* list_iterator_get(ListConstIterator* iter)
{
	if (!iter)
	{
		return NULL;
	}

	return iter->current;
}

NONOWNING ListNode* list_iterator_advance(ListConstIterator* iter)
{
	if (!iter || !iter->current)
	{
		return NULL;
	}

	ListNode* current = iter->current;

	if(!iter->current->next)
	{
		iter->current = NULL;
	}
	else
	{
		iter->current = iter->current->next;
		iter->index++;
	}

	return current;
}

size_t list_interator_index(ListConstIterator* iter)
{
	assert(iter);

	return iter->index;
}

NONOWNING void* list_node_data_get(ListNode* node)
{
	if (!node)
	{
		return NULL;
	}

	return node->data;
}

void* list_node_data_release(ListNode* node)
{
	if (!node)
	{
		return NULL;
	}

	void* data = node->data;

	node->data = NULL;
	node->destructor = NULL;

	return data;
}
