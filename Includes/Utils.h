#pragma once

#include <stddef.h>
#include <stdbool.h>

// -------------- String Manipulation

struct StringView
{
	const char* data;
	size_t size;
};

// Non-owning string
typedef struct StringView StringView;
// Owning string (same type as non-owning, just an 
// annotation to the user that data should be freed)
typedef struct StringView String;
// TODO: sv_free(String sv);

StringView sv_create(const char* data);
StringView sv_create_s(const char* data, size_t length);
StringView sv_substring(const StringView sv, size_t index_start, size_t length);
StringView sv_substring_to_end(const StringView sv, size_t index_start);
bool sv_begins_with(const StringView sv, const StringView match);
bool sv_equal(const StringView sv, const StringView match);
bool sv_is_empty(const StringView sv);
StringView sv_create_empty();

// -------------- Error Logging

void LogError(const char* format, ...);
void LogInfo(const char* format, ...);
void LogDebug(const char* format, ...);

// -------------- Print Utils
typedef struct List List;
typedef struct AstNode AstNode;
typedef struct Value Value;

void PrintTokens(List* list);
void PrintAst(AstNode* tree);
void PrintValue(Value* value);

typedef enum TokenType TokenType;
typedef enum AstNodeType AstNodeType;
typedef enum ValueType ValueType;

const char* GetTokenTypeString(TokenType type);
const char* GetAstNodeTypeString(AstNodeType type);
const char* GetValueTypeString(ValueType type);