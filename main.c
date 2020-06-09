#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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
    int len;        // Token length
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
bool consume(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        return false;
    }
    token = token->next;
    return true;
}

// If the next token is the symbol you are expecting, read one token forward.
// Otherwise, report an error.
void expect(char *op)
{
    if (token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "expected \"%s\" token->kind=%d strlen(op)=%d token->len=%d\n",
                 token->kind, op, strlen(op), token->len);
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

// Create a new token and connect it to cur.
Token *new_token(TokenKind kind, Token *cur, char *str, int len)
{
    Token *tok = calloc(1, sizeof(Token));
    tok->kind  = kind;
    tok->str   = str;
    tok->len   = len;
    cur->next  = tok;
    return tok;
}

bool startswith(char *p, char *q)
{
    return memcmp(p, q, strlen(q)) == 0;
}

// Tokenize 'p' and return it.
Token *tokenize(char *p)
{
    Token head;
    head.next  = NULL;
    Token *cur = &head;

    while (*p) {

        if (startswith(p, "==") || startswith(p, "!=") || startswith(p, "<=") ||
            startswith(p, ">=")) {
            cur = new_token(TK_RESERVED, cur, p, 2);
            p += 2;
            continue;
        }

        // Skip the space.
        if (isspace(*p)) {
            p++;
            continue;
        }

        // Punctuator
        if (strchr("+-*/()<>", *p)) {
            cur = new_token(TK_RESERVED, cur, p++, 1);
            continue;
        }

        // Integer literal
        if (isdigit(*p)) {
            cur      = new_token(TK_NUM, cur, p, 1);
            char *q  = p;
            cur->val = strtol(p, &p, 10);
            cur->len = p - q;
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
    return head.next;
}

void debug_print_token(Token *t)
{
    char tmp[16];
    do {
        memset(tmp, 0, 16);
        memcpy(tmp, t->str, t->len);
        fprintf(stderr, "token->kind=%d token->len=%d token->str\"%s\"\n", t->kind, t->len, tmp);
    } while (t = t->next);
}

//
// Parser
//

typedef enum {
    ND_ADD, // +
    ND_SUB, // -
    ND_MUL, // *
    ND_DIV, // /
    ND_EQ,  // ==
    ND_NE,  // !=
    ND_LT,  // <
    ND_LE,  // <=
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

Node *expr(void);

// primary = "(" expr ")" | num
Node *primary(void)
{
    // If next is '(' ,then '(' expr ')'.
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    // If otherwise then number.
    return new_num(expect_number());
}

// unary = ("+" | "-")? primary
Node *unary(void)
{
    if (consume("+"))
        return primary();
    if (consume("-"))
        return new_node(ND_SUB, new_num(0), primary());
    return primary();
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(void)
{
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_node(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_node(ND_DIV, node, unary());
        else
            return node;
    }
}

// add  = mul ("+" mul | "-" mul)*
Node *add(void)
{
    Node *node = mul();

    for (;;) {
        if (consume("+"))
            node = new_node(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_node(ND_SUB, node, mul());
        else
            return node;
    }
}

// relational = add ("<" add | "<=" add | ">" add | ">=" add)*
Node *relational(void)
{
    Node *node = add();
    for (;;) {
        if (consume("<=")) {
            node = new_node(ND_LE, node, add());
        } else if (consume(">=")) {
            node = new_node(ND_LE, add(), node);
        } else if (consume("<")) {
            node = new_node(ND_LT, node, add());
        } else if (consume(">")) {
            node = new_node(ND_LT, add(), node);
        } else {
            return node;
        }
    }
}

// equality = relational ("==" relational | "!=" relational)*
Node *equality(void)
{
    Node *node = relational();
    for (;;) {
        if (consume("==")) {
            node = new_node(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_node(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// expr = equality
Node *expr(void)
{
    Node *node = equality();
}

//
// Code generator
//

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

    // A result must be at the top of the stack, so pop it
    // to RAX to make it a program exit code.
    printf("  ldr x0, [sp], #16\n");

    printf("  ret\n");
    return 0;
}
