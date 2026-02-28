#include <stdio.h>
#include <string.h>

#include "Lexer.h"

const char* _GetTokenTypeString(TokenType type)
{
	switch (type)
	{
	case TT_NUMBER: return "TT_NUMBER";
	case TT_STRING: return "TT_STRING";
	case TT_OP_NOT: return "TT_OP_NOT";
	case TT_OP_OR: return "TT_OP_OR";
	case TT_OP_AND: return "TT_OP_AND";
	case TT_OP_ADD: return "TT_OP_ADD";
	case TT_OP_SUB: return "TT_OP_SUB";
	case TT_OP_MUL: return "TT_OP_MUL";
	case TT_OP_DIV: return "TT_OP_DIV";
	case TT_OP_EQ: return "TT_OP_EQ";
	case TT_OP_NEQ: return "TT_OP_NEQ";
	case TT_OP_GT: return "TT_OP_GT";
	case TT_OP_LT: return "TT_OP_LT";
	case TT_OP_GTE: return "TT_OP_GTE";
	case TT_OP_LTE: return "TT_OP_LTE";
	case TT_ID: return "TT_ID";
	case TT_ASSIGN: return "TT_ASSIGN";
	case TT_WSPC: return "TT_WSPC";
	case TT_K_FN: return "TT_K_FN";
	case TT_K_RET: return "TT_K_RET";
	case TT_K_IF: return "TT_K_IF";
	case TT_K_FOR: return "TT_K_FOR";
	case TT_K_ELSE: return "TT_K_ELSE";
	case TT_K_WHILE: return "TT_K_WHILE";
	case TT_K_COMMA: return "TT_K_COMMA";
	case TT_O_PAREN: return "TT_O_PAREN";
	case TT_C_PAREN: return "TT_C_PAREN";
	case TT_O_BRACE: return "TT_O_BRACE";
	case TT_C_BRACE: return "TT_C_BRACE";
	case TT_DELIM: return "TT_DELIM";

	default: return "<UNKNOWN>";
	}
}

void _GetTokenDataString(Token* token, char* buffer, size_t size)
{
	memset(buffer, 0, size);

	switch (token->type)
	{
	case TT_NUMBER:
		snprintf(buffer, size - 1, "%lf", token->value.as_number);
		break;
	case TT_STRING:
	case TT_ID:
		strncpy_s(buffer, size, token->value.as_string.data, token->value.as_string.size);
		break;
	}
}

void _PrintToken(Token* token)
{
	if (!token)
	{
		printf("<Token Error>\n");
		return;
	}

	const char* token_type = _GetTokenTypeString(token->type);
	size_t length = token->length;
	
	char data[256];
	_GetTokenDataString(token, data, sizeof(data));

	printf("[Token type=%s length=%llu data=%s]\n", token_type, length, data);
}

void PrintTokens(TokenList* list)
{
	if (!list)
	{
		printf("Empty list\n");
		return;
	}

	Token* t = list->first;
	while (t)
	{
		_PrintToken(t);
		t = t->next;
	}
}

int main(int argc, char* argv[])
{
	StringView input = sv_create("if(x) { y  = 4; } else { y = 5; } function a(b, c, d){ b + c * d; }");
	TokenList* list = lexer_perform(input);

	PrintTokens(list);

	return 0;
}