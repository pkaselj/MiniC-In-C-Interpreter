#pragma once

#include <Parser.h>

typedef int(*fn_compiled_main)(void);

fn_compiled_main JIT_compile(AstNode* tree);
