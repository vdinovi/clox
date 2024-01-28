#include "parser.h"
#include "scanner.h"

#define PARSER_ARENA_INITIAL_SIZE 1024

#pragma region Declare
#pragma endregion

#pragma region Public

void parser_init(Parser *parser, Allocator *alloc, Scanner *scanner) {
    parser->alloc = alloc;
    parser->scanner = scanner;
    parser->state = PARSER_OK;
}

#pragma endregion

#pragma region Private
#pragma endregion

