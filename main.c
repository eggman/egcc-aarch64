#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is not correct.\n");
        return 1;
    }

    printf(".globl main\n");
    printf("main:\n");
    printf("  mov x0, #%d\n", atoi(argv[1]));
    printf("  ret\n");
    return 0;
}
