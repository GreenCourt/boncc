#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* BNF
program    = toplevel*
toplevel   = func | vardec | struct ";"
type       = "void" | "int" | "char" | ("short" "int"?) | ("long" "long"? "int"?) | struct
struct     = ("struct" ident ("{" member* "}")?) | ("struct" ident? "{" member* "}")
member     = type "*"* ident ("[" num "]")* ("," "*"* ident ("[" num "]")* )* ";"
vardec     = type "*"* ident ("[" "]")? ("[" num "]")* ("=" varinit)?  ("," "*"* ident ("[" "]")? ("[" num "]")* ("=" varinit)?)* ";"
varinit    = expr
             | "{" varinit ("," varinit)* "}"
func       = type "*"* ident "(" funcparam? ")" "{" stmt* "}"
funcparam  = type "*"* ident ("[" num? "]")* ("," type "*"* ident ("[" num? "]")* )*
stmt       = ";"
             | expr ";"
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
unary      = postfix
             | ("sizeof" unary)
             | ("sizeof" "(" type ")")
             | ("+" unary)
             | ("-" unary)
             | ("*" unary)
             | ("&" unary)
postfix    = primary tail*
tail       =  ("[" expr "]") | ("." ident) | ("->" ident)
primary    = "(" expr ")"
             | ident ("(" (expr ("," expr)*)? ")")?
             | ident
             | num
             | str
*/

// node.c -->
Node *new_node_num(Token *tok, int val);
Node *new_node_long(Token *tok, long long val);
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
Node *new_node_member(Token *tok, Node *x, Member *y);
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

Type *consume_type();
Type *consume_type_star(Type *type);
Type *consume_array_brackets(Type *type);

typedef struct Scope Scope;
struct Scope {
  Scope *prev;
  Vector *local_variables; // vector of Variable*
  Vector *local_structs;   // vector of Type*
};
static Scope *current_scope = NULL;
static int local_variable_offset;

void new_scope() {
  Scope *s = calloc(1, sizeof(Scope));
  s->prev = current_scope;
  s->local_variables = new_vector(0, sizeof(Variable *));
  s->local_structs = new_vector(0, sizeof(Type *));
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
    error_at(&token->pos, "'%s' expected but not found", token_text[kind]);
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

Type *find_local_struct_in_scope(Token *tok, Scope *scope) {
  Vector *vec = scope->local_structs;
  int sz = vec->size;
  for (int i = 0; i < sz; ++i) {
    Type *st = *(Type **)vector_get(vec, i);
    if (same_ident(st->ident, tok->ident))
      return st;
  }
  return NULL;
}

Type *find_local_structure(Token *tok) {
  Scope *scope = current_scope;
  while (scope) {
    Type *st = find_local_struct_in_scope(tok, scope);
    if (st)
      return st;
    scope = scope->prev;
  }
  return NULL;
}

Type *find_global_struct(Token *tok) {
  int sz = structs->size;
  for (int i = 0; i < sz; ++i) {
    Type *st = *(Type **)vector_get(structs, i);
    if (same_ident(st->ident, tok->ident))
      return st;
  }
  return NULL;
}

Type *find_struct(Token *tok) {
  Type *st = find_local_structure(tok);
  if (st)
    return st;
  return find_global_struct(tok);
}

Member *find_member(Type *st, Token *tok) {
  assert(st->kind == TYPE_STRUCT);
  assert(tok->kind == TK_IDENT);
  Member *member = st->member;
  while (member) {
    if (same_ident(member->ident, tok->ident))
      return member;
    member = member->next;
  }
  return NULL;
}

Type *consume_struct(Token *struct_name) {
  static int idx = 0;
  Ident *struct_ident = NULL;

  if (struct_name) {
    assert(struct_name->kind == TK_IDENT);
    struct_ident = struct_name->ident;
    if (consume(TK_LBRACE)) {
      if (current_scope ? find_local_struct_in_scope(struct_name, current_scope) : find_global_struct(struct_name))
        error_at(&struct_name->pos, "duplicated struct identifier");
    } else {
      Type *predefined_struct = find_struct(struct_name);
      if (predefined_struct == NULL)
        error_at(&struct_name->pos, "undefined struct identifier");
      return predefined_struct;
    }
  } else {
    // unnamed struct
    expect(TK_LBRACE);
    struct_ident = calloc(1, sizeof(Ident));
    struct_ident->name = calloc(20, sizeof(char));
    sprintf(struct_ident->name, ".struct%d", idx++);
    struct_ident->len = strlen(struct_ident->name);
  }

  Member head;
  head.next = NULL;
  head.offset = 0;
  Member *tail = &head;
  int offset = 0;
  int align = 0;

  while (!consume(TK_RBRACE)) {
    Token *tok_type = token;
    Type *base = consume_type();
    if (!base)
      error_at(&token->pos, "invalid member type");

    do {
      Type *type = consume_type_star(base);

      if (type->kind == TYPE_VOID)
        error_at(&tok_type->pos, "void type is not allowed");

      Token *var_name = expect(TK_IDENT);
      type = consume_array_brackets(type);
      if (type->kind == TYPE_ARRAY && type->size < 0)
        error_at(&var_name->pos, "invalid member array size");

      Member *m = calloc(1, sizeof(Member));
      m->ident = var_name->ident;
      m->type = type;

      int padding = (offset % type->size) ? type->size - (offset % type->size) : 0;
      offset += padding;
      m->offset = offset;
      offset += type->size;
      if (type->size > align)
        align = type->size;

      tail->next = m;
      tail = m;
    } while (consume(TK_COMMA));
    expect(TK_SEMICOLON);
  }

  if (align && offset % align)
    offset += align - offset % align;

  Type *st = struct_type(struct_ident, head.next, offset);
  if (current_scope == NULL)
    vector_push(structs, &st);
  else
    vector_push(current_scope->local_structs, &st);
  return st;
}

Type *consume_type_star(Type *type) {
  while (consume(TK_STAR))
    type = pointer_type(type);
  return type;
}

Type *consume_type() {
  if (consume(TK_VOID))
    return base_type(TYPE_VOID);

  if (consume(TK_INT))
    return base_type(TYPE_INT);

  if (consume(TK_CHAR))
    return base_type(TYPE_CHAR);

  if (consume(TK_SHORT)) {
    consume(TK_INT);
    return base_type(TYPE_SHORT);
  }

  if (consume(TK_LONG)) {
    consume(TK_LONG);
    consume(TK_INT);
    return base_type(TYPE_LONG);
  }

  if (consume(TK_STRUCT))
    return consume_struct(consume(TK_IDENT));

  return NULL;
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

Variable *find_local_variable_in_scope(Token *tok, Scope *scope) {
  Vector *vec = scope->local_variables;
  int sz = vec->size;
  for (int i = 0; i < sz; ++i) {
    Variable *var = *(Variable **)vector_get(vec, i);
    if (same_ident(var->ident, tok->ident))
      return var;
  }
  return NULL;
}

Variable *find_local_variable(Token *tok) {
  Scope *scope = current_scope;
  while (scope) {
    Variable *var = find_local_variable_in_scope(tok, scope);
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
    if (same_ident(var->ident, tok->ident))
      return var;
  }
  return NULL;
}

Variable *find_variable(Token *tok) {
  Variable *var = find_local_variable(tok);
  if (var)
    return var;
  return find_global(tok);
}

Variable *new_variable(Token *tok, Type *type, VariableKind kind) {
  Variable *var = calloc(1, sizeof(Variable));
  var->ident = tok->ident;
  var->type = type;
  var->kind = kind;
  var->token = tok;
  return var;
}

Variable *new_local_variable(Token *tok, Type *type) {
  if (find_local_variable_in_scope(tok, current_scope))
    error_at(&tok->pos, "duplicated identifier");
  Variable *var = new_variable(tok, type, VK_LOCAL);
  vector_push(current_scope->local_variables, &var);
  return var;
}

void set_offset(Variable *var) {
  assert(var->kind == VK_LOCAL);
  assert(var->offset == 0);
  var->offset = local_variable_offset + var->type->size;
  local_variable_offset = var->offset;
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
  var->ident = calloc(1, sizeof(Ident));
  var->ident->name = calloc(15, sizeof(char));
  sprintf(var->ident->name, ".LC%d", idx++);
  var->ident->len = strlen(var->ident->name);
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
    if (same_ident(f->ident, tok->ident))
      return f;
  }
  return NULL;
}

void program() {
  functions = new_vector(0, sizeof(Node *));
  globals = new_vector(0, sizeof(Variable *));
  strings = new_vector(0, sizeof(Variable *));
  structs = new_vector(0, sizeof(Type *));
  while (!at_eof())
    toplevel();
}

Vector *vardec(Type *type, Token *name, VariableKind kind) {
  Type *base = type;
  while (base->kind == TYPE_ARRAY || base->kind == TYPE_PTR)
    base = base->base;

  Vector *variables = new_vector(0, sizeof(Variable *));
  while (true) {
    if (type->kind == TYPE_VOID)
      error_at(&name->pos, "void type is not allowed");
    type = consume_array_brackets(type);
    Variable *var = NULL;

    if (kind == VK_GLOBAL)
      var = new_global(name, type);
    else if (kind == VK_LOCAL)
      var = new_local_variable(name, type);
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
    type = consume_type_star(base);
    name = expect(TK_IDENT);
  }
  expect(TK_SEMICOLON);
  return variables;
}

void toplevel() {
  Type *base = expect_type();
  if (base->kind == TYPE_STRUCT && consume(TK_SEMICOLON)) {
    // struct declaration
    return;
  }
  Type *type = consume_type_star(base);
  Token *name = expect(TK_IDENT);
  if (token->kind == TK_LPAREN)
    func(type, name);
  else
    vardec(type, name, VK_GLOBAL);
}

void func(Type *type, Token *tok) {
  Node *node = calloc(1, sizeof(Node));
  vector_push(functions, &node);
  node->token = tok;
  node->type = type;
  node->kind = ND_FUNC;
  node->func = calloc(1, sizeof(Function));
  node->func->ident = tok->ident;
  node->func->params = new_vector(0, sizeof(Variable *));
  node->func->type = type;
  assert(local_variable_offset == 0);
  assert(current_scope == NULL);
  new_scope();
  expect(TK_LPAREN);
  if (!consume(TK_RPAREN)) {
    funcparam(node->func->params);
    expect(TK_RPAREN);
  }
  expect(TK_LBRACE);
  node->body = stmt_block();
  node->func->offset = local_variable_offset;
  restore_scope();
  local_variable_offset = 0;
}

void funcparam(Vector *params) {
  do {
    Token *tok_type = token;
    Type *ty = consume_type_star(expect_type());
    if (ty->kind == TYPE_VOID)
      error_at(&tok_type->pos, "void type is not allowed");
    Token *id = expect(TK_IDENT);
    while (consume(TK_LBRACKET)) {
      consume(TK_NUM);       // currently not used
      ty = pointer_type(ty); // array as a pointer
      expect(TK_RBRACKET);
    }
    Variable *var = new_local_variable(id, ty);
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
  if (consume(TK_SEMICOLON)) {
    return NULL;
  } else if (consume(TK_LBRACE)) {
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
    if (ty->kind == TYPE_STRUCT && consume(TK_SEMICOLON)) {
      // struct declaration
      return NULL;
    } else {
      ty = consume_type_star(ty);
      Token *id = expect(TK_IDENT);
      Vector *vars = vardec(ty, id, VK_LOCAL);
      return init_multiple_local_variables(vars);
    }
  } else {
    Node *node = expr();
    expect(TK_SEMICOLON);
    return node;
  }
}

Node *stmt_block() {
  if (consume(TK_RBRACE))
    return NULL;

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
  Token *tok = expect(TK_RPAREN);

  if (consume_type())
    error_at(&tok->next->pos, "vardec is not allowed here");

  new_scope();
  node->body = stmt();
  restore_scope();

  if ((tok = consume(TK_ELSE))) {
    if (consume_type())
      error_at(&tok->next->pos, "vardec is not allowed here");
    node->else_ = stmt();
  }

  return node;
}

Node *stmt_while() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;

  expect(TK_LPAREN);
  node->condition = expr();
  Token *tok = expect(TK_RPAREN);

  if (consume_type())
    error_at(&tok->next->pos, "vardec is not allowed here");

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
      type = consume_type_star(type);
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

  Token *tok = NULL;
  if (!(tok = consume(TK_RPAREN))) {
    node->update = expr();
    tok = expect(TK_RPAREN);
  }

  if (consume_type())
    error_at(&tok->next->pos, "vardec is not allowed here");

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
  if ((tok = consume(TK_SIZEOF))) {
    Token *keep = token;
    Token *left_paren = consume(TK_LPAREN);
    Type *type = consume_type();
    if (left_paren && type) {
      // sizeof(type)
      type = consume_array_brackets(consume_type_star(type));
      if (type->size < 0)
        error_at(&tok->pos, "invalid array size");
      expect(TK_RPAREN);
    } else {
      // sizeof unary
      token = keep; // rollback token
      type = unary()->type;
    }
    if (type->kind == TYPE_VOID)
      error_at(&tok->pos, "invalud sizeof operation for void type");
    return new_node_long(tok, type->size);
  }

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
        // error_at(&tok->pos, "undefined function: '%.*s'", tok->token_length, tok->pos.pos);
        node->func = calloc(1, sizeof(Function));
        node->func->ident = tok->ident;
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
        error_at(&tok->pos, "undefined identifier: '%.*s'", tok->token_length, tok->pos.pos);
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

Node *tail(Node *x) {
  if (consume(TK_LBRACKET)) {
    // x[y] --> *(x+y)
    Node *y = expr();
    Token *tok = expect(TK_RBRACKET);
    return new_node_deref(tok, new_node_add(tok, x, y));
  }

  Token *op;
  if ((op = consume(TK_DOT))) {
    // struct member access (x.y)
    if (x->type == NULL || x->type->kind != TYPE_STRUCT)
      error_at(&op->pos, "not a struct");
    Token *y = expect(TK_IDENT);
    Member *member = find_member(x->type, y);
    if (member == NULL)
      error_at(&y->pos, "unknown struct member");
    return new_node_member(op, x, member);
  }
  if ((op = consume(TK_ARROW))) {
    // struct member access
    // x->y is (*x).y
    if (x->type == NULL || x->type->kind != TYPE_PTR || x->type->base->kind != TYPE_STRUCT)
      error_at(&op->pos, "not a struct pointer");
    Token *y = expect(TK_IDENT);
    Member *member = find_member(x->type->base, y);
    if (member == NULL)
      error_at(&y->pos, "unknown struct member");
    return new_node_member(op, new_node_deref(NULL, x), member);
  }
  return x;
}

Node *postfix() {
  Node *p = primary();
  Node *q = tail(p);
  while (p != q) {
    p = q;
    q = tail(p);
  }
  return p;
}
