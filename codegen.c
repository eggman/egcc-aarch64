#include <stdio.h>

#include "egcc.h"

void gen(Node *node)
{
    if (node->kind == ND_NUM) {
        printf("  mov x0, #%d\n", node->val);
        printf("  str x0, [sp, #-16]!\n");
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  ldr x1, [sp], #16\n");
    printf("  ldr x0, [sp], #16\n");

    switch (node->kind) {
    case ND_ADD:
        printf("  add x0, x0, x1\n");
        break;
    case ND_SUB:
        printf("  sub x0, x0, x1\n");
        break;
    case ND_MUL:
        printf("  mul x0, x0, x1\n");
        break;
    case ND_DIV:
        printf("  udiv x0, x0, x1\n");
        break;
    case ND_EQ:
        printf("  cmp x0, x1\n");
        printf("  cset x0, eq\n");
        break;
    case ND_NE:
        printf("  cmp x0, x1\n");
        printf("  cset x0, ne\n");
        break;
    case ND_LT:
        printf("  cmp x0, x1\n");
        printf("  cset x0, lt\n");
        break;
    case ND_LE:
        printf("  cmp x0, x1\n");
        printf("  cset x0, le\n");
        break;
    }
    printf("  str x0, [sp, #-16]!\n");
}
