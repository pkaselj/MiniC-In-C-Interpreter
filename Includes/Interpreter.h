#pragma once

#include <Utils.h>

#include <stdbool.h>

// Forward declarations
typedef struct AstNode AstNode;

// Internal types forward declarations
typedef struct ProgramState ProgramState;

// Public API
void interpreter_perform(ProgramState* state, AstNode* program);