#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* BNF
program    = toplevel*
toplevel   = func | global
type       = "int" "*"*
global     = type ident ("[" num "]")* ("=" varinit)? ";"
varinit    = expr
             | "{" varinit ("," varinit)* "}"
func       = type ident "(" funcparam? ")" "{" stmt* "}"
funcparam  = type ident ("[" num? "]")* ("," type ident ("[" num? "]")* )*
stmt       = expr ";"
             | type ident ("[" num "]")* ("=" varinit)? ";"
             | "{" stmt* "}"
             | "if" "(" expr ")" stmt ("else" stmt)?
             | "while" "(" expr ")" stmt
             | "for" "(" expr? ";" expr? ";" expr? ")" stmt
             | "return" expr ";"
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary)*
unary      = "sizeof"? unary
             | "+"? unary
             | "-"? unary
             | "*"? unary
             | "&"? unary
             | postfix
postfix    = primary ("[" expr "]")*
primary    = "(" expr ")"
             | ident ("(" (expr ("," expr)*)? ")")?
             | ident
             | num
             | str
*/

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

Variable *new_variable(Token *tok, Type *type, VariableKind kind,
                       VariableInit *init) {
  Variable *var = calloc(1, sizeof(Variable));
  var->name = tok->pos;
  var->name_length = tok->token_length;
  var->type = type;
  var->kind = kind;
  var->init = init;
  return var;
}

Variable *new_local(Token *tok, Type *type, VariableInit *init) {
  if (find_local_in_scope(tok, current_scope))
    error_at(tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_LOCAL, init);
  var->offset = offset + type->size;
  offset = var->offset;
  vector_push(current_scope->local_variables, &var);
  return var;
}

Variable *new_global(Token *tok, Type *type, VariableInit *init) {
  if (find_global(tok))
    error_at(tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_GLOBAL, init);
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

Function *find_function(Token *tok) {
  for (int i = 0; i < functions->size; i++) {
    Function *f = (*(Node **)vector_get(functions, i))->func;
    if (strncmp(tok->pos, f->name, f->name_length) == 0)
      return f;
  }
  return NULL;
}

Node *new_node_num(Token *tok, int val);
Node *new_node_mul(Token *tok, Node *lhs, Node *rhs);
Node *new_node_div(Token *tok, Node *lhs, Node *rhs);
Node *new_node_add(Token *tok, Node *lhs, Node *rhs);
Node *new_node_sub(Token *tok, Node *lhs, Node *rhs);
Node *new_node_eq(Token *tok, Node *lhs, Node *rhs);
Node *new_node_ne(Token *tok, Node *lhs, Node *rhs);
Node *new_node_lt(Token *tok, Node *lhs, Node *rhs);
Node *new_node_le(Token *tok, Node *lhs, Node *rhs);
Node *new_node_addr(Token *tok, Node *operand);
Node *new_node_deref(Token *tok, Node *operand);
Node *new_node_assign(Token *tok, Node *lhs, Node *rhs);
Node *new_node_var(Token *tok, Variable *var);

void program();
void toplevel();
void func(Type *type, Token *name);
void funcparam(Vector *params);
VariableInit *varinit();
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
    VariableInit *init = NULL;
    if (consume(TK_ASSIGN))
      init = varinit();
    if (init && type->kind == TYPE_ARRAY && init->vec == NULL)
      error_at(name->pos, "invalid initializer for an array");
    expect(TK_SEMICOLON);
    new_global(name, type, init);
  }
}

void func(Type *type, Token *name) {
  Node *node = calloc(1, sizeof(Node));
  vector_push(functions, &node);
  node->token = name;
  node->type = type;
  node->kind = ND_FUNC;
  node->func = calloc(1, sizeof(Function));
  node->func->name = name->pos;
  node->func->name_length = name->token_length;
  node->func->params = new_vector(0, sizeof(Variable *));
  node->func->type = type;
  assert(offset == 0);
  assert(current_scope == NULL);
  new_scope();
  expect(TK_LPAREN);
  if (!consume(TK_RPAREN)) {
    funcparam(node->func->params);
    expect(TK_RPAREN);
  }
  expect(TK_LBRACE);
  node->body = stmt_block();
  node->func->offset = offset;
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
    Variable *var = new_local(id, ty, NULL);
    vector_push(params, &var);
  } while (consume(TK_COMMA));
}

VariableInit *varinit() {
  VariableInit *init = calloc(1, sizeof(VariableInit));
  if (consume(TK_LBRACE)) {
    Token *tok = consume(TK_RBRACE);
    if (tok)
      error_at(tok->pos, "empty brace initializer is not allowed");
    init->vec = new_vector(0, sizeof(VariableInit *));
    do {
      VariableInit *i = varinit();
      vector_push(init->vec, &i);
    } while (consume(TK_COMMA));
    expect(TK_RBRACE);
  } else {
    init->expr = expr();
  }
  return init;
}

Node *init_local_variable(Variable *var) {
  if (var->init == NULL)
    return NULL;

  if (var->type->kind == TYPE_ARRAY) {
    if (var->type->base->kind == TYPE_ARRAY)
      error("initilizing an multi-dimensional array is not implemented.");
    if (var->init->expr) {
      assert(false);
    } else if (var->init->vec) {
      assert(var->init->vec->size > 0);

      Node *node = calloc(1, sizeof(Node));
      node->kind = ND_BLOCK;
      node->blk_stmts = new_vector(0, sizeof(Node *));

      for (int i = 0; i < (int)var->type->array_size; i++) {
        if (i < var->init->vec->size) {
          VariableInit *init = *(VariableInit **)vector_get(var->init->vec, i);
          if (init->vec)
            error("invalid array initializer (multi-dimension is not "
                  "implemented)");
          Node *s = new_node_assign(
              NULL,
              new_node_deref(NULL, new_node_add(NULL, new_node_var(NULL, var),
                                                new_node_num(NULL, i))),
              init->expr);
          vector_push(node->blk_stmts, &s);
        } else {
          Node *s = new_node_assign(
              NULL,
              new_node_deref(NULL, new_node_add(NULL, new_node_var(NULL, var),
                                                new_node_num(NULL, i))),
              new_node_num(NULL, 0));
          vector_push(node->blk_stmts, &s);
        }
      }
      return node;
    }
  } else if (var->type->kind == TYPE_PTR || var->type->kind == TYPE_INT ||
             var->type->kind == TYPE_CHAR) {
    VariableInit *init = var->init;
    while (init->vec) {
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    if (init->expr)
      return new_node_assign(NULL, new_node_var(NULL, var), init->expr);
  } else
    assert(false);
  return NULL;
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
    VariableInit *init = NULL;
    if (consume(TK_ASSIGN))
      init = varinit();
    if (init && ty->kind == TYPE_ARRAY && init->vec == NULL)
      error_at(id->pos, "invalid initializer for an array");
    expect(TK_SEMICOLON);
    Variable *var = new_local(id, ty, init);
    return init_local_variable(var);
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
  Token *tok = consume(TK_ASSIGN);
  if (tok)
    node = new_node_assign(tok, node, assign());
  return node;
}

Node *equality() {
  Node *node = relational();
  while (true) {
    Token *tok;
    if ((tok = consume(TK_EQ)))
      node = new_node_eq(tok, node, relational());
    else if ((tok = consume(TK_NE)))
      node = new_node_ne(tok, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();
  while (true) {
    Token *tok;
    if ((tok = consume(TK_LT)))
      node = new_node_lt(tok, node, add());
    else if ((tok = consume(TK_LE)))
      node = new_node_le(tok, node, add());
    if ((tok = consume(TK_GT)))
      node = new_node_lt(tok, add(), node);
    else if ((tok = consume(TK_GE)))
      node = new_node_le(tok, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  while (true) {
    Token *tok = token; // for error message
    if (consume(TK_PLUS)) {
      node = new_node_add(tok, node, mul());
    } else if (consume(TK_MINUS)) {
      node = new_node_sub(tok, node, mul());
    } else
      return node;
  }
}

Node *mul() {
  Node *node = unary();
  while (true) {
    Token *tok;
    if ((tok = consume(TK_STAR)))
      node = new_node_mul(tok, node, unary());
    else if ((tok = consume(TK_SLASH)))
      node = new_node_div(tok, node, unary());
    else
      return node;
  }
}

Node *unary() {
  Token *tok;
  if ((tok = consume(TK_SIZEOF)))
    return new_node_num(tok, unary()->type->size);
  if ((tok = consume(TK_PLUS)))
    return unary();
  if ((tok = consume(TK_MINUS)))
    return new_node_sub(tok, new_node_num(NULL, 0), unary());
  if ((tok = consume(TK_AMP)))
    return new_node_addr(tok, unary());
  if ((tok = consume(TK_STAR)))
    return new_node_deref(tok, unary());
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
      node->token = tok;

      node->func = find_function(tok);
      if (node->func == NULL) {
        // TODO
        // error_at(tok->pos, "undefined function: '%.*s'", tok->token_length,
        //     tok->pos);
        node->func = calloc(1, sizeof(Function));
        node->func->name = tok->pos;
        node->func->name_length = tok->token_length;
      }
      node->type = node->func->type;
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
      Variable *var = find_variable(tok);
      if (!var)
        error_at(tok->pos, "undefined identifier: '%.*s'", tok->token_length,
                 tok->pos);
      return new_node_var(tok, var);
    }
  }

  if ((tok = consume(TK_STR))) {
    Variable *var = new_string_literal(tok->string_literal);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_NUM)))
    return new_node_num(tok, tok->val);

  error_at(token->pos, "primary expected but not found", token->token_length,
           token->pos);
  return NULL;
}

Node *postfix() {
  Node *node = primary();
  while (consume(TK_LBRACKET)) {
    // x[y] --> *(x+y)
    Node *y = expr();
    Token *tok = expect(TK_RBRACKET);
    node = new_node_deref(tok, new_node_add(tok, node, y));
  }
  return node;
}
