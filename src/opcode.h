#ifndef clox_opcode_h
#define clox_opcode_h

#include <stdio.h>

#include "assert.h"
#include "vector.h"
#include "common.h"

#define DEFAULT_LINE_NUMBER_ARRAY_CAPACITY 32
#define DEFAULT_CONSTANTS_ARRAY_CAPACITY 32

typedef enum {
    // Store a constant value in the constant pool.
    // The next byte is the index of the constant in the constant pool.
    OP_CONSTANT,
    // Store a constant value in the constant pool.
    // The next 3 bytes are the index of the constant in the constant pool.
    OP_CONSTANT_LONG,
    // Binary addition
    OP_ADD,
    // Binary subtraction
    OP_SUBTRACT,
    // Binary multiplication
    OP_MULTIPLY,
    // Binary division
    OP_DIVIDE,
    // Negate the subsequent value
    OP_NEGATE,
    // Pop the top value from the stack and print it.
    OP_RETURN,
} OpCode;

typedef struct OpCodeArray {
    Vector codes;
} OpCodeArray;

/**
 * Temporary typedef for the Value type.
 * (wip) This will be replaced with a more complex type.
 */
typedef double Value;

typedef struct ValueArray {
    Vector values;
} ValueArray;


typedef struct LineNumberEncoding {
    int line;       // the line number
    int size_count; // the sum of opcode sizes that share this line
} LineNumberEncoding;

typedef struct LineNumberArray {
    Vector encodings;
} LineNumberArray;

typedef struct OpCodeChunk {
    OpCodeArray codes;
    LineNumberArray lines;
    ValueArray constants;
    ValueArray long_constants;
    Allocator *alloc;
} OpCodeChunk;

void value_write_repr(Value *value, FILE *out);
int value_write(ValueArray *array, Value value);
Value *value_at(ValueArray *array, int index);

void opcode_chunk_init(OpCodeChunk *chunk, Allocator *alloc);
void opcode_chunk_write_repr(OpCodeChunk *chunk, FILE *out, const char *name);
int opcode_chunk_instruction_write_repr(OpCodeChunk *chunk, FILE *out, int offset);
void opcode_chunk_destroy(OpCodeChunk *chunk);

static inline const char *opcode_name(OpCode code) {
    switch (code) {
    case OP_CONSTANT:
        return "OP_CONSTANT";
    case OP_CONSTANT_LONG:
        return "OP_CONSTANT_LONG";
    case OP_ADD:
        return "OP_ADD";
    case OP_SUBTRACT:
        return "OP_SUBTRACT";
    case OP_MULTIPLY:
        return "OP_MULTIPLY";
    case OP_DIVIDE:
        return "OP_DIVIDE";
    case OP_NEGATE:
        return "OP_NEGATE";
    case OP_RETURN:
        return "OP_RETURN";
    default:
        Panicf("Unknown opcode %d", code);
    }
}

static inline int opcode_size(OpCode code) {
    switch (code) {
    case OP_CONSTANT:
        return 2;
    case OP_CONSTANT_LONG:
        return 4;
    case OP_ADD:
    case OP_SUBTRACT:
    case OP_MULTIPLY:
    case OP_DIVIDE:
    case OP_NEGATE:
    case OP_RETURN:
        return 1;
    default:
        Panicf("Unknown opcode %d", code);
    }
}

#endif