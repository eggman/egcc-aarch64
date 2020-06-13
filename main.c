#include <stdio.h>

#include "egcc.h"

// Current token
Token *token;

// Input program.
char *user_input;

int main(int argc, char **argv)
{
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
    }

    // Tokenize and parse.
    user_input = argv[1];
    token      = tokenize(user_input);
    // debug_print_token(token);
    int stack_size = program();

    // Print out the first half of assembly.
    printf(".globl main\n");
    printf("main:\n");

    // Prologue
    // reserve stack for variables
    printf("  str x5, [sp, #-8]!\n"); // push
    printf("  mov x5, sp\n");         // stroe var base pointer
    printf("  sub sp, sp, #%d\n", stack_size);

    // Code generation from the first expression
    for (int i = 0; code[i]; i++) {
        gen(code[i]);

        // There should be one value left on the stack as a result of the evaluation of the
        // expression, so I'll pop the stack so it doesn't overflow.
        printf("  ldr x0, [sp], #8\n"); // pop
    }

    // Epilogue
    // The result of the last expression is still in x0 and that is the return value.
    printf("  mov sp, x5\n");
    printf("  ldr x5, [sp], #8\n"); // pop
    printf("  ret\n");

    return 0;
}
