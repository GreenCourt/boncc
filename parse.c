#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct Scope Scope;
struct Scope {
  Scope *prev;
  Vector *local_variables;
};
static Scope *current_scope;
static int offset; // for local variables

void new_scope() {
  Scope *s = calloc(1, sizeof(Scope));
  s->prev = current_scope;
  s->local_variables = new_vector(0, sizeof(Variable *));
  current_scope = s;
}

void restore_scope() {
  assert(current_scope);
  current_scope = current_scope->prev;
}

Token *consume(TokenKind kind) {
  if (token->kind != kind)
    return NULL;
  Token *tok = token;
  token = token->next;
  return tok;
}

Token *expect(TokenKind kind) {
  if (token->kind != kind)
    error_at(token->pos, "'%s' expected but not found", token_str[kind]);
  Token *tok = token;
  token = token->next;
  return tok;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->pos, "number expected but not found");
  int val = token->val;
  token = token->next;
  return val;
}

Type *consume_type() {
  TypeKind kind;
  if (consume(TK_INT))
    kind = TYPE_INT;
  else if (consume(TK_CHAR))
    kind = TYPE_CHAR;
  else
    return NULL;
  Type *type = base_type(kind);
  while (consume(TK_STAR))
    type = pointer_type(type);
  return type;
}

Type *consume_array_brackets(Type *type) {
  if (!consume(TK_LBRACKET))
    return type;
  int size = expect_number();
  expect(TK_RBRACKET);
  return array_type(consume_array_brackets(type), size);
}

Type *expect_type() {
  Type *ty = consume_type();
  if (!ty)
    error_at(token->pos, "type expected but not found");
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

Node *new_node_add(Node *left, Node *right) {
  Type *lt = get_type(left);
  Type *rt = get_type(right);
  bool left_is_ptr = lt->kind == TYPE_PTR || lt->kind == TYPE_ARRAY;
  bool right_is_ptr = rt->kind == TYPE_PTR || rt->kind == TYPE_ARRAY;

  if (left_is_ptr && right_is_ptr)
    error("invalid operands to binary + operator (pointer and pointer)");
  if (left_is_ptr)
    right = new_node(ND_MUL, right, new_node_num(lt->base->size));
  else if (right_is_ptr)
    left = new_node(ND_MUL, left, new_node_num(rt->base->size));
  return new_node(ND_ADD, left, right);
}

Variable *find_local_in_scope(Token *tok, Scope *scope) {
  Vector *locals = scope->local_variables;
  int sz = locals->size;
  for (int i = 0; i < sz; ++i) {
    Variable *var = *(Variable **)vector_get(locals, i);
    if (var->name_length == tok->token_length &&
        !memcmp(tok->pos, var->name, var->name_length))
      return var;
  }
  return NULL;
}

Variable *find_local(Token *tok) {
  Scope *scope = current_scope;
  while (scope) {
    Variable *var = find_local_in_scope(tok, scope);
    if (var)
      return var;
    scope = scope->prev;
  }
  return NULL;
}

Variable *find_global(Token *tok) {
  int sz = globals->size;
  for (int i = 0; i < sz; ++i) {
    Variable *var = *(Variable **)vector_get(globals, i);
    if (var->name_length == tok->token_length &&
        !memcmp(tok->pos, var->name, var->name_length))
      return var;
  }
  return NULL;
}

Variable *find_variable(Token *tok) {
  Variable *var = find_local(tok);
  if (var)
    return var;
  return find_global(tok);
}

Variable *new_variable(Token *tok, Type *type, VariableKind kind) {
  Variable *var = calloc(1, sizeof(Variable));
  var->name = tok->pos;
  var->name_length = tok->token_length;
  var->type = type;
  var->kind = kind;
  return var;
}

Variable *new_local(Token *tok, Type *type) {
  if (find_local_in_scope(tok, current_scope))
    error_at(tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_LOCAL);
  var->offset = offset + type->size;
  offset = var->offset;
  vector_push(current_scope->local_variables, &var);
  return var;
}

Variable *new_global(Token *tok, Type *type) {
  if (find_global(tok))
    error_at(tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_GLOBAL);
  vector_push(globals, &var);
  return var;
}

Variable *new_string_literal(char *literal) {
  static int idx = 0;
  Variable *var = calloc(1, sizeof(Variable));
  var->name = calloc(15, sizeof(char));
  sprintf(var->name, ".LC%d", idx++);
  var->name_length = strlen(var->name);
  var->type = array_type(base_type(TYPE_CHAR), strlen(literal) + 1);
  var->kind = VK_STRLIT;
  var->string_literal = literal;
  vector_push(strings, &var);
  return var;
}

void program();
void toplevel();
void func(Type *type, Token *name);
void funcparam(Vector *params);
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
  globals = new_vector(0, sizeof(Variable *));
  strings = new_vector(0, sizeof(Variable *));
  while (!at_eof())
    toplevel();
}

void toplevel() {
  Type *type = expect_type();
  Token *name = expect(TK_IDENT);
  if (token->kind == TK_LPAREN) {
    func(type, name);
  } else {
    type = consume_array_brackets(type);
    new_global(name, type);
    expect(TK_SEMICOLON);
  }
}

void func(Type *type, Token *name) {
  Node *node = calloc(1, sizeof(Node));
  vector_push(functions, &node);
  node->type = type;
  node->kind = ND_FUNC;
  node->name = name->pos;
  node->name_length = name->token_length;
  node->params = new_vector(0, sizeof(Variable *));
  assert(offset == 0);
  assert(current_scope == NULL);
  new_scope();
  expect(TK_LPAREN);
  if (!consume(TK_RPAREN)) {
    funcparam(node->params);
    expect(TK_RPAREN);
  }
  expect(TK_LBRACE);
  node->body = stmt_block();
  node->offset = offset;
  restore_scope();
  offset = 0;
}

void funcparam(Vector *params) {
  do {
    Type *ty = expect_type();
    Token *id = expect(TK_IDENT);
    while (consume(TK_LBRACKET)) {
      consume(TK_NUM);       // currently not used
      ty = pointer_type(ty); // array as a pointer
      expect(TK_RBRACKET);
    }
    Variable *var = new_local(id, ty);
    vector_push(params, &var);
  } while (consume(TK_COMMA));
}

Node *stmt() {
  Type *ty;
  if (consume(TK_LBRACE)) {
    new_scope();
    Node *node = stmt_block();
    restore_scope();
    return node;
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
    ty = consume_array_brackets(ty);
    new_local(id, ty);
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
  new_scope();
  node->body = stmt();
  restore_scope();

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
  new_scope();
  node->body = stmt();
  restore_scope();

  return node;
}

Node *stmt_for() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;

  expect(TK_LPAREN);

  new_scope();
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
  restore_scope();

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

      bool left_is_ptr = lt->kind == TYPE_PTR || lt->kind == TYPE_ARRAY;
      bool right_is_ptr = rt->kind == TYPE_PTR || rt->kind == TYPE_ARRAY;
      if (left_is_ptr && !right_is_ptr)
        right = new_node(ND_MUL, right, new_node_num(lt->base->size));
      else if (!left_is_ptr && right_is_ptr)
        error_at(tok->pos, "pointer is not allowed here");
      node = new_node(ND_SUB, left, right);
      if (left_is_ptr && right_is_ptr)
        node = new_node(ND_DIV, node, new_node_num(lt->base->size));
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
    return new_node_num(get_type(unary())->size);
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

  Token *tok;
  if ((tok = consume(TK_IDENT))) {
    if (consume(TK_LPAREN)) { // function call
      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_CALL;
      node->name = tok->pos;
      node->name_length = tok->token_length;
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
      node->kind = ND_VAR;
      node->variable = find_variable(tok);
      if (!node->variable)
        error_at(tok->pos, "undefined identifier: '%.*s'", tok->token_length,
                 tok->pos);
      return node;
    }
  }

  if ((tok = consume(TK_STR))) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_VAR;
    node->variable = new_string_literal(tok->string_literal);
    return node;
  }

  if ((tok = consume(TK_NUM)))
    return new_node_num(tok->val);

  error_at(token->pos, "primary expected but not found", token->token_length,
           token->pos);
  return NULL;
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
