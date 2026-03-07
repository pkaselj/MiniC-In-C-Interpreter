#include <Utils.h>
#include <Lexer.h>
#include <Parser.h>
#include <Interpreter.h>
#include <Value.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

StringView sv_create(const char* data)
{
	assert(data);

	return (StringView)
	{
		.data = data,
		.size = strlen(data)
	};
}

StringView sv_create_s(const char* data, size_t length)
{
	assert(data);
	assert(length > 0);

	return (StringView)
	{
		.data = data,
		.size = length
	};
}

StringView sv_substring(const StringView sv, size_t index_start, size_t length)
{
	if (index_start + length > sv.size)
	{
		return sv_create_empty();
	}

	StringView subsv = sv_create_empty();
	subsv.data = &sv.data[index_start];
	subsv.size = length;
	return subsv;
}

StringView sv_substring_to_end(const StringView sv, size_t index_start)
{
	if (!sv.data || sv.size <= 0)
	{
		return sv_create_empty();
	}

	StringView subsv = sv_create_empty();
	subsv.data = &sv.data[index_start];
	subsv.size = sv.size - index_start;
	return subsv;
}

bool sv_is_empty(const StringView sv)
{
	return (!sv.data || sv.size <= 0);
}

StringView sv_create_empty()
{
	return (StringView)
	{
		.data = NULL,
		.size = 0
	};
}

bool sv_begins_with(const StringView sv, const StringView match)
{
	if (   !sv.data
		||  sv.size <= 0
		|| !match.data
		||  match.size	<= 0
		)
	{
		return false;
	}

	if (sv.size < match.size)
	{
		return false;
	}

	return (0 == strncmp(sv.data, match.data, match.size));
}

bool sv_equal(const StringView sv, const StringView match)
{
	return (sv.size == match.size) && sv_begins_with(sv, match);
}

// -----------------------

void LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[ERR] :: ");
	vprintf(format, args);
	va_end(args);
}

void LogInfo(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[INF] :: ");
	vprintf(format, args);
	va_end(args);
}

void LogDebug(const char* format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	printf("[INF] :: ");
	vprintf(format, args);
	va_end(args);
#else
	(void)format;
#endif
}

// -----------------------

const char* GetTokenTypeString(enum TokenType type)
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
		memset(buffer, 0, size);
		memcpy(buffer, token->value.as_string.data, token->value.as_string.size);
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

	const char* token_type = GetTokenTypeString(token->type);
	size_t length = token->length;

	char data[256];
	_GetTokenDataString(token, data, sizeof(data));

	printf("[Token type=%s length=%llu data=%s]\n", token_type, length, data);
}

void PrintTokens(List* list)
{
	if (!list)
	{
		printf("Empty list\n");
		return;
	}

	ListConstIterator* iter = list_create_iterator(list);
	ListNode* current = NULL;
	while ((current = list_iterator_advance(iter)))
	{
		_PrintToken((Token*)list_node_data_get(current));
	}
	list_free_iterator(iter);
}

// -----------------------

const char* GetAstNodeTypeString(enum AstNodeType type)
{
	switch (type)
	{
	case AST_S: return "AST_S";
	case AST_FN_CALL_EXPR: return "AST_FN_CALL_EXPR";
	case AST_ASSIGN_EXPR: return "AST_ASSIGN_EXPR";
	case AST_UNARY_EXPR: return "AST_UNARY_EXPR";
	case AST_BINARY_EXPR: return "AST_BINARY_EXPR";
	case AST_ID_EXPR: return "AST_ID_EXPR";
	case AST_NUM_EXPR: return "AST_NUM_EXPR";
	case AST_STR_EXPR: return "AST_STR_EXPR";
	case AST_EXPR_STMT: return "AST_EXPR_STMT";
	case AST_IF_STMT: return "AST_IF_STMT";
	case AST_FOR_STMT: return "AST_FOR_STMT";
	case AST_WHILE_STMT: return "AST_WHILE_STMT";
	case AST_BLOCK_STMT: return "AST_BLOCK_STMT";
	case AST_FN_DEF_STMT: return "AST_FN_DEF_STMT";

	default: return "<UNKNOWN>";
	}
}

void _PrintIndented(int indent, const char* format, ...)
{
	if (indent > 0)
	{
		for (size_t i = 0; i < indent - 1; i++)
		{
			printf(".");
		}

		if (indent % 2 == 0)
		{
			printf(">");
		}
		else
		{
			printf(" ");
		}
	}

	va_list args;
	va_start(args, format);
	vprintf(format, args);
	va_end(args);
}

void _PrintAstNode(int indent, AstNode* node);

void _PrintAstList(int indent, List* nodes)
{
	ListConstIterator* iter = list_create_iterator(nodes);
	ListNode* node = NULL;
	while ((node = list_iterator_advance(iter)))
	{
		_PrintAstNode(indent, list_node_data_get(node));
	}
	list_free_iterator(iter);
}

void _PrintAstNode(int indent, AstNode* node)
{
	if (!node)
		return;

	const char* type_string = GetAstNodeTypeString(node->type);
	_PrintIndented(indent, "%s\n", type_string);

	switch (node->type)
	{
	case AST_S:
		_PrintAstList(indent + 1, node->u.program.function_definitions);
		_PrintAstList(indent + 1, node->u.program.statements);
		break;
	case AST_FN_CALL_EXPR:
		_PrintAstNode(indent + 1, node->u.fn_call.symbol);
		_PrintAstList(indent + 1, node->u.fn_call.args);
		break;
	case AST_ASSIGN_EXPR:
		_PrintAstNode(indent + 1, node->u.assign.left);
		_PrintAstNode(indent + 1, node->u.assign.right);
		break;
	case AST_UNARY_EXPR:
		_PrintAstNode(indent + 1, node->u.unary_expr.child);
		_PrintIndented(indent + 1, "%s\n", GetTokenTypeString(node->u.unary_expr.op));
		break;
	case AST_BINARY_EXPR:
		_PrintAstNode(indent + 1, node->u.binary_expr.left);
		_PrintAstNode(indent + 1, node->u.binary_expr.right);
		_PrintIndented(indent + 1, "%s\n", GetTokenTypeString(node->u.binary_expr.op));
		break;
	case AST_ID_EXPR:
		_PrintIndented(indent + 1, "%s\n",	node->u.identifier.value.data);
		break;
	case AST_NUM_EXPR:
		_PrintIndented(indent + 1, "%lf\n", node->u.number.value);
		break;
	case AST_STR_EXPR:
		_PrintIndented(indent + 1, "%s\n", node->u.string.value.data);
		break;
	case AST_EXPR_STMT:
		_PrintAstNode(indent + 1, node->u.expr_stmt.expression);
		break;
	case AST_IF_STMT:
		_PrintAstNode(indent + 1, node->u.if_stmt.condition);
		_PrintAstNode(indent + 1, node->u.if_stmt.block_if);
		_PrintAstNode(indent + 1, node->u.if_stmt.block_else);
		break;
	case AST_FOR_STMT:
		_PrintAstNode(indent + 1, node->u.for_stmt.initial);
		_PrintAstNode(indent + 1, node->u.for_stmt.end_condition);
		_PrintAstNode(indent + 1, node->u.for_stmt.next_action);
		_PrintAstNode(indent + 1, node->u.for_stmt.block);
		break;
	case AST_WHILE_STMT:
		_PrintAstNode(indent + 1, node->u.while_stmt.condition);
		_PrintAstNode(indent + 1, node->u.while_stmt.block);
		break;
	case AST_BLOCK_STMT:
		_PrintAstList(indent + 1, node->u.block.statements);
		break;
	case AST_FN_DEF_STMT:
		_PrintAstNode(indent + 1, node->u.fn_def.symbol);
		_PrintAstList(indent + 1, node->u.fn_def.params);
		_PrintAstNode(indent + 1, node->u.fn_def.block);
		break;
	default:
		break;
	}
}

void PrintAst(AstNode* node)
{
	if (!node)
	{
		printf("Empty AST\n");
		return;
	}

	_PrintAstNode(0, node);
}

// -----------------------------------------------------

const char* GetValueTypeString(enum ValueType type)
{
	switch (type)
	{
	case VT_NUMBER: return "VT_NUMBER";
	case VT_STRING: return "VT_STRING";
	case VT_BOOL:	return "VT_BOOL";
	case VT_NONE:	return "VT_NONE";
	default:		return "UNKNOWN";
	}
}

void PrintValue(Value* value)
{
	printf("[%s]=", GetValueTypeString(value->type));
	switch (value->type)
	{
	case VT_NUMBER: printf("%lf\n", value->as._number); break;
	case VT_STRING: printf("%s\n", value->as._string.data); break;
	case VT_BOOL:	printf("%s\n", (value->as._bool) ? "TRUE" : "FALSE"); break;
	case VT_NONE:	printf("<NONE>\n"); break;
	default:		printf("<UNKNOWN>\n"); break;
	}
}