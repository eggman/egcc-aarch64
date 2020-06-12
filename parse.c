#include <stdio.h>
#include <stdlib.h>

#include "egcc.h"

Node *code[100];

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

Node *new_lvar_node(Token *tok)
{
    Node *node   = calloc(1, sizeof(Node));
    node->kind   = ND_LVAR;
    node->offset = (tok->str[0] - 'a' + 1) * 8;
    return node;
}

// primary = num | ident | "(" expr ")"
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
        return new_lvar_node(tok);
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

// assign = equality ("=" assign)?
Node *assign(void)
{
    Node *node = equality();

    if (consume("=")) {
        node = new_node(ND_ASSIGN, node, assign());
    }
    return node;
}

// expr  = assign
Node *expr(void)
{
    Node *node = assign();
}

// stmt = expr ";"
Node *stmt(void)
{
    Node *node = expr();

    expect(";");
    return node;
}

// program = stmt*
void program(void)
{
    int i = 0;
    while (!at_eof()) {
        code[i++] = stmt();
    }
    code[i] = NULL;
}
