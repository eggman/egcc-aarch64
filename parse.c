#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "egcc.h"

// aarch64's  sring.h does not define strndup
char *strndup(const char *s, size_t n);

Node *code[100];

typedef struct LVar LVar;

// Type of local var
struct LVar {
    LVar *next; // next lvar or NULL
    char *name; // Name of lvar
    int len;    // Lengh of lvar
    int offset; // offset from base pointer
};

// Local vars
LVar *locals;

Node *new_node(NodeKind kind)
{
    Node *node = calloc(1, sizeof(Node));
    node->kind = kind;
    return node;
}

Node *new_binary(NodeKind kind, Node *lhs, Node *rhs)
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

// Searches for a variable by name. If the variable is not found, it returns NULL.
LVar *find_lvar(Token *tok)
{
    for (LVar *var = locals; var; var = var->next)
        if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
            return var;
    return NULL;
}

LVar *new_lvar(Token *tok)
{
    LVar *lvar   = calloc(1, sizeof(LVar));
    lvar->next   = locals;
    lvar->name   = tok->str;
    lvar->len    = tok->len;
    lvar->offset = locals->offset + 8;
    return lvar;
}

// func-args = "(" (assign ("," assign)*)? ")"
static Node *func_args(void)
{
    if (consume(")")) {
        return NULL;
    }

    Node *head = assign();
    Node *cur  = head;
    while (consume(",")) {
        cur->next = assign();
        cur       = cur->next;
    }
    expect(")");
    return head;
}

// primary = num
//         | func-args?
//         | "(" expr ")"
Node *primary(void)
{
    // If next is '(' ,then '(' expr ')'.
    if (consume("(")) {
        Node *node = expr();
        expect(")");
        return node;
    }

    Token *tok = consume_ident();
    if (tok) {

        // Function call
        if (consume("(")) {
            Node *node     = new_node(ND_FUNCALL);
            node->funcname = strndup(tok->str, tok->len);
            node->args     = func_args();
            return node;
        }

        Node *node = new_node(ND_LVAR);
        LVar *lvar = find_lvar(tok);
        if (lvar) {
            node->offset = lvar->offset;
        } else {
            lvar         = new_lvar(tok);
            node->offset = lvar->offset;
            locals       = lvar;
        }
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
        return new_binary(ND_SUB, new_num(0), primary());
    return primary();
}

// mul = unary ("*" unary | "/" unary)*
Node *mul(void)
{
    Node *node = unary();

    for (;;) {
        if (consume("*"))
            node = new_binary(ND_MUL, node, unary());
        else if (consume("/"))
            node = new_binary(ND_DIV, node, unary());
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
            node = new_binary(ND_ADD, node, mul());
        else if (consume("-"))
            node = new_binary(ND_SUB, node, mul());
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
            node = new_binary(ND_LE, node, add());
        } else if (consume(">=")) {
            node = new_binary(ND_LE, add(), node);
        } else if (consume("<")) {
            node = new_binary(ND_LT, node, add());
        } else if (consume(">")) {
            node = new_binary(ND_LT, add(), node);
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
            node = new_binary(ND_EQ, node, relational());
        } else if (consume("!=")) {
            node = new_binary(ND_NE, node, relational());
        } else {
            return node;
        }
    }
}

// assign = equality ("=" assign)?
Node *assign(void)
{
    Node *node = equality();

    if (consume("=")) {
        node = new_binary(ND_ASSIGN, node, assign());
    }
    return node;
}

// expr  = assign
Node *expr(void)
{
    Node *node = assign();
}

// stmt = expr ";"
//      | "return" expr ";"
//      | "if" "(" expr ")" stmt ("else" stmt)?
//      | "while" "(" expr ")" stmt
//      | "for" "(" expr? ";" expr? ";"  expr? ")" stmt
//      | "{" stmt* "}"
Node *stmt(void)
{
    Node *node;

    if (consume("return")) {
        node      = new_node(ND_RETURN);
        node->lhs = expr();
        expect(";");
        return node;
    }

    if (consume("if")) {
        node = new_node(ND_IF);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        if (consume("else"))
            node->els = stmt();
        return node;
    }

    if (consume("while")) {
        node = new_node(ND_WHILE);
        expect("(");
        node->cond = expr();
        expect(")");
        node->then = stmt();
        return node;
    }

    if (consume("for")) {
        node = new_node(ND_FOR);
        expect("(");
        if (!consume(";")) {
            node->init = expr();
            expect(";");
        }
        if (!consume(";")) {
            node->cond = expr();
            expect(";");
        }
        if (!consume(")")) {
            node->inc = expr();
            expect(")");
        }
        node->then = stmt();
        return node;
    }

    if (consume("{")) {
        Node head = {};
        Node *cur = &head;

        while (!consume("}")) {
            cur->next = stmt();
            cur       = cur->next;
        }

        node = new_node(ND_BLOCK);

        node->body = head.next;
        return node;
    }

    node = expr();
    expect(";");
    return node;
}

// program = stmt*
int program(void)
{
    int i      = 0;
    LVar lhead = {0};
    locals     = &lhead;

    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;

    return locals->offset;
}
