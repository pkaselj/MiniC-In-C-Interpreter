#pragma once

#include <Lexer.h>
#include <Interpreter.h>
#include <Value.h>

Value operator_impl_unary(LexTokenType op, Value value);
Value operator_impl_binary(LexTokenType op, Value left, Value right);