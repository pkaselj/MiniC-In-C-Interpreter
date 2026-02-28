#include "lexer.h"
#include "Utils.h"

#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <assert.h>


// ---------------- Lexer

void tok_free_string(Token* token);
void tok_free_identifier(Token* token);

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
		break;
	case TT_ID:
		tok_free_identifier(token);
		break;
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
	memset((char*)token->value.as_string.data, 0, length + 1);
	strncpy((char*)token->value.as_string.data, data, length);
	token->value.as_string.size = length;
	token->length = length; // Doesn't account for quotes
	return token;
}

Token* tok_create_operator(TokenType operator, size_t length)
{
	Token* token = _tok_create_empty();
	token->type = operator;
	token->length = length;
	return token;
}

Token* tok_create_number(double number, size_t length)
{
	Token* token = _tok_create_empty();
	token->type = TT_NUMBER;
	token->value.as_number = number;
	token->length = length;
	return token;
}

Token* tok_create_keyword(TokenType keyword, size_t length)
{
	Token* token = _tok_create_empty();
	token->type = keyword;
	token->length = length;
	return token;
}

Token* tok_create_identifier(StringView id)
{
	Token* token = _tok_create_empty();
	token->type = TT_ID;
	token->value.as_string = sv_create_empty();
	token->value.as_string.data = (char*)malloc((id.size + 1) * sizeof(char));
	assert(token->value.as_string.data);
	memset((char*)token->value.as_string.data, 0, id.size + 1);
	strncpy((char*)token->value.as_string.data, id.data, id.size);
	token->value.as_string.size = id.size;
	token->length = id.size;
	return token;
}

Token* tok_create_punctuation(TokenType punctuation, size_t length)
{
	Token* token = _tok_create_empty();
	token->type = punctuation;
	token->length = length;
	return token;
}

// -----

void tok_free_string(Token* token)
{
	assert(token);
	assert(token->type == TT_STRING);

	assert(token->value.as_string.data);
	free((char*)token->value.as_string.data);
	token->value.as_string.data = NULL;
	token->value.as_string.size = 0;
}

void tok_free_identifier(Token* token)
{
	assert(token);
	assert(token->type == TT_ID);

	assert(token->value.as_string.data);
	free((char*)token->value.as_string.data);
	token->value.as_string.data = NULL;
	token->value.as_string.size = 0;
}

// Lexing sub-functions

// Returns number of consumed characters
size_t lexer_parse_whitespace(const StringView buffer)
{
	size_t i = 0;
	while (i < buffer.size && isblank(buffer.data[i]))
	{
		i++;
	}
	return i;
}

Token* lexer_parse_string(const StringView buffer)
{

	if (buffer.size < 2)
	{
		return NULL;
	}

	if (buffer.data[0] != '"')
	{
		return NULL;
	}

	size_t i = 1;
	while (i < buffer.size && buffer.data[i] != '"')
	{
		i++;
	}
	
	if (buffer.data[i] != '"')
	{
		return NULL;
	}

	StringView literal = sv_substring(buffer, 1, i - 1);
	assert(sv_is_empty(literal) == false);
	Token* token = tok_create_string(literal.data, literal.size);
	token->length += 2; // Account for quotes
	return token;
}


Token* lexer_parse_operator(const StringView buffer)
{
		 if (sv_begins_with(buffer, sv_create("==")))
		return tok_create_operator(TT_OP_EQ, 2);
	else if (sv_begins_with(buffer, sv_create("!=")))
		return tok_create_operator(TT_OP_NEQ, 2);
	else if (sv_begins_with(buffer, sv_create(">=")))
		return tok_create_operator(TT_OP_GTE, 2);
	else if (sv_begins_with(buffer, sv_create("<=")))
		return tok_create_operator(TT_OP_LTE, 2);
	else if (sv_begins_with(buffer, sv_create("||")))
		return tok_create_operator(TT_OP_OR, 2);
	else if (sv_begins_with(buffer, sv_create("&&")))
		return tok_create_operator(TT_OP_AND, 2);
	else if (sv_begins_with(buffer, sv_create("=")))
		return tok_create_operator(TT_ASSIGN, 1);
	else if (sv_begins_with(buffer, sv_create("+")))
		return tok_create_operator(TT_OP_ADD, 1);
	else if (sv_begins_with(buffer, sv_create("-")))
		return tok_create_operator(TT_OP_SUB, 1);
	else if (sv_begins_with(buffer, sv_create("*")))
		return tok_create_operator(TT_OP_MUL, 1);
	else if (sv_begins_with(buffer, sv_create("/")))
		return tok_create_operator(TT_OP_DIV, 1);
	else if (sv_begins_with(buffer, sv_create(">")))
		return tok_create_operator(TT_OP_GT, 1);
	else if (sv_begins_with(buffer, sv_create("<")))
		return tok_create_operator(TT_OP_LT, 1);
	else if (sv_begins_with(buffer, sv_create("!")))
		return tok_create_operator(TT_OP_NOT, 1);
	else if (sv_begins_with(buffer, sv_create(",")))
		return tok_create_operator(TT_K_COMMA, 1);

	return NULL;
}

// Only 123.456 format for now - TODO
Token* lexer_parse_number(const StringView buffer)
{
	double number = 0;
	double current = 0;

	bool has_point = false;
	size_t i = 0;
	while (i < buffer.size)
	{
		char c = buffer.data[i];
		if (isdigit(c))
		{
			if (!has_point)
			{
				current = c - '0';
				number = number * 10 + current;
			}
			else
			{
				double n = c - '0';
				number += n * current;
				current *= 0.1;
			}
		}
		else if (c == '.' && !has_point)
		{
			has_point = true;
			current = 0.1; // Fracitonal multiplier
		}
		else
		{
			break;
		}
		i++;
	}

	// Nothing was parsed
	if (i == 0)
		return NULL;

	return tok_create_number(number, i);
}

Token* lexer_parse_identifier_or_keyword(const StringView buffer)
{
	size_t i = 0;
	while (i < buffer.size)
	{
		char c = buffer.data[i];
		if (!isupper(c) && !islower(c) && c != '_' && !(i != 0 && isdigit(c)))
		{
			break;
		}
		i++;
	}

	// Nothing was parsed
	if (i == 0)
		return NULL;

	StringView id = sv_substring(buffer, 0, i);

		 if (sv_equal(id, sv_create("function")))
		return tok_create_keyword(TT_K_FN, id.size);
	else if (sv_equal(id, sv_create("return")))
		return tok_create_keyword(TT_K_RET, id.size);
	else if (sv_equal(id, sv_create("if")))
		return tok_create_keyword(TT_K_IF, id.size);
	else if (sv_equal(id, sv_create("else")))
		return tok_create_keyword(TT_K_ELSE, id.size);
	else if (sv_equal(id, sv_create("while")))
		return tok_create_keyword(TT_K_WHILE, id.size);
	else if (sv_equal(id, sv_create("for")))
		return tok_create_keyword(TT_K_FOR, id.size);

	return tok_create_identifier(id);
}

Token* lexer_parse_punctuation(const StringView buffer)
{
		 if (sv_begins_with(buffer, sv_create("(")))
		return tok_create_keyword(TT_O_PAREN, 1);
	else if (sv_begins_with(buffer, sv_create(")")))
		return tok_create_keyword(TT_C_PAREN, 1);
	else if (sv_begins_with(buffer, sv_create("{")))
		return tok_create_keyword(TT_O_BRACE, 1);
	else if (sv_begins_with(buffer, sv_create("}")))
		return tok_create_keyword(TT_C_BRACE, 1);
	else if (sv_begins_with(buffer, sv_create(";")))
		return tok_create_keyword(TT_DELIM, 1);

	return NULL;
}

TokenList* lexer_perform(const StringView input)
{
	TokenList* list = tl_create();

	size_t i = 0;
	while (i < input.size)
	{
		StringView rem = sv_substring_to_end(input, i);
		if (sv_is_empty(rem))
		{
			LogError("Failed to get substring at position %llu\n", i);
			goto _cleanup;
		}

		size_t consumed = lexer_parse_whitespace(rem);
		if (consumed != 0)
		{
			i += consumed;
			continue;
		}

		Token* token;

		token = lexer_parse_string(rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		token = lexer_parse_operator(rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		token = lexer_parse_number(rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		token = lexer_parse_identifier_or_keyword(rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		token = lexer_parse_punctuation(rem);
		if (token != NULL)
		{
			i += token->length;
			tl_push(list, token);
			continue;
		}

		LogError("lexer_perform() - Lexer error @ index %llu, symbol [%c].\n", i, input.data[i]);
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

void tl_free(TRANSFER TokenList* list)
{
	if (!list)
		return;

	while(list->first)
	{
		Token* token = tl_pop(list);
		tok_free(token);
	}

	list->first = NULL;
	list->last = NULL;
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
