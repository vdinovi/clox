#include "allocator.h"
#include "array.h"
#include "helpers.h"
#include "unity.h"

static T t;

void setUp(void) {
    setup(&t);
}

void tearDown(void) {
    teardown(&t);
}

void test_empty_array(void) {
    Array *array = array_init(&t.alloc, sizeof(uint32_t), 0);
    TEST_ASSERT_EQUAL_size_t(sizeof(uint32_t), array->unit_size);
    TEST_ASSERT_EQUAL_size_t(0, array_length(array));
    array_destroy(array, &t.alloc);
}

void test_array(void) {
    struct {
        size_t unit_size;
        size_t length;
        void *data;
    } test_cases[] = {
        { .unit_size = sizeof(uint8_t), .length = 5, .data = (uint8_t[]){ 0, 1, 2, 3, 4 } },
    };
    int num_test_cases = sizeof(test_cases) / sizeof(test_cases[0]);

    for (int test = 0; test < num_test_cases; test++) {
        void *data = test_cases[test].data;
        size_t unit_size = test_cases[test].unit_size;
        size_t length = test_cases[test].length;

        Array *array = array_init(&t.alloc, unit_size, length);
        for (int i = 0; i < (int)length; i++) {
            array_set(array, i, (void *)&data[i]);
        }

        TEST_ASSERT_EQUAL_size_t(length, array_length(array));
        for (int i = 0; i < (int)length; i++) {
            TEST_ASSERT_EQUAL_UINT8(i, *(uint8_t *)array_at(array, i));
        }

        array_destroy(array, &t.alloc);
    }
}

// TODO: test array_copy

// TODO: test String

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_array);
    RUN_TEST(test_array);
    return UNITY_END();
}