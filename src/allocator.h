#ifndef clox_allocator_h
#define clox_allocator_h

#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "common.h"

/**
 * Implementation of a simple arena allocator containing re-usable static memory blocks.
 * The Allocator wraps an arena which is a dynamic, linked-list of memory chunks.
 * Each chunk is allocated from the heap via libc malloc and contains a fixed-size array of bytes.
 * The chunks contain blocks of memory which are allocated as needed. Each block is composed of
 * a memory header followed by a sequence of bytes allocated for the user.
 */

#define ALIGNMENT sizeof(uintptr_t)

#define MAX_SMALL_CHUNK_SIZE 1UL << 12 // 4kb
#define MAX_SMALL_ALLOC_SIZE 1UL << 10 // 1kb

#define MAX_MEDIUM_CHUNK_SIZE 1UL << 22 // 4mb
#define MAX_MEDIUM_ALLOC_SIZE 1UL << 20 // 1mb

#define MAX_LARGE_CHUNK_SIZE 1UL << 29 // 536mb
#define MAX_LARGE_ALLOC_SIZE 1UL << 27 // 128mb

// The memory header is a 64 bit value
// - The first 8 bits are used for the magic number
// - The next 39 bits are used for size
// - The next bit is used to indicate if the memory is in use
// - The final 16 bits are reserved for future use
typedef uint64_t BlockHeader;

typedef struct ArenaChunk {
    size_t bytes_used;
    size_t bytes_total;
    struct ArenaChunk *next;
    uint8_t data[FLEXIBLE_ARRAY_MEMBER];
} ArenaChunk;

typedef struct Arena {
    size_t chunk_size;
    ArenaChunk *begin;
    ArenaChunk *end;
} Arena;

struct Logger;

typedef struct Allocator {
    Arena small;
    Arena medium;
    Arena large;
    struct Logger *logger;
} Allocator;

static inline size_t allocator_aligned_size(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

void allocator_init(Allocator *alloc, struct Logger *logger);
void allocator_destroy(Allocator *alloc);
void *allocator_alloc(Allocator *alloc, size_t size);
void allocator_free(Allocator *alloc, void *data);
void *allocator_realloc(Allocator *alloc, void *data, size_t size, size_t new_size);
void *allocator_memcopy(Allocator *alloc, void *data, size_t size);

// TODO: wrap in DEBUG_EXPOSE_INTERNALS
void allocator_write_repr(Allocator *alloc, FILE *out);

#endif