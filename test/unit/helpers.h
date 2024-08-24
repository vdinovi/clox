#ifndef clox_helpers_h
#define clox_helpers_h

#include <stdbool.h>

#include "allocator.h"
#include "array.h"
#include "logging.h"
#include "scanner.h"

typedef struct T {
    Logger log;
    Allocator alloc;
    FILE *log_stream;
    LogLevel log_level;
    unsigned seed;
} T;

void setup(T *t);
void teardown(T *t);

String *escape_string(T *t, const char *source);
int random_int(int min, int max);

#endif