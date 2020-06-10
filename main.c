#include <stdio.h>

#include "egcc.h"

// Curren token
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
    Node *node = expr();

    // Print out the first half of assembly.
    printf(".globl main\n");
    printf("main:\n");

    // Traverse the AST to emit assembly.
    gen(node);

    // A result must be at the top of the stack,
    // so pop it to x0 to make it a program exit code.
    printf("  ldr x0, [sp], #16\n");

    printf("  ret\n");
    return 0;
}
