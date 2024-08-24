#include <stdio.h>

#include "array.h"
#include "common.h"
#include "compiler.h"
#include "parser.h"
#include "scanner.h"

#pragma region Declare

static void advance(Parser *parser);
static void expression(Parser *parser);
static void consume(Parser *parser, TokenType expected, const char *message);
static ParseError parse_error(Parser *parser, Token token);

#pragma endregion

#pragma region Public

CompileResult compile(Allocator *alloc, const char *source) {
    CompileResult result = COMPILE_OK;

    Scanner scanner;
    scanner_init(&scanner, alloc, source);

    Parser parser;
    parser_init(&parser, alloc, &scanner);

    advance(&parser);
    expression(&parser);
    consume(&parser, TOKEN_EOF, "Expected end of expression");

    if (parser.state == PARSER_ERROR) {
        fputs(parser.error.reason, stderr);
        result = COMPILE_PARSE_ERROR;
    }

    scanner_destroy(&scanner);
    return result;
}

#pragma endregion

#pragma region Private

static void advance(Parser *parser) {
    parser->previous = parser->current;

    for (;;) {
        parser->current = scanner_scan(parser->scanner);
        if (parser->current.type != TOKEN_ERROR)
            break;
        parser->error = parse_error(parser, parser->current);
    }
}

static void expression(Parser *parser) {
    (void)parser;
    // TODO
}

static void consume(Parser *parser, TokenType expected, const char *message) {
    (void)parser;
    (void)expected;
    (void)message;
    // TODO
}

static ParseError parse_error(Parser *parser, Token token) {
    Assert(token.type == TOKEN_ERROR);
    String *error = string_dup_cstr(parser->alloc, token.start);
    return (ParseError){
        .line = token.line,
        .reason = error->data,
    };
}

#pragma endregion
