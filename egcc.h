#include <stdbool.h>

//
// tokenize.c
//

typedef enum {
    TK_RESERVED, // Symbol
    TK_IDENT,    // Identifier
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

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);
bool consume(char *op);
Token *consume_ident(void);
void expect(char *op);
int expect_number(void);
bool at_eof(void);
Token *tokenize(char *p);
void debug_print_token(Token *t);

extern char *user_input;
extern Token *token;

//
// parse.c
//

typedef enum {
    ND_ADD,    // +
    ND_SUB,    // -
    ND_MUL,    // *
    ND_DIV,    // /
    ND_EQ,     // ==
    ND_NE,     // !=
    ND_LT,     // <
    ND_LE,     // <=
    ND_ASSIGN, // =
    ND_RETURN, // "return"
    ND_IF,     // "if"
    ND_LVAR,   // Local var
    ND_NUM,    // Integer
} NodeKind;

// AST node type
typedef struct Node Node;
struct Node {
    NodeKind kind; // Node kind
    Node *lhs;     // Left-hand side
    Node *rhs;     // Right-hand side
    Node *cond;    // if conditoin block
    Node *then;    // if then block
    Node *els;     // if else block
    int val;       // Used if kind == ND_NUM
    int offset;    // Used if kind == ND_LVAR
};

Node *expr(void);
Node *stmt(void);
int program(void);
void debug_print_node(Node *n);

extern Node *code[100];

//
// codegen.c
//

void gen(Node *node);
