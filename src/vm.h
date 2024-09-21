#ifndef clox_vm_h
#define clox_vm_h

#include "allocator.h"
#include "assert.h"
#include "common.h"
#include "instruction.h"

#define STACK_MAX 256

typedef struct ValueStack {
    Value values[STACK_MAX];
    Value *top;
} ValueStack;

typedef struct VirtualMachine {
    OpCodeChunk *chunk;
    uint8_t *ip;
    ValueStack stack;
    Allocator *alloc;
} VirtualMachine;

typedef enum InterpretResult {
    INTERPRET_OK,
    INTERPRET_COMPILE_ERROR,
    INTERPRET_RUNTIME_ERROR,
} InterpretResult;

void virtual_machine_init(VirtualMachine *vm, Allocator *alloc);
InterpretResult interpret(VirtualMachine *vm, const char *source);

static inline const char *InterpretResult_name(InterpretResult result) {
    switch (result) {
    case INTERPRET_OK:
        return "OP_CONSTANT";
    case INTERPRET_COMPILE_ERROR:
        return "OP_CONSTANT_LONG";
    case INTERPRET_RUNTIME_ERROR:
        return "OP_RETURN";
    default:
        Panicf("Unknown result %d", result);
    }
}

#endif