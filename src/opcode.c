#include <stdint.h>

#include "allocator.h"
#include "assert.h"
#include "opcode.h"

#ifndef UINT24_MAX
#define UINT24_MAX 16777215
#endif

#define DEFAULT_OPCODE_CAPACITY 32
#define DEFAULT_VALUE_CAPACITY  32

#pragma region Declare

static int opcode_write(OpCodeArray *array, uint8_t *bytes, size_t count);
static uint8_t *opcode_at(OpCodeArray *array, int index);
static int line_number_write(LineNumberArray *array, int line, size_t size);
static int line_number_for(OpCodeChunk *chunk, int offset);
static inline int simple_instruction(OpCodeChunk *chunk, FILE *out, OpCode code, int offset);
static inline int constant_instruction(OpCodeChunk *chunk, FILE *out, OpCode code, int offset);

#pragma endregion

#pragma region Public

void value_write_repr(Value *value, FILE *out) {
    fprintf(out, "Value(%g)", *value);
}

int value_write(ValueArray *array, Value value) {
    vector_append(&array->values, (void *)&value);
    return vector_len(&array->values) - 1;
}

Value *value_at(ValueArray *array, int index) {
    Value *ptr = (Value *)vector_at(&array->values, index);
    Assert(ptr != NULL);
    return ptr;
}

int opcode_chunk_instruction_write_repr(OpCodeChunk *chunk, FILE *out, int offset) {
    fprintf(out, "%04d ", offset);
    int prev_line = line_number_for(chunk, offset - 1);
    int cur_line = line_number_for(chunk, offset);
    if (offset > 0 && prev_line == cur_line) {
        fprintf(out, "   | ");
    } else {
        fprintf(out, "%4d ", cur_line);
    }
    uint8_t code = *opcode_at(&chunk->codes, offset);
    switch (code) {
    case OP_CONSTANT_LONG:
    case OP_CONSTANT:
        return constant_instruction(chunk, out, code, offset);
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_NEGATE:
    case OP_RETURN:
        return simple_instruction(chunk, out, code, offset);
    default:
        Panicf("Unknown opcode %d", code);
    }
}

void opcode_chunk_init(OpCodeChunk *chunk, Allocator *alloc) {
    chunk->alloc = alloc;
    vector_init(&chunk->codes.codes, alloc, DEFAULT_OPCODE_CAPACITY, sizeof(uint8_t));
    vector_init(&chunk->lines.encodings, alloc, DEFAULT_OPCODE_CAPACITY, sizeof(uint8_t));
    vector_init(&chunk->constants.values, alloc, DEFAULT_VALUE_CAPACITY, sizeof(Value));
    // Lazy initialize long_constants since its unlikely to be used
    chunk->long_constants = (ValueArray){ 0 };
}

int OpCodeChunk_write_code(OpCodeChunk *chunk, uint8_t code, int line) {
    line_number_write(&chunk->lines, line, opcode_size(code));
    return opcode_write(&chunk->codes, &code, 1);
}

int OpCodeChunk_write_constant(OpCodeChunk *chunk, Value value, int line) {
    if (vector_len(&chunk->constants.values) < UINT8_MAX) {
        // Use OP_CONSTANT when the constant pool is small (less than UINT8_MAX)
        int offset = value_write(&chunk->constants, value);
        uint8_t code[2] = { OP_CONSTANT, offset };
        line_number_write(&chunk->lines, line, sizeof(code));
        return opcode_write(&chunk->codes, code, sizeof(code));
    }
    if (vector_len(&chunk->long_constants.values) < UINT24_MAX) {
        // Use OP_CONSTANT_LONG when the constant pool is large
        if (chunk->long_constants.values.data == NULL) {
            // Lazy initialize long_constants since its unlikely to be used
            vector_init(&chunk->long_constants.values, chunk->alloc, DEFAULT_VALUE_CAPACITY,
                        sizeof(Value));
        }
        int offset = value_write(&chunk->constants, value);
        uint8_t code[4] = {
            OP_CONSTANT_LONG,
            (offset >> 16) & 0xFF,
            (offset >> 8) & 0xFF,
            offset & 0xFF,
        };
        line_number_write(&chunk->lines, line, sizeof(code));
        return opcode_write(&chunk->codes, code, sizeof(code));
    }
    // More than UINT24_MAX constants is not supported
    Unreachable();
}

void opcode_chunk_write_repr(OpCodeChunk *chunk, FILE *out, const char *name) {
    fprintf(out, "== OpCodeChunk(%s) ==\n", name);
    for (size_t offset = 0; offset < vector_len(&chunk->codes.codes);) {
        offset = opcode_chunk_instruction_write_repr(chunk, out, offset);
        fputc('\n', out);
    }
}

void opcode_chunk_destroy(OpCodeChunk *chunk) {
    vector_destroy(&chunk->codes.codes);
    vector_destroy(&chunk->constants.values);
    vector_destroy(&chunk->lines.encodings);
}

#pragma endregion

#pragma region Private

static int opcode_write(OpCodeArray *array, uint8_t *bytes, size_t count) {
    vector_extend(&array->codes, (void *)bytes, count);
    return vector_len(&array->codes) - count;
}

static uint8_t *opcode_at(OpCodeArray *array, int index) {
    uint8_t *ptr = (uint8_t *)vector_at(&array->codes, index);
    Assert(ptr != NULL);
    return ptr;
}

static int line_number_write(LineNumberArray *array, int line, size_t size) {
    int end = vector_len(&array->encodings) - 1;
    LineNumberEncoding encoding = { .line = line, .size_count = size };
    if (end < 0) {
        vector_append(&array->encodings, (void *)&encoding);
        return 0;
    }
    LineNumberEncoding *last = (LineNumberEncoding *)vector_at(&array->encodings, end);
    if (last->line == line) {
        last->size_count += size;
        return end;
    }
    vector_append(&array->encodings, (void *)&encoding);
    return end + 1;
}

static int line_number_for(OpCodeChunk *chunk, int offset) {
    if (offset < 0) {
        return -1;
    }
    for (size_t i = 0; i < vector_len(&chunk->lines.encodings); i++) {
        LineNumberEncoding *encoding = (LineNumberEncoding *)vector_at(&chunk->lines.encodings, i);
        if (offset < encoding->size_count) {
            return encoding->line;
        }
        offset -= encoding->size_count;
    }
    Unreachable();
}

static inline int simple_instruction(OpCodeChunk *chunk, FILE *out, OpCode code, int offset) {
    (void)chunk;
    fprintf(out, "%s", opcode_name(code));
    return offset + 1;
}

static inline int constant_instruction(OpCodeChunk *chunk, FILE *out, OpCode code, int offset) {
    uint32_t index = code == OP_CONSTANT_LONG ? *opcode_at(&chunk->codes, offset + 1) << 16
                                                    | *opcode_at(&chunk->codes, offset + 2) << 8
                                                    | *opcode_at(&chunk->codes, offset + 3)
                                              : *opcode_at(&chunk->codes, offset + 1);
    fprintf(out, "%-16s %4d ", opcode_name(code), index);
    Value value = *value_at(&chunk->constants, index);
    value_write_repr(&value, out);
    return offset + opcode_size(code);
}

#pragma endregion