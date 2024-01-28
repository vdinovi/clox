#ifndef clox_array_h
#define clox_array_h

#include <stdlib.h>
#include <string.h>

#include "assert.h"
#include "allocator.h"

typedef struct Array {
    size_t unit_size;
    size_t length;
    uint8_t* data;
} Array;

Array* array_alloc(Allocator *alloc, size_t unit_size, size_t length);
void array_free(Array *array, Allocator *alloc);

static inline void* array_at(Array *arr, int index) {
    Assert(index >= 0 && index <= (int)arr->length);
    return (void*)(&arr->data[arr->unit_size * index]);
}

static inline void array_copy(Array *target, Array *source) {
    Assert(target->length >= source->length);
    for (size_t i = 0; i <= source->unit_size * source->length; i++) target->data[i] = target->data[i];
}

static inline size_t array_capacity(Array *arr) {
    return arr->unit_size * arr->length;
}

static inline size_t array_length(Array *arr) {
    return arr->length;
}

typedef struct String {
    size_t length;
    char* data;
} String;

String *string_alloc(Allocator *alloc, size_t capacity);
String* string_dup_cstr(Allocator *alloc, const char *source);
String* string_dup(Allocator *alloc, String *source);
String* string_sprintf(Allocator *alloc, const char *format, ...);

static inline char string_char_at(String *str, int index) {
    Assert(index >= 0 && index <= (int)str->length);
    return str->data[index];
}

static inline void string_copy(String *target, String *source) {
    Assert(target->length >= source->length);
    for (size_t i = 0; i <= source->length; i++) source[i] = target[i];
    target->data[source->length] = '\0';
}

static inline size_t string_length(String *str) {
    return str->length;
}

#endif