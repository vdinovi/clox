#ifndef clox_helpers_h
#define clox_helpers_h

#include "allocator.h"
#include "array.h"
#include "scanner.h"

String* escape_string(Allocator *alloc, const char *source);
void assert_token_equal(Allocator *alloc, const char *source, Token *expected, Token *actual);

#endif