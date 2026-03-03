#include "Parser.h"

#include <assert.h>
#include <stdlib.h>

// ---- token list manipulation

NONOWNING Token* token_peek(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_get(iter));
}

Token* token_advance(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_advance(iter));
}

Token* token_match(ListConstIterator* iter, TokenType required)
{
	assert(iter);

	ListNode* current = list_iterator_get(iter);
	Token* _token = list_node_data_get(current);

	if (!current || !_token)
	{
		return NULL;
	}

	if (_token->type == required)
	{
		UNUSED(list_iterator_advance(iter));
		return _token;
	}

	return NULL;
}

Token* token_match_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
{
	assert(iter);
	assert(required_array);

	ListNode* current = list_peek(iter);
	Token* _token = list_node_data_get(current);

	if (!current || !_token)
	{
		return NULL;
	}

	for (size_t i = 0; i < items; i++)
	{
		if (_token->type == required_array[i])
		{
			UNUSED(list_iterator_advance(iter));
			return _token;
		}
	}

	return NULL;
}

Token* token_expect(ListConstIterator* iter, TokenType required)
{
	// TODO: track index and handle errors better.
	Token* _token = token_match(iter, required);
	if (!_token)
	{
		_token = token_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [%d], got [%d]!\n", index, _token->type, required);
		exit(-1);
	}
}

Token* token_expect_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
{
	// TODO: track index and handle errors better.
	Token* _token = token_match_any(iter, required_array, items);
	if (!_token)
	{
		_token = token_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [??], got [%d]!\n", index, _token->type); // TODO expected
		exit(-1);
	}
}

// ---- private parser functions



// ---- public api

AstTree* parser_perform(List* tokens)
{

}