#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* BNF
program     = declaration*
declaration = func | vardec | (struct ";") | (enum ";") | typedef
declarator  = "*"* ident dectail?
              | "*"* "(" declarator ")" dectail?
dectail     = ("[" num "]")*
              | "(" funcparam? ")"
type        = "void" | int_type | float_type | struct | enum
int_type    = ("signed" | "unsigned")? ("int" | "char" | ("short" "int"?) | ("long" "long"? "int"?))
float_type  = "float" | ("long"? double "long"?)
struct      = (("struct" | "union") ident ("{" member* "}")?) | (("struct" | "union") ident? "{" member* "}")
member      = "const"* type declarator ("," declarator )* ";"
enum        = ("enum" ident ("{" enumval ("," enumval)* ","? "}")?) | ("enum" ident? "{" enumval ("," enumval)* ","? "}")
enumval     = indent ("=" expr)?
typedef     = "typedef" type declarator ("," declarator)* ";"
qualifier   = ("const" | "static" | "extern")*
vardec      = qualifier? type declarator ("=" varinit)?  ("," declarator ("=" varinit)?)* ";"
varinit     = expr
              | "{" varinit ("," varinit)* ","? "}"
func        = qualifier? type declarator (("{" stmt* "}") | ";")
funcparam   = qualifier? type declarator ("," type declarator )*
              | void
stmt        = ";"
              | expr ";"
              | "{" (declaration | stmt)* "}"
              | "if" "(" expr ")" stmt ("else" stmt)?
              | "do" stmt "while" "(" expr ")" ";"
              | "while" "(" expr ")" stmt
              | "for" "(" (expr | vardec)? ";" expr? ";" expr? ")" stmt
              | "return" expr? ";"
              | "break" ";"
              | "continue" ";"
              | "switch" "(" expr ")" stmt
              | "case" expr ":" stmt
              | "default" ":" stmt
expr        = assign
assign      = condtional (("=" | "+=" | "-=" | "*=" | "/=" | "%=" | "^=" | "&=" | "|=" | "<<=" | ">>=" ) assign)?
conditional = logor ("?" expr ":" conditional)?
logor       = logand ("||" logand)*
logand      = bitor ("&&" bitor)*
bitor       = bitxor ("|" bitxor)*
bitxor      = bitand ("^" bitand)*
bitand      = equality ("&" equality)*
equality    = relational (("==" | "!=") relational)*
relational  = bitshift (("<" | "<=" | ">" | ">=") bitshift)*
bitshift    = add (("<<" | ">>") add)*
add         = mul (("+" | "-") mul)*
mul         = unary (("*" | "/" | "%") unary)*
unary       = postfix
              | ("sizeof" unary)
              | ("sizeof" "(" type ")")
              | ("++" unary)
              | ("--" unary)
              | ("+" unary)
              | ("-" unary)
              | ("*" unary)
              | ("&" unary)
              | ("!" unary)
              | ("~" unary)
              | ("(" type ")" unary)
postfix     = primary tail*
tail        =  ("[" expr "]")
               | ("." ident)
               | ("->" ident)
               | ("++") | ("--")
               | ("(" (expr ("," expr)*)? ")")
primary     = "(" expr ")"
              | ident
              | num
              | str
*/

// node.c -->
Node *new_node_nop();
Node *new_node_num(Token *tok, long long val, Type *type);
Node *new_node_mul(Token *tok, Node *lhs, Node *rhs);
Node *new_node_div(Token *tok, Node *lhs, Node *rhs);
Node *new_node_mod(Token *tok, Node *lhs, Node *rhs);
Node *new_node_add(Token *tok, Node *lhs, Node *rhs);
Node *new_node_sub(Token *tok, Node *lhs, Node *rhs);
Node *new_node_eq(Token *tok, Node *lhs, Node *rhs);
Node *new_node_ne(Token *tok, Node *lhs, Node *rhs);
Node *new_node_lt(Token *tok, Node *lhs, Node *rhs);
Node *new_node_le(Token *tok, Node *lhs, Node *rhs);
Node *new_node_addr(Token *tok, Node *operand);
Node *new_node_deref(Token *tok, Node *operand);
Node *new_node_lognot(Token *tok, Node *operand);
Node *new_node_logand(Token *tok, Node *lhs, Node *rhs, int label_index);
Node *new_node_logor(Token *tok, Node *lhs, Node *rhs, int label_index);
Node *new_node_lshift(Token *tok, Node *lhs, Node *rhs);
Node *new_node_rshift(Token *tok, Node *lhs, Node *rhs);
Node *new_node_bitand(Token *tok, Node *lhs, Node *rhs);
Node *new_node_bitor(Token *tok, Node *lhs, Node *rhs);
Node *new_node_bitxor(Token *tok, Node *lhs, Node *rhs);
Node *new_node_bitnot(Token *tok, Node *lhs);
Node *new_node_assign(Token *tok, Node *lhs, Node *rhs);
Node *new_node_assign_ignore_const(Token *tok, Node *lhs, Node *rhs);
Node *new_node_conditional(Token *tok, Node *cond, Node *lhs, Node *rhs, int label_index);
Node *new_node_cast(Token *tok, Type *type, Node *lhs);
Node *new_node_member(Token *tok, Node *x, Member *y);
Node *new_node_var(Token *tok, Variable *var);
Node *new_node_array_set_expr(Variable *var, int idx, Node *expr);
Node *new_node_array_set_val(Variable *var, int idx, int val);
int eval(Node *node);
// <-- node.c

typedef enum {
  IS_STATIC = 1,
  IS_CONST = 2,
  IS_EXTERN = 4,
} Qualifier;

int consume_qualifier();
Node *declaration();
Type *declarator(Type *base);
void func(Type *type, int qualifier);
void funcparam(Type *ft);
VariableInit *varinit();
Node *stmt();
Node *expr();
Node *assign();
Node *conditional();
Node *logor();
Node *logand();
Node * bitor ();
Node *bitxor();
Node *bitand();
Node *equality();
Node *relational();
Node *bitshift();
Node *add();
Node *mul();
Node *primary();
Node *unary();
Node *postfix();

Node *stmt_if();
Node *stmt_do();
Node *stmt_while();
Node *stmt_for();
Node *stmt_switch();
Node *stmt_block();

void expect_typedef();
Type *consume_type();
Type *consume_type_star(Type *type);
Type *consume_array_brackets(Type *type);
Node *init_multiple_local_variables(Vector *variables);

static Token *next_token;
static Scope *current_scope = NULL;
static int local_variable_offset;
static int label_index = 0;
static Vector *continue_label; // stack of int
static Vector *break_label;    // stack of int
static Vector *switch_nodes;   // stack of Node*

void new_scope() {
  Scope *s = calloc(1, sizeof(Scope));
  s->prev = current_scope;
  s->objects = new_map();
  s->types = new_map();
  s->enum_elements = new_map();
  s->typedefs = new_map();
  current_scope = s;
}

void restore_scope() {
  assert(current_scope);
  current_scope = current_scope->prev;
}

Token *consume(TokenKind kind) {
  if (next_token->kind != kind)
    return NULL;
  Token *tok = next_token;
  next_token = next_token->next;
  return tok;
}

Token *expect(TokenKind kind) {
  if (next_token->kind != kind)
    error(&next_token->pos, "'%s' expected but not found", token_text[kind]);
  Token *tok = next_token;
  next_token = next_token->next;
  return tok;
}

long long expect_number() {
  if (next_token->kind != TK_NUM)
    error(&next_token->pos, "number expected but not found");
  long long val = next_token->val;
  next_token = next_token->next;
  return val;
}

Type *find_typedef(Ident *id) {
  Scope *scope = current_scope;
  while (scope) {
    Type *t = map_get(scope->typedefs, id);
    if (t)
      return t;
    scope = scope->prev;
  }
  return NULL;
}

Type *find_type(Ident *id) {
  Scope *scope = current_scope;
  while (scope) {
    Type *t = map_get(scope->types, id);
    if (t)
      return t;
    scope = scope->prev;
  }
  return NULL;
}

int *find_enum_element(Ident *id) {
  Scope *scope = current_scope;
  while (scope) {
    int *e = map_get(scope->enum_elements, id);
    if (e)
      return e;
    scope = scope->prev;
  }
  return NULL;
}

Member *find_member(Type *st, Token *tok) {
  assert(st->kind == TYPE_STRUCT || st->kind == TYPE_UNION);
  assert(tok->kind == TK_IDENT);
  Member *member = st->member;
  while (member) {
    if (same_ident(member->ident, tok->ident))
      return member;
    member = member->next;
  }
  return NULL;
}

Type *consume_struct(TypeKind kind) {
  assert(kind == TYPE_STRUCT || kind == TYPE_UNION);
  Token *tag = consume(TK_IDENT);
  Type *st = NULL;

  if (tag) {
    if (consume(TK_LBRACE)) {
      // define struct/union
      // struct/union can be defined only for the current-scope
      st = map_get(current_scope->types, tag->ident);
      if (st == NULL) {
        st = kind == TYPE_UNION ? union_type(false) : struct_type(false);
        map_push(current_scope->types, tag->ident, st);
      } else if (kind != st->kind || st->size > 0) {
        error(&tag->pos, "conflicted type tag");
      }
    } else {
      // declare struct/union
      // multipe time declaration is allowed
      st = find_type(tag->ident);
      if (st == NULL) {
        st = kind == TYPE_UNION ? union_type(false) : struct_type(false);
        map_push(current_scope->types, tag->ident, st);
      } else if (kind != st->kind) {
        error(&tag->pos, "conflicted type tag");
      }
      return st;
    }
  } else {
    // unnamed struct/union
    expect(TK_LBRACE);
    st = kind == TYPE_UNION ? union_type(true) : struct_type(true);
  }

  assert(st != NULL);

  Member head;
  head.next = NULL;
  head.offset = 0;
  Member *tail = &head;
  int offset = 0;
  int align = 0;

  while (!consume(TK_RBRACE)) {
    Token *tok_qualifier = next_token;
    int qualifier = consume_qualifier();
    if (qualifier & (IS_STATIC | IS_EXTERN))
      error(&tok_qualifier->pos, "invalid storage class for a member");

    Token *tok_type = next_token;
    Type *base = consume_type();
    if (!base)
      error(&tok_type->pos, "invalid member type");
    base->is_const = (qualifier & IS_CONST) == IS_CONST;

    if (next_token->kind == TK_SEMICOLON) {
      if (base->kind != TYPE_STRUCT && base->kind != TYPE_UNION)
        error(&tok_type->pos, "identifier required for the member");
      if (!base->is_unnamed)
        error(&tok_type->pos, "identifier required for the member");
      expect(TK_SEMICOLON);
      // merge unnamed struct/union
      Member *child = base->member;
      if (kind == TYPE_UNION) {
        if (offset < base->size)
          offset = base->size;
      } else if (base->kind == TYPE_STRUCT) {
        Member *m = child;
        while (m) {
          Type *type = m->type;
          int padding = (offset % type->size) ? type->size - (offset % type->size) : 0;
          offset += padding;
          m->offset += offset;
          m = m->next;
          offset += type->size;
          if (type->size > align)
            align = type->size;
        }
      } else if (base->kind == TYPE_UNION) {
        int padding = (offset % base->size) ? base->size - (offset % base->size) : 0;
        offset += padding;
        Member *m = child;
        while (m) {
          m->offset += offset;
          m = m->next;
        }
        offset += base->size;
        if (base->size > align)
          align = base->size;
      } else {
        assert(false);
      }
      tail->next = child;
      tail = child;
      continue;
    }

    do {
      Type *type = declarator(base);

      if (type->size < 0)
        error(&tok_type->pos, "incomplete type");

      if (type->kind == TYPE_VOID)
        error(&tok_type->pos, "void type is not allowed");

      if (type->kind == TYPE_FUNC)
        error(&tok_type->pos, "function type is not allowed");

      Member *m = calloc(1, sizeof(Member));
      m->ident = type->objdec->ident;
      m->type = type;

      if (kind == TYPE_UNION) {
        m->offset = 0;
        if (offset < type->size)
          offset = type->size;
      } else {
        int padding = (offset % type->size) ? type->size - (offset % type->size) : 0;
        offset += padding;
        m->offset = offset;
        offset += type->size;
        if (type->size > align)
          align = type->size;
      }

      tail->next = m;
      tail = m;
    } while (consume(TK_COMMA));
    expect(TK_SEMICOLON);
  }

  if (align && offset % align)
    offset += align - offset % align;

  st->member = head.next;
  st->size = offset;
  return st;
}

Type *consume_enum() {
  Token *tag = consume(TK_IDENT);
  Type *et = NULL;

  if (tag) {
    // named enum
    if (consume(TK_LBRACE)) {
      // define enum
      // enum can be defined only for the current-scope
      et = map_get(current_scope->types, tag->ident);
      if (et == NULL) {
        et = enum_type(false);
        map_push(current_scope->types, tag->ident, et);
      } else if (et->kind != TYPE_ENUM || et->size > 0) {
        error(&tag->pos, "conflicted type tag");
      }
    } else {
      // declare enum
      // multipe time declaration is allowed
      et = find_type(tag->ident);
      if (et == NULL) {
        et = enum_type(false);
        map_push(current_scope->types, tag->ident, et);
      } else if (et->kind != TYPE_ENUM) {
        error(&tag->pos, "conflicted type tag");
      }
      return et;
    }
  } else {
    // unnamed enum
    expect(TK_LBRACE);
    et = enum_type(true);
  }

  assert(et != NULL);
  et->size = base_type(TYPE_INT)->size;

  int val = -1;
  while (true) {
    Token *id = expect(TK_IDENT);
    if (map_get(current_scope->enum_elements, id->ident))
      error(&id->pos, "duplicated identifier for enum element");

    if (consume(TK_ASSIGN))
      val = eval(expr()) - 1;
    int *v = calloc(1, sizeof(int));
    *v = ++val;
    map_push(current_scope->enum_elements, id->ident, v);
    if (consume(TK_COMMA)) {
      if (consume(TK_RBRACE))
        break;
    } else {
      expect(TK_RBRACE);
      break;
    }
  }

  return et;
}

Type *consume_type_star(Type *type) {
  Token *tok = NULL;
  bool is_const = false;
  while ((tok = consume(TK_STAR)) || (tok = consume(TK_CONST))) {
    if (tok->kind == TK_CONST) {
      is_const = true;
    } else {
      type = pointer_type(type);
      type->is_const = is_const;
      is_const = false;
    }
  }
  return type;
}

Type *consume_type() {
  if (next_token->kind == TK_IDENT) {
    Type *t = find_typedef(next_token->ident);
    if (t)
      expect(TK_IDENT);
    return t;
  }

  if (consume(TK_VOID))
    return base_type(TYPE_VOID);

  if (consume(TK_STRUCT))
    return consume_struct(TYPE_STRUCT);

  if (consume(TK_UNION))
    return consume_struct(TYPE_UNION);

  if (consume(TK_ENUM))
    return consume_enum();

  int has_signed = 0;
  int has_unsigned = 0;
  int has_long = 0;
  int has_short = 0;
  int has_char = 0;
  int has_int = 0;
  int has_float = 0;
  int has_double = 0;

  Token *itok = NULL;
  while ((itok = consume(TK_UNSIGNED)) ||
         (itok = consume(TK_SIGNED)) ||
         (itok = consume(TK_INT)) ||
         (itok = consume(TK_CHAR)) ||
         (itok = consume(TK_SHORT)) ||
         (itok = consume(TK_LONG)) ||
         (itok = consume(TK_FLOAT)) ||
         (itok = consume(TK_DOUBLE))) {

    switch (itok->kind) {
    case TK_SIGNED:
      if (has_signed || has_unsigned || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_signed = 1;
      break;
    case TK_UNSIGNED:
      if (has_signed || has_unsigned || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_unsigned = 1;
      break;
    case TK_CHAR:
      if (has_int || has_short || has_long || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_char = 1;
      break;
    case TK_INT:
      if (has_int || has_char || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_int = 1;
      break;
    case TK_SHORT:
      if (has_char || has_short || has_long || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_short = 1;
      break;
    case TK_LONG:
      if (has_long == 2 || (has_double && has_long))
        error(&itok->pos, "too long");
      if (has_short || has_char || has_float)
        error(&itok->pos, "conflicted type");
      has_long++;
      break;
    case TK_FLOAT:
      if (has_signed || has_unsigned || has_char || has_short || has_int || has_long || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_float = 1;
      break;
    case TK_DOUBLE:
      if (has_signed || has_unsigned || has_char || has_short || has_int || has_float || has_double)
        error(&itok->pos, "conflicted type");
      if (has_long == 2)
        error(&itok->pos, "too long double");
      has_double = 1;
      break;
    default:
      assert(false);
    }
  }

  if (has_float)
    return base_type(TYPE_FLOAT);

  if (has_double)
    return base_type(has_long ? TYPE_LDOUBLE : TYPE_DOUBLE);

  if (has_char)
    return base_type(has_unsigned ? TYPE_UCHAR : TYPE_CHAR);

  if (has_short)
    return base_type(has_unsigned ? TYPE_USHORT : TYPE_SHORT);

  if (has_long)
    return base_type(has_unsigned ? TYPE_ULONG : TYPE_LONG);

  if (has_unsigned)
    return base_type(TYPE_UINT);

  if (has_signed || has_int)
    return base_type(TYPE_INT);

  return NULL;
}

Type *consume_array_brackets(Type *type) {
  if (!consume(TK_LBRACKET))
    return type;
  int size = -1; // -1 will be assumed by rhs initializer
  if (next_token->kind != TK_RBRACKET) {
    Token *tok_sz = next_token;
    Node *expr_sz = expr();
    if (expr_sz) {
      size = eval(expr_sz);
      if (size < 0)
        error(&tok_sz->pos, "invalid array size");
    }
  }
  Token *r = expect(TK_RBRACKET);
  type = consume_array_brackets(type);
  if (type->size < 0)
    error(&r->pos, "array size assumption is allowed only in the first dimension");
  return array_type(type, size);
}

Type *expect_type() {
  Type *ty = consume_type();
  if (!ty)
    error(&next_token->pos, "type expected but not found");
  return ty;
}

void expect_typedef() {
  Type *base = expect_type();
  do {
    Type *type = declarator(base);
    Type *pre = map_get(current_scope->typedefs, type->objdec->ident);

    // multiple typedef for same type is allowed
    if (pre && !same_type(type, pre))
      error(&type->objdec->pos, "duplicated typedef identifier");
    if (!pre)
      map_push(current_scope->typedefs, type->objdec->ident, type);
  } while (consume(TK_COMMA));
  expect(TK_SEMICOLON);
}

bool at_eof() { return next_token->kind == TK_EOF; }

Object *find_object(Token *tok) {
  assert(tok);
  assert(tok->ident);
  Scope *scope = current_scope;
  while (scope) {
    Object *obj = map_get(scope->objects, tok->ident);
    if (obj)
      return obj;
    scope = scope->prev;
  }
  return NULL;
}

Variable *new_variable(Type *type, ObjectKind kind, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->ident);
  Variable *var = calloc(1, sizeof(Variable));
  var->ident = type->objdec->ident;
  var->type = type;
  var->kind = kind;
  var->token = type->objdec;
  var->is_static = (qualifier & IS_STATIC) != 0;
  var->is_extern = (qualifier & IS_EXTERN) != 0;
  return var;
}

Variable *new_local_variable(Type *type, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->ident);
  if (map_get(current_scope->objects, type->objdec->ident))
    error(&type->objdec->pos, "duplicated identifier");
  Variable *var = new_variable(type, OBJ_LVAR, qualifier);
  map_push(current_scope->objects, var->ident, var);

  static int idx = 0;
  if (qualifier & IS_STATIC) {
    vector_push(static_local_variables, &var);

    // give internal ident
    var->internal_ident = calloc(1, sizeof(Ident));
    var->internal_ident->name = calloc(var->ident->len + 21, sizeof(char));
    sprintf(var->internal_ident->name, ".static_%d_", idx++);
    strncpy(var->internal_ident->name + strlen(var->internal_ident->name), var->ident->name, var->ident->len);
    var->internal_ident->len = strlen(var->internal_ident->name);
  }

  return var;
}

void set_offset(Variable *var) {
  assert(var->kind == OBJ_LVAR);
  assert(var->offset == 0);
  var->offset = local_variable_offset + var->type->size;
  local_variable_offset = var->offset;
}

Variable *new_global(Type *type, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->ident);
  if (map_get(global_scope->objects, type->objdec->ident))
    error(&type->objdec->pos, "duplicated identifier");
  Variable *var = new_variable(type, OBJ_GVAR, qualifier);
  map_push(global_scope->objects, var->ident, var);
  return var;
}

Variable *new_string_literal(Token *tok) {
  static int idx = 0;
  assert(tok->kind == TK_STR);

  Ident *key = calloc(1, sizeof(Ident));
  key->name = tok->string_literal;
  key->len = strlen(tok->string_literal);

  Variable *var = map_get(strings, key);
  if (var)
    return var;

  var = calloc(1, sizeof(Variable));
  var->ident = calloc(1, sizeof(Ident));
  var->ident->name = calloc(15, sizeof(char));
  sprintf(var->ident->name, ".LC%d", idx++);
  var->ident->len = strlen(var->ident->name);
  var->kind = OBJ_STRLIT;
  var->string_literal = tok->string_literal;
  var->token = tok;
  var->is_static = true;
  var->is_extern = false;

  // Because of escaped charactors, actual array length is not always equal to (strlen(var->string_literal) + 1).
  int len = strlen(var->string_literal);
  int array_length = len + 1;
  for (int i = 0; i < len; ++i) {
    char c = *(var->string_literal + i);
    char d = *(var->string_literal + i + 1);
    if (c == '\\' && d == '\n') {
      i++;
      array_length -= 2;
    } else if (c == '\\') {
      i++;
      array_length--;
    }
  }
  var->type = array_type(base_type(TYPE_CHAR), array_length);
  map_push(strings, key, var);
  return var;
}

Vector *vardec(Type *type, ObjectKind kind, int qualifier) {
  assert(type->objdec);

  Type *base = type;
  while (base->kind == TYPE_ARRAY || base->kind == TYPE_PTR || base->kind == TYPE_FUNC) {
    if (base->base)
      base = base->base;
    else if (base->return_type)
      base = base->return_type;
    else
      assert(false);
  }

  bool is_static = (qualifier & IS_STATIC) != 0;
  bool is_extern = (qualifier & IS_EXTERN) != 0;

  Vector *variables = new_vector(0, sizeof(Variable *));
  while (true) {
    if (type->kind == TYPE_VOID)
      error(&type->objdec->pos, "void type is not allowed");
    if (type->kind == TYPE_FUNC)
      error(&type->objdec->pos, "func type is not allowed");
    Variable *var = NULL;
    if (kind == OBJ_GVAR)
      var = new_global(type, qualifier);
    else if (kind == OBJ_LVAR)
      var = new_local_variable(type, qualifier);
    else
      assert(false);

    if (consume(TK_ASSIGN)) {
      if (is_extern)
        error(&type->objdec->pos, "cannot initialize extern variable");
      var->init = varinit();
      if (type->kind == TYPE_ARRAY) {
        if (var->init->vec == NULL) {
          if (type->base->kind != TYPE_CHAR) {
            error(&type->objdec->pos, "invalid initializer for an array");
          } else if (var->init->expr->kind != ND_VAR || var->init->expr->variable->kind != OBJ_STRLIT) {
            error(&type->objdec->pos, "invalid initializer for an array");
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
    } else if (!is_extern && type->array_size < 0) {
      error(&type->objdec->pos, "invalid array size");
    }

    if (kind == OBJ_LVAR && !is_static && !is_extern)
      set_offset(var);

    vector_push(variables, &var);
    if (!consume(TK_COMMA))
      break;
    type = declarator(base);
  }
  expect(TK_SEMICOLON);
  return variables;
}

void func(Type *type, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->ident);

  Function *f = calloc(1, sizeof(Function));
  f->type = type;
  f->kind = OBJ_FUNC;
  f->token = type->objdec;
  f->ident = type->objdec->ident;
  f->is_static = (qualifier & IS_STATIC) != 0;

  Function *prev = map_get(global_scope->objects, f->ident);
  if (prev && prev->kind != OBJ_FUNC)
    error(&type->objdec->pos, "duplicated identifier");

  if (prev && !same_type(prev->type, f->type))
    error(&type->objdec->pos, "conflicted function declaration");

  if (prev == NULL)
    map_push(global_scope->objects, f->ident, f);

  if (!consume(TK_LBRACE)) {
    expect(TK_SEMICOLON);
    return;
  }

  if (prev && prev->body)
    error(&type->objdec->pos, "duplicated function definition");

  if (prev) {
    prev->type = f->type;
    f = prev;
  }

  assert(local_variable_offset == 0);
  assert(current_scope == global_scope);
  new_scope();

  // push params to local scope
  f->params = new_vector(0, sizeof(Variable *));
  for (int i = 0; i < f->type->params->size; ++i) {
    Type *ty = *(Type **)vector_get(f->type->params, i);
    if (ty->objdec == NULL)
      error(&type->objdec->pos, "missing parameter name");
    if (map_get(current_scope->objects, ty->objdec->ident))
      error(&type->objdec->pos, "duplicated parameter identifier");
    Variable *var = new_local_variable(ty, 0);
    set_offset(var);
    vector_push(f->params, &var);
  }

  f->body = stmt_block();
  f->offset = local_variable_offset;
  restore_scope();
  local_variable_offset = 0;
}

void funcparam(Type *ft) {
  if (next_token->kind == TK_VOID && next_token->next->kind == TK_RPAREN) {
    // (void)
    expect(TK_VOID);
    return;
  }
  do {
    if (consume(TK_3DOTS)) {
      ft->is_variadic = true;
      return;
    }
    Token *tok_qualifier = next_token;
    int qualifier = consume_qualifier();
    if (qualifier & (IS_STATIC | IS_EXTERN))
      error(&tok_qualifier->pos, "invalid storage class for funcparam");

    Token *tok_type = next_token;
    Type *ty = expect_type();
    ty->is_const = (qualifier & IS_CONST) != 0;

    ty = declarator(ty);

    if (ty->kind == TYPE_VOID)
      error(&tok_type->pos, "void type is not allowed");

    if (ty->kind == TYPE_FUNC)
      error(&tok_type->pos, "func type is not allowed");

    if (ty->kind == TYPE_ARRAY)
      ty = pointer_type(ty->base); // array as a pointer

    vector_push(ft->params, &ty);
  } while (consume(TK_COMMA));
  return;
}

int consume_qualifier() {
  int qualifier = 0;
  Token *tok = NULL;
  while ((tok = consume(TK_CONST)) || (tok = consume(TK_STATIC)) || (tok = consume(TK_EXTERN))) {
    if (tok->kind == TK_CONST) {
      qualifier |= IS_CONST;
      continue;
    }

    if (qualifier & (IS_STATIC | IS_EXTERN))
      error(&tok->pos, "conflicted storage class");

    if (tok->kind == TK_STATIC)
      qualifier |= IS_STATIC;

    if (tok->kind == TK_EXTERN)
      qualifier |= IS_EXTERN;
  }

  return qualifier;
}

Type *combine_dectype(Type *mid, Type *tail) {
  assert(mid);
  assert(tail);
  assert(tail->kind != TYPE_NONE);

  switch (mid->kind) {
  case TYPE_NONE:
    tail->objdec = mid->objdec;
    return tail;
  case TYPE_PTR:
    assert(mid->base->objdec);
    mid->base = combine_dectype(mid->base, tail);
    assert(mid->base->objdec);
    mid->objdec = mid->base->objdec;
    return mid;
  case TYPE_ARRAY:
    mid->base = combine_dectype(mid->base, tail);
    assert(mid->base->objdec);
    mid->objdec = mid->base->objdec;
    mid->size = mid->base->size * mid->array_size;
    return mid;
  case TYPE_FUNC:
    mid->return_type = combine_dectype(mid->return_type, tail);
    assert(mid->return_type->objdec);
    mid->objdec = mid->return_type->objdec;
    return mid;
  default:
    assert(false);
  }
  return mid;
}

Type *dectail(Type *base) {
  if (next_token->kind == TK_LBRACKET)
    return consume_array_brackets(base);

  if (consume(TK_LPAREN)) {
    Type *ft = func_type(base);
    if (!consume(TK_RPAREN)) {
      funcparam(ft);
      expect(TK_RPAREN);
    }
    return ft;
  }
  return base;
}

Type *declarator(Type *base) {
  Type *type = consume_type_star(base);

  if (consume(TK_LPAREN)) {
    Type dummy;
    dummy.objdec = NULL;
    dummy.kind = TYPE_NONE;
    dummy.size = 0;
    Type *mid = declarator(&dummy);
    expect(TK_RPAREN);
    Type *tail = dectail(base);
    return combine_dectype(mid, tail);
  }

  type->objdec = base->objdec = consume(TK_IDENT);
  return dectail(type);
}

Node *declaration() {
  if (consume(TK_TYPEDEF)) {
    expect_typedef();
    return new_node_nop();
  }

  Token *tok_qualifier = next_token;
  int qualifier = consume_qualifier();

  Token *tk = next_token;
  Type *type = consume_type();
  if (type == NULL) {
    if (current_scope == global_scope)
      error(&tk->pos, "unknown type");
    if (qualifier)
      error(&tok_qualifier->pos, "invalid qualifier");
    return NULL;
  }

  if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION || type->kind == TYPE_ENUM) && consume(TK_SEMICOLON)) {
    // type declaration only (qualifier is allowed)
    return new_node_nop();
  }

  type = declarator(type);

  if (type->kind == TYPE_FUNC) {
    if (current_scope != global_scope)
      error(&tk->pos, "function declaration is only allowed in global scope");
    func(type, qualifier);
    return NULL;
  }

  ObjectKind kind = current_scope == global_scope ? OBJ_GVAR : OBJ_LVAR;
  Vector *vars = vardec(type, kind, qualifier);
  if (kind == OBJ_GVAR)
    return NULL;
  if (qualifier & IS_STATIC)
    return new_node_nop();
  Node *node = init_multiple_local_variables(vars);
  if (node == NULL)
    node = new_node_nop();
  return node;
}

VariableInit *varinit() {
  VariableInit *init = calloc(1, sizeof(VariableInit));
  if (consume(TK_LBRACE)) {
    Token *tok = consume(TK_RBRACE);
    if (tok)
      error(&tok->pos, "empty brace initializer is not allowed");
    init->vec = new_vector(0, sizeof(VariableInit *));

    while (true) {
      VariableInit *i = varinit();
      if (i->vec)
        init->nested = true;
      vector_push(init->vec, &i);
      if (consume(TK_COMMA)) {
        if (consume(TK_RBRACE))
          break;
      } else {
        expect(TK_RBRACE);
        break;
      }
    }
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
      if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == OBJ_STRLIT) {
        // initilize the array as a string
        char *lit = init->expr->variable->string_literal;
        if (type->array_size != (int)strlen(lit) + 1)
          error(&var->token->pos, "miss-match between array-size and string-length");

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
  } else if (is_integer(type) || type->kind == TYPE_PTR) {
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    if (var->type->kind == TYPE_ARRAY) {
      return new_node_array_set_expr(var, array_index_offset, init->expr);
    } else
      return new_node_assign_ignore_const(NULL, new_node_var(NULL, var), init->expr);
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
  Token *tok = next_token;

  if (consume(TK_SEMICOLON))
    return NULL;

  if (consume(TK_LBRACE)) {
    new_scope();
    Node *node = stmt_block();
    restore_scope();
    return node;
  }

  if (consume(TK_IF))
    return stmt_if();

  if (consume(TK_DO))
    return stmt_do();

  if (consume(TK_WHILE))
    return stmt_while();

  if (consume(TK_FOR))
    return stmt_for();

  if (consume(TK_SWITCH))
    return stmt_switch();

  if (consume(TK_CASE)) {
    if (switch_nodes->size == 0)
      error(&tok->pos, "invalid case");
    Node *sw = *(Node **)vector_last(switch_nodes);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CASE;
    node->token = tok;
    node->label_index = label_index++;

    Node *e = expr();
    long long val = eval(e);
    Node *c = sw->next_case;
    while (c) {
      assert(c->condition->kind == ND_NUM);
      if (c->condition->val == val)
        error(&tok->pos, "duplicated case value detected");
      c = c->next_case;
    }
    node->condition = new_node_num(NULL, val, e->type);
    expect(TK_COLON);
    node->body = stmt();
    node->next_case = sw->next_case;
    sw->next_case = node;
    return node;
  }

  if (consume(TK_DEFAULT)) {
    expect(TK_COLON);
    if (switch_nodes->size == 0)
      error(&tok->pos, "invalid default");
    Node *sw = *(Node **)vector_last(switch_nodes);
    if (sw->default_)
      error(&tok->pos, "multiple default for one switch");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFAULT;
    node->token = tok;
    node->label_index = label_index++;
    node->body = stmt();
    sw->default_ = node;
    return node;
  }

  if (consume(TK_RETURN)) {
    Node *node = calloc(1, sizeof(Node));
    node->token = tok;
    node->kind = ND_RETURN;
    if (!consume(TK_SEMICOLON)) {
      node->lhs = expr();
      expect(TK_SEMICOLON);
    }
    return node;
  }

  if (consume(TK_CONTINUE)) {
    if (continue_label->size == 0)
      error(&tok->pos, "invalid continue");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CONTINUE;
    node->label_index = vector_lasti(continue_label);
    node->token = tok;
    expect(TK_SEMICOLON);
    return node;
  }

  if (consume(TK_BREAK)) {
    if (break_label->size == 0)
      error(&tok->pos, "invalid break");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BREAK;
    node->label_index = vector_lasti(break_label);
    node->token = tok;
    expect(TK_SEMICOLON);
    return node;
  }

  Node *node = expr();
  expect(TK_SEMICOLON);
  return node;
}

Node *stmt_block() {
  Token *tok;
  if ((tok = consume(TK_RBRACE)))
    return NULL;

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));
  node->token = tok;

  do {
    Node *s;
    if ((s = declaration())) {
      if (s->kind != ND_NOP)
        vector_push(node->blk_stmts, &s);
    } else if ((s = stmt())) {
      vector_push(node->blk_stmts, &s);
    }
  } while (!consume(TK_RBRACE));

  return node;
}

Node *stmt_if() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;
  node->label_index = label_index++;

  expect(TK_LPAREN);
  node->condition = expr();
  Token *tok = expect(TK_RPAREN);

  new_scope();
  node->body = stmt();
  restore_scope();

  if ((tok = consume(TK_ELSE)))
    node->else_ = stmt();

  return node;
}

Node *stmt_do() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DO;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  new_scope();
  node->body = stmt();
  restore_scope();

  expect(TK_WHILE);
  expect(TK_LPAREN);
  node->condition = expr();
  expect(TK_RPAREN);
  expect(TK_SEMICOLON);

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_while() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  expect(TK_LPAREN);
  node->condition = expr();
  expect(TK_RPAREN);

  new_scope();
  node->body = stmt();
  restore_scope();

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_for() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  expect(TK_LPAREN);

  new_scope();
  if (!consume(TK_SEMICOLON)) {
    Token *tok = next_token;
    Type *type = consume_type();
    if (type) {
      if (type->size < 0)
        error(&tok->pos, "incomplete type");

      type = declarator(type);
      Vector *vars = vardec(type, OBJ_LVAR, 0);
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

  node->body = stmt();
  restore_scope();

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_switch() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_SWITCH;
  node->label_index = label_index++;
  vector_pushi(break_label, node->label_index);
  vector_push(switch_nodes, &node);

  expect(TK_LPAREN);
  node->condition = expr();
  expect(TK_RPAREN);

  new_scope();
  node->body = stmt();
  restore_scope();

  vector_pop(break_label);
  vector_pop(switch_nodes);

  return node;
}

Node *expr() { return assign(); }

Node *assign() {
  Node *node = conditional();
  Token *tok = next_token;
  if (consume(TK_ASSIGN))
    node = new_node_assign(tok, node, assign());
  if (consume(TK_ADDEQ))
    node = new_node_assign(tok, node, new_node_add(NULL, node, assign()));
  if (consume(TK_SUBEQ))
    node = new_node_assign(tok, node, new_node_sub(NULL, node, assign()));
  if (consume(TK_MULEQ))
    node = new_node_assign(tok, node, new_node_mul(NULL, node, assign()));
  if (consume(TK_DIVEQ))
    node = new_node_assign(tok, node, new_node_div(NULL, node, assign()));
  if (consume(TK_MODEQ))
    node = new_node_assign(tok, node, new_node_mod(NULL, node, assign()));
  if (consume(TK_XOREQ))
    node = new_node_assign(tok, node, new_node_bitxor(NULL, node, assign()));
  if (consume(TK_ANDEQ))
    node = new_node_assign(tok, node, new_node_bitand(NULL, node, assign()));
  if (consume(TK_OREQ))
    node = new_node_assign(tok, node, new_node_bitor(NULL, node, assign()));
  if (consume(TK_LSHIFTEQ))
    node = new_node_assign(tok, node, new_node_lshift(NULL, node, assign()));
  if (consume(TK_RSHIFTEQ))
    node = new_node_assign(tok, node, new_node_rshift(NULL, node, assign()));
  return node;
}

Node *conditional() {
  Node *node = logor();
  Token *tok = consume(TK_QUESTION);
  if (!tok)
    return node;
  Node *lhs = expr();
  expect(TK_COLON);
  return new_node_conditional(tok, node, lhs, conditional(), label_index++);
}

Node *logor() {
  Node *node = logand();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_LOGOR))
      node = new_node_logor(tok, node, logand(), label_index++);
    else
      return node;
  }
}

Node *logand() {
  Node *node = bitor ();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_LOGAND))
      node = new_node_logand(tok, node, bitor (), label_index++);
    else
      return node;
  }
}

Node * bitor () {
  Node *node = bitxor();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_BAR))
      node = new_node_bitor(tok, node, bitxor());
    else
      return node;
  }
}

Node *bitxor() {
  Node *node = bitand();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_HAT))
      node = new_node_bitxor(tok, node, bitand());
    else
      return node;
  }
}

Node *bitand() {
  Node *node = equality();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_AMP))
      node = new_node_bitand(tok, node, equality());
    else
      return node;
  }
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
  Node *node = bitshift();
  while (true) {
    Token *tok;
    if ((tok = consume(TK_LT)))
      node = new_node_lt(tok, node, bitshift());
    else if ((tok = consume(TK_LE)))
      node = new_node_le(tok, node, bitshift());
    if ((tok = consume(TK_GT)))
      node = new_node_lt(tok, bitshift(), node);
    else if ((tok = consume(TK_GE)))
      node = new_node_le(tok, bitshift(), node);
    else
      return node;
  }
}

Node *bitshift() {
  Node *node = add();
  while (true) {
    Token *tok = next_token;
    if (consume(TK_LSHIFT)) {
      node = new_node_lshift(tok, node, add());
    } else if (consume(TK_RSHIFT)) {
      node = new_node_rshift(tok, node, add());
    } else
      return node;
  }
}

Node *add() {
  Node *node = mul();
  while (true) {
    Token *tok = next_token;
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
    Token *tok = next_token;
    if (consume(TK_STAR))
      node = new_node_mul(tok, node, unary());
    else if (consume(TK_SLASH))
      node = new_node_div(tok, node, unary());
    else if (consume(TK_PERCENT))
      node = new_node_mod(tok, node, unary());
    else
      return node;
  }
}

Node *unary() {
  Token *tok = next_token;
  if (consume(TK_SIZEOF)) {
    Token *keep = next_token;
    Token *left_paren = consume(TK_LPAREN);
    Type *type = consume_type();
    if (left_paren && type) {
      // sizeof(type)
      type = consume_array_brackets(consume_type_star(type));
      if (type->size < 0)
        error(&tok->pos, "invalid array size");
      expect(TK_RPAREN);
    } else {
      // sizeof unary
      next_token = keep; // rollback token
      type = unary()->type;
    }
    if (type->kind == TYPE_VOID)
      error(&tok->pos, "invalud sizeof operation for void type");
    if (type->size < 0)
      error(&tok->pos, "invalud sizeof operation for incomplete type");
    return new_node_num(tok, type->size, base_type(TYPE_ULONG));
  }
  if (consume(TK_INC)) {
    Node *u = unary();
    return new_node_assign(NULL, u, new_node_add(NULL, u, new_node_num(NULL, 1, u->type)));
  }
  if (consume(TK_DEC)) {
    Node *u = unary();
    return new_node_assign(NULL, u, new_node_sub(NULL, u, new_node_num(NULL, 1, u->type)));
  }

  if (consume(TK_PLUS))
    return unary();

  if (consume(TK_MINUS)) {
    Node *u = unary();
    return new_node_sub(tok, new_node_num(NULL, 0, u->type), u);
  }

  if (consume(TK_AMP))
    return new_node_addr(tok, unary());
  if (consume(TK_STAR))
    return new_node_deref(tok, unary());
  if (consume(TK_LOGNOT))
    return new_node_lognot(tok, unary());
  if (consume(TK_TILDE))
    return new_node_bitnot(tok, unary());

  if (consume(TK_LPAREN)) {
    Type *type = consume_type();
    if (type) {
      type = consume_type_star(type);
      if (consume(TK_RPAREN))
        return new_node_cast(tok, type, unary());
    }
    next_token = tok; // rollback
  }
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
    int *enum_val = find_enum_element(tok->ident);
    if (enum_val)
      return new_node_num(tok, *enum_val, base_type(TYPE_INT));

    Variable *var = find_object(tok);
    if (!var)
      error(&tok->pos, "undefined identifier: '%.*s'", tok->token_length, tok->pos.pos);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_STR))) {
    Variable *var = new_string_literal(tok);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_NUM)))
    return new_node_num(tok, tok->val, tok->type);

  error(&next_token->pos, "primary expected but not found", next_token->token_length, next_token->pos);
  return NULL;
}

Node *tail(Node *x) {
  if (consume(TK_LBRACKET)) {
    // x[y] --> *(x+y)
    Node *y = expr();
    Token *tok = expect(TK_RBRACKET);
    return new_node_deref(tok, new_node_add(tok, x, y));
  }

  Token *op = next_token;

  if (consume(TK_DOT)) {
    // struct member access (x.y)
    if (x->type == NULL || (x->type->kind != TYPE_STRUCT && x->type->kind != TYPE_UNION))
      error(&op->pos, "not a struct");
    Token *y = expect(TK_IDENT);
    Member *member = find_member(x->type, y);
    if (member == NULL)
      error(&y->pos, "unknown struct member");
    return new_node_member(op, x, member);
  }

  if (consume(TK_ARROW)) {
    // struct member access
    // x->y is (*x).y
    if (x->type == NULL || x->type->kind != TYPE_PTR || (x->type->base->kind != TYPE_STRUCT && x->type->base->kind != TYPE_UNION))
      error(&op->pos, "not a struct pointer");
    Token *y = expect(TK_IDENT);
    Member *member = find_member(x->type->base, y);
    if (member == NULL)
      error(&y->pos, "unknown struct member");
    return new_node_member(op, new_node_deref(NULL, x), member);
  }

  if (consume(TK_INC)) {
    // (x = x + 1) - 1
    return new_node_sub(NULL, new_node_assign(NULL, x, new_node_add(NULL, x, new_node_num(NULL, 1, x->type))), new_node_num(NULL, 1, x->type));
  }

  if (consume(TK_DEC)) {
    // (x = x - 1) + 1
    return new_node_add(NULL, new_node_assign(NULL, x, new_node_sub(NULL, x, new_node_num(NULL, 1, x->type))), new_node_num(NULL, 1, x->type));
  }

  if (consume(TK_LPAREN)) {
    // function call
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CALL;
    node->token = op;

    if (!is_funcptr(x->type))
      error(&op->pos, "not a function");

    Type *f = x->type->base;

    node->lhs = x;
    node->type = f->return_type;
    node->args = new_vector(0, sizeof(Node *));

    if (consume(TK_RPAREN))
      return node;

    do {
      Node *e = expr();

      // TODO
      if (!f->is_variadic && f->params->size == node->args->size)
        error(&op->pos, "too many arguments");

      if (f->params->size > node->args->size) {
        // argument type conversion
        Type *t = *(Type **)vector_get(f->params, node->args->size);
        e = new_node_cast(op, t, e);
      }

      vector_push(node->args, &e);
    } while (consume(TK_COMMA));

    expect(TK_RPAREN);
    return node;
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

void parse(Token *input) {
  assert(input);
  next_token = input;
  strings = new_map();
  static_local_variables = new_vector(0, sizeof(Variable *));
  break_label = new_vector(0, sizeof(int));
  continue_label = new_vector(0, sizeof(int));
  switch_nodes = new_vector(0, sizeof(Node *));
  assert(current_scope == NULL);
  new_scope();
  global_scope = current_scope;
  while (!at_eof())
    declaration();
}
