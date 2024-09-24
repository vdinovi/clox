#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#include "helpers.h"
#include "unity.h"

#pragma region Declare
#pragma endregion

#pragma region Public

#define TEST_LOGGER_NAME "test"

void setup(T *t) {
    srand(t->seed != 0 ? t->seed : time(NULL));
    logger_init(&t->log, TEST_LOGGER_NAME, t->log_stream != NULL ? t->log_stream : stderr,
                t->log_level > _LOG_LEVEL_MINIMUM ? t->log_level : LOG_LEVEL_TRACE);
    allocator_init(&t->alloc, &t->log);
}

void teardown(T *t) {
    allocator_destroy(&t->alloc);
    logger_destroy(&t->log);
}

String *escape_string(T *t, const char *source) {
    int length = 0;
    for (int i = 0; source[i] != '\0'; i++) {
        switch (source[i]) {
        case '\n':
        case '\t':
        case '\\':
        case '\"':
        case '\'':
            length += 2;
            break;
        default:
            length++;
        }
    }
    String *escaped = (String *)string_create(&t->alloc, length + 1);
    for (int i = 0; source[i] != '\0'; i++) {
        switch (source[i]) {
        case '\n':
            escaped->data[escaped->length++] = '\\';
            escaped->data[escaped->length++] = 'n';
            break;
        case '\t':
            escaped->data[escaped->length++] = '\\';
            escaped->data[escaped->length++] = 't';
            break;
        case '\\':
            escaped->data[escaped->length++] = '\\';
            escaped->data[escaped->length++] = '\\';
            break;
        case '\"':
            escaped->data[escaped->length++] = '\\';
            escaped->data[escaped->length++] = '\"';
            break;
        case '\'':
            escaped->data[escaped->length++] = '\\';
            escaped->data[escaped->length++] = '\'';
            break;
        default:
            escaped->data[escaped->length++] = source[i];
        }
    }
    escaped->data[escaped->length] = '\0';
    return escaped;
}

int random_int(int min, int max) {
    // NOTE: should convert to rand_r for multithreaded testing.
    return min + rand() % (max - min + 1);
}

#pragma endregion

#pragma region Private
#pragma endregion
