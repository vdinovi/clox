#ifndef clox_error_h
#define clox_error_h

#include "allocator.h"
#include "array.h"

typedef struct Error {
    int code;
    String *reason;
    struct Error *from;
} Error;

Error *create_error(Allocator *alloc, int code, const char *reason, Error *from);
String *error_repr(Error *error, Allocator *alloc);

#endif