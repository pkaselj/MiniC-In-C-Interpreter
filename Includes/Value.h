#pragma once

#include <Utils.h>
#include <stdbool.h>

typedef enum
{
	VT_NUMBER,
	VT_STRING,
	VT_BOOL,
	VT_NONE
} ValueType;

typedef struct Value Value;
struct Value
{
	ValueType type;
	union
	{
		double _number;
		StringView _string;
		bool _bool;
	} as;
};

// Public API
Value value_create_empty();
Value value_create_bool(bool value);
Value value_create_number(double value);
Value value_create_string_nonowned(StringView value);
//Value value_create_string(String value);

bool value_cast_bool(Value value);