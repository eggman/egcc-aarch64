#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>

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

int main(int argc, char **argv)
{
    if (argc != 2) {
        error("%s: invalid number of arguments", argv[0]);
        return 1;
    }

    user_input = argv[1];
    token      = tokenize(user_input);

    printf(".globl main\n");
    printf("main:\n");

    // The first token must be a number
    printf("  mov x0, #%d\n", expect_number());

    // ... followed by either `+ <number>` or `- <number>`
    while (!at_eof()) {
        if (consume('+')) {
            printf("  add x0, x0, #%d\n", expect_number());
            continue;
        }

        expect('-');
        printf("  sub x0, x0, #%d\n", expect_number());
    }

    printf("  ret\n");
    return 0;
}
