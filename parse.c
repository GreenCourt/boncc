#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* BNF
program    = toplevel*
toplevel   = func | vardec
type       = "int" "*"*
vardec     = type ident ("[" "]")? ("[" num "]")* ("=" varinit)?  ("," "*"* ident ("[" "]")? ("[" num "]")* ("=" varinit)?)* ";"
varinit    = expr
             | "{" varinit ("," varinit)* "}"
func       = type ident "(" funcparam? ")" "{" stmt* "}"
funcparam  = type ident ("[" num? "]")* ("," type ident ("[" num? "]")* )*
stmt       = expr ";"
             | vardec
             | "{" stmt* "}"
             | "if" "(" expr ")" stmt ("else" stmt)?
             | "while" "(" expr ")" stmt
             | "for" "(" (expr | vardec)? ";" expr? ";" expr? ")" stmt
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
    error_at(&token->pos, "'%s' expected but not found", token_str[kind]);
  Token *tok = token;
  token = token->next;
  return tok;
}

int expect_number() {
  if (token->kind != TK_NUM)
    error_at(&token->pos, "number expected but not found");
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
  Token *num = consume(TK_NUM);
  if (num && num->val < 0)
    error_at(&num->pos, "invalid array size");
  int size = num ? num->val : -1; // -1 will be assumed by rhs initializer
  Token *r = expect(TK_RBRACKET);
  type = consume_array_brackets(type);
  if (type->size < 0)
    error_at(&r->pos, "array size assumption is allowed only in the first dimension");
  return array_type(type, size);
}

Type *expect_type() {
  Type *ty = consume_type();
  if (!ty)
    error_at(&token->pos, "type expected but not found");
  return ty;
}

bool at_eof() { return token->kind == TK_EOF; }

Variable *find_local_in_scope(Token *tok, Scope *scope) {
  Vector *locals = scope->local_variables;
  int sz = locals->size;
  for (int i = 0; i < sz; ++i) {
    Variable *var = *(Variable **)vector_get(locals, i);
    if (var->name_length == tok->token_length && !memcmp(tok->pos.pos, var->name, var->name_length))
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
    if (var->name_length == tok->token_length && !memcmp(tok->pos.pos, var->name, var->name_length))
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
  var->name = tok->pos.pos;
  var->name_length = tok->token_length;
  var->type = type;
  var->kind = kind;
  var->token = tok;
  return var;
}

Variable *new_local(Token *tok, Type *type) {
  if (find_local_in_scope(tok, current_scope))
    error_at(&tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_LOCAL);
  vector_push(current_scope->local_variables, &var);
  return var;
}

void set_offset(Variable *var) {
  assert(var->kind == VK_LOCAL);
  assert(var->offset == 0);
  var->offset = offset + var->type->size;
  offset = var->offset;
}

Variable *new_global(Token *tok, Type *type) {
  if (find_global(tok))
    error_at(&tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_GLOBAL);
  vector_push(globals, &var);
  return var;
}

Variable *new_string_literal(Token *tok) {
  static int idx = 0;
  assert(tok->kind == TK_STR);
  Variable *var = calloc(1, sizeof(Variable));
  var->name = calloc(15, sizeof(char));
  sprintf(var->name, ".LC%d", idx++);
  var->name_length = strlen(var->name);
  var->type = array_type(base_type(TYPE_CHAR), strlen(tok->string_literal) + 1);
  var->kind = VK_STRLIT;
  var->string_literal = tok->string_literal;
  var->token = tok;
  vector_push(strings, &var);
  return var;
}

Function *find_function(Token *tok) {
  for (int i = 0; i < functions->size; i++) {
    Function *f = (*(Node **)vector_get(functions, i))->func;
    if (strncmp(tok->pos.pos, f->name, f->name_length) == 0)
      return f;
  }
  return NULL;
}

// node.c -->
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
Node *new_node_array_set_expr(Variable *var, int idx, Node *expr);
Node *new_node_array_set_val(Variable *var, int idx, int val);
// <-- node.c

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

Vector *vardec(Type *type, Token *name, VariableKind kind) {
  Type *base = type;
  while (base->kind == TYPE_ARRAY || base->kind == TYPE_PTR)
    base = base->base;

  Vector *variables = new_vector(0, sizeof(Variable *));
  while (true) {
    type = consume_array_brackets(type);
    Variable *var = NULL;

    if (kind == VK_GLOBAL)
      var = new_global(name, type);
    else if (kind == VK_LOCAL)
      var = new_local(name, type);
    else
      assert(false);

    if (consume(TK_ASSIGN)) {
      var->init = varinit();
      if (type->kind == TYPE_ARRAY) {
        if (var->init->vec == NULL) {
          if (type->base->kind != TYPE_CHAR) {
            error_at(&name->pos, "invalid initializer for an array");
          } else if (var->init->expr->kind != ND_VAR || var->init->expr->variable->kind != VK_STRLIT) {
            error_at(&name->pos, "invalid initializer for an array");
          } else if (type->array_size < 0) {
            type->array_size = var->init->expr->variable->type->array_size;
            type->size = var->init->expr->variable->type->size;
          }
        } else if (type->array_size < 0) {
          type->array_size = var->init->vec->size;
          type->size = type->base->size * type->array_size;
        }
        assert(type->size >= 0);
        assert(type->array_size >= 0);
      }
    } else if (type->array_size < 0) {
      error_at(&name->pos, "invalid array size");
    }

    if (kind == VK_LOCAL)
      set_offset(var);

    vector_push(variables, &var);
    if (!consume(TK_COMMA))
      break;
    type = base;
    while (consume(TK_STAR))
      type = pointer_type(type);
    name = expect(TK_IDENT);
  }
  expect(TK_SEMICOLON);
  return variables;
}

void toplevel() {
  Type *type = expect_type();
  Token *name = expect(TK_IDENT);
  if (token->kind == TK_LPAREN)
    func(type, name);
  else
    vardec(type, name, VK_GLOBAL);
}

void func(Type *type, Token *name) {
  Node *node = calloc(1, sizeof(Node));
  vector_push(functions, &node);
  node->token = name;
  node->type = type;
  node->kind = ND_FUNC;
  node->func = calloc(1, sizeof(Function));
  node->func->name = name->pos.pos;
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
    Variable *var = new_local(id, ty);
    set_offset(var);
    vector_push(params, &var);
  } while (consume(TK_COMMA));
}

VariableInit *varinit() {
  VariableInit *init = calloc(1, sizeof(VariableInit));
  if (consume(TK_LBRACE)) {
    Token *tok = consume(TK_RBRACE);
    if (tok)
      error_at(&tok->pos, "empty brace initializer is not allowed");
    init->vec = new_vector(0, sizeof(VariableInit *));
    do {
      VariableInit *i = varinit();
      if (i->vec)
        init->nested = true;
      vector_push(init->vec, &i);
    } while (consume(TK_COMMA));
    expect(TK_RBRACE);
  } else {
    init->expr = expr();
  }
  return init;
}

Node *init_local_variable(Variable *var, VariableInit *init, Type *type, int array_index_offset) {
  assert(array_index_offset == 0 || (var->type->kind == TYPE_ARRAY && var->type != type && var->init != init));

  if (init == NULL)
    return NULL;

  if (type->kind == TYPE_ARRAY) {
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->blk_stmts = new_vector(0, sizeof(Node *));

    if (init->expr) {
      if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
        // initilize the array as a string
        char *lit = init->expr->variable->string_literal;
        if (type->array_size != (int)strlen(lit) + 1)
          error_at(&var->token->pos, "miss-match between array-size and string-length");

        for (int i = 0; i < type->array_size; ++i) {
          Node *s = new_node_array_set_val(var, i + array_index_offset, (int)lit[i]);
          vector_push(node->blk_stmts, &s);
        }
      } else {
        // When init->expr is given for an array, only the first element will be initialized.
        Type *ty = type;
        while (ty->kind == TYPE_ARRAY)
          ty = ty->base;
        Node *s = init_local_variable(var, init, ty, array_index_offset);
        vector_push(node->blk_stmts, &s);
        for (int i = 1; i < type->array_size; ++i) {
          s = new_node_array_set_val(var, i + array_index_offset, 0);
          vector_push(node->blk_stmts, &s);
        }
      }
    } else if (init->vec) {
      assert(init->vec->size > 0);
      Type *base = type;
      while (base->kind == TYPE_ARRAY)
        base = base->base;
      if (init->nested) {
        // init arrays recursively
        int len1d = type->size / base->size;
        int ofs = len1d / type->array_size;
        int len = type->array_size < init->vec->size ? type->array_size : init->vec->size;
        for (int i = 0; i < len; i++) {
          Node *s = init_local_variable(var, *(VariableInit **)vector_get(init->vec, i), type->base, array_index_offset + i * ofs);
          vector_push(node->blk_stmts, &s);
        }
        // fill zero
        for (int i = len * ofs; i < len1d; ++i) {
          Node *s = new_node_array_set_val(var, array_index_offset + i, 0);
          vector_push(node->blk_stmts, &s);
        }
      } else {
        // init as a one-dimensional array
        int len1d = type->size / base->size;
        int len = len1d < init->vec->size ? len1d : init->vec->size;
        for (int i = 0; i < len; i++) {
          VariableInit *vi = *(VariableInit **)vector_get(init->vec, i);
          assert(vi->expr);
          Node *s = new_node_array_set_expr(var, array_index_offset + i, vi->expr);
          vector_push(node->blk_stmts, &s);
        }
        // fill zero
        for (int i = len; i < len1d; ++i) {
          Node *s = new_node_array_set_val(var, array_index_offset + i, 0);
          vector_push(node->blk_stmts, &s);
        }
      }
    } else
      assert(false);
    return node;
  } else if (type->kind == TYPE_PTR || type->kind == TYPE_INT || type->kind == TYPE_CHAR) {
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    if (var->type->kind == TYPE_ARRAY) {
      return new_node_array_set_expr(var, array_index_offset, init->expr);
    } else
      return new_node_assign(NULL, new_node_var(NULL, var), init->expr);
  } else
    assert(false);
  return NULL;
}

Node *init_multiple_local_variables(Vector *variables) {
  assert(variables->size > 0);
  bool no_init = true;
  for (int i = 0; i < variables->size; ++i) {
    Variable *var = *(Variable **)vector_get(variables, i);
    if (var->init) {
      no_init = false;
      break;
    }
  }
  if (no_init)
    return NULL;

  if (variables->size == 1) {
    Variable *var = *(Variable **)vector_get(variables, 0);
    return init_local_variable(var, var->init, var->type, 0);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  for (int i = 0; i < variables->size; ++i) {
    Variable *var = *(Variable **)vector_get(variables, i);
    Node *s = init_local_variable(var, var->init, var->type, 0);
    if (s)
      vector_push(node->blk_stmts, &s);
  }
  return node;
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
    Vector *vars = vardec(ty, id, VK_LOCAL);
    return init_multiple_local_variables(vars);
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
    Type *type = consume_type();
    if (type) {
      Token *name = expect(TK_IDENT);
      Vector *vars = vardec(type, name, VK_LOCAL);
      node->init = init_multiple_local_variables(vars);
    } else {
      node->init = expr();
      expect(TK_SEMICOLON);
    }
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
        // error_at(tok->pos, "undefined function: '%.*s'", tok->token_length, tok->pos);
        node->func = calloc(1, sizeof(Function));
        node->func->name = tok->pos.pos;
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
        error_at(&tok->pos, "undefined identifier: '%.*s'", tok->token_length, tok->pos);
      return new_node_var(tok, var);
    }
  }

  if ((tok = consume(TK_STR))) {
    Variable *var = new_string_literal(tok);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_NUM)))
    return new_node_num(tok, tok->val);

  error_at(&token->pos, "primary expected but not found", token->token_length, token->pos);
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
