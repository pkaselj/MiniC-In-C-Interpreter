#pragma once

#include <stddef.h>
#include <List.h>
#include <Lexer.h>

// --- Types

typedef enum AstNodeType
{
	AST_S = 0,

	AST_FN_CALL_EXPR,
	AST_ASSIGN_EXPR,
	AST_UNARY_EXPR,
	AST_BINARY_EXPR,
	AST_ID_EXPR,
	AST_NUM_EXPR,
	AST_STR_EXPR,

	AST_EXPR_STMT,
	AST_IF_STMT,
	AST_FOR_STMT,
	AST_WHILE_STMT,
	AST_BLOCK_STMT,
	AST_FN_DEF_STMT,
} AstNodeType;

struct AstNode
{
	AstNodeType type;
	union
	{
		// List === List<AstNode>
		struct
		{
			struct AstNode* symbol;
			List* args;
		} fn_call;

		struct
		{
			struct AstNode* left;
			struct AstNode* right;
		} assign;

		struct
		{
			struct AstNode* child;
			TokenType op;
		} unary_expr;

		struct
		{
			struct AstNode* right;
			struct AstNode* left;
			TokenType op;
		} binary_expr;

		struct
		{
			String value;
		} identifier;

		struct
		{
			double value;
		} number;

		struct
		{
			String value;
		} string;

		struct
		{
			struct AstNode* expression;
		} expr_stmt;

		struct
		{
			struct AstNode* condition;
			struct AstNode* block_if;
			struct AstNode* block_else;
		} if_stmt;

		struct
		{
			struct AstNode* initial;
			struct AstNode* end_condition;
			struct AstNode* next_action;
			struct AstNode* block;
		} for_stmt;

		struct
		{
			struct AstNode* condition;
			struct AstNode* block;
		} while_stmt;

		struct
		{
			List* statements;
		} block;

		struct
		{
			struct AstNode* symbol;
			List* params;
			struct AstNode* block;
		} fn_def;

		struct
		{
			List* function_definitions;
			List* statements;
		} program;

	} u;
};

typedef struct AstNode AstNode;

// ---- Public functions

void ast_tree_free(AstNode* tree);
AstNode* parser_perform(List* tokens);