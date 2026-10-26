#include <stdio.h>
#include <time.h>

#include "chunk.h"
#include "vm.h"

#define NEGATE_COUNT 10000
#define ITERATIONS 2000

int main(void) {
    Chunk chunk;
    initChunk(&chunk);
    writeConstant(&chunk, 1.0, 1);
    for (int index = 0; index < NEGATE_COUNT; index++) {
        writeChunk(&chunk, OP_NEGATE, 1);
    }
    writeChunk(&chunk, OP_RETURN, 1);

    initVM();
    Value result = 0;
    clock_t start = clock();
    for (int iteration = 0; iteration < ITERATIONS; iteration++) {
        if (interpret(&chunk, &result) != INTERPRET_OK) {
            freeVM();
            freeChunk(&chunk);
            return 1;
        }
    }
    clock_t elapsed = clock() - start;

#ifdef VM_BENCHMARK_POP_PUSH
    puts("Mode: pop/negate/push");
#else
    puts("Mode: in-place");
#endif
    printf("Elapsed CPU time: %.6f seconds; result: %.0f\n",
           (double)elapsed / CLOCKS_PER_SEC, result);

    freeVM();
    freeChunk(&chunk);
    return 0;
}