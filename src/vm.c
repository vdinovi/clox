#include <stdio.h>

#include "compiler.h"
#include "vm.h"

#pragma region Declare

static inline void stack_reset(ValueStack *stack);
static inline void stack_push(ValueStack *stack, Value value);
static inline Value stack_pop(ValueStack *stack);
// static void stack_write_repr(ValueStack *stack, FILE *out);
static InterpretResult virtual_machine_exec(VirtualMachine *vm);

#pragma endregion

#pragma region Public

InterpretResult interpret(VirtualMachine *vm, const char *source) {
    InterpretResult result = INTERPRET_OK;

    OpCodeChunk chunk;
    opcode_chunk_init(&chunk, vm->alloc);
    if (compile(vm->alloc, source) != COMPILE_OK) {
        result = INTERPRET_COMPILE_ERROR;
        goto cleanup;
    }
    vm->chunk = &chunk;
    vm->ip = vm->chunk->codes.codes.data->data;
    opcode_chunk_write_repr(vm->chunk, stderr, "main");
    result = virtual_machine_exec(vm);

cleanup:
    opcode_chunk_destroy(&chunk);
    return result;
}

void virtual_machine_init(VirtualMachine *vm, Allocator *alloc) {
    vm->alloc = alloc;
    stack_reset(&vm->stack);
}

static inline void stack_reset(ValueStack *stack) {
    stack->top = stack->values;
}

#pragma endregion

#pragma region Private

static inline void stack_push(ValueStack *stack, Value value) {
    *stack->top = value;
    stack->top++;
}

static inline Value stack_pop(ValueStack *stack) {
    stack->top--;
    Assert(stack->top >= stack->values);
    return *stack->top;
}

// static void stack_write_repr(ValueStack *stack, FILE *out) {
//     fputs("          [", out);
//     for (Value *slot = stack->values; slot < stack->top; slot++) {
//         value_write_repr(slot, out);
//         fputc(' ', out);
//     }
//     fputs("]\n", out);
// }

static InterpretResult virtual_machine_exec(VirtualMachine *vm) {
#define READ_BYTE()          (*vm->ip++)
#define READ_CONSTANT(index) (((Value *)vm->chunk->constants.values.data)[(index)])
#define OFFSET()             ((int)(vm->ip - vm->chunk->codes.codes.data->data))
#define BINARY_OP(op)                                                                              \
    do {                                                                                           \
        Value right = stack_pop(&vm->stack);                                                       \
        Value left = stack_pop(&vm->stack);                                                        \
        stack_push(&vm->stack, left op right);                                                     \
    } while (false)

    for (;;) {
#ifdef DEBUG_TRACE_EXECUTION
        stack_write_repr(&vm->stack, stderr);
        opcode_chunk_instruction_write_repr(vm->chunk, stderr, OFFSET());
        fputc('\n', stderr);
#endif
        uint8_t code;
        switch (code = READ_BYTE()) {
        default:
            Panicf("Unknown opcode %d", code);
        case OP_CONSTANT:
        case OP_CONSTANT_LONG: {
            uint32_t index = code == OP_CONSTANT_LONG
                                 ? READ_BYTE() << 16 | READ_BYTE() << 8 | READ_BYTE()
                                 : READ_BYTE();
            Value constant = READ_CONSTANT(index);
            stack_push(&vm->stack, constant);
            break;
        }
        case OP_ADD:
            BINARY_OP(+);
            break;
        case OP_SUBTRACT:
            BINARY_OP(-);
            break;
        case OP_MULTIPLY:
            BINARY_OP(*);
            break;
        case OP_DIVIDE:
            BINARY_OP(/);
            break;
        case OP_NEGATE:
            stack_push(&vm->stack, -stack_pop(&vm->stack));
            break;
        case OP_RETURN: {
            Value value = stack_pop(&vm->stack);
            value_write_repr(&value, stderr);
            fputc('\n', stderr);
            return INTERPRET_OK;
        }
        }
    }

#undef READ_BYTE
#undef READ_CONSTANT
#undef OFFSET
}

#pragma endregion