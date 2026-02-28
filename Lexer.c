#include "lexer.h"
#include "Utils.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>


// ---------------- Lexer

void tok_free_string(Token* token);

// Token creation and destruction
Token* _tok_create_empty(void)
{
	Token* token = (Token*)malloc(sizeof(Token) / sizeof(char));
	assert(token);
	memset(token, 0, sizeof(token));
	return token;
}

void tok_free(TRANSFER Token* token)
{
	assert(token);

	switch (token->type)
	{
	case TT_STRING:
		tok_free_string(token);
	}

	memset(token, 0, sizeof(Token));
	free(token);
}

Token* tok_create_string(const char* data, size_t length)
{
	Token* token = _tok_create_empty();
	token->type = TT_STRING;
	token->value.as_string = sv_create_empty();
	token->value.as_string.data = (char*)malloc((length + 1) * sizeof(char));
	assert(token->value.as_string.data);
	memset(token->value.as_string.data, 0, length + 1);
	strncpy(token->value.as_string.data, data, length);
	token->value.as_string.size = length;
	token->length = length; // Doesn't account for quotes
	return token;
}

void tok_free_string(Token* token)
{
	assert(token);
	assert(token->type == TT_STRING);

	assert(token->value.as_string.data);
	free(token->value.as_string.data);
	token->value.as_string.data = NULL;
	token->value.as_string.size = 0;
}

// Lexing sub-functions

// Returns number of consumed characters
size_t lexer_parse_whitespace(const StringView* buffer)
{
	size_t i = 0;
	while (i < buffer->size && isblank(buffer->data[i]))
	{
		i++;
	}
	return i;
}

Token* lexer_parse_string(const StringView* buffer)
{

	if (buffer->size < 2)
	{
		return NULL;
	}

	if (buffer->data[0] != '"')
	{
		return NULL;
	}

	size_t i = 1;
	while (i < buffer->size && buffer->data[i] != '"')
	{
		i++;
	}
	
	if (buffer->data[i] != '"')
	{
		return NULL;
	}

	StringView literal = sv_substring(buffer, 1, i - 1);
	assert(sv_is_empty(&literal) == false);
	Token* token = tok_create_string(literal.data, literal.size);
	token->length += 2; // Account for quotes
	return token;
}

TokenList* lexer_perform(const StringView* input)
{
	TokenList* list = tl_create();

	size_t i = 0;
	while (i < input->size)
	{
		StringView rem = sv_substring_to_end(input, i);
		if (sv_is_empty(&rem))
		{
			LogError("Failed to get substring at position %llu\n", i);
			goto _cleanup;
		}

		size_t consumed = lexer_parse_whitespace(&rem);
		if (consumed != 0)
		{
			i += consumed;
			continue;
		}

		Token* token;

		token = lexer_parse_string(&rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		LogError("lexer_perform() - Lexer error @ index %llu, symbol [%c].\n", i, input->data[i]);
		goto _cleanup;
	}

	return list;

_cleanup:
	tl_free(list);
	return NULL;
}


// ---------------- Token List

TokenList* tl_create(void)
{
	TokenList* list = (TokenList*)malloc(sizeof(TokenList) / sizeof(char));
	if (list)
	{
		list->first = NULL;
		list->last = NULL;
	}
	return list;
}

void tl_free(TokenList* list)
{
	if (!list)
		return;

	while(list->first)
	{
		Token* token = tl_pop(list);

	}
}

Token* tl_pop(TokenList* list)
{
	if (!list)
		return NULL;

	if (!list->first)
		return NULL;

	Token* token = list->first;

	if (list->first == list->last)
	{
		list->first = NULL;
		list->last = NULL;
	}
	else
	{
		Token* next = list->first->next;

		list->first = next;
		list->first->previous = NULL;
	}

	token->next = NULL;
	token->previous = NULL;

	return token;
}

void tl_push(TokenList* list, TRANSFER Token* token)
{
	if (!list || !token)
		return;

	if (!list->first)
	{
		assert(list->last == NULL);

		list->first = list->last = token;
		token->next = token->previous = NULL;
	}
	else
	{
		list->last->next = token;
		token->previous = list->last;
		list->last = token;

		token->next = NULL;
	}
}
