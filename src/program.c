#include <stdbool.h>

#include "assert.h"
#include "program.h"

#pragma region Declare

Program program = { 0 };

#pragma endregion

#pragma region Public

void program_init(Program *program, int log_level, FILE *log_stream) {
    Assert(program != NULL);
    program->alloc = (Allocator *)malloc(sizeof(Allocator));
    Assert(program->alloc != NULL);
    program->logger = (Logger *)malloc(sizeof(Logger));
    Assert(program->logger != NULL);

    allocator_init(program->alloc, program->logger);
    logger_init(program->logger, "program", log_stream, log_level);

    program->initialized = true;
}

void program_destroy(Program *program) {
    Assert(program != NULL);
    if (!program->initialized) {
        return;
    }
    logger_destroy(program->logger);
    free(program->logger);

    allocator_destroy(program->alloc);
    free(program->alloc);
}

#pragma endregion

#pragma region Private
#pragma endregion
