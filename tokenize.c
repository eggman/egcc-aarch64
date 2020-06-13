#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egcc.h"

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

int is_alnum(char c)
{
    return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || (c == '_');
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

Token *consume_ident(void)
{
    if (token->kind != TK_IDENT) {
        return NULL;
    }
    Token *t = token;
    token    = token->next;
    return t;
}

// If the next token is the symbol you are expecting, read one token forward.
// Otherwise, report an error.
void expect(char *op)
{
    if (token == NULL || token->kind != TK_RESERVED || strlen(op) != token->len ||
        memcmp(token->str, op, token->len)) {
        error_at(token->str, "expected \"%s\" token->kind=%d strlen(op)=%d token->len=%d\n", op,
                 token->kind, strlen(op), token->len);
    }
    token = token->next;
}

// If the next token is a number, read one token and return that number.
// Otherwise, report an error.
int expect_number(void)
{
    if (token->kind != TK_NUM) {
        error_at(token->str, "expected a number");
    }
    int val = token->val;
    token   = token->next;
    return val;
}

bool at_eof()
{
    return token->kind == TK_EOF;
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

// Consumes the current token if it matches `op`.
bool equal(Token *tok, char *op)
{
    return strlen(op) == tok->len && !strncmp(tok->str, op, tok->len);
}

static bool is_keyword(Token *tok)
{
    static char *kw[] = {"return", "if", "else", "while"};

    for (int i = 0; i < sizeof(kw) / sizeof(*kw); i++) {
        if (equal(tok, kw[i])) {
            return true;
        }
    }
    return false;
}

static void convert_keywords(Token *tok)
{
    for (Token *t = tok; t->kind != TK_EOF; t = t->next)
        if (t->kind == TK_IDENT && is_keyword(t)) {
            t->kind = TK_RESERVED;
        }
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
        if (strchr("+-*/()<>=;", *p)) {
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

        // Identifier
        if (isalpha(*p)) {
            char *q = p++;
            while (is_alnum(*p)) {
                p++;
            }
            cur = new_token(TK_IDENT, cur, q, p - q);
            continue;
        }

        error_at(p, "invalid token");
    }

    new_token(TK_EOF, cur, p, 0);
    convert_keywords(&head);
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
