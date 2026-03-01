#pragma once

#include <stddef.h>

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
		 // TODO: Create Generic List in C
		struct
		{
			int _;
		} ast_program;
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