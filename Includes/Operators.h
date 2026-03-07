#pragma once

#include <Lexer.h>
#include <Interpreter.h>
#include <Value.h>

Value operator_impl_unary(TokenType op, Value value);
Value operator_impl_binary(TokenType op, Value left, Value right);