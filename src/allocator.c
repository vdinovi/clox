#include <stdarg.h>
#include <stdio.h>
#include <string.h>

#include "allocator.h"
#include "assert.h"
#include "common.h"
#include "logging.h"

#pragma region Declare

#define BLOCK_HEADER_MAGIC_SHIFT  56
#define BLOCK_HEADER_SIZE_SHIFT   17
#define BLOCK_HEADER_IN_USE_SHIFT 16

#define BLOCK_HEADER_MAGIC_MASK    0xFF00000000000000ULL // 8 bits for the magic number
#define BLOCK_HEADER_SIZE_MASK     0x00FFFFFFFFFFFE00ULL // 39 bits for size
#define BLOCK_HEADER_IN_USE_MASK   0x0000000000000100ULL // 1 bit for in use marker
#define BLOCK_HEADER_RESERVED_MASK 0x00000000000000FFULL // 16 bits reserved for future use
#define BLOCK_HEADER_MAGIC_NUMBER  0x4C                  // the magic number value

// #define MAX_ALLOC_SIZE (1ULL << 39)
#define MAX_ALLOC_SIZE (1ULL << 20)

#define ARENA_DEFAULT_CHUNK_SIZE 4096

#define MALLOC(size) malloc(size)

static void arena_init(Arena *arena, size_t capacity);
static void arena_destroy(Arena *arena);
static void *arena_alloc(Arena *arena, Logger *logger, size_t size);
static void arena_free(Arena *arena, Logger *logger, void *data);

static inline bool arena_contains(Arena *arena, void *data);
static inline ArenaChunk *chunk_create(Logger *logger, size_t size);
static inline BlockHeader *chunk_alloc_block(ArenaChunk *chunk, Logger *logger, size_t size);
static inline uint8_t block_magic(BlockHeader header);
static inline size_t block_size(BlockHeader header);
static inline bool block_available(BlockHeader header);
static inline void block_checkout(BlockHeader *header);
static inline void block_checkin(BlockHeader *header);
static inline void block_init(BlockHeader *header, size_t size);
static inline void block_repr(const char *buffer, BlockHeader *block);

#pragma endregion

#pragma region Public

#define MAX_SMALL_CHUNK_SIZE 1UL << 12 // 4kb
#define MAX_SMALL_ALLOC_SIZE 1UL << 10 // 1kb

#define MAX_MEDIUM_CHUNK_SIZE 1UL << 22 // 4mb
#define MAX_MEDIUM_ALLOC_SIZE 1UL << 20 // 1mb

#define MAX_LARGE_CHUNK_SIZE 1UL << 29 // 536mb
#define MAX_LARGE_ALLOC_SIZE 1UL << 27 // 128mb

void allocator_init(Allocator *alloc, Logger *logger) {
    Assert(alloc != NULL);
    Assert(logger != NULL);
    alloc->logger = logger;
    arena_init(&alloc->small, MAX_SMALL_CHUNK_SIZE);
    arena_init(&alloc->medium, MAX_MEDIUM_CHUNK_SIZE);
    arena_init(&alloc->large, MAX_LARGE_CHUNK_SIZE);
}

void allocator_destroy(Allocator *alloc) {
    Assert(alloc != NULL);
    alloc->logger = NULL;
    alloc->large = (Arena){ 0 };
    arena_destroy(&alloc->large);
    alloc->medium = (Arena){ 0 };
    arena_destroy(&alloc->medium);
    alloc->small = (Arena){ 0 };
    arena_destroy(&alloc->small);
}

void *allocator_alloc(Allocator *alloc, size_t size) {
    Assert(alloc != NULL);
    Assert(size > 0);
    size_t alloc_size = allocator_aligned_size(size);
    void *data = NULL;
    if (alloc_size <= MAX_SMALL_ALLOC_SIZE) {
        data = arena_alloc(&alloc->small, alloc->logger, alloc_size);
    } else if (alloc_size <= MAX_MEDIUM_ALLOC_SIZE) {
        data = arena_alloc(&alloc->medium, alloc->logger, alloc_size);
    } else if (alloc_size <= MAX_LARGE_ALLOC_SIZE) {
        data = arena_alloc(&alloc->large, alloc->logger, alloc_size);
    } else {
        Panicf("Requested allocation size %zu exceeds maximum size %zu", size,
               MAX_LARGE_ALLOC_SIZE);
    }
    if (data == NULL) {
        Panicf("Failed to allocate %zu bytes", size);
    }
    return data;
}

void allocator_free(Allocator *alloc, void *data) {
    Assert(alloc != NULL);
    Assert(data != NULL);
    if (arena_contains(&alloc->small, data)) {
        arena_free(&alloc->small, alloc->logger, data);
    } else if (arena_contains(&alloc->medium, data)) {
        arena_free(&alloc->medium, alloc->logger, data);
    } else if (arena_contains(&alloc->large, data)) {
        arena_free(&alloc->large, alloc->logger, data);
    } else {
        Panic("Attempted to free memory not allocated by this allocator");
    }
}

void *allocator_realloc(Allocator *alloc, void *data, size_t size, size_t new_size) {
    Assert(alloc != NULL);
    Assert(data != NULL);
    TRACE(alloc->logger, "allocator_realloc(alloc=%p, data=%p, size=%zu, new_size=%zu)", alloc,
          data, size, new_size);

    uint8_t *target = allocator_alloc(alloc, new_size);
    Assert(target != NULL);
    uint8_t *source = (uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        target[i] = source[i];
    }
    for (size_t i = size; i < new_size; i++) {
        target[i] = 0;
    }
    allocator_free(alloc, data);
    return (void *)target;
}

void *allocator_memcopy(Allocator *alloc, void *data, size_t size) {
    Assert(alloc != NULL);
    Assert(data != NULL);
    TRACE(alloc->logger, "allocator_memcopy(alloc=%p, data=%p, size=%zu)", alloc, data, size);

    uint8_t *target = allocator_alloc(alloc, size);
    Assert(target != NULL);
    uint8_t *source = (uint8_t *)data;
    for (size_t i = 0; i < size; i++) {
        target[i] = source[i];
    }
    return (void *)target;
}

void allocator_write_repr(Allocator *alloc, FILE *out) {
    static char buffer[4096] = { 0 };
    fprintf(out, "Allocator (%p) {\n", alloc);
    fprintf(out, "  logger: %p { ... },\n", alloc->logger);
    struct {
        const char *name;
        Arena *arena;
    } arenas[] = {
        { .name = "small", .arena = &alloc->small },
        { .name = "medium", .arena = &alloc->medium },
        { .name = "large", .arena = &alloc->large },
    };
    for (int a = 0; a < 3; a++) {
        Arena *arena = arenas[a].arena;
        fprintf(out, "  %s: (%p) {\n", arenas[a].name, arena);
        fprintf(out, "    chunk_size: %zu,\n", arena->chunk_size);
        if (arena->begin == NULL) {
            fprintf(out, "    chunks: [],\n  },\n");
            continue;
        }
        fprintf(out, "    chunks: [\n");
        for (ArenaChunk *chunk = arena->begin; chunk != NULL; chunk = chunk->next) {
            fprintf(out, "      ArenaChunk (%p) {\n", chunk);
            fprintf(out, "        bytes_used: %zu,\n", chunk->bytes_used);
            fprintf(out, "        bytes_total: %zu,\n", chunk->bytes_total);
            if (chunk->bytes_used == 0) {
                fprintf(out, "        blocks: [],\n},\n");
                continue;
            }
            fprintf(out, "        blocks: [\n");
            for (int offset = 0; offset < (int)chunk->bytes_used;) {
                BlockHeader *header = (BlockHeader *)&chunk->data[offset];
                block_repr(buffer, header);
                fprintf(out, "          [%d]: %s,\n", offset, buffer);
                offset += sizeof(BlockHeader) + block_size(*header);
            }
            fprintf(out, "        ],\n");
            fprintf(out, "      },\n");
        }
        fprintf(out, "    ],\n");
        fprintf(out, "  },\n");
    }
    fprintf(out, "}\n");
}

#pragma endregion

#pragma region Private

static void arena_init(Arena *arena, size_t capacity) {
    arena->begin = arena->end = NULL;
    arena->chunk_size = allocator_aligned_size(capacity);
}

static void arena_destroy(Arena *arena) {
    ArenaChunk *chunk = arena->begin;
    ArenaChunk *follower = chunk;
    while (chunk != NULL) {
        chunk = chunk->next;
        free(follower);
        follower = chunk;
    }
    arena->begin = arena->end = NULL;
}

static void *arena_alloc(Arena *arena, Logger *logger, size_t size) {
    size_t bytes_needed = allocator_aligned_size(sizeof(BlockHeader) + size);
    size_t block_size = bytes_needed - sizeof(BlockHeader);
    TRACE(logger, "arena_alloc(arena=%p, size=%zu, blocksize=%zu)", arena, bytes_needed,
          block_size);

    if (arena->begin == NULL) {
        // create first chunk
        arena->begin = arena->end = chunk_create(logger, arena->chunk_size);
    }

    BlockHeader *header = NULL;
    for (ArenaChunk *chunk = arena->begin; chunk != NULL; chunk = chunk->next) {
        // scan for free block
        header = chunk_alloc_block(chunk, logger, block_size);
        if (header != NULL) {
            return (void *)((uint8_t *)header + sizeof(BlockHeader));
        }
        DEBUG(logger, "No free blocks in chunk %p", chunk);
    }

    // no free blocks found, allocate new chunk
    ArenaChunk *chunk = arena->end = arena->end->next = chunk_create(logger, arena->chunk_size);
    header = chunk_alloc_block(chunk, logger, block_size);
    Assert(header != NULL);
    return (void *)((uint8_t *)header + sizeof(BlockHeader));
}

static void arena_free(Arena *arena, Logger *logger, void *data) {
    TRACE(logger, "arena_free(arena=%p, data=%p)", arena, data);
    BlockHeader *header = (BlockHeader *)((uint8_t *)data - sizeof(BlockHeader));
    Assert(block_magic(*header) == BLOCK_HEADER_MAGIC_NUMBER);
    Assert(!block_available(*header));
    block_checkin(header);
    DEBUG(logger, "Freed block of size %zu at offset %zu from chunk %p", block_size(*header),
          (uint8_t *)header - (uint8_t *)arena->begin, arena->begin);
}

static inline bool arena_contains(Arena *arena, void *data) {
    for (ArenaChunk *chunk = arena->begin; chunk != NULL; chunk = chunk->next) {
        if ((uint8_t *)data >= chunk->data && (uint8_t *)data < chunk->data + chunk->bytes_total) {
            return true;
        }
    }
    return false;
}

static inline ArenaChunk *chunk_create(Logger *logger, size_t size) {
    Assert(size > 0);
    TRACE(logger, "chunk_create(size=%zu)", size);
    size_t bytesize = sizeof(ArenaChunk) + sizeof(uint8_t) * size;
    ArenaChunk *chunk = (ArenaChunk *)MALLOC(bytesize);
#ifdef DEBUG_ALLOCATIONS
    DEBUG(logger, "stdlib.malloc(%zu)", bytesize);
#endif
    memset(chunk, 0, bytesize);

    Assert(chunk != NULL);
    chunk->next = NULL;
    chunk->bytes_used = 0;
    chunk->bytes_total = size;
    return chunk;
}

static inline BlockHeader *chunk_alloc_block(ArenaChunk *chunk, Logger *logger, size_t size) {
    TRACE(logger, "chunk_alloc_block(chunk=%p, size=%zu)", chunk, size);
    BlockHeader *header = NULL;
    for (int offset = 0; offset < (int)chunk->bytes_used;) {
        header = (BlockHeader *)&chunk->data[offset];
        Assert(block_magic(*header) == BLOCK_HEADER_MAGIC_NUMBER);
        size_t capacity = block_size(*header);
        if (block_available(*header) && capacity >= size) {
            // found free block
            Assert(capacity < MAX_ALLOC_SIZE);
            block_checkout(header);
            DEBUG(logger,
                  "Repurposed block of capacity %zu for size %zu at offset %d from chunk %p",
                  capacity, size, offset, chunk);
            return header;
        }
        // skip to next block
        offset += sizeof(BlockHeader) + capacity;
    }
    if (chunk->bytes_used + size + sizeof(BlockHeader) <= chunk->bytes_total) {
        // allocate new block
        int offset = chunk->bytes_used;
        header = (BlockHeader *)&chunk->data[offset];
        block_init(header, size);
        block_checkout(header);
        for (int i = 0; i < (int)size; i++) {
            chunk->data[offset + sizeof(BlockHeader) + i] = 0;
        }
        chunk->bytes_used += sizeof(BlockHeader) + size;
        DEBUG(logger, "Initialized block with size %zu at offset %d from chunk %p", size, offset,
              chunk);
        return header;
    }
    // no free blocks found
    return NULL;
}

static inline uint8_t block_magic(BlockHeader header) {
    return (header & BLOCK_HEADER_MAGIC_MASK) >> BLOCK_HEADER_MAGIC_SHIFT;
}

static inline size_t block_size(BlockHeader header) {
    return (header & BLOCK_HEADER_SIZE_MASK) >> BLOCK_HEADER_SIZE_SHIFT;
}

static inline bool block_available(BlockHeader header) {
    return (header & BLOCK_HEADER_IN_USE_MASK) == 0;
}

static inline void block_checkout(BlockHeader *header) {
    *header |= BLOCK_HEADER_IN_USE_MASK;
}

static inline void block_checkin(BlockHeader *header) {
    *header &= ~BLOCK_HEADER_IN_USE_MASK;
}

static inline void block_init(BlockHeader *header, size_t size) {
    *header = ((uint64_t)(BLOCK_HEADER_MAGIC_NUMBER & 0xFF) << BLOCK_HEADER_MAGIC_SHIFT)
              | ((uint64_t)(size & 0x7FFFFFFFFF) << BLOCK_HEADER_SIZE_SHIFT);
}

static inline void block_repr(const char *buffer, BlockHeader *block) {
    sprintf(buffer, "BlockHeader { magic: %u, size: %zu, available: %s, data = %p }",
            block_magic(*block), block_size(*block), block_available(*block) ? "true" : "false",
            ((uint8_t *)block) + sizeof(BlockHeader));
}

#pragma endregion