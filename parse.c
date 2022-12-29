#include "boncc.h"
#include <stdlib.h>
#include <string.h>

static Vector *locals;

Token *consume(TokenKind kind) {
  if (token->kind != kind)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *expect(TokenKind kind) {
  if (token->kind != kind)
    error_at(token->str, "'%s' expected but not found", token_str[kind]);
  Token *tok = token;
  token = token->next;
  return tok;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "number expected but not found");
  int val = token->val;
  token = token->next;
  return val;
}

Type *consume_type() {
  if (!consume(TK_INT))
    return NULL;
  Type *type = calloc(1, sizeof(Type));
  Type *ty = type;
  while (consume(TK_STAR)) {
    ty->kind = TYPE_PTR;
    ty->ptr_to = calloc(1, sizeof(Type));
    ty = ty->ptr_to;
  }
  ty->kind = TYPE_INT;
  return type;
}

Type *expect_type() {
  Type *ty = consume_type();
  if (!ty)
    error_at(token->str, "type expected but not found");
  return ty;
}

bool at_eof() { return token->kind == TK_EOF; }

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_add(Node* left, Node* right) {
  Type *lt = get_type(left);
  Type *rt = get_type(right);
  if (lt->kind == TYPE_PTR || lt->kind == TYPE_ARRAY)
    right = new_node(ND_MUL, right, new_node_num(size_of(lt->ptr_to)));
  else if (rt->kind == TYPE_PTR || rt->kind == TYPE_ARRAY)
    left = new_node(ND_MUL, left, new_node_num(size_of(rt->ptr_to)));
  return new_node(ND_ADD, left, right);
}

LVar *new_lvar(Token *tok, Type *type) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->name = tok->str;
  lvar->len = tok->len;
  lvar->type = type;
  if (type->kind == TYPE_ARRAY)
    lvar->offset = size_of(type->ptr_to) * type->array_size;
  else
    lvar->offset = size_of(type);
  if (locals->size)
    lvar->offset += (*(LVar **)vector_last(locals))->offset;
  vector_push(locals, &lvar);
  return lvar;
}

LVar *find_lvar(Token *tok) {
  int sz = locals->size;
  for (int i = 0; i < sz; ++i) {
    LVar *var = *(LVar **)vector_get(locals, i);
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  }
  return NULL;
}

void program();
Node *func();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();
Node *postfix();

Node *stmt_if();
Node *stmt_while();
Node *stmt_for();
Node *stmt_block();

void program() {
  functions = new_vector(0, sizeof(Node *));
  while (!at_eof())
    func();
}

Node *func() {
  Node *node = calloc(1, sizeof(Node));
  vector_push(functions, &node);
  node->type = expect_type();
  Token *tok = expect(TK_IDENT);
  node->kind = ND_FUNC;
  node->name = tok->str;
  node->len = tok->len;
  node->locals = new_vector(0, sizeof(LVar *));
  locals = node->locals;

  expect(TK_LPAREN);
  if (!consume(TK_RPAREN)) {
    // read params
    do {
      Type *ty = expect_type();
      Token *id = expect(TK_IDENT);
      if (find_lvar(id))
        error_at(id->str, "duplicated identifier");
      new_lvar(id, ty);
    } while (consume(TK_COMMA));
    expect(TK_RPAREN);
  }
  node->nparams = node->locals->size;
  expect(TK_LBRACE);
  node->body = stmt_block();
  locals = NULL;
  return node;
}

Node *stmt() {
  Type *ty;
  if (consume(TK_LBRACE)) {
    return stmt_block();
  } else if (consume(TK_IF)) {
    return stmt_if();
  } else if (consume(TK_WHILE)) {
    return stmt_while();
  } else if (consume(TK_FOR)) {
    return stmt_for();
  } else if (consume(TK_RETURN)) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_RETURN;
    node->lhs = expr();
    expect(TK_SEMICOLON);
    return node;
  } else if ((ty = consume_type())) {
    Token *id = expect(TK_IDENT);
    if (find_lvar(id))
      error_at(id->str, "duplicated identifier");
    if (consume(TK_LBRACKET)) {
      Type *aty = calloc(1, sizeof(Type));
      aty->kind = TYPE_ARRAY;
      aty->ptr_to = ty;
      aty->array_size = (size_t)expect_number();
      new_lvar(id, aty);
      expect(TK_RBRACKET);
    } else {
      new_lvar(id, ty);
    }
    expect(TK_SEMICOLON);
    return NULL;
  } else {
    Node *node = expr();
    expect(TK_SEMICOLON);
    return node;
  }
}

Node *stmt_block() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  do {
    Node *s = stmt();
    if (s)
      vector_push(node->blk_stmts, &s);
  } while (!consume(TK_RBRACE));

  return node;
}

Node *stmt_if() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;

  expect(TK_LPAREN);
  node->condition = expr();
  expect(TK_RPAREN);
  node->body = stmt();

  if (consume(TK_ELSE))
    node->else_ = stmt();

  return node;
}

Node *stmt_while() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;

  expect(TK_LPAREN);
  node->condition = expr();
  expect(TK_RPAREN);
  node->body = stmt();

  return node;
}

Node *stmt_for() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;

  expect(TK_LPAREN);

  if (!consume(TK_SEMICOLON)) {
    node->init = expr();
    expect(TK_SEMICOLON);
  }

  if (!consume(TK_SEMICOLON)) {
    node->condition = expr();
    expect(TK_SEMICOLON);
  }

  if (!consume(TK_RPAREN)) {
    node->update = expr();
    expect(TK_RPAREN);
  }

  node->body = stmt();

  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = equality();
  if (consume(TK_ASSIGN))
    node = new_node(ND_ASSIGN, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();
  while (true) {
    if (consume(TK_EQ))
      node = new_node(ND_EQ, node, relational());
    else if (consume(TK_NE))
      node = new_node(ND_NE, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  while (true) {
    if (consume(TK_LT))
      node = new_node(ND_LT, node, add());
    else if (consume(TK_LE))
      node = new_node(ND_LE, node, add());
    if (consume(TK_GT))
      node = new_node(ND_LT, add(), node);
    else if (consume(TK_GE))
      node = new_node(ND_LE, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  while (true) {
    Token *tok = token; // for error message
    if (consume(TK_PLUS)) {
      node = new_node_add(node, mul());
    } else if (consume(TK_MINUS)) {
      Node *left = node;
      Node *right = mul();
      Type *lt = get_type(left);
      Type *rt = get_type(right);
      if (lt->kind == TYPE_PTR || lt->kind == TYPE_ARRAY)
        right = new_node(ND_MUL, right, new_node_num(size_of(lt->ptr_to)));
      else if (rt->kind == TYPE_PTR || rt->kind == TYPE_ARRAY)
        error_at(tok->str,
                 "pointer is not allowed at right-side of - operetor");
      node = new_node(ND_SUB, left, right);
    } else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  while (true) {
    if (consume(TK_STAR))
      node = new_node(ND_MUL, node, unary());
    else if (consume(TK_SLASH))
      node = new_node(ND_DIV, node, unary());
    else
      return node;
  }
}

Node *unary() {
  if (consume(TK_SIZEOF))
    return new_node_num(size_of(get_type(unary())));
  if (consume(TK_PLUS))
    return unary();
  if (consume(TK_MINUS))
    return new_node(ND_SUB, new_node_num(0), unary());
  if (consume(TK_AMP))
    return new_node(ND_ADDR, unary(), NULL);
  if (consume(TK_STAR))
    return new_node(ND_DEREF, unary(), NULL);
  return postfix();
}

Node *primary() {
  if (consume(TK_LPAREN)) {
    Node *node = expr();
    expect(TK_RPAREN);
    return node;
  }

  Token *tok = consume(TK_IDENT);
  if (tok) {
    if (consume(TK_LPAREN)) { // function call
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->name = tok->str;
      node->len = tok->len;
      node->args = new_vector(0, sizeof(Node *));

      if (consume(TK_RPAREN))
        return node;

      do {
        Node *e = expr();
        vector_push(node->args, &e);
      } while (consume(TK_COMMA));

      expect(TK_RPAREN);
      return node;
    } else { // variable
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_LVAR;

      LVar *lvar = find_lvar(tok);
      if (!lvar)
        error_at(tok->str, "undefined identifier: '%.*s'", tok->len, tok->str);
      node->lvar = lvar;
      return node;
    }
  }

  return new_node_num(expect_number());
}

Node *postfix() {
  Node *node = primary();
  while (consume(TK_LBRACKET)) {
    // x[y] --> *(x+y)
    Node *y = expr();
    expect(TK_RBRACKET);
    node = new_node(ND_DEREF, new_node_add(node, y), NULL);
  }
  return node;
}
