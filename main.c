#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

//
// Tokenize
//

typedef enum {
    TK_RESERVED, // Symbol
    TK_NUM,      // Integer
    TK_EOF,      // End of the input
} TokenKind;

typedef struct Token Token;

struct Token {
    TokenKind kind; // Token Type
    Token *next;    // Next input token
    int val;        // If kind is TK_NUM, the number
    char *str;      // Token string
};

// Curren token
Token *token;

// Input program.
char *user_input;

// Reports an error and exit.
// Takes the same arguments as printf().
void error(char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// Reports an error location and exit.
void error_at(char *loc, char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);

    int pos = loc - user_input;
    fprintf(stderr, "%s\n", user_input);
    fprintf(stderr, "%*s", pos, ""); // Output pos spaces.
    fprintf(stderr, "^ ");
    vfprintf(stderr, fmt, ap);
    fprintf(stderr, "\n");
    exit(1);
}

// When the next token is the symbol you're expecting, read one token forward and
// Return true. Otherwise, return false.
bool consume(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        return false;
    }
    token = token->next;
    return true;
}

// If the next token is the symbol you are expecting, read one token forward.
// Otherwise, report an error.
void expect(char op)
{
    if (token->kind != TK_RESERVED || token->str[0] != op) {
        error_at(token->str, "expected '%d'", op);
    }
    token = token->next;
}

// If the next token is a number, read one token and return that number.
// Otherwise, report an error.
int expect_number()
{
    if (token->kind != TK_NUM) {
        error_at(token->str, "expected a number");
    }
    int val = token->val;
    token   = token->next;
    return val;
}

bool at_eof() { return token->kind == TK_EOF; }

// Create a new token and connect it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind  = kind;
    tok->str   = str;
    cur->next  = tok;
    return tok;
}

// Tokenize 'p' and return it.
Token *tokenize(char *p)
{
    Token head;
    head.next  = NULL;
    Token *cur = &head;

    while (*p) {
        // Skip the space.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Punctuator
        if (*p == '+' || *p == '-') {
            cur = new_token(TK_RESERVED, cur, p++);
            continue;
        }

        // Integer literal
        if (isdigit(*p)) {
            cur      = new_token(TK_NUM, cur, p);
            cur->val = strtol(p, &p, 10);
            continue;
        }

        error_at(p, "expected a number");
    }

    new_token(TK_EOF, cur, p);
    return head.next;
}

//
// Parser
//

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_NUM, // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
    NodeKind kind; // Node kind
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    int val;       // Used if kind == ND_NUM
};

Node *new_node(NodeKind kind, Node *lhs, Node *rhs)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    node->lhs  = lhs;
    node->rhs  = rhs;
    return node;
}

Node *new_num(int val)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_NUM;
    node->val  = val;
    return node;
}

Node *expr(void)
{
    Node *node = new_num(expect_number());

    for (;;) {
        if (consume('+'))
            node = new_node(ND_ADD, node, new_num(expect_number()));
        else if (consume('-'))
            node = new_node(ND_SUB, node, new_num(expect_number()));
        else
            return node;
    }
}

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
    }
    printf("  str x0, [sp, #-16]!\n");
}

int main(int argc, char **argv)
{
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
    }

    // Tokenize and parse.
    user_input = argv[1];
    token      = tokenize(user_input);
    Node *node = expr();

    // Print out the first half of assembly.
    printf(".globl main\n");
    printf("main:\n");

    // Traverse the AST to emit assembly.
    gen(node);

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("  ldr x0, [sp], #16\n");

    printf("  ret\n");
    return 0;
}
