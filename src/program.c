#include <stdbool.h>

#include "program.h"

#pragma region Declare

Program program = { 0 };

#pragma endregion

#pragma region Public

void program_init(Program *program, int log_level, FILE *log_stream) {
    allocator_init(&program->alloc, &program->logger);
    logger_init(&program->logger, log_stream, log_level);
    program->initialized = true;
}

void program_destroy(Program *program) {
    if (!program->initialized) {
        return;
    }
    logger_destroy(&program->logger);
    allocator_destroy(&program->alloc);
}

#pragma endregion

#pragma region Private
#pragma endregion
