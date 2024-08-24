#ifndef clox_vector_h
#define clox_vector_h

#include <stdlib.h>

#include "allocator.h"
#include "array.h"
#include "assert.h"

typedef struct Vector {
    size_t count;
    Array *data;
    Allocator *alloc;
} Vector;

void vector_init(Vector *vec, Allocator *alloc, size_t capacity, size_t unit_size);
void vector_destroy(Vector *vec);
void *vector_append(Vector *vec, void *data);
void *vector_extend(Vector *vec, void *data, size_t count);

static inline void *vector_at(Vector *vec, int index) {
    Assert(index >= 0 && index < (int)vec->count);
    return array_at(vec->data, index);
}

static inline size_t vector_len(Vector *vec) {
    return vec->count;
}

#endif