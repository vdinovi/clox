#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "assert.h"
#include "common.h"
#include "logging.h"

#define DEFAULT_ARENA_CHUNK_CAPACITY 256

#pragma region Declare

static void arena_init(Arena *arena, size_t capacity);
static void arena_destroy(Arena *arena);
static void *arena_alloc(Allocator *alloc, size_t size);
static ArenaChunk *chunk_create(Logger *logger, size_t size);
static FreeListNode *freelist_node_alloc(Allocator *alloc, size_t capacity);
static void *freelist_alloc(Allocator *alloc, size_t size);
static void freelist_free(Allocator *alloc, void *data, size_t size);

#pragma endregion

#pragma region Public

void allocator_init(Allocator *alloc, Logger *logger) {
    Assert(alloc != NULL);
    Assert(logger != NULL);
    alloc->freelist = NULL;
    arena_init(&alloc->arena, DEFAULT_ARENA_CHUNK_CAPACITY);
    alloc->logger = logger;
}

void allocator_destroy(Allocator *alloc) {
    Assert(alloc != NULL);
    arena_destroy(&alloc->arena);
}

void *allocator_alloc(Allocator *alloc, size_t size) {
    Assert(alloc != NULL);
    if (size == 0)
        return NULL;
    void *data = freelist_alloc(alloc, size);
    if (data != NULL) {
        DEBUG(alloc->logger, "Allocator(%p).freelist_alloc(%zu) from %p", alloc, size, data);
        return data;
    }
    return arena_alloc(alloc, size);
}

void allocator_free(Allocator *alloc, void *data, size_t size) {
    Assert(alloc != NULL);
    freelist_free(alloc, data, size);
}

void *allocator_realloc(Allocator *alloc, void *data, size_t size, size_t new_size) {
    Assert(alloc != NULL);
    TRACE(alloc->logger, "allocator_realloc(alloc=%p, data=%p, size=%zu, new_size=%zu)", alloc,
          data, size, new_size);
    uint8_t *target = (uint8_t *)allocator_alloc(alloc, new_size);
    uint8_t *source = (uint8_t *)data;
    for (size_t i = 0; i < size; i++)
        target[i] = source[i];
    freelist_free(alloc, data, size);
    return (void *)target;
}

void *allocator_memcopy(Allocator *alloc, void *data, size_t size) {
    Assert(alloc != NULL);
    TRACE(alloc->logger, "allocator_memcopy(alloc=%p, data=%p, size=%zu)", alloc, data, size);
    uint8_t *target = (uint8_t *)allocator_alloc(alloc, size);
    uint8_t *source = (uint8_t *)data;
    for (size_t i = 0; i < size; i++)
        target[i] = source[i];
    return (void *)target;
}

#pragma endregion

#pragma region Private

static void arena_init(Arena *arena, size_t capacity) {
    arena->begin = NULL;
    arena->end = NULL;
    arena->chunk_capacity = aligned_size(capacity);
}

static void arena_destroy(Arena *arena) {
    ArenaChunk *chunk = arena->begin;
    ArenaChunk *follower = chunk;
    while (chunk != NULL) {
        chunk = chunk->next;
        free(follower);
        follower = chunk;
    }
    arena_init(arena, arena->chunk_capacity);
}

static void *arena_alloc(Allocator *alloc, size_t size) {
    size = aligned_size(size);
    TRACE(alloc->logger, "allocator_alloc(alloc=%p, size=%zu)", alloc, size);
    Arena *arena = &alloc->arena;

    if (arena->end == NULL) {
        // insert first chunk
        Assert(arena->begin == NULL);
        size_t capacity = arena->chunk_capacity;
        if (capacity < size)
            capacity = size;
        arena->begin = arena->end = chunk_create(alloc->logger, capacity);
    }

    ArenaChunk *chunk = arena->begin;
    while (chunk->count + size > chunk->capacity && chunk->next != NULL) {
        // advance through tail
        chunk = chunk->next;
    }

    if (chunk->count + size > chunk->capacity) {
        // create new tail
        Assert(chunk->next == NULL);
        size_t capacity = arena->chunk_capacity;
        if (capacity < size)
            capacity = size;
        chunk = arena->end = chunk->next = chunk_create(alloc->logger, capacity);
    }

#ifdef DEBUG_ALLOCATIONS
    size_t num_chunks = 0;
    for (ArenaChunk *c = arena->begin; c != NULL; c = c->next)
        num_chunks++;
    DEBUG(alloc->logger, "Allocator(%p).alloc(%zu) { chunks=%zu }", alloc, size, num_chunks);
#endif

    // allocate
    uint8_t *result = &chunk->data[chunk->count];
    chunk->count += size;
    return (void *)result;
}

static ArenaChunk *chunk_create(Logger *logger, size_t size) {
    size_t total_size = aligned_size(sizeof(ArenaChunk) + size);
    size_t capacity = total_size - sizeof(ArenaChunk);
    TRACE(logger, "chunk_create(total_size=%zu, capacity=%zu)", total_size, capacity);

#ifdef DEBUG_ALLOCATIONS
    DEBUG(logger, "stdlib.malloc(%zu)", total_size);
#endif

    ArenaChunk *chunk = (ArenaChunk *)malloc(total_size);
    Assert(chunk != NULL);
    chunk->next = NULL;
    chunk->count = 0;
    chunk->capacity = capacity;
    chunk->data = (uint8_t *)chunk + (total_size - capacity);
    return chunk;
}

static FreeListNode *freelist_node_alloc(Allocator *alloc, size_t capacity) {
    size_t total_size = aligned_size(sizeof(FreeListNode) + capacity);
    capacity = total_size - sizeof(FreeListNode);
    TRACE(alloc->logger, "freelist_node_alloc(total_size=%zu, capacity=%zu)", total_size, capacity);
    FreeListNode *node = (FreeListNode *)arena_alloc(alloc, total_size);
    Assert(node != NULL);
    node->capacity = capacity;
    node->next = NULL;
    node->data = (uint8_t *)node + (total_size - capacity);
    return node;
}

static void *freelist_alloc(Allocator *alloc, size_t size) {
    TRACE(alloc->logger, "freelist_alloc(alloc=%p, size=%zu)", alloc, size);
    FreeListNode *iter = alloc->freelist;
    FreeListNode *follower = NULL;
    while (iter != NULL && iter->capacity < size) {
        follower = iter;
        iter = iter->next;
    }

    // no available nodes
    if (iter == NULL)
        return NULL;
    if (follower == NULL) {
        // head is available, remove head
        iter->next = alloc->freelist->next;
        alloc->freelist = iter->next;
        return (void *)iter->data;
    }
    // subsequent node is available, splice node out
    follower->next = iter->next;
    iter->next = NULL;
    return (void *)iter->data;
}

static void freelist_free(Allocator *alloc, void *data, size_t size) {
    TRACE(alloc->logger, "freelist_free(alloc=%p, data=%p, size=%zu)", alloc, data, size);
    FreeListNode *iter = alloc->freelist;
    while (iter != NULL && iter->next != NULL)
        iter = iter->next;

    FreeListNode *node = freelist_node_alloc(alloc, size);
    if (iter == NULL) {
        alloc->freelist = node;
    } else {
        iter->next = node;
    }
}

#pragma endregion