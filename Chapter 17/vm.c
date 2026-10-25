#include <limits.h>
#include <stdio.h>

#include "compiler.h"
#include "memory.h"
#include "vm.h"

typedef struct {
    Chunk* chunk;
    uint8_t* ip;
    Value* stack;
    int stackCount;
    int stackCapacity;
} VM;

static VM vm;

static void resetStack(void) {
    vm.stackCount = 0;
}

static bool push(Value value) {
    if (vm.stackCount == vm.stackCapacity) {
        if (vm.stackCapacity > INT_MAX / 2) {
            fprintf(stderr, "Runtime error: value stack is too large.\n");
            return false;
        }
        int oldCapacity = vm.stackCapacity;
        vm.stackCapacity = GROW_CAPACITY(oldCapacity);
        vm.stack = GROW_ARRAY(Value, vm.stack, oldCapacity, vm.stackCapacity);
    }
    vm.stack[vm.stackCount++] = value;
    return true;
}

static bool pop(Value* value) {
    if (vm.stackCount == 0) {
        fprintf(stderr, "Runtime error: value stack underflow.\n");
        return false;
    }
    *value = vm.stack[--vm.stackCount];
    return true;
}

void initVM(void) {
    vm.chunk = NULL;
    vm.ip = NULL;
    vm.stack = NULL;
    vm.stackCount = 0;
    vm.stackCapacity = 0;
}

void freeVM(void) {
    FREE_ARRAY(Value, vm.stack, vm.stackCapacity);
    initVM();
}

static InterpretResult run(Value* result) {
#define READ_BYTE() (*vm.ip++)
#define READ_CONSTANT() (vm.chunk->constants.values[READ_BYTE()])
#define BINARY_OP(operator) \
    do { \
        if (vm.stackCount < 2) { \
            fprintf(stderr, "Runtime error: binary operator needs two values.\n"); \
            return INTERPRET_RUNTIME_ERROR; \
        } \
        Value right = vm.stack[--vm.stackCount]; \
        Value left = vm.stack[--vm.stackCount]; \
        if (!push(left operator right)) return INTERPRET_RUNTIME_ERROR; \
    } while (false)

    while ((size_t)(vm.ip - vm.chunk->code) < (size_t)vm.chunk->count) {
        uint8_t instruction = READ_BYTE();
        switch (instruction) {
            case OP_CONSTANT: {
                if ((size_t)(vm.ip - vm.chunk->code) >= (size_t)vm.chunk->count) {
                    fprintf(stderr, "Runtime error: missing constant operand.\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!push(READ_CONSTANT())) return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_CONSTANT_LONG: {
                if ((size_t)(vm.ip - vm.chunk->code) + 3 > (size_t)vm.chunk->count) {
                    fprintf(stderr, "Runtime error: missing long constant operand.\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
                uint32_t constant = READ_BYTE();
                constant |= (uint32_t)READ_BYTE() << 8;
                constant |= (uint32_t)READ_BYTE() << 16;
                if (constant >= (uint32_t)vm.chunk->constants.count) {
                    fprintf(stderr, "Runtime error: constant index out of range.\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
                if (!push(vm.chunk->constants.values[constant])) return INTERPRET_RUNTIME_ERROR;
                break;
            }
            case OP_ADD:      BINARY_OP(+); break;
            case OP_SUBTRACT: BINARY_OP(-); break;
            case OP_MULTIPLY: BINARY_OP(*); break;
            case OP_DIVIDE:   BINARY_OP(/); break;
            case OP_NEGATE:
                if (vm.stackCount < 1) {
                    fprintf(stderr, "Runtime error: negate needs one value.\n");
                    return INTERPRET_RUNTIME_ERROR;
                }
#ifdef VM_BENCHMARK_POP_PUSH
                {
                    Value operand = vm.stack[--vm.stackCount];
                    if (!push(-operand)) return INTERPRET_RUNTIME_ERROR;
                }
#else
                vm.stack[vm.stackCount - 1] = -vm.stack[vm.stackCount - 1];
#endif
                break;
            case OP_RETURN:
                if (!pop(result)) return INTERPRET_RUNTIME_ERROR;
                return INTERPRET_OK;
            default:
                fprintf(stderr, "Runtime error: unknown opcode %u.\n", instruction);
                return INTERPRET_RUNTIME_ERROR;
        }
    }

    fprintf(stderr, "Runtime error: chunk ended without OP_RETURN.\n");
    return INTERPRET_RUNTIME_ERROR;

#undef READ_BYTE
#undef READ_CONSTANT
#undef BINARY_OP
}

InterpretResult interpret(Chunk* chunk, Value* result) {
    resetStack();
    vm.chunk = chunk;
    vm.ip = chunk->code;
    return run(result);
}

InterpretResult interpretSource(const char* source, Value* result) {
    Chunk chunk;
    initChunk(&chunk);

    if (!compile(source, &chunk)) {
        freeChunk(&chunk);
        return INTERPRET_COMPILE_ERROR;
    }

    InterpretResult status = interpret(&chunk, result);
    freeChunk(&chunk);
    return status;
}