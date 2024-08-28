#ifndef clox_program_h
#define clox_program_h

#include <stdarg.h>
#include <stdbool.h>

#include "allocator.h"
#include "logging.h"

struct Logger;
struct Allocator;

typedef struct Program {
    struct Logger *logger;
    struct Allocator *alloc;
    bool initialized;
} Program;

extern Program program;

void program_init(Program *program, int log_level, FILE *log_stream);
void program_destroy(Program *program);

#endif