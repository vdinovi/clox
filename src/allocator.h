#ifndef clox_allocator_h
#define clox_allocator_h

#include <stddef.h>
#include <stdint.h>
#include <stdlib.h>

#define ALIGNMENT sizeof(uintptr_t)

typedef struct ArenaChunk {
    size_t count;
    size_t capacity;
    struct ArenaChunk *next;
    uint8_t *data;
} ArenaChunk;

typedef struct Arena {
    size_t chunk_capacity;
    ArenaChunk *begin;
    ArenaChunk *end;
} Arena;

typedef struct FreeListNode {
    size_t capacity;
    struct FreeListNode *next;
    uint8_t *data;
} FreeListNode;

struct Logger;

typedef struct Allocator {
    Arena arena;
    FreeListNode *freelist;
    struct Logger *logger;
} Allocator;

void allocator_init(Allocator *alloc, struct Logger *logger);
void allocator_destroy(Allocator *alloc);
void* allocator_alloc(Allocator *alloc, size_t size);
void allocator_free(Allocator *alloc, void *data, size_t size);
void* allocator_realloc(Allocator *alloc, void *data, size_t size, size_t new_size);
void* allocator_memcopy(Allocator *alloc, void *data, size_t size);

static inline size_t aligned_size(size_t size) {
    return (size + ALIGNMENT - 1) & ~(ALIGNMENT - 1);
}

#endif