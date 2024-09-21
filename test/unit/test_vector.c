#include "allocator.h"
#include "array.h"
#include "helpers.h"
#include "unity.h"
#include "vector.h"

static T t;

void setUp(void) {
    setup(&t);
}

void tearDown(void) {
    teardown(&t);
}

void test_vector(void) {
    Vector vec;
    vector_init(&vec, &t.alloc, 0, sizeof(uint32_t));
    TEST_ASSERT_EQUAL_size_t(0, vector_len(&vec));

    int appends = 256;
    for (int i = 0; i < appends; i++) {
        uint32_t *value = vector_append(&vec, &i);
        TEST_ASSERT_EQUAL_size_t(i + 1, vector_len(&vec));
        TEST_ASSERT_EQUAL_UINT32(i, *value);
        TEST_ASSERT_EQUAL_UINT32(i, *(uint32_t *)array_at(vec.data, i));
    }
    TEST_ASSERT_EQUAL_size_t(appends, vector_len(&vec));

    uint32_t extend[] = { 12345, 23456, 34567 };
    int extends = sizeof(extend) / sizeof(extend[0]);
    uint32_t *values = vector_extend(&vec, (void *)extend, extends);
    TEST_ASSERT_EQUAL_size_t(appends + extends, vector_len(&vec));
    for (int i = appends; i < appends + extends; i++) {
        TEST_ASSERT_EQUAL_UINT32(extend[i - appends], values[i - appends]);
        TEST_ASSERT_EQUAL_UINT32(extend[i - appends], *(uint32_t *)array_at(vec.data, i));
    }

    vector_destroy(&vec);
}

// TODO: test vec_realloc

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_vector);
    return UNITY_END();
}