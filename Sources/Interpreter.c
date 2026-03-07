#include <Interpreter.h>
#include <Parser.h>
#include <Operators.h>
#include <Value.h>

#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>

// ---- Program state

//typedef struct CallStack


struct ProgramState
{
	int _;
};


// ---- Private interpreter functions

// Utils


// Forward declarations
static Value _interpret_expression(ProgramState* state, AstNode* stmt);
static Value _interpret_if_stmt(ProgramState* state, AstNode* stmt);
static Value _interpret_while_stmt(ProgramState* state, AstNode* stmt);
static Value _interpret_for_stmt(ProgramState* state, AstNode* stmt);
static Value _interpret_block(ProgramState* state, AstNode* stmt);
static Value _interpret_statement(ProgramState* state, AstNode* stmt);
static Value _interpret_fn_def(ProgramState* state, AstNode* fn_def);
static Value _interpret_program(ProgramState* state, AstNode* program);
static Value _interpret_unary_expr(ProgramState* state, AstNode* stmt);
static Value _interpret_binary_expr(ProgramState* state, AstNode* stmt);
static Value _interpret_string(ProgramState* state, AstNode* stmt);
static Value _interpret_number(ProgramState* state, AstNode* stmt);
static Value _interpret_identifier(ProgramState* state, AstNode* stmt);
static Value _interpret_assign(ProgramState* state, AstNode* stmt);
static Value _interpret_fn_call(ProgramState* state, AstNode* stmt);


// Definitions

static Value _interpret_unary_expr(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_UNARY_EXPR);

	Value value = _interpret_expression(state, stmt->u.unary_expr.child);
	return operator_impl_unary(stmt->u.unary_expr.op, value);
}

static Value _interpret_binary_expr(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_BINARY_EXPR);

	Value left = _interpret_expression(state, stmt->u.binary_expr.left);
	Value right = _interpret_expression(state, stmt->u.binary_expr.right);
	return operator_impl_binary(stmt->u.binary_expr.op, left, right);
}

static Value _interpret_string(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_STR_EXPR);

	Value value;
	value.type = VT_STRING;
	value.as._string = stmt->u.string.value;
	return value;
}

static Value _interpret_number(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_NUM_EXPR);

	Value value;
	value.type = VT_NUMBER;
	value.as._number = stmt->u.number.value;
	return value;
}

static Value _interpret_identifier(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_ID_EXPR);
}

static Value _interpret_assign(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_ASSIGN_EXPR);
}

static Value _interpret_fn_call(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_FN_CALL_EXPR);
}

static Value _interpret_expression(ProgramState* state, AstNode* stmt)
{
	assert(stmt);

	switch (stmt->type)
	{
	case AST_UNARY_EXPR:
		return _interpret_unary_expr(state, stmt);
	case AST_BINARY_EXPR:
		return _interpret_binary_expr(state, stmt);
	case AST_STR_EXPR:
		return _interpret_string(state, stmt);
	case AST_NUM_EXPR:
		return _interpret_number(state, stmt);
	case AST_ID_EXPR:
		return _interpret_identifier(state, stmt);
	case AST_ASSIGN_EXPR:
		return _interpret_assign(state, stmt);
	case AST_FN_CALL_EXPR:
		return _interpret_fn_call(state, stmt);
	default:
		break;
	}
}

static Value _interpret_if_stmt(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_IF_STMT);

	Value condition = _interpret_expression(state, stmt->u.if_stmt.condition);

	if (value_cast_bool(condition))
		return _interpret_statement(state, stmt->u.if_stmt.block_if);
	else
		return _interpret_statement(state, stmt->u.if_stmt.block_else);
}

static Value _interpret_while_stmt(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_WHILE_STMT);

	Value last_value = value_create_empty();
	do
	{
		if (stmt->u.while_stmt.condition)
		{
			Value cond_eval = _interpret_expression(state, stmt->u.while_stmt.condition);
			if (value_cast_bool(cond_eval))
			{
				break;
			}
		}

		last_value = _interpret_statement(state, stmt->u.while_stmt.block);
	} while (true);

	return last_value;
}

static Value _interpret_for_stmt(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_FOR_STMT);


	if (stmt->u.for_stmt.initial)
	{
		_interpret_expression(state, stmt->u.for_stmt.initial);
	}

	Value last_value = value_create_empty();
	do
	{
		if (stmt->u.for_stmt.end_condition)
		{
			Value end_cond_eval = _interpret_expression(state, stmt->u.for_stmt.end_condition);
			if (value_cast_bool(end_cond_eval))
			{
				break;
			}
		}

		last_value = _interpret_statement(state, stmt->u.for_stmt.block);

		if (stmt->u.for_stmt.next_action)
		{
			_interpret_expression(state, stmt->u.for_stmt.next_action);
		}
	} while (true);

	return last_value;
}

static Value _interpret_block(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	assert(stmt->type == AST_BLOCK_STMT);

	ListConstIterator* iter = list_create_iterator(stmt->u.block.statements);
	ListNode* node = NULL;
	Value last_value = value_create_empty();

	while (node = list_iterator_advance(iter))
	{
		last_value = _interpret_statement(state, list_node_data_get(node));
	}

	return last_value;
}

static Value _interpret_statement(ProgramState* state, AstNode* stmt)
{
	assert(stmt);
	//assert(stmt->type == ??);

	switch (stmt->type)
	{
	//case AST_EXPR_STMT:
	//	return _interpret_expression(state, stmt);
	case AST_IF_STMT:
		return _interpret_if_stmt(state, stmt);
	case AST_WHILE_STMT:
		return _interpret_while_stmt(state, stmt);
	case AST_FOR_STMT:
		return _interpret_for_stmt(state, stmt);
	case AST_BLOCK_STMT:
		return _interpret_block(state, stmt);
	default:
		return _interpret_expression(state, stmt);
		//LogError("_interpret_statement() - invalid statement node [%d / %s]\n", stmt->type, GetAstNodeTypeString(stmt->type));
		//exit(-1);
		break;
	}
}

static Value _interpret_fn_def(ProgramState* state, AstNode* fn_def)
{
	assert(fn_def);
	assert(fn_def->type == AST_FN_DEF_STMT);


}

static Value _interpret_program(ProgramState* state, AstNode* program)
{
	assert(program);
	assert(program->type == AST_S);

	if (program->u.program.function_definitions)
	{
		ListConstIterator* iter = list_create_iterator(program->u.program.function_definitions);
		ListNode* node = NULL;
		while (node = list_iterator_advance(iter))
		{
			_interpret_fn_def(state, list_node_data_get(node));
		}
	}

	Value last_value;
	if (program->u.program.statements)
	{
		ListConstIterator* iter = list_create_iterator(program->u.program.statements);
		ListNode* node = NULL;
		while (node = list_iterator_advance(iter))
		{
			last_value = _interpret_statement(state, list_node_data_get(node));
		}
	}

	return last_value;
}

// ---- Public API
void interpreter_perform(ProgramState* state, AstNode* program)
{
	UNUSED(state); // TODO
	Value value = _interpret_program(state, program);
	PrintValue(&value);
}