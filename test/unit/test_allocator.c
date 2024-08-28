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

    // test a bunch of random size allocations
    uint8_t *freeptrs[10] = { NULL };
    uint8_t *ptr = NULL;
    for (int test = 0; test < 100; test++) {
        allocator_write_repr(&alloc, stderr);
        int length = random_int(1, MAX_MEDIUM_ALLOC_SIZE);
        ptr = (uint8_t *)allocator_alloc(&alloc, sizeof(uint8_t) * length);
        for (int i = 0; i < length; i++) {
            ptr[i] = i;
        }
        for (int i = 0; i < length; i++) {
            TEST_ASSERT_EQUAL_UINT8(i, ptr[i]);
        }
        // mark pointer to be randomly freed later
        freeptrs[test % 10] = ptr;

        uint8_t *freeptr = freeptrs[length % 10];
        if (freeptr != NULL) {
            // free a randomly marked pointer
            allocator_free(&alloc, freeptr);
            freeptrs[length % 10] = NULL;
        }
    }

    allocator_destroy(&alloc);
}

int main(void) {
    UNITY_BEGIN();
    RUN_TEST(test_allocator);
    return UNITY_END();
}
