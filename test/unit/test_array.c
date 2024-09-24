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
    Array *array = array_create(&t.alloc, sizeof(uint32_t), 0);
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

        Array *array = array_create(&t.alloc, unit_size, length);
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

void test_array_copy(void) {
    uint32_t values[] = {
        (uint32_t)random_int(0, UINT32_MAX),
        (uint32_t)random_int(0, UINT32_MAX),
        (uint32_t)random_int(0, UINT32_MAX),
    };
    size_t unit_size = sizeof(uint32_t);
    size_t num_values = sizeof(values) / sizeof(values[0]);

    Array *base = array_create(&t.alloc, unit_size, num_values);
    for (size_t i = 0; i < num_values; i++) {
        array_set(base, i, &values[i]);
    }

    // Copy base into equally sized array
    Array *copy = array_create(&t.alloc, unit_size, num_values);
    array_copy(copy, base);
    TEST_ASSERT_EQUAL_size_t(unit_size, copy->unit_size);
    TEST_ASSERT_EQUAL_size_t(num_values, copy->length);
    for (size_t i = 0; i < num_values; i++) {
        TEST_ASSERT_EQUAL_UINT32(*(uint32_t *)array_at(base, i), *(uint32_t *)array_at(copy, i));
    }

    // Copy base into longer array (padded with zeroes)
    Array *longer = array_create(&t.alloc, unit_size, num_values + 1);
    array_copy(longer, base);
    TEST_ASSERT_EQUAL_size_t(unit_size, longer->unit_size);
    TEST_ASSERT_EQUAL_size_t(num_values + 1, longer->length);
    for (size_t i = 0; i < num_values; i++) {
        TEST_ASSERT_EQUAL_UINT32(*(uint32_t *)array_at(base, i), *(uint32_t *)array_at(longer, i));
    }
    TEST_ASSERT_EQUAL_UINT32(0, *(uint32_t *)array_at(longer, num_values));

    array_destroy(base, &t.alloc);
    array_destroy(copy, &t.alloc);
    array_destroy(longer, &t.alloc);
}

void test_array_resize(void) {
    uint32_t values[] = {
        (uint32_t)random_int(0, UINT32_MAX),
        (uint32_t)random_int(0, UINT32_MAX),
        (uint32_t)random_int(0, UINT32_MAX),
    };
    size_t unit_size = sizeof(uint32_t);
    size_t num_values = sizeof(values) / sizeof(values[0]);

    Array *base = array_create(&t.alloc, unit_size, num_values);
    for (size_t i = 0; i < num_values; i++) {
        array_set(base, i, &values[i]);
    }

    // resize into a shorter array (remainder truncated)
    Array *shorter = array_resize(base, &t.alloc, num_values - 1);
    TEST_ASSERT_EQUAL_size_t(unit_size, shorter->unit_size);
    TEST_ASSERT_EQUAL_size_t(num_values - 1, shorter->length);
    for (size_t i = 0; i < num_values - 1; i++) {
        TEST_ASSERT_EQUAL_UINT32(*(uint32_t *)array_at(base, i), *(uint32_t *)array_at(shorter, i));
    }

    // resize into a longer array (extra zero-padded)
    Array *longer = array_resize(base, &t.alloc, num_values + 1);
    TEST_ASSERT_EQUAL_size_t(unit_size, longer->unit_size);
    TEST_ASSERT_EQUAL_size_t(num_values + 1, longer->length);
    for (size_t i = 0; i < num_values; i++) {
        TEST_ASSERT_EQUAL_UINT32(*(uint32_t *)array_at(base, i), *(uint32_t *)array_at(longer, i));
    }
    TEST_ASSERT_EQUAL_UINT32(0, *(uint32_t *)array_at(longer, num_values));

    array_destroy(base, &t.alloc);
    array_destroy(shorter, &t.alloc);
    array_destroy(longer, &t.alloc);
}

void test_string(void) {
    String *str = string_create(&t.alloc, 5);
    TEST_ASSERT_EQUAL_size_t(0, string_length(str));
    TEST_ASSERT_EQUAL_CHAR('\0', string_at(str, 0));

    const char *hello = "hello";
    for (int i = 0; i < (int)strlen(hello); i++) {
        string_set(str, i, hello[i]);
        TEST_ASSERT_EQUAL_size_t(i + 1, string_length(str));
    }
    for (int i = 0; i < (int)strlen(hello); i++) {
        TEST_ASSERT_EQUAL_CHAR(hello[i], string_at(str, i));
    }

    string_destroy(str, &t.alloc);
}

void test_string_dup(void) {
    const char *cstr = "some c string 1234asdf";
    size_t cstr_length = strlen(cstr);

    String *from_cstr = string_dup_cstr(&t.alloc, cstr);
    TEST_ASSERT_EQUAL_size_t(cstr_length, string_length(from_cstr));
    TEST_ASSERT_EQUAL_STRING(cstr, string_cstr(from_cstr));
    TEST_ASSERT_EQUAL_CHAR('\0', string_at(from_cstr, cstr_length));

    String *from_string = string_dup(&t.alloc, from_cstr);
    TEST_ASSERT_EQUAL_size_t(from_cstr->length, string_length(from_cstr));
    for (size_t i = 0; i < from_cstr->length; i++) {
        TEST_ASSERT_EQUAL_CHAR(string_at(from_cstr, i), string_at(from_string, i));
    }

    string_destroy(from_cstr, &t.alloc);
    string_destroy(from_string, &t.alloc);
}

void test_string_sprintf(void) {
    String *formatted
        = string_sprintf(&t.alloc, "int=%d unsigned=%u float=%1.2f size_t=%zu char='%c' str=\"%s\"",
                         -1234, 1234, 3.14, 256, 'x', "asdf");

    TEST_ASSERT_EQUAL_STRING("int=-1234 unsigned=1234 float=3.14 size_t=256 char='x' str=\"asdf\"",
                             string_cstr(formatted));

    string_destroy(formatted, &t.alloc);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_empty_array);
    RUN_TEST(test_array);
    RUN_TEST(test_array_copy);
    RUN_TEST(test_array_resize);
    RUN_TEST(test_string);
    RUN_TEST(test_string_dup);
    RUN_TEST(test_string_sprintf);
    return UNITY_END();
}