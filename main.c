#include <stdio.h>
#include <stdlib.h>

int main(int argc, char **argv)
{
    if (argc != 2) {
        fprintf(stderr, "The number of arguments is not correct.\n");
        return 1;
    }

    char *p = argv[1];

    printf(".globl main\n");
    printf("main:\n");
    printf("  mov x0, #%ld\n", strtol(p, &p, 10));

    while (*p) {
        if (*p == '+') {
            p++;
            printf("  add x0, x0, #%ld\n", strtol(p, &p, 10));
            continue;
        }

        if (*p == '-') {
            p++;
            printf("  sub x0, x0, #%ld\n", strtol(p, &p, 10));
            continue;
        }

        fprintf(stderr, "unexpected character: '%c'\n", *p);
        return 1;
    }

    printf("  ret\n");
    return 0;
}
