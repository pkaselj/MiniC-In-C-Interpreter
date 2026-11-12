#pragma once

#include <Parser.h>

typedef unsigned long long(*fn_compiled_entry)(void);

size_t JIT_compile(AstNode* tree, fn_compiled_entry* pp_entry_point);
