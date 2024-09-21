#include <stdarg.h>

#include "array.h"

#pragma region Declare

static String *empty_string(Allocator *alloc);

#pragma endregion

#pragma region Public

Array *array_init(Allocator *alloc, size_t unit_size, size_t length) {
    size_t size = allocator_aligned_size(sizeof(Array) + unit_size * length);
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, size);
    Array *array = (Array *)data;
    array->unit_size = unit_size;
    array->length = length;
    array->data = data + sizeof(Array);
    return array;
}

void array_destroy(Array *array, Allocator *alloc) {
    allocator_free(alloc, (void *)array);
}

String *string_alloc(Allocator *alloc, size_t capacity) {
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + capacity);
    String *string = (String *)data;
    string->length = 0;
    string->data = (char *)(data + sizeof(String));
    string->data[0] = '\0';
    return string;
}

String *string_dup_cstr(Allocator *alloc, const char *source) {
    int length = strlen(source);
    if (length == 0) {
        return empty_string(alloc);
    }
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + length + 1);
    String *string = (String *)data;
    string->length = length;
    string->data = (char *)(data + sizeof(String));
    char *target = string->data;
    for (int i = 0; i < length; i++)
        target[i] = source[i];
    target[length] = '\0';
    return string;
}

String *string_dup(Allocator *alloc, String *source) {
    if (source->length == 0) {
        return empty_string(alloc);
    }
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + source->length + 1);
    String *string = (String *)data;
    string->length = source->length;
    string->data = (char *)(data + sizeof(String));
    char *target = string->data;
    for (size_t i = 0; i < source->length; i++)
        target[i] = source->data[i];
    target[string->length] = '\0';
    return string;
}

String *string_sprintf(Allocator *alloc, const char *format, ...) {
    va_list args;
    va_start(args, format);
    int length = vsnprintf(NULL, 0, format, args);
    va_end(args);
    Assert(length >= 0);

    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + length + 1);
    String *string = (String *)data;
    string->data = (char *)(data + sizeof(String));
    char *target = string->data;

    va_start(args, format);
    length = vsnprintf(target, length + 1, format, args);
    va_start(args, format);
    Assert(length >= 0);
    string->length = length;
    string->data = target;
    return string;
}

#pragma endregion

#pragma region Private

// TODO: could be optimized by returning the same instance but I worry about mutability
static String *empty_string(Allocator *alloc) {
    String *string = (String *)allocator_alloc(alloc, sizeof(String));
    string->data = "";
    string->length = 0;
    return string;
}

#pragma endregion