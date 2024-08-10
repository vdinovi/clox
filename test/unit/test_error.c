#include <stdio.h>

#include "unity.h"
#include "helpers.h"
#include "allocator.h"
#include "error.h"

Allocator alloc;
char message[4096];

void setUp(void) {
    allocator_init(&alloc);
}

void tearDown(void) {
    allocator_destroy(&alloc);
}

void test_error(void) {
    struct {
        int code;
        const char *reason;
    } test_cases[] = {
        { .code = 1, .reason = "" },
        { .code = -127, .reason = "fatal error" },
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);
    char expected_repr[1024];

    for (int test = 0; test < num_test_cases; test++) {
        int code = test_cases[test].code;
        const char *reason = test_cases[test].reason;
        Error *error = create_error(&alloc, code, reason);

        TEST_ASSERT_NOT_NULL(error);
        TEST_ASSERT_EQUAL_INT(code, error->code);
        TEST_ASSERT_EQUAL_STRING(reason, error->reason->data);

        sprintf(expected_repr, "Error{code=%d, reason='%s'}", code, reason);
        String *repr = error_repr(error, &alloc);
        TEST_ASSERT_EQUAL_STRING(expected_repr, repr->data);
    }
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_error);
    return UNITY_END();
}