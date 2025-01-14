#include <stdarg.h>

#include "array.h"

#pragma region Declare

static String *empty_string(Allocator *alloc);

#pragma endregion

#pragma region Public

Array *array_create(Allocator *alloc, size_t unit_size, size_t length) {
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

Array *array_resize(Array *array, Allocator *alloc, size_t length) {
    Array *copy = array_create(alloc, array->unit_size, length);
    for (size_t i = 0; i < array->unit_size * length; i++) {
        copy->data[i] = i < array->unit_size * array->length ? array->data[i] : 0;
    }
    // TODO: should this free the source array?
    // array_destroy(array, alloc);
    return copy;
}

String *string_create(Allocator *alloc, size_t capacity) {
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + capacity);
    String *string = (String *)data;
    string->capacity = capacity;
    string->length = 0;
    string->data = (char *)(data + sizeof(String));
    memset(string->data, 0, capacity);
    return string;
}

void string_destroy(String *str, Allocator *alloc) {
    allocator_free(alloc, (void *)str);
}

String *string_dup_cstr(Allocator *alloc, const char *source) {
    int length = strlen(source);
    if (length == 0) {
        return empty_string(alloc);
    }
    size_t capacity = length + 1;
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + capacity);
    String *string = (String *)data;
    string->capacity = capacity;
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
    size_t capacity = source->length + 1;
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + capacity);
    String *string = (String *)data;
    string->capacity = capacity;
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

    size_t capacity = length + 1;
    uint8_t *data = (uint8_t *)allocator_alloc(alloc, sizeof(String) + capacity);
    String *string = (String *)data;
    string->capacity = capacity;
    string->data = (char *)(data + sizeof(String));
    char *target = string->data;

    va_start(args, format);
    length = vsnprintf(target, capacity, format, args);
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
    string->capacity = 0;
    string->data = "";
    string->length = 0;
    return string;
}

#pragma endregion