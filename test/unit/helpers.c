#include <stdio.h>

#include "helpers.h"
#include "unity.h"

#pragma region Declare
#pragma endregion

#pragma region Public

String* escape_string(Allocator *alloc, const char *source) {
    int length = 0;
    for (int i = 0; source[i] != '\0'; i++) {
        switch(source[i]) {
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
    String* escaped = (String*)string_alloc(alloc, length + 1);
    for (int i = 0; source[i] != '\0'; i++) {
        switch(source[i]) {
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

void assert_token_equal(Allocator *alloc, const char *source, Token *expected, Token *actual) {
    static char message[4096];

    String *escaped_source = escape_string(alloc, source);
    String *expected_repr = token_repr(expected, alloc);
    String *actual_repr = token_repr(actual, alloc);
    sprintf(message, 
        "[%.*s], expected=%.*s, actual=%.*s", 
        (int)escaped_source->length, escaped_source->data, 
        (int)expected_repr->length, expected_repr->data, 
        (int)actual_repr->length, actual_repr->data); 

    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->type, actual->type, message);
    if (expected->length == 0) {
        TEST_ASSERT_NULL_MESSAGE(actual->start, message);
    } else {
        TEST_ASSERT_EQUAL_STRING_LEN_MESSAGE(expected->start, actual->start, expected->length, message);
    }
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->length, actual->length, message);
    TEST_ASSERT_EQUAL_INT_MESSAGE(expected->line, actual->line, message);
}

#pragma endregion

#pragma region Private
#pragma endregion

