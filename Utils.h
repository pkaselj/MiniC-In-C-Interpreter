#pragma once

#include <stddef.h>
#include <stdbool.h>

// -------------- String Manipulation

struct StringView
{
	const char* data;
	size_t size;
};

typedef struct StringView StringView;

StringView sv_create(const char* data);
StringView sv_create_s(const char* data, size_t length);
StringView sv_substring(const StringView* sv, size_t index_start, size_t length);
StringView sv_substring_to_end(const StringView* sv, size_t index_start);
bool sv_begins_with(const StringView* sv, const char* match);
bool sv_is_empty(const StringView* sv);
StringView sv_create_empty();

// -------------- Error Logging

void LogError(const char* format, ...);
void LogInfo(const char* format, ...);
void LogDebug(const char* format, ...);