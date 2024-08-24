
#include "vector.h"
#include "assert.h"

#define GROWTH_FACTOR 2

#pragma region Declare

static void vec_realloc(Vector *vec, size_t size);

#pragma endregion

#pragma region Public

void vector_init(Vector *vec, Allocator *alloc, size_t capacity, size_t unit_size) {
    vec->data = array_alloc(alloc, unit_size, capacity);
    vec->count = 0;
    vec->alloc = alloc;
}

void vector_destroy(Vector *vec) {
    array_free(vec->data, vec->alloc);
    vec->data = NULL;
    vec->count = 0;
}

void *vector_append(Vector *vec, void *data) {
    return vector_extend(vec, data, 1);
}

void *vector_extend(Vector *vec, void *data, size_t count) {
    if (vec->count + count > vec->data->length) {
        vec_realloc(vec, vec->count * GROWTH_FACTOR);
    }
    Assert(vec->count + count <= vec->data->length);
    Array source = {
        .data = (uint8_t *)data,
        .unit_size = vec->data->unit_size,
        .length = count,
    };
    uint8_t *target = vec->data->data + (vec->count * vec->data->unit_size);
    for (size_t i = 0; i <= source.unit_size * source.length; i++) {
        target[i] = source.data[i];
    }
    vec->count += count;
    return (void *)target;
}

#pragma endregion

#pragma region Private

static void vec_realloc(Vector *vec, size_t size) {
    Array *arr = array_alloc(vec->alloc, vec->data->unit_size, size);
    Assert(arr != NULL);
    array_copy(arr, vec->data);
    array_free(vec->data, vec->alloc);
    vec->data = arr;
}

#pragma endregion