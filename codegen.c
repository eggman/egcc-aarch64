#include <stdio.h>

#include "egcc.h"

// aarch64 registers
//
// sp              stack pointer
// r30   x30   lr  link register
// r29   x29   fp  frame pointer
// r0-r7 x0-x7     parameter/result registers

static int labelseq   = 1;
static char *argreg[] = {"x0", "x1", "x2", "x3", "x4", "x5", "x6", "x7"};

void gen_lval(Node *node)
{
    if (node->kind != ND_LVAR) {
        error("not assign,  because lhs is not var.");
    }

    printf("  mov x0, fp\n");                    // get frame pointer
    printf("  sub x0, x0, #%d\n", node->offset); // compute var address
    printf("  str x0, [sp, #-8]!\n");            // push
}

void gen(Node *node)
{
    switch (node->kind) {
    case ND_IF: {
        int seq = labelseq++;
        if (node->els) {
            gen(node->cond);
            printf("  ldr x0, [sp], #8\n"); // pop
            printf("  cmp x0, #0\n");
            printf("  beq  .L.else.%d\n", seq);
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
    case ND_WHILE: {
        int seq = labelseq++;
        printf(".L.begin.%d:\n", seq);
        gen(node->cond);
        printf("  ldr x0, [sp], #8\n"); // pop
        printf("  cmp x0, #0\n");
        printf("  beq  .L.end.%d\n", seq);
        gen(node->then);
        printf("  b .L.begin.%d\n", seq);
        printf(".L.end.%d:\n", seq);
        return;
    }
    case ND_FOR: {
        int seq = labelseq++;
        if (node->init) {
            gen(node->init);
        }
        printf(".L.begin.%d:\n", seq);
        if (node->cond) {
            gen(node->cond);
            printf("  ldr x0, [sp], #8\n"); // pop
            printf("  cmp x0, #0\n");
            printf("  beq  .L.end.%d\n", seq);
        }
        gen(node->then);
        if (node->inc) {
            gen(node->inc);
        }
        printf("  b .L.begin.%d\n", seq);
        printf(".L.end.%d:\n", seq);
        return;
    }
    case ND_BLOCK: {
        for (Node *n = node->body; n; n = n->next) {
            gen(n);
        }
        return;
    }
    case ND_FUNCALL: {
        int nargs = 0;
        for (Node *arg = node->args; arg; arg = arg->next) {
            gen(arg);
            nargs++;
        }

        for (int i = nargs - 1; i >= 0; i--) {
            printf("  ldr %s, [sp], #8\n", argreg[i]); // pop arg
        }

        printf("  str lr, [sp, #-8]!\n");     // push link register
        printf("  bl  %s\n", node->funcname); //
        printf("  ldr lr, [sp], #8\n");       // pop link register
        printf("  str x0, [sp, #-8]!\n");     // push result
        return;
    }
    case ND_RETURN:
        gen(node->lhs);
        printf("  ldr x0, [sp], #8\n"); // pop result
        printf("  b .L.return\n");
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

void codegen(int stack_size)
{
    // Print out the first half of assembly.
    printf(".globl main\n");
    printf("main:\n");

    // Prologue
    // reserve stack for variables
    printf("  str fp, [sp, #-8]!\n"); // push
    printf("  mov fp, sp\n");         // stroe sp to frame pointer
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
    printf(".L.return:\n");
    printf("  mov sp, fp\n");
    printf("  ldr fp, [sp], #8\n"); // pop
    printf("  ret\n");
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
