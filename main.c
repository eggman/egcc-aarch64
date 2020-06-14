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

    // Tokenize, parse and codegen.
    user_input = argv[1];
    token      = tokenize(user_input);
    // debug_print_token(token);
    int stack_size = program();
    codegen(stack_size);

    return 0;
}
