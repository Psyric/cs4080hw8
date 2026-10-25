#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "vm.h"

static void assertExpression(const char* source, double expected) {
    Value result = 0;
    assert(interpretSource(source, &result) == INTERPRET_OK);
    assert(result > expected - 0.000001 && result < expected + 0.000001);
}

static void testChallenge1(void) {
    assertExpression("1 + 2 * 3", 7.0);
    assertExpression("(1 + 2) * 3", 9.0);
    assertExpression("-1 + 2 * 3", 5.0);
    assertExpression("(-1 + 2) * 3 - -4", 7.0);
    puts("Challenge 1 passed: precedence and unary handling work for arithmetic expressions.");
}

static void testChallenge2(void) {
    assertExpression("(2 + 3) * 4", 20.0);
    assertExpression("2 + 3 * 4", 14.0);
    puts("Challenge 2 passed: equivalent arithmetic expressions evaluate as expected.");
}

int main(int argc, char** argv) {
    initVM();

    if (argc == 1 || (argc == 2 && strcmp(argv[1], "--all") == 0)) {
        testChallenge1();
        testChallenge2();
    } else if (argc == 2 && strcmp(argv[1], "--challenge1") == 0) {
        testChallenge1();
    } else if (argc == 2 && strcmp(argv[1], "--challenge2") == 0) {
        testChallenge2();
    } else {
        fprintf(stderr, "Usage: %s [--all|--challenge1|--challenge2]\n", argv[0]);
        freeVM();
        return EXIT_FAILURE;
    }

    freeVM();
    return EXIT_SUCCESS;
}
