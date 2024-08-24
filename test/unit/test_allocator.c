#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "allocator.h"
#include "helpers.h"
#include "logging.h"
#include "unity.h"

static T t;

void setUp(void) {
    setup(&t);
}

void tearDown(void) {
    teardown(&t);
}

void test_allocator(void) {
    Allocator alloc;
    allocator_init(&alloc, &t.log);
    uint8_t *ptr;

    // test a zero size allocation
    ptr = (uint8_t *)allocator_alloc(&alloc, 0);
    TEST_ASSERT_NULL(ptr);

    // test a bunch of random size allocations
    for (int test = 0; test < 1000; test++) {
        int length = random_int(1, 1000);
        ptr = (uint8_t *)allocator_alloc(&alloc, sizeof(uint8_t) * length);
        for (int i = 0; i < length; i++) {
            ptr[i] = i;
        }
        for (int i = 0; i < length; i++) {
            TEST_ASSERT_EQUAL_UINT8(i, ptr[i]);
        }
    }

    allocator_free(&alloc, ptr, sizeof(uint8_t));
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_allocator);
    return UNITY_END();
}
