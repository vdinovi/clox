#ifndef clox_compiler_h
#define clox_compiler_h

#include "allocator.h"

typedef enum CompileResult {
    COMPILE_OK,
    COMPILE_SCAN_ERROR,
    COMPILE_PARSE_ERROR,
} CompileResult;

CompileResult compile(Allocator *alloc, const char* source);

#endif