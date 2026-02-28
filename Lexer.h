#pragma once

#include <stddef.h>
#include "Common.h"
#include "Utils.h"

typedef enum TokenType
{
    TT_NUMBER = 0,
    TT_STRING,
    TT_OP_NOT, // !
    TT_OP_OR,  //  ||
    TT_OP_AND, //  &&
    TT_OP_ADD, //  +
    TT_OP_SUB, //  -
    TT_OP_MUL, //  *
    TT_OP_DIV, //  /
    TT_OP_EQ,  //  ==
    TT_OP_NEQ, //  !=
    TT_OP_GT,  //  >
    TT_OP_LT,  //  <
    TT_OP_GTE, //  >=
    TT_OP_LTE, //  <=
    TT_ID,
    TT_ASSIGN, //  =
    TT_WSPC,
    TT_K_FN,
    TT_K_RET,
    TT_K_IF,
    TT_K_FOR,
    TT_K_ELSE,
    TT_K_WHILE,
    TT_K_COMMA,    // ,
    TT_O_PAREN,    // (
    TT_C_PAREN,    // )
    //TT_O_BRACK,  // [
    //TT_C_BRACK,  // ]
    TT_O_BRACE,    // {
    TT_C_BRACE,    // }
    TT_DELIM,      // ;
} TokenType;

struct Token
{
    struct Token* next;
    struct Token* previous;

    TokenType type;

    union
    {
        StringView as_string;
    } value;

    long length; // Length of token in characters
};

typedef struct Token Token;

void tok_free(TRANSFER Token* token);

struct TokenList
{
    struct Token* first;
    struct Token* last;
};

typedef struct TokenList TokenList;

TokenList* tl_create(void);
void tl_free(TRANSFER TokenList* list);

// Pops the first token. NULL on empty
Token* tl_pop(TokenList* list);
// Pushed to the end.
void tl_push(TokenList* list, TRANSFER Token* token);

TokenList* lexer_perform(const StringView* input);
