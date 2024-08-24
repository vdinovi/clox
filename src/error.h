#ifndef clox_error_h
#define clox_error_h

#include "allocator.h"
#include "array.h"

typedef struct Error {
    int code;
    String *reason;
} Error;

Error *create_error(Allocator *alloc, int code, const char *reason);
String *error_repr(Error *error, Allocator *alloc);

#endif