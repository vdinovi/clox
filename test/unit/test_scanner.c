#include <stdio.h>
#include <string.h>

#include "unity.h"

#include "allocator.h"
#include "helpers.h"
#include "logging.h"
#include "scanner.h"

static T t;

#define TOKEN(_type, _start, _length, _line)                                                       \
    (Token) {                                                                                      \
        .type = (_type), .start = (_start), .length = (_length), .line = (_line)                   \
    }

#define EOF_TOKEN(_line)                                                                           \
    (Token) {                                                                                      \
        .type = TOKEN_EOF, .start = NULL, .length = 0, .line = (_line)                             \
    }

static void assert_token_equal(const char *source, Token *expected, Token *actual);

void setUp(void) {
    setup(&t);
}

void tearDown(void) {
    teardown(&t);
}

void test_scan(void) {
    struct {
        const char *source;
        int num_tokens;
        const Token tokens[15];
    } test_cases[] = {
        { .source = "and",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_AND, "and", 3, 1), EOF_TOKEN(1) } },
        { .source = "class",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_CLASS, "class", 5, 1), EOF_TOKEN(1) } },
        { .source = "else",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_ELSE, "else", 4, 1), EOF_TOKEN(1) } },
        { .source = "false",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_FALSE, "false", 5, 1), EOF_TOKEN(1) } },
        { .source = "for",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_FOR, "for", 3, 1), EOF_TOKEN(1) } },
        { .source = "fun",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_FUN, "fun", 3, 1), EOF_TOKEN(1) } },
        { .source = "if",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IF, "if", 2, 1), EOF_TOKEN(1) } },
        { .source = "nil",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_NIL, "nil", 3, 1), EOF_TOKEN(1) } },
        { .source = "or",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_OR, "or", 2, 1), EOF_TOKEN(1) } },
        { .source = "return",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_RETURN, "return", 6, 1), EOF_TOKEN(1) } },
        { .source = "super",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_SUPER, "super", 5, 1), EOF_TOKEN(1) } },
        { .source = "this",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_THIS, "this", 4, 1), EOF_TOKEN(1) } },
        { .source = "true",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_TRUE, "true", 4, 1), EOF_TOKEN(1) } },
        { .source = "var",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_VAR, "var", 3, 1), EOF_TOKEN(1) } },
        { .source = "while",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_WHILE, "while", 5, 1), EOF_TOKEN(1) } },
        { .source = "1",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_NUMBER, "1", 1, 1), EOF_TOKEN(1) } },
        { .source = "1.23",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_NUMBER, "1.23", 4, 1), EOF_TOKEN(1) } },
        { .source = "\"text\"",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_STRING, "text", 4, 1), EOF_TOKEN(1) } },
        { .source = "// comment",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_COMMENT, " comment", 8, 1), EOF_TOKEN(1) } },
        { .source = "_",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "_", 1, 1), EOF_TOKEN(1) } },
        { .source = "foo",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "foo", 3, 1), EOF_TOKEN(1) } },
        { .source = "_foo",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "_foo", 4, 1), EOF_TOKEN(1) } },
        { .source = "_123",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "_123", 4, 1), EOF_TOKEN(1) } },
        { .source = "foo123",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "foo123", 6, 1), EOF_TOKEN(1) } },
        { .source = "x",
          .num_tokens = 2,
          .tokens = { TOKEN(TOKEN_IDENTIFIER, "x", 1, 1), EOF_TOKEN(1) } },
        { .source = "1 + 2;",
          .num_tokens = 5,
          .tokens
          = { TOKEN(TOKEN_NUMBER, "1", 1, 1), TOKEN(TOKEN_PLUS, "+", 1, 1),
              TOKEN(TOKEN_NUMBER, "2", 1, 1), TOKEN(TOKEN_SEMICOLON, ";", 1, 1), EOF_TOKEN(1) } },
        { .source = "fun add(x, y) {\n return x + y;\n }",
          .num_tokens = 15,
          .tokens = { TOKEN(TOKEN_FUN, "fun", 3, 1), TOKEN(TOKEN_IDENTIFIER, "add", 3, 1),
                      TOKEN(TOKEN_LEFT_PAREN, "(", 1, 1), TOKEN(TOKEN_IDENTIFIER, "x", 1, 1),
                      TOKEN(TOKEN_COMMA, ",", 1, 1), TOKEN(TOKEN_IDENTIFIER, "y", 1, 1),
                      TOKEN(TOKEN_RIGHT_PAREN, ")", 1, 1), TOKEN(TOKEN_LEFT_BRACE, "{", 1, 1),
                      TOKEN(TOKEN_RETURN, "return", 6, 2), TOKEN(TOKEN_IDENTIFIER, "x", 1, 2),
                      TOKEN(TOKEN_PLUS, "+", 1, 2), TOKEN(TOKEN_IDENTIFIER, "y", 1, 2),
                      TOKEN(TOKEN_SEMICOLON, ";", 1, 2), TOKEN(TOKEN_RIGHT_BRACE, "}", 1, 3),
                      EOF_TOKEN(3) } },

    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    Token expected, actual;
    Scanner scanner;
    for (int test = 0; test < num_test_cases; test++) {
        const char *source = test_cases[test].source;
        const Token *tokens = test_cases[test].tokens;

        scanner_init(&scanner, &t.alloc, source);

        for (int n = 0; n < test_cases[test].num_tokens; n++) {
            expected = tokens[n];
            actual = scanner_scan(&scanner);
            assert_token_equal(source, &expected, &actual);
        }

        scanner_destroy(&scanner);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_scan);
    return UNITY_END();
}

static void assert_token_equal(const char *source, Token *expected, Token *actual) {
    static char message[4096];

    String *escaped_source = escape_string(&t, source);
    String *expected_repr = token_repr(expected, &t.alloc);
    String *actual_repr = token_repr(actual, &t.alloc);
    sprintf(message, "[%.*s], expected=%.*s, actual=%.*s", (int)escaped_source->length,
            escaped_source->data, (int)expected_repr->length, expected_repr->data,
            (int)actual_repr->length, actual_repr->data);

    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->type, actual->type, message);
    if (expected->length == 0) {
        TEST_ASSERT_NULL_MESSAGE(actual->start, message);
    } else {
        TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected->start, actual->start, expected->length,
                                             message);
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->length, actual->length, message);
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->line, actual->line, message);
}