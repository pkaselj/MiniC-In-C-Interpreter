#pragma once

#include <Operators.h>
#include <Value.h>

#include <stdlib.h>


// ----------- Public api
Value operator_impl_unary(TokenType op, Value value)
{
	if (value.type == VT_NUMBER)
	{
		if (op == TT_OP_ADD)
		{
			return value;
		}
		else if (op == TT_OP_SUB)
		{
			value.as._number = -1 * value.as._number;
			return value;
		}
	}
	else if (value.type == VT_BOOL)
	{
		if (op == TT_OP_NOT)
		{
			value.as._bool = !value.as._bool;
			return value;
		}
	}
	

}

Value operator_impl_binary(TokenType op, Value left, Value right)
{

	if (left.type == VT_NUMBER && right.type == VT_NUMBER)
	{
		switch (op)
		{
		case TT_OP_ADD: return value_create_number(left.as._number + right.as._number);
		case TT_OP_SUB: return value_create_number(left.as._number - right.as._number);
		case TT_OP_MUL: return value_create_number(left.as._number * right.as._number);
		case TT_OP_DIV: return value_create_number(left.as._number / right.as._number);
		case TT_OP_EQ:	return value_create_bool(left.as._number == right.as._number);
		case TT_OP_NEQ: return value_create_bool(left.as._number != right.as._number);
		case TT_OP_GT:	return value_create_bool(left.as._number >  right.as._number);
		case TT_OP_GTE: return value_create_bool(left.as._number >= right.as._number);
		case TT_OP_LT:	return value_create_bool(left.as._number <  right.as._number);
		case TT_OP_LTE: return value_create_bool(left.as._number <= right.as._number);
		case TT_OP_AND: return value_create_bool(left.as._number && right.as._number);
		case TT_OP_OR:	return value_create_bool(left.as._number || right.as._number);
		}
	}
	else if (left.type == VT_BOOL && right.type == VT_BOOL)
	{
		switch (op)
		{
		case TT_OP_EQ:	return value_create_bool(left.as._bool == right.as._bool);
		case TT_OP_NEQ: return value_create_bool(left.as._bool != right.as._bool);
		case TT_OP_GT:	return value_create_bool(left.as._bool >  right.as._bool);
		case TT_OP_GTE: return value_create_bool(left.as._bool >= right.as._bool);
		case TT_OP_LT:	return value_create_bool(left.as._bool <  right.as._bool);
		case TT_OP_LTE: return value_create_bool(left.as._bool <= right.as._bool);
		case TT_OP_AND: return value_create_bool(left.as._bool && right.as._bool);
		case TT_OP_OR:	return value_create_bool(left.as._bool || right.as._bool);
		}
	}

	LogError("operator_impl_unary() - No implementation for operator [%d / %s] and left [%d / %s], right [%d / %s]\n",
		op,
		GetTokenTypeString(op),
		left.type,
		GetValueTypeString(left.type),
		right.type,
		GetValueTypeString(right.type)
	);
	exit(-1);
}