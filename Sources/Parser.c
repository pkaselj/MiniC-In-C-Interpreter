#include "Parser.h"

#include <assert.h>
#include <stdlib.h>
#include <string.h>

// ---- token list manipulation

static NONOWNING Token* _parser_peek(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_get(iter));
}

static Token* _parser_advance(ListConstIterator* iter)
{
	assert(iter);

	return list_node_data_get(list_iterator_advance(iter));
}

static Token* _parser_match(ListConstIterator* iter, TokenType required)
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

static Token* _parser_match_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
{
	assert(iter);
	assert(required_array);

	ListNode* current = list_iterator_get(iter);
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

static Token* _parser_expect(ListConstIterator* iter, TokenType required)
{
	// TODO: track index and handle errors better.
	Token* _token = _parser_match(iter, required);
	if (!_token)
	{
		_token = _parser_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [%d / %s], got [%d / %s]!\n", 
			index, 
			_token->type,
			GetTokenTypeString(_token->type),
			required,
			GetTokenTypeString(required)
		);
		exit(-1);
	}
	return _token;
}

static Token* _parser_expect_any(ListConstIterator* iter, const TokenType* required_array, size_t items)
{
	// TODO: track index and handle errors better.
	Token* _token = _parser_match_any(iter, required_array, items);
	if (!_token)
	{
		_token = _parser_peek(iter);
		size_t index = list_interator_index(iter);
		LogError("Parser: Token at index [%llu] - expected token of type [??], got [%d / %s]!\n",
			index,
			_token->type,
			GetTokenTypeString(_token->type)
		); // TODO expected
		exit(-1);
	}
	return _token;
}


// ---- private AST functions / CREATE

static AstNode* _ast_create_empty()
{
	AstNode* node = (AstNode*)malloc(sizeof(AstNode));
	assert(node);

	memset(node, 0, sizeof(AstNode));
	return node;
}

static AstNode* ast_create_fn_call(TRANSFER AstNode* symbol, TRANSFER List* args)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_FN_CALL_EXPR;
	node->u.fn_call.symbol = symbol;
	node->u.fn_call.args = args;
	return node;
}

static AstNode* ast_create_assign(TRANSFER AstNode* left, TRANSFER AstNode* right)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_ASSIGN_EXPR;
	node->u.assign.left = left;
	node->u.assign.right = right;
	return node;
}

static AstNode* ast_create_unary_expr(TRANSFER AstNode* child, TokenType op)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_UNARY_EXPR;
	node->u.unary_expr.child = child;
	node->u.unary_expr.op = op;
	return node;
}

static AstNode* ast_create_binary_expr(TRANSFER AstNode* left, TRANSFER AstNode* right, TokenType op)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_BINARY_EXPR;
	node->u.binary_expr.left = left;
	node->u.binary_expr.right = right;
	node->u.binary_expr.op = op;
	return node;
}

static AstNode* ast_create_identifier(StringView value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_ID_EXPR;
	node->u.identifier.value =  sv_create_empty();
	node->u.identifier.value.size = value.size;
	node->u.identifier.value.data = (char*)malloc(sizeof(char) * (value.size + 1));
	assert(node->u.identifier.value.data);
	memset((void*)node->u.identifier.value.data, 0, value.size + 1);
	memcpy((void*)node->u.identifier.value.data, value.data, value.size);
	return node;
}

static AstNode* ast_create_number(double value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_NUM_EXPR;
	node->u.number.value = value;
	return node;
}

static AstNode* ast_create_string(StringView value)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_STR_EXPR;
	node->u.string.value = sv_create_empty();
	node->u.string.value.size = value.size;
	node->u.string.value.data = (char*)malloc(sizeof(char) * (value.size + 1));
	assert(node->u.string.value.data);
	memset((void*)node->u.string.value.data, 0, value.size + 1);
	memcpy((void*)node->u.string.value.data, value.data, value.size);
	return node;
}

static AstNode* ast_create_expr_stmt(TRANSFER AstNode* expression)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_EXPR_STMT;
	node->u.expr_stmt.expression = expression;
	return node;
}

static AstNode* ast_create_if_stmt(
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

static AstNode* ast_create_for_stmt(
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

static AstNode* ast_create_while_stmt(TRANSFER AstNode* condition,	TRANSFER AstNode* block)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_WHILE_STMT;
	node->u.while_stmt.condition = condition;
	node->u.while_stmt.block = block;
	return node;
}

static AstNode* ast_create_block(TRANSFER List* statements)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_BLOCK_STMT;
	node->u.block.statements = statements;
	return node;
}

static AstNode* ast_create_fn_def(
	TRANSFER AstNode* symbol,
	TRANSFER List* params,
	TRANSFER AstNode* block)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_FN_DEF_STMT;
	node->u.fn_def.block = block;
	node->u.fn_def.params = params;
	node->u.fn_def.symbol = symbol;
	return node;
}

static AstNode* ast_create_program(TRANSFER List* fn_defs, TRANSFER List* statements)
{
	AstNode* node = _ast_create_empty();
	node->type = AST_S;
	node->u.program.function_definitions = fn_defs;
	node->u.program.statements = statements;
	return node;
}

// ---- private AST functions / FREE

// Forward def
static void ast_node_free(AstNode* node);

static void _ast_fn_call_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_FN_CALL_EXPR);

	ast_node_free(node->u.fn_call.symbol);
	node->u.fn_call.symbol = NULL;

	list_free(node->u.fn_call.args);
	node->u.fn_call.args = NULL;

	free(node);
}

static void _ast_assign_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_ASSIGN_EXPR);

	ast_node_free(node->u.assign.left);
	node->u.assign.left = NULL;

	ast_node_free(node->u.assign.right);
	node->u.assign.right = NULL;

	free(node);
}

static void _ast_unary_expr_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_UNARY_EXPR);

	ast_node_free(node->u.unary_expr.child);
	node->u.unary_expr.child = NULL;

	free(node);
}

static void _ast_binary_expr_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_BINARY_EXPR);

	ast_node_free(node->u.binary_expr.left);
	node->u.binary_expr.left = NULL;

	ast_node_free(node->u.binary_expr.right);
	node->u.binary_expr.right = NULL;

	free(node);
}


static void _ast_identifier_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_ID_EXPR);

	assert(node->u.identifier.value.data);
	free((void*)node->u.identifier.value.data);
	node->u.identifier.value.data = NULL;
	node->u.identifier.value.size = 0;

	free(node);
}

static void _ast_string_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_STR_EXPR);

	assert(node->u.string.value.data);
	free((void*)node->u.string.value.data);
	node->u.string.value.data = NULL;
	node->u.string.value.size = 0;

	free(node);
}

static void _ast_expr_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_EXPR_STMT);

	ast_node_free(node->u.expr_stmt.expression);
	node->u.expr_stmt.expression = NULL;

	free(node);
}

static void _ast_if_stmt_free(AstNode* node)
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

static void _ast_for_stmt_free(AstNode* node)
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

static void _ast_while_stmt_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_WHILE_STMT);

	ast_node_free(node->u.while_stmt.condition);
	node->u.while_stmt.condition = NULL;

	ast_node_free(node->u.while_stmt.block);
	node->u.while_stmt.block = NULL;

	free(node);
}

static void _ast_block_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_BLOCK_STMT);

	list_free(node->u.block.statements);
	node->u.block.statements = NULL;

	free(node);
}

static void _ast_fn_definition_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_FN_DEF_STMT);

	ast_node_free(node->u.fn_def.symbol);
	node->u.fn_def.symbol = NULL;

	list_free(node->u.fn_def.params);
	node->u.fn_def.params = NULL;

	ast_node_free(node->u.fn_def.block);
	node->u.fn_def.block = NULL;

	free(node);
}

static void _ast_program_free(AstNode* node)
{
	assert(node);
	assert(node->type == AST_S);

	list_free(node->u.program.function_definitions);
	node->u.program.function_definitions = NULL;

	list_free(node->u.program.statements);
	node->u.program.statements = NULL;

	free(node);
}

static void ast_node_free(AstNode* node)
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

// Declarations

static AstNode* parse_primary(ListConstIterator* iter);
static AstNode* parse_fn_call(ListConstIterator* iter);
static AstNode* parse_unary(ListConstIterator* iter);
static AstNode* parse_term(ListConstIterator* iter);
static AstNode* parse_additive(ListConstIterator* iter);
static AstNode* parse_comparee(ListConstIterator* iter);
static AstNode* parse_logical_AND(ListConstIterator* iter);
static AstNode* parse_logical_OR(ListConstIterator* iter);
static AstNode* parse_assignee(ListConstIterator* iter);
static AstNode* parse_expression(ListConstIterator* iter);
static AstNode* parse_block(ListConstIterator* iter);
static AstNode* parse_for_stmt(ListConstIterator* iter);
static AstNode* parse_while_stmt(ListConstIterator* iter);
static AstNode* parse_if_stmt(ListConstIterator* iter);
static AstNode* parse_statement(ListConstIterator* iter);
static AstNode* parse_fn_def(ListConstIterator* iter);
static AstNode* parse_program(ListConstIterator* iter);

// Definitions

static AstNode* parse_primary(ListConstIterator* iter)
{
	Token* token;

	if (token = _parser_match(iter, TT_ID))
	{
		return ast_create_identifier(token->value.as_string);
	}
	else if (token = _parser_match(iter, TT_NUMBER))
	{
		return ast_create_number(token->value.as_number);
	}
	else if (token = _parser_match(iter, TT_STRING))
	{
		return ast_create_string(token->value.as_string);
	}

	_parser_expect(iter, TT_O_PAREN);
	AstNode* expr = parse_expression(iter);
	_parser_expect(iter, TT_C_PAREN);

	return expr;
}

static AstNode* parse_fn_call(ListConstIterator* iter)
{
	AstNode* node = parse_primary(iter);
	Token* nt = NULL;

	while ((nt = _parser_peek(iter)) && nt->type == TT_O_PAREN)
	{
		_parser_expect(iter, TT_O_PAREN);
		List* params = list_create();
		Token* t;
		
		while ((t = _parser_peek(iter)) && t->type != TT_C_PAREN)
		{
			AstNode* param = parse_expression(iter);
			list_push(params, list_create_node(param, ast_node_free));
			_parser_match(iter, TT_K_COMMA);
		}

		_parser_expect(iter, TT_C_PAREN);
		node = ast_create_fn_call(node, params);
	}

	return node;
}

static AstNode* parse_unary(ListConstIterator* iter)
{
	Token* token;
	const TokenType required[] = { TT_OP_ADD, TT_OP_SUB, TT_OP_NOT };

	if (token = _parser_match_any(iter, required, sizeof(required) / sizeof(required[0])))
	{
		AstNode* base = parse_unary(iter);
		return ast_create_unary_expr(base, token->type);
	}

	return parse_fn_call(iter);
}

static AstNode* parse_term(ListConstIterator* iter)
{
	AstNode* node = parse_unary(iter);
	Token* token;
	const TokenType required[] = { TT_OP_MUL, TT_OP_DIV };

	while (token = _parser_match_any(iter, required, sizeof(required) / sizeof(required[0])))
	{
		AstNode* right = parse_unary(iter);
		node = ast_create_binary_expr(node, right, token->type);
	}

	return node;
}

static AstNode* parse_additive(ListConstIterator* iter)
{
	AstNode* node = parse_term(iter);
	Token* token;
	const TokenType required[] = { TT_OP_ADD, TT_OP_SUB };

	while (token = _parser_match_any(iter, required, sizeof(required) / sizeof(required[0])))
	{
		AstNode* right = parse_term(iter);
		node = ast_create_binary_expr(node, right, token->type);
	}

	return node;
}

static AstNode* parse_comparee(ListConstIterator* iter)
{
	AstNode* node = parse_additive(iter);
	Token* token;
	const TokenType required[] = {
		TT_OP_EQ,
		TT_OP_NEQ,
		TT_OP_GT,
		TT_OP_GTE,
		TT_OP_LT,
		TT_OP_LTE
	};

	while (token = _parser_match_any(iter, required, sizeof(required) / sizeof(required[0])))
	{
		AstNode* right = parse_additive(iter);
		node = ast_create_binary_expr(node, right, token->type);
	}

	return node;
}

static AstNode* parse_logical_AND(ListConstIterator* iter)
{
	AstNode* node = parse_comparee(iter);
	Token* token;

	while ((token = _parser_peek(iter)) && token->type == TT_OP_AND)
	{
		_parser_expect(iter, TT_OP_AND);
		AstNode* right = parse_comparee(iter);
		node = ast_create_binary_expr(node, right, TT_OP_AND);
	}

	return node;
}

static AstNode* parse_logical_OR(ListConstIterator* iter)
{
	AstNode* node = parse_logical_AND(iter);
	Token* token;

	while ((token = _parser_peek(iter)) && token->type == TT_OP_OR)
	{
		_parser_expect(iter, TT_OP_OR);
		AstNode* right = parse_logical_AND(iter);
		node = ast_create_binary_expr(node, right, TT_OP_OR);
	}

	return node;
}

static AstNode* parse_assignee(ListConstIterator* iter)
{
	return parse_logical_OR(iter);
}

static AstNode* parse_expression(ListConstIterator* iter)
{
	AstNode* node = parse_assignee(iter);

	if (_parser_match(iter, TT_ASSIGN))
	{
		// TODO Check assignability
		AstNode* right = parse_expression(iter);
		node = ast_create_assign(node, right);
	}

	return node;
}

static AstNode* parse_block(ListConstIterator* iter)
{
	List* statements = list_create();

	_parser_expect(iter, TT_O_BRACE);
	while (!_parser_match(iter, TT_C_BRACE))
	{
		AstNode* stmt = parse_statement(iter);
		list_push(statements, list_create_node(stmt, ast_node_free));
	}

	return ast_create_block(statements);
}

static AstNode* parse_for_stmt(ListConstIterator* iter)
{
	AstNode* initial = NULL;
	AstNode* end_cond = NULL;
	AstNode* next_action = NULL;

	Token* token;

	_parser_expect(iter, TT_K_FOR);
	_parser_expect(iter, TT_O_PAREN);
	if ((token = _parser_peek(iter)) && token->type != TT_DELIM)
	{
		initial = parse_expression(iter);
	}
	_parser_expect(iter, TT_DELIM);
	if ((token = _parser_peek(iter)) && token->type != TT_DELIM)
	{
		end_cond = parse_expression(iter);
	}
	_parser_expect(iter, TT_DELIM);
	if ((token = _parser_peek(iter)) && token->type != TT_C_PAREN)
	{
		next_action = parse_expression(iter);
	}
	_parser_expect(iter, TT_C_PAREN);

	AstNode* block = parse_block(iter);

	return ast_create_for_stmt(initial, end_cond, next_action, block);
}

static AstNode* parse_while_stmt(ListConstIterator* iter)
{
	_parser_expect(iter, TT_K_WHILE);
	_parser_expect(iter, TT_O_PAREN);
	AstNode* cond = parse_expression(iter);
	_parser_expect(iter, TT_C_PAREN);
	AstNode* block = parse_block(iter);

	return ast_create_while_stmt(cond, block);
}

static AstNode* parse_if_stmt(ListConstIterator* iter)
{
	_parser_expect(iter, TT_K_IF);
	_parser_expect(iter, TT_O_PAREN);
	AstNode* cond = parse_expression(iter);
	_parser_expect(iter, TT_C_PAREN);
	AstNode* block_if = parse_block(iter);
	AstNode* block_else = NULL;
	if (_parser_match(iter, TT_K_ELSE))
	{
		block_else = parse_block(iter);
	}

	return ast_create_if_stmt(cond, block_if, block_else);
}

static AstNode* parse_statement(ListConstIterator* iter)
{
	Token* token = _parser_peek(iter);
	if (!token)
	{
		LogError("Parser: No tokens to parse in parse_statement()\n"); // TODO: Log position
		exit(-1);
	}
	else if (token->type == TT_K_IF)
		return parse_if_stmt(iter);
	else if (token->type == TT_K_WHILE)
		return parse_while_stmt(iter);
	else if (token->type == TT_K_FOR)
		return parse_for_stmt(iter);

	AstNode* expr = parse_expression(iter);
	_parser_expect(iter, TT_DELIM);
	return expr;
}

static AstNode* parse_fn_def(ListConstIterator* iter)
{
	_parser_expect(iter, TT_K_FN);
	Token* symbol = _parser_expect(iter, TT_ID);
	AstNode* sym_node = ast_create_identifier(symbol->value.as_string);
	_parser_expect(iter, TT_O_PAREN);
	List* params = list_create();
	Token* token;
	while ((token = _parser_peek(iter)) && token->type != TT_C_PAREN)
	{
		Token* param = _parser_expect(iter, TT_ID);
		AstNode* param_node = ast_create_identifier(param->value.as_string);
		list_push(params, list_create_node(param_node, ast_node_free));
		_parser_match(iter, TT_K_COMMA);
	}
	_parser_expect(iter, TT_C_PAREN);
	AstNode* block = parse_block(iter);
	return ast_create_fn_def(sym_node, params, block);
}

static AstNode* parse_program(ListConstIterator* iter)
{
	List* statements = list_create(); // List<AstNode*>
	List* fn_defs = list_create(); // List<AstNode*>

	Token* token;
	while ((token = _parser_peek(iter)) && token->type == TT_K_FN)
	{
		AstNode* node = parse_fn_def(iter);

		assert(node);
		assert(node->type == AST_FN_DEF_STMT);

		list_push(fn_defs, list_create_node(node, ast_node_free));
	}
	while (token = _parser_peek(iter))
	{
		AstNode* node = parse_statement(iter);

		assert(node);

		list_push(statements, list_create_node(node, ast_node_free));
	}

	return ast_create_program(fn_defs, statements);
}

// ---- public api

AstNode* parser_perform(List* tokens)
{
	ListConstIterator* iter = list_create_iterator(tokens);
	return parse_program(iter);
}