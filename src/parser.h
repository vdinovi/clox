#ifndef clox_parser_h
#define clox_parser_h

#include <stdbool.h>

#include "allocator.h"
#include "scanner.h"

typedef struct ParseError {
    const char *reason;
    int line;
} ParseError;

typedef enum ParserState {
    PARSER_OK,
    PARSER_ERROR,
} ParserState;

typedef struct Parser {
    Token current;
    Token previous;
    Scanner *scanner;
    ParserState state;
    ParseError error;
    Allocator *alloc;
} Parser;

void parser_init(Parser *parser, Allocator *alloc, Scanner *scanner);

#endif