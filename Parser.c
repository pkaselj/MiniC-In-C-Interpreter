#include "Parser.h"

#include <assert.h>
#include <stdlib.h>

// ---- token list manipulation

NONOWNING Token* _parser_peek(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_get(iter));
}

Token* _parser_advance(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_advance(iter));
}

Token* _parser_match(ListConstIterator* iter, TokenType required)
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

Token* _parser_match_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
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

Token* _parser_expect(ListConstIterator* iter, TokenType required)
{
	// TODO: track index and handle errors better.
	Token* _token = _parser_match(iter, required);
	if (!_token)
	{
		_token = _parser_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [%d], got [%d]!\n", index, _token->type, required);
		exit(-1);
	}
}

Token* _parser_expect_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
{
	// TODO: track index and handle errors better.
	Token* _token = _parser_match_any(iter, required_array, items);
	if (!_token)
	{
		_token = _parser_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [??], got [%d]!\n", index, _token->type); // TODO expected
		exit(-1);
	}
}


// ---- private AST functions / CREATE

AstNode* _ast_create_empty()
{
	AstNode* node = (AstNode*)malloc(sizeof(AstNode));
	assert(node);

	memset(node, 0, sizeof(AstNode));
	return node;
}

AstNode* ast_create_fn_call(TRANSFER AstNode* symbol, TRANSFER List* args)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_FN_CALL_EXPR;
	node->u.fn_call.symbol = symbol;
	node->u.fn_call.args = args;
	return node;
}

AstNode* ast_create_assign(TRANSFER AstNode* left, TRANSFER AstNode* right)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_ASSIGN_EXPR;
	node->u.assign.left = left;
	node->u.assign.right = right;
	return node;
}

AstNode* ast_create_unary_expr(TRANSFER AstNode* child, TokenType op)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_UNARY_EXPR;
	node->u.unary_expr.child = child;
	node->u.unary_expr.op = op;
	return node;
}

AstNode* ast_create_binary_expr(TRANSFER AstNode* left, TRANSFER AstNode* right, TokenType op)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_BINARY_EXPR;
	node->u.binary_expr.left = left;
	node->u.binary_expr.right = right;
	node->u.binary_expr.op = op;
	return node;
}

AstNode* ast_create_identifier(StringView value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_ID_EXPR;
	node->u.identifier.value =  sv_create_empty();
	node->u.identifier.value.size = value.size;
	node->u.identifier.value.data = (char*)malloc(sizeof(char) * (value.size + 1));
	memset(node->u.identifier.value.data, 0, value.size + 1);
	memcpy(node->u.identifier.value.data, value.data, value.size);
	return node;
}

AstNode* ast_create_number(double value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_NUM_EXPR;
	node->u.number.value = value;
	return node;
}

AstNode* ast_create_string(StringView value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_STR_EXPR;
	node->u.string.value = sv_create_empty();
	node->u.string.value.size = value.size;
	node->u.string.value.data = (char*)malloc(sizeof(char) * (value.size + 1));
	memset(node->u.string.value.data, 0, value.size + 1);
	memcpy(node->u.string.value.data, value.data, value.size);
	return node;
}

AstNode* ast_create_expr_stmt(TRANSFER AstNode* expression)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_EXPR_STMT;
	node->u.expr_stmt.expression = expression;
	return node;
}

AstNode* ast_create_if_stmt(
	TRANSFER AstNode* condition,
	TRANSFER AstNode* block_if,
	TRANSFER AstNode* block_else)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_IF_STMT;
	node->u.if_stmt.condition = condition;
	node->u.if_stmt.block_if = block_if;
	node->u.if_stmt.block_else = block_else;
	return node;
}

AstNode* ast_create_for_stmt(
	TRANSFER AstNode* initial,
	TRANSFER AstNode* end_condition,
	TRANSFER AstNode* next_action,
	TRANSFER AstNode* block)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_FOR_STMT;
	node->u.for_stmt.initial = initial;
	node->u.for_stmt.end_condition = end_condition;
	node->u.for_stmt.next_action = next_action;
	node->u.for_stmt.block = block;
	return node;
}

AstNode* ast_create_while_stmt(TRANSFER AstNode* condition,	TRANSFER AstNode* block)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_WHILE_STMT;
	node->u.while_stmt.condition = condition;
	node->u.while_stmt.block = block;
	return node;
}

AstNode* ast_create_block(TRANSFER List* statements)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_BLOCK_STMT;
	node->u.block.statements = statements;
	return node;
}

AstNode* ast_create_fn_def(
	TRANSFER AstNode* symbol,
	TRANSFER AstNode* params,
	TRANSFER AstNode* block)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_FN_DEF_STMT;
	node->u.fn_def.block = block;
	node->u.fn_def.params = params;
	node->u.fn_def.symbol = symbol;
	return node;
}

AstNode* ast_create_program(TRANSFER List* fn_defs, TRANSFER List* statements)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_S;
	node->u.program.function_definitions = fn_defs;
	node->u.program.statements = statements;
	return node;
}

// ---- private AST functions / FREE

// Forward def
void ast_node_free(AstNode* node);

void _ast_fn_call_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_FN_CALL_EXPR);

	ast_node_free(node->u.fn_call.symbol);
	node->u.fn_call.symbol = NULL;

	list_free(node->u.fn_call.args);
	node->u.fn_call.args = NULL;

	free(node);
}

void _ast_assign_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_ASSIGN_EXPR);

	ast_node_free(node->u.assign.left);
	node->u.assign.left = NULL;

	ast_node_free(node->u.assign.right);
	node->u.assign.right = NULL;

	free(node);
}

void _ast_unary_expr_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_UNARY_EXPR);

	ast_node_free(node->u.unary_expr.child);
	node->u.unary_expr.child = NULL;

	free(node);
}

void _ast_binary_expr_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_BINARY_EXPR);

	ast_node_free(node->u.binary_expr.left);
	node->u.binary_expr.left = NULL;

	ast_node_free(node->u.binary_expr.right);
	node->u.binary_expr.right = NULL;

	free(node);
}


void _ast_identifier_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_ID_EXPR);

	assert(node->u.identifier.value.data);
	free(node->u.identifier.value.data);
	node->u.identifier.value.data = NULL;
	node->u.identifier.value.size = 0;

	free(node);
}

void _ast_string_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_STR_EXPR);

	assert(node->u.string.value.data);
	free(node->u.string.value.data);
	node->u.string.value.data = NULL;
	node->u.string.value.size = 0;

	free(node);
}

void _ast_expr_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_EXPR_STMT);

	ast_node_free(node->u.expr_stmt.expression);
	node->u.expr_stmt.expression = NULL;

	free(node);
}

void _ast_if_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_IF_STMT);

	ast_node_free(node->u.if_stmt.condition);
	node->u.if_stmt.condition = NULL;

	ast_node_free(node->u.if_stmt.block_if);
	node->u.if_stmt.block_if = NULL;

	ast_node_free(node->u.if_stmt.block_else);
	node->u.if_stmt.block_else = NULL;

	free(node);
}

void _ast_for_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_FOR_STMT);

	ast_node_free(node->u.for_stmt.initial);
	node->u.for_stmt.initial = NULL;

	ast_node_free(node->u.for_stmt.end_condition);
	node->u.for_stmt.end_condition = NULL;

	ast_node_free(node->u.for_stmt.next_action);
	node->u.for_stmt.next_action = NULL;

	ast_node_free(node->u.for_stmt.block);
	node->u.for_stmt.block = NULL;

	free(node);
}

void _ast_while_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_WHILE_STMT);

	ast_node_free(node->u.while_stmt.condition);
	node->u.while_stmt.condition = NULL;

	ast_node_free(node->u.while_stmt.block);
	node->u.while_stmt.block = NULL;

	free(node);
}

void _ast_block_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_BLOCK_STMT);

	list_free(node->u.block.statements);
	node->u.block.statements = NULL;

	free(node);
}

void _ast_fn_definition_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_FN_DEF_STMT);

	ast_node_free(node->u.fn_def.symbol);
	node->u.fn_def.symbol = NULL;

	ast_node_free(node->u.fn_def.params);
	node->u.fn_def.params = NULL;

	ast_node_free(node->u.fn_def.block);
	node->u.fn_def.block = NULL;

	free(node);
}

void _ast_program_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_S);

	list_free(node->u.program.function_definitions);
	node->u.program.function_definitions = NULL;

	list_free(node->u.program.statements);
	node->u.program.statements = NULL;

	free(node);
}

void ast_node_free(AstNode* node)
{
	if (!node)
		return;

	switch (node->type)
	{
	case AST_S:
		_ast_program_free(node);
		break;
	case AST_FN_CALL_EXPR:
		_ast_fn_call_free(node);
		break;
	case AST_ASSIGN_EXPR:
		_ast_assign_free(node);
		break;
	case AST_UNARY_EXPR:
		_ast_unary_expr_free(node);
		break;
	case AST_BINARY_EXPR:
		_ast_binary_expr_free(node);
		break;
	case AST_ID_EXPR:
		_ast_identifier_free(node);
		break;
	case AST_NUM_EXPR:
		free(node);
		break;
	case AST_STR_EXPR:
		_ast_string_free(node);
		break;
	case AST_EXPR_STMT:
		_ast_expr_stmt_free(node);
		break;
	case AST_IF_STMT:
		_ast_if_stmt_free(node);
		break;
	case AST_FOR_STMT:
		_ast_for_stmt_free(node);
		break;
	case AST_WHILE_STMT:
		_ast_while_stmt_free(node);
		break;
	case AST_BLOCK_STMT:
		_ast_block_free(node);
		break;
	case AST_FN_DEF_STMT:
		_ast_fn_definition_free(node);
		break;
	default:
		free(node);
		LogError("Parser: Could not find node destructor for type %d\n", node->type);
		break;
	}
}

// ---- private parser functions

AstNode* parse_fn_def(ListConstIterator* iter)
{
	_parser_expect(iter, TT_K_FN);
	Token* symbol = _parser_expect(iter, TT_ID);
	AstNode* sym_node = 
}

AstNode* parse_statement(ListConstIterator* iter)
{

}

AstNode* parse_program(ListConstIterator* iter)
{
	List* statements = list_create(); // List<AstNode*>
	List* fn_defs = list_create(); // List<AstNode*>

	Token* token;
	while (token = _parser_peek(iter) && token->type == TT_K_FN)
	{
		AstNode* node = parse_fn_definition(iter);

		assert(node);
		assert(node->type == AST_FN_DEF_STMT);

		list_push(fn_defs, list_create_node(node, ast_node_free));
	}
	while (token = _parser_peek(iter))
	{
		AstNode* node = parse_statement(iter);

		assert(node);

		list_push(fn_defs, list_create_node(node, ast_node_free));
	}

	return ast_create_program(fn_defs, statements);
}

// ---- public api

AstTree* parser_perform(List* tokens)
{

}