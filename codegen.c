#include <stdio.h>

#include "egcc.h"

static int labelseq = 1;

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR) {
        error("not assign,  because lhs is not var.");
    }

    printf("  mov x0, x5\n");                    // get base pointer
    printf("  sub x0, x0, #%d\n", node->offset); // compute var address
    printf("  str x0, [sp, #-8]!\n");            // push
}

void gen(Node *node)
{
    switch (node->kind) {
    case ND_IF: {
        fprintf(stderr, "1 \n");
        int seq = labelseq++;
        if (node->els) {
            gen(node->cond);
            printf("  ldr x0, [sp], #8\n"); // pop
            printf("  cmp x0, #0\n");
            printf("  jeq  .L.else.%d\n", seq);
            gen(node->then);
            printf("  b .L.end.%d\n", seq);
            printf(".L.else.%d:\n", seq);
            gen(node->els);
            printf(".L.end.%d:\n", seq);
        } else {
            gen(node->cond);
            printf("  ldr x0, [sp], #8\n"); // pop
            printf("  cmp x0, #0\n");
            printf("  beq  .L.end.%d\n", seq);
            gen(node->then);
            printf(".L.end.%d:\n", seq);
        }
        return;
    }
    case ND_RETURN:
        gen(node->lhs);
        printf("  ldr x0, [sp], #8\n"); // pop result
        printf("  mov sp, x5\n");
        printf("  ldr x5, [sp], #8\n"); // pop
        printf("  ret\n");
        return;
    case ND_NUM:
        printf("  mov x0, #%d\n", node->val);
        printf("  str x0, [sp, #-8]!\n"); // push result
        return;
    case ND_LVAR:
        gen_lval(node);
        printf("  ldr x1, [sp], #8\n");   // pop var address
        printf("  ldr x0, [x1]\n");       // load var
        printf("  str x0, [sp, #-8]!\n"); // push result
        return;
    case ND_ASSIGN:
        gen_lval(node->lhs);
        gen(node->rhs);
        printf("  ldr x1, [sp], #8\n");   // pop rhs
        printf("  ldr x0, [sp], #8\n");   // pop var address
        printf("  str x1, [x0]\n");       // assign
        printf("  str x1, [sp, #-8]!\n"); // push result
        return;
    }

    gen(node->lhs);
    gen(node->rhs);

    printf("  ldr x1, [sp], #8\n"); // pop rhs
    printf("  ldr x0, [sp], #8\n"); // pop lhs

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
    printf("  str x0, [sp, #-8]!\n"); // push result
}

void debug_print_node(Node *n)
{
    fprintf(stderr, "node->kind=%d\n", n->kind);
    switch (n->kind) {
    case ND_NUM:
        fprintf(stderr, "num val=%d\n", n->val);
        break;
    case ND_LVAR:
        fprintf(stderr, "lvar offset=%d\n", n->offset);
        break;
    case ND_ASSIGN:
        fprintf(stderr, "lhs kind=%d\n", n->lhs->kind);
        fprintf(stderr, "rhs kind=%d\n", n->rhs->kind);
        break;
    }
}
