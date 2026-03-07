#include <Value.h>

#include <stdlib.h>

Value value_create_empty()
{
	return (Value) { .type = VT_NONE };
}

Value value_create_bool(bool value)
{
	return (Value) { .type = VT_BOOL, .as._bool = value };
}

Value value_create_number(double value)
{
	return (Value) { .type = VT_NUMBER, .as._number = value };
}

Value value_create_string_nonowned(StringView value)
{
	return (Value) { .type = VT_STRING, .as._string = value };
}

bool value_cast_bool(Value value)
{
	if (value.type != VT_BOOL && value.type != VT_NUMBER)
	{
		LogError("cast_bool() - Expected value of type VT_BOOL or VT_NUMBER, instead got [%d / %s]",
			value.type,
			GetValueTypeString(value.type)
		);
		exit(-1);
	}

	return ((value.type == VT_BOOL) && (value.as._bool == true))
		|| ((value.type == VT_NUMBER) && (value.as._number != 0.0));
}
