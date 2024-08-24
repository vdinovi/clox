#include <stdio.h>

#include "unity.h"

#include "allocator.h"
#include "helpers.h"
#include "logging.h"
#include "parser.h"

static T t;

void setUp(void) {
    setup(&t);
}

void tearDown(void) {
    teardown(&t);
}

void test_parse(void) {
    struct {
        const char *source;
    } test_cases[] = {
        { .source = "1 + 2;" },
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    Scanner scanner;
    Parser parser;
    for (int test = 0; test < num_test_cases; test++) {
        const char *source = test_cases[test].source;

        scanner_init(&scanner, &t.alloc, source);
        parser_init(&parser, &t.alloc, &scanner);

        // TODO

        // parser_destroy(&parser);
        scanner_destroy(&scanner);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_parse);
    return UNITY_END();
}