#include "Utils.h"

#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <assert.h>

StringView sv_create(const char* data)
{
	assert(data);

	return (StringView)
	{
		.data = data,
		.size = strlen(data)
	};
}

StringView sv_create_s(const char* data, size_t length)
{
	assert(data);
	assert(length > 0);

	return (StringView)
	{
		.data = data,
		.size = length
	};
}

StringView sv_substring(const StringView sv, size_t index_start, size_t length)
{
	if (index_start + length > sv.size)
	{
		return sv_create_empty();
	}

	StringView subsv = sv_create_empty();
	subsv.data = &sv.data[index_start];
	subsv.size = length;
	return subsv;
}

StringView sv_substring_to_end(const StringView sv, size_t index_start)
{
	if (!sv.data || sv.size <= 0)
	{
		return sv_create_empty();
	}

	StringView subsv = sv_create_empty();
	subsv.data = &sv.data[index_start];
	subsv.size = sv.size - index_start;
	return subsv;
}

bool sv_is_empty(const StringView sv)
{
	return (!sv.data || sv.size <= 0);
}

StringView sv_create_empty()
{
	return (StringView)
	{
		.data = NULL,
		.size = 0
	};
}

bool sv_begins_with(const StringView sv, const StringView match)
{
	if (   !sv.data
		||  sv.size <= 0
		|| !match.data
		||  match.size	<= 0
		)
	{
		return false;
	}

	if (sv.size < match.size)
	{
		return false;
	}

	return (0 == strncmp(sv.data, match.data, match.size));
}

bool sv_equal(const StringView sv, const StringView match)
{
	return (sv.size == match.size) && sv_begins_with(sv, match);
}

// -----------------------

void LogError(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[ERR] :: ");
	vprintf(format, args);
	va_end(args);
}

void LogInfo(const char* format, ...)
{
	va_list args;
	va_start(args, format);
	printf("[INF] :: ");
	vprintf(format, args);
	va_end(args);
}

void LogDebug(const char* format, ...)
{
#ifdef _DEBUG
	va_list args;
	va_start(args, format);
	printf("[INF] :: ");
	vprintf(format, args);
	va_end(args);
#else
	(void)format;
#endif;
}