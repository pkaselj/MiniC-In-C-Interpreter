#pragma once

#include <Parser.h>

typedef unsigned long long(*fn_compiled_entry)(void);

fn_compiled_entry JIT_compile(AstNode* tree);
