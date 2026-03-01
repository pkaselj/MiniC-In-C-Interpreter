#pragma once

#include <stddef.h>
#include "List.h"
#include "Lexer.h"

// --- Forward declarations
typedef struct TokenList TokenList;

// --- Types

typedef enum
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
			TokenType Op;
		} unary_expr;

		struct
		{
			struct AstNode* right;
			struct AstNode* left;
			TokenType op;
		} binary_expr;

		struct
		{
			StringView value;
		} identifier;

		struct
		{
			float value;
		} number;

		struct
		{
			StringView value;
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
			struct AstNode* intial;
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
			struct AstNode* block;
			struct AstNode* params;
			struct AstNode* symbol;
		} fn_def;

		struct
		{
			List* function_definitions;
			List* statements;
		} program;

	} u;
};

typedef struct AstNode AstNode;

struct AstTree
{
	AstNode* root;
};

typedef struct AstTree AstTree;

// ---- Public functions

AstTree* parser_perform(TokenList* tokens) {
	return NULL;
}