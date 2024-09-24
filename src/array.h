#ifndef clox_array_h
#define clox_array_h

#include <stdlib.h>
#include <string.h>

#include "allocator.h"
#include "assert.h"

typedef struct Array {
    size_t unit_size;
    size_t length;
    uint8_t *data;
} Array;

Array *array_create(Allocator *alloc, size_t unit_size, size_t length);
void array_destroy(Array *array, Allocator *alloc);
Array *array_resize(Array *array, Allocator *alloc, size_t length);

static inline void *array_at(Array *arr, int index) {
    Assert(index >= 0 && index <= (int)arr->length);
    return (void *)(&arr->data[arr->unit_size * index]);
}

static inline void *array_set(Array *arr, int index, void *value) {
    Assert(index >= 0 && index <= (int)arr->length);
    void *target = array_at(arr, index);
    memcpy(target, value, arr->unit_size);
    return target;
}

// TODO: merge this into array_resize
static inline void array_copy(Array *target, Array *source) {
    Assert(target->length >= source->length);
    Assert(target->unit_size == source->unit_size);
    for (size_t i = 0; i < target->unit_size * target->length; i++) {
        target->data[i] = i < source->unit_size * source->length ? source->data[i] : 0;
    }
}

static inline size_t array_capacity(Array *arr) {
    return arr->unit_size * arr->length;
}

static inline size_t array_length(Array *arr) {
    return arr->length;
}

// TODO: this should distinguish length and capacity if these are meant to be mutable.
// Alternatively make strings immutable, but that requires different construction semantics.
typedef struct String {
    size_t capacity;
    size_t length;
    char *data;
} String;

String *string_create(Allocator *alloc, size_t capacity);
void string_destroy(String *string, Allocator *alloc);
String *string_dup_cstr(Allocator *alloc, const char *source);
String *string_dup(Allocator *alloc, String *source);
String *string_sprintf(Allocator *alloc, const char *format, ...);

static inline const char *string_cstr(String *str) {
    return (const char *)str->data;
}

static inline char string_at(String *str, size_t index) {
    Assert(index >= 0 && index <= str->capacity);
    return str->data[index];
}

static inline void string_set(String *str, size_t index, const char ch) {
    Assert(index >= 0 && index <= str->capacity);
    str->data[index] = ch;
    str->length = index + 1 > str->length ? index + 1 : str->length;
}

static inline void string_copy(String *target, String *source) {
    Assert(target->length >= source->capacity);
    for (size_t i = 0; i <= source->length; i++)
        source[i] = target[i];
    target->data[source->length] = '\0';
}

static inline size_t string_length(String *str) {
    return str->length;
}

#endif