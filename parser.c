#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// clang-format off
/* BNF
program     = declaration*
declaration = func | vardec | (struct ";") | (enum ";") | typedef
declarator  = "*"* ident dectail?
              | "*"* "(" declarator ")" dectail?
dectail     = ("[" num "]")*
              | "(" funcparam? ")"
type        = "void" | "_Bool" | int_type | float_type | struct | enum
int_type    = ("signed" | "unsigned")? ("int" | "char" | ("short" "int"?) | ("long" "long"? "int"?))
float_type  = "float" | ("long"? double "long"?)
struct      = (("struct" | "union") ident ("{" member* "}")?) | (("struct" | "union") ident? "{" member* "}")
member      = "const"* type declarator ("," declarator )* ";"
enum        = ("enum" ident ("{" enumval ("," enumval)* ","? "}")?) | ("enum" ident? "{" enumval ("," enumval)* ","? "}")
enumval     = indent ("=" assign)?
typedef     = "typedef" qualifier? type declarator ("," declarator)* ";"
qualifier   = ("const" | "static" | "extern")*
vardec      = qualifier? type declarator ("=" varinit)?  ("," declarator ("=" varinit)?)* ";"
varinit     = assign
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
              | ident ":" stmt
              | "goto" ident ";"
expr        = comma
comma       = assign ("," comma)?
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
               | ("(" (assign ("," assign)*)? ")")
primary     = "(" expr ")"
              | ident
              | num
              | str
*/
// clang-format on

// node.c -->
Node *new_node_nop();
Node *new_node_num_int(Token *tok, int val);
Node *new_node_num_char(Token *tok, char val);
Node *new_node_num_ulong(Token *tok, unsigned long long val);
Node *new_node_num_zero(Token *tok, Type *type);
Node *new_node_num(Token *tok, Number *num);
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
Node *new_node_comma(Token *tok, Node *lhs, Node *rhs);
Node *new_node_assign(Token *tok, Node *lhs, Node *rhs);
Node *new_node_assign_ignore_const(Token *tok, Node *lhs, Node *rhs);
Node *new_node_conditional(Token *tok, Node *cond, Node *lhs, Node *rhs,
                           int label_index);
Node *new_node_cast(Token *tok, Type *type, Node *lhs);
Node *new_node_member(Token *tok, Node *x, Member *y);
Node *new_node_var(Token *tok, Variable *var);
Node *new_node_array_access(Token *tok, Node *array, int idx);
Node *new_node_array_access_as_1D(Token *tok, Node *array, int idx);
Number *eval(Node *node);
// <-- node.c

typedef enum {
  IS_STATIC = 1,
  IS_CONST = 2,
  IS_EXTERN = 4,
} Qualifier;

int consume_qualifier(Token **nx);
Node *declaration(Token **nx);
Type *declarator(Type *base, Token **nx);
void func(Type *type, int qualifier, Token **nx);
void funcparam(Type *ft, Token **nx);
VariableInit *varinit(Token **nx);
Node *stmt(Token **nx);
Node *expr(Token **nx);
Node *comma(Token **nx);
Node *assign(Token **nx);
Node *conditional(Token **nx);
Node *logor(Token **nx);
Node *logand(Token **nx);
Node * bitor (Token * *nx);
Node *bitxor(Token **nx);
Node *bitand(Token **nx);
Node *equality(Token **nx);
Node *relational(Token **nx);
Node *bitshift(Token **nx);
Node *add(Token **nx);
Node *mul(Token **nx);
Node *primary(Token **nx);
Node *unary(Token **nx);
Node *postfix(Token **nx);

Node *stmt_if(Token **nx);
Node *stmt_do(Token **nx);
Node *stmt_while(Token **nx);
Node *stmt_for(Token **nx);
Node *stmt_switch(Token **nx);
Node *stmt_block(Token **nx);

void expect_typedef(Token **nx);
Type *consume_type(Token **nx);
Type *consume_type_star(Type *type, Token **nx);
Type *consume_array_brackets(Type *type, Token **nx);
Node *init_multiple_local_variables(Vector *variables);

static Scope *current_scope = NULL;
static Function *current_function;
static Vector *goto_in_current_function;  // Node*
static Vector *label_in_current_function; // Node*

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
  if (current_scope)
    s->offset = current_scope->offset;
  current_scope = s;
}

void restore_scope() {
  assert(current_scope);
  current_scope = current_scope->prev;
}

Token *consume(TokenKind kind, Token **nx) {
  if ((*nx)->kind != kind)
    return NULL;
  Token *tok = *nx;
  *nx = (*nx)->next;
  return tok;
}

Token *expect(TokenKind kind, Token **nx) {
  if ((*nx)->kind != kind)
    error(&(*nx)->pos, "'%s' expected but not found", token_text[kind]);
  Token *tok = *nx;
  *nx = (*nx)->next;
  return tok;
}

Type *find_typedef(String *id) {
  Scope *scope = current_scope;
  while (scope) {
    Type *t = map_get(scope->typedefs, id);
    if (t)
      return t;
    scope = scope->prev;
  }
  return NULL;
}

Type *find_type(String *id) {
  Scope *scope = current_scope;
  while (scope) {
    Type *t = map_get(scope->types, id);
    if (t)
      return t;
    scope = scope->prev;
  }
  return NULL;
}

int *find_enum_element(String *id) {
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
  assert(is_struct_union(st));
  assert(tok->kind == TK_IDENT);
  Member *member = st->member;
  while (member) {
    if (same_string(member->ident, tok->str))
      return member;
    member = member->next;
  }
  return NULL;
}

Type *consume_struct(TypeKind kind, Token **nx) {
  assert(kind == TYPE_STRUCT || kind == TYPE_UNION);
  Token *tag = consume(TK_IDENT, nx);
  Type *st = NULL;

  if (tag) {
    if (consume(TK_LBRACE, nx)) {
      // define struct/union
      // struct/union can be defined only for the current-scope
      st = map_get(current_scope->types, tag->str);
      if (st == NULL) {
        st = kind == TYPE_UNION ? union_type(false) : struct_type(false);
        map_push(current_scope->types, tag->str, st);
      } else if (kind != st->kind || st->size > 0) {
        error(&tag->pos, "conflicted type tag");
      }
    } else {
      // declare struct/union
      // multipe time declaration is allowed
      st = find_type(tag->str);
      if (st == NULL) {
        st = kind == TYPE_UNION ? union_type(false) : struct_type(false);
        map_push(current_scope->types, tag->str, st);
      } else if (kind != st->kind) {
        error(&tag->pos, "conflicted type tag");
      }
      return st;
    }
  } else {
    // unnamed struct/union
    expect(TK_LBRACE, nx);
    st = kind == TYPE_UNION ? union_type(true) : struct_type(true);
  }

  assert(st != NULL);

  Member head;
  head.next = NULL;
  head.offset = 0;
  Member *tail = &head;
  int offset = 0;
  int align = 0;

  while (!consume(TK_RBRACE, nx)) {
    Token *tok_qualifier = *nx;
    int qualifier = consume_qualifier(nx);
    if (qualifier & (IS_STATIC | IS_EXTERN))
      error(&tok_qualifier->pos, "invalid storage class for a member");

    Token *tok_type = *nx;
    Type *base = consume_type(nx);
    if (!base)
      error(&tok_type->pos, "invalid member type");
    base->is_const = (qualifier & IS_CONST) == IS_CONST;

    if ((*nx)->kind == TK_SEMICOLON) {
      if (!is_struct_union(base))
        error(&tok_type->pos, "identifier required for the member");
      if (!base->is_unnamed)
        error(&tok_type->pos, "identifier required for the member");
      expect(TK_SEMICOLON, nx);

      // merge unnamed struct/union

      if (kind == TYPE_UNION) {
        if (offset < base->size)
          offset = base->size;
        if (align < base->align)
          align = base->align;

        tail->next = base->member;
        Member *m = base->member;
        while (m->next)
          m = m->next;
        tail = m;
        continue;
      }

      if (base->kind == TYPE_STRUCT) {
        Member *m = base->member;
        assert(m);

        while (m) {
          Type *type = m->type;
          tail->padding = iceil(offset, type->align) - offset;
          offset += tail->padding;
          m->offset = offset;
          offset += type->size;
          if (type->align > align)
            align = type->align;
          tail->next = m;
          tail = m;

          m = m->next;
        }

        continue;
      }

      if (base->kind == TYPE_UNION) {
        tail->padding = iceil(offset, base->align) - offset;
        offset += tail->padding;

        Member *m = base->member;
        while (m) {
          m->offset += offset;
          tail->next = m;
          tail = m;
          m = m->next;
        }
        offset += base->size;
        if (base->align > align)
          align = base->align;

        continue;
      }
      assert(false);
    }

    do {
      Type *type = declarator(base, nx);

      if (type->size < 0)
        error(&tok_type->pos, "incomplete type");

      if (type->kind == TYPE_VOID)
        error(&tok_type->pos, "void type is not allowed");

      if (type->kind == TYPE_FUNC)
        error(&tok_type->pos, "function type is not allowed");

      Member *m = calloc(1, sizeof(Member));
      m->ident = type->objdec->str;
      m->type = type;

      if (kind == TYPE_UNION) {
        m->offset = 0;
        if (offset < type->size)
          offset = type->size;
        if (align < type->align)
          align = type->align;
      } else {
        tail->padding = iceil(offset, type->align) - offset;
        offset += tail->padding;
        m->offset = offset;
        offset += type->size;
        if (type->align > align)
          align = type->align;
      }

      tail->next = m;
      tail = m;
    } while (consume(TK_COMMA, nx));
    expect(TK_SEMICOLON, nx);
  }

  if (align && offset % align) {
    tail->padding = iceil(offset, align) - offset;
    offset += tail->padding;
  }

  st->member = head.next;
  st->size = offset;
  st->align = align;

  if (kind == TYPE_UNION) {
    Member *m = head.next;
    while (m) {
      m->padding = st->size - m->type->size;
      m = m->next;
    }
  }

  return st;
}

Type *consume_enum(Token **nx) {
  Token *tag = consume(TK_IDENT, nx);
  Type *et = NULL;

  if (tag) {
    // named enum
    if (consume(TK_LBRACE, nx)) {
      // define enum
      // enum can be defined only for the current-scope
      et = map_get(current_scope->types, tag->str);
      if (et == NULL) {
        et = enum_type(false);
        map_push(current_scope->types, tag->str, et);
      } else if (et->kind != TYPE_ENUM || et->size > 0) {
        error(&tag->pos, "conflicted type tag");
      }
    } else {
      // declare enum
      // multipe time declaration is allowed
      et = find_type(tag->str);
      if (et == NULL) {
        et = enum_type(false);
        map_push(current_scope->types, tag->str, et);
      } else if (et->kind != TYPE_ENUM) {
        error(&tag->pos, "conflicted type tag");
      }
      return et;
    }
  } else {
    // unnamed enum
    expect(TK_LBRACE, nx);
    et = enum_type(true);
  }

  assert(et != NULL);
  et->size = base_type(TYPE_INT)->size;

  int val = -1;
  while (true) {
    Token *id = expect(TK_IDENT, nx);
    if (map_get(current_scope->enum_elements, id->str))
      error(&id->pos, "duplicated identifier for enum element");

    if (consume(TK_ASSIGN, nx)) {
      Node *nd = assign(nx);
      if (!nd->is_constant_expr)
        error(&id->pos, "enum value must be a constant expr");
      if (!is_integer(nd->num->type))
        error(&id->pos, "enum value must be an integer");
      val = number2int(nd->num) - 1;
    }
    int *v = calloc(1, sizeof(int));
    *v = ++val;
    map_push(current_scope->enum_elements, id->str, v);
    if (consume(TK_COMMA, nx)) {
      if (consume(TK_RBRACE, nx))
        break;
    } else {
      expect(TK_RBRACE, nx);
      break;
    }
  }

  return et;
}

Type *consume_type_star(Type *type, Token **nx) {
  Token *tok = NULL;
  bool is_const = false;
  while ((tok = consume(TK_STAR, nx)) || (tok = consume(TK_CONST, nx))) {
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

Type *consume_type(Token **nx) {
  if ((*nx)->kind == TK_IDENT) {
    Type *t = find_typedef((*nx)->str);
    if (t)
      expect(TK_IDENT, nx);
    return t;
  }

  if (consume(TK_VOID, nx))
    return base_type(TYPE_VOID);

  if (consume(TK_BOOL, nx))
    return base_type(TYPE_BOOL);

  if (consume(TK_STRUCT, nx))
    return consume_struct(TYPE_STRUCT, nx);

  if (consume(TK_UNION, nx))
    return consume_struct(TYPE_UNION, nx);

  if (consume(TK_ENUM, nx))
    return consume_enum(nx);

  int has_signed = 0;
  int has_unsigned = 0;
  int has_long = 0;
  int has_short = 0;
  int has_char = 0;
  int has_int = 0;
  int has_float = 0;
  int has_double = 0;

  Token *itok = NULL;
  while ((itok = consume(TK_UNSIGNED, nx)) || (itok = consume(TK_SIGNED, nx)) ||
         (itok = consume(TK_INT, nx)) || (itok = consume(TK_CHAR, nx)) ||
         (itok = consume(TK_SHORT, nx)) || (itok = consume(TK_LONG, nx)) ||
         (itok = consume(TK_FLOAT, nx)) || (itok = consume(TK_DOUBLE, nx))) {

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
      if (has_signed || has_unsigned || has_char || has_short || has_int ||
          has_long || has_float || has_double)
        error(&itok->pos, "conflicted type");
      has_float = 1;
      break;
    case TK_DOUBLE:
      if (has_signed || has_unsigned || has_char || has_short || has_int ||
          has_float || has_double)
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

Type *consume_array_brackets(Type *type, Token **nx) {
  if (!consume(TK_LBRACKET, nx))
    return type;
  int size = -1; // if size==-1, size will be assumed by rhs initializer
  if ((*nx)->kind != TK_RBRACKET) {
    Token *tok_sz = *nx;
    Node *expr_sz = expr(nx);
    if (expr_sz) {
      if (!expr_sz->is_constant_expr)
        error(&tok_sz->pos, "invalid array size (not a constant expr)");
      size = number2int(expr_sz->num);
      if (!is_integer(expr_sz->num->type) || size < 0)
        error(&tok_sz->pos, "invalid array size");
    }
  }
  Token *r = expect(TK_RBRACKET, nx);
  type = consume_array_brackets(type, nx);
  if (type->size < 0)
    error(&r->pos,
          "array size assumption is allowed only in the first dimension");
  return array_type(type, size);
}

Type *expect_type(Token **nx) {
  Type *ty = consume_type(nx);
  if (!ty)
    error(&(*nx)->pos, "type expected but not found");
  return ty;
}

void expect_typedef(Token **nx) {
  Token *tok_qualifier = *nx;
  int qualifier = consume_qualifier(nx);
  if (qualifier & (IS_STATIC | IS_EXTERN))
    error(&tok_qualifier->pos, "invalid storage class for typedef");

  Type *base = expect_type(nx);
  base->is_const = (qualifier & IS_CONST) != 0;
  do {
    Type *type = declarator(base, nx);
    Type *pre = map_get(current_scope->typedefs, type->objdec->str);

    // multiple typedef for same type is allowed
    if (pre && !same_type(type, pre))
      error(&type->objdec->pos, "duplicated typedef identifier");
    if (!pre)
      map_push(current_scope->typedefs, type->objdec->str, type);
  } while (consume(TK_COMMA, nx));
  expect(TK_SEMICOLON, nx);
}

Object *find_object(Token *tok) {
  assert(tok);
  assert(tok->str);
  Scope *scope = current_scope;
  while (scope) {
    Object *obj = map_get(scope->objects, tok->str);
    if (obj)
      return obj;
    scope = scope->prev;
  }
  return NULL;
}

Variable *new_variable(Type *type, ObjectKind kind, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->str);
  Variable *var = calloc(1, sizeof(Variable));
  var->ident = type->objdec->str;
  var->type = type;
  var->kind = kind;
  var->token = type->objdec;
  var->is_static = (qualifier & IS_STATIC) != 0;
  var->is_extern = (qualifier & IS_EXTERN) != 0;
  return var;
}

Variable *new_local_variable(Type *type, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->str);
  bool is_extern = qualifier & IS_EXTERN;
  bool is_static = qualifier & IS_STATIC;

  Variable *prev = map_get(current_scope->objects, type->objdec->str);
  if (prev) {
    if (!is_extern || !prev->is_extern || !same_type(type, prev->type))
      error(&type->objdec->pos, "conflicted identifier %.*s",
            type->objdec->str->len, type->objdec->str->str);
    return prev;
  }

  Variable *var = new_variable(type, OBJ_LVAR, qualifier);
  map_push(current_scope->objects, var->ident, var);

  static int idx = 0;
  if (is_static) {
    vector_push(static_local_variables, &var);

    // give internal ident
    char *id = calloc(var->ident->len + 21, sizeof(char));
    sprintf(id, ".static_%d_", idx++);
    strncpy(id + strlen(id), var->ident->str, var->ident->len);
    var->internal_ident = new_string(id, 0);
  }

  return var;
}

void set_offset(Variable *var) {
  assert(var->kind == OBJ_LVAR);
  assert(var->offset == 0);
  var->offset = current_scope->offset + var->type->size;
  current_scope->offset = var->offset;
  if (current_function->offset < var->offset)
    current_function->offset = var->offset;
}

Variable *new_global(Type *type, int qualifier) {
  assert(type->objdec);
  assert(type->objdec->str);

  bool is_extern = qualifier & IS_EXTERN;
  bool is_static = qualifier & IS_STATIC;

  Variable *prev = map_get(global_scope->objects, type->objdec->str);
  if (prev) {
    if (is_static != prev->is_static || !same_type(type, prev->type))
      error(&type->objdec->pos, "conflicted identifier");
    if (!is_extern)
      prev->is_extern = false;

    if (prev->type->kind == TYPE_ARRAY && prev->type->size < -1 &&
        type->size >= 0)
      prev->type = type;
    return prev;
  }

  Variable *var = new_variable(type, OBJ_GVAR, qualifier);
  map_push(global_scope->objects, var->ident, var);
  return var;
}

Variable *new_string_literal(Token *tok) {
  static int idx = 0;
  assert(tok->kind == TK_STR);

  String *key = new_string(tok->string_literal, 0);

  Variable *var = map_get(strings, key);
  if (var)
    return var;

  var = calloc(1, sizeof(Variable));

  char *id = calloc(15, sizeof(char));
  sprintf(id, ".LC%d", idx++);
  var->ident = new_string(id, 0);

  var->kind = OBJ_STRLIT;
  var->string_literal = tok->string_literal;
  var->token = tok;
  var->is_static = true;
  var->is_extern = false;

  // Because of escaped charactors, actual array length is not always equal to
  // (strlen(var->string_literal) + 1).
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

Vector *vardec(Type *type, ObjectKind kind, int qualifier, Token **nx) {
  assert(type->objdec);

  Type *base = type;
  while (base->kind == TYPE_ARRAY || base->kind == TYPE_PTR ||
         base->kind == TYPE_FUNC) {
    if (base->base)
      base = base->base;
    else if (base->return_type)
      base = base->return_type;
    else
      assert(false);
  }

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

    if (consume(TK_ASSIGN, nx)) {
      if (var->is_extern)
        error(&type->objdec->pos, "cannot initialize extern variable");
      var->token = type->objdec;
      var->init = varinit(nx);

      if (is_struct_union(var->type) && var->init->vec == NULL &&
          !same_type(var->type, var->init->expr->type))
        error(&type->objdec->pos, "invalid initializer for a struct/union");

      if (var->type->kind == TYPE_ARRAY) {
        if (var->init->vec == NULL) {
          if (var->type->base->kind != TYPE_CHAR) {
            error(&type->objdec->pos, "invalid initializer for an array");
          } else if (var->init->expr->kind != ND_VAR ||
                     var->init->expr->variable->kind != OBJ_STRLIT) {
            error(&type->objdec->pos, "invalid initializer for an array");
          } else if (var->type->array_size < 0) {
            var->type->array_size = var->init->expr->variable->type->array_size;
            var->type->size = var->init->expr->variable->type->size;
          }
        } else if (var->type->array_size < 0) {
          var->type->array_size = var->init->vec->size;
          var->type->size = var->type->base->size * var->type->array_size;
        }
        assert(var->type->size >= 0);
        assert(var->type->array_size >= 0);
      }
    } else if (!var->is_extern && kind == OBJ_LVAR && type->array_size < 0) {
      error(&type->objdec->pos, "invalid array size");
    }

    if (kind == OBJ_LVAR && !var->is_static && !var->is_extern)
      set_offset(var);

    vector_push(variables, &var);
    if (!consume(TK_COMMA, nx))
      break;
    type = declarator(base, nx);
  }
  expect(TK_SEMICOLON, nx);
  return variables;
}

void func(Type *type, int qualifier, Token **nx) {
  assert(type->kind == TYPE_FUNC);
  assert(type->objdec);
  assert(type->objdec->str);

  Function *f = calloc(1, sizeof(Function));
  f->type = type;
  f->kind = OBJ_FUNC;
  f->token = type->objdec;
  f->ident = type->objdec->str;
  f->is_static = (qualifier & IS_STATIC) != 0;

  Function *prev = map_get(global_scope->objects, f->ident);
  if (prev && prev->kind != OBJ_FUNC)
    error(&type->objdec->pos, "duplicated identifier");

  if (prev && !same_type(prev->type, f->type))
    error(&type->objdec->pos, "conflicted function declaration");

  if (prev == NULL)
    map_push(global_scope->objects, f->ident, f);

  if (!consume(TK_LBRACE, nx)) {
    expect(TK_SEMICOLON, nx);
    return;
  }

  if (prev && prev->body)
    error(&type->objdec->pos, "duplicated function definition");

  if (prev) {
    prev->type = f->type;
    f = prev;
  }

  assert(current_function == NULL);
  current_function = f;

  assert(goto_in_current_function == NULL);
  goto_in_current_function = new_vector(0, sizeof(Node *));

  assert(label_in_current_function == NULL);
  label_in_current_function = new_vector(0, sizeof(Node *));

  assert(current_scope == global_scope);
  new_scope();

  // push params to local scope
  f->params = new_vector(0, sizeof(Variable *));
  for (int i = 0; i < f->type->params->size; ++i) {
    Type *ty = *(Type **)vector_get(f->type->params, i);
    if (ty->objdec == NULL)
      error(&type->objdec->pos, "missing parameter name");
    Variable *var = new_local_variable(ty, 0);
    set_offset(var);
    vector_push(f->params, &var);
  }

  if (type->is_variadic) {
    // push hidden local variable for variadic arguments
    Type *t = base_type(TYPE_UCHAR);
    Token *dummy_tok = calloc(1, sizeof(Token));
    *dummy_tok = *type->objdec;
    dummy_tok->str = new_string("__hidden_va_area__", 0);

    t->objdec = dummy_tok;
    static const int sizeof_register_save_area =
        (/* gp */ 8 * 6) + (/* xmm0 - xmm7 */ 16 * 8);
    static const int sizeof_va_list = 24;
    static const int va_arg_buffer_size =
        16; // a buffer used by __builtin_va_arg
    t = array_type(t, sizeof_va_list + sizeof_register_save_area +
                          va_arg_buffer_size);
    Variable *var = new_local_variable(t, 0);
    set_offset(var);
    f->hidden_va_area = var;
  }

  if (pass_on_memory(type->return_type)) {
    // push hidden local variable to keep return_buffer address
    Type *t = base_type(TYPE_ULONG);
    Token *dummy_tok = calloc(1, sizeof(Token));
    *dummy_tok = *type->objdec;
    dummy_tok->str = new_string("", 0);

    t->objdec = dummy_tok;
    Variable *var = new_local_variable(t, 0);
    set_offset(var);
    f->return_buffer_address = var;
  }

  f->body = stmt_block(nx);

  // give label_index to labels in the function
  for (int i = 0; i < label_in_current_function->size; ++i) {
    Node *node = *(Node **)vector_get(label_in_current_function, i);
    node->label_index = label_index++;
  }

  // resolve goto label index
  for (int i = 0; i < goto_in_current_function->size; ++i) {
    Node *node_goto = *(Node **)vector_get(goto_in_current_function, i);
    node_goto->label_index = -1;

    for (int j = 0; j < label_in_current_function->size; ++j) {
      Node *node_label = *(Node **)vector_get(label_in_current_function, j);
      if (same_string(node_goto->token->str, node_label->token->str)) {
        node_goto->label_index = node_label->label_index;
        break;
      }
    }
    if (node_goto->label_index == -1)
      error(&node_goto->token->pos, "undefined label");
  }

  restore_scope();
  current_function->offset += current_function->return_buffer_offset;
  current_function->return_buffer_offset = current_function->offset;
  current_function = NULL;
  goto_in_current_function = NULL;
  label_in_current_function = NULL;
}

void funcparam(Type *ft, Token **nx) {
  if ((*nx)->kind == TK_VOID && (*nx)->next->kind == TK_RPAREN) {
    // (void)
    expect(TK_VOID, nx);
    return;
  }
  do {
    if (consume(TK_3DOTS, nx)) {
      ft->is_variadic = true;
      return;
    }
    Token *tok_qualifier = *nx;
    int qualifier = consume_qualifier(nx);
    if (qualifier & (IS_STATIC | IS_EXTERN))
      error(&tok_qualifier->pos, "invalid storage class for funcparam");

    Token *tok_type = *nx;
    Type *ty = expect_type(nx);
    ty->is_const = (qualifier & IS_CONST) != 0;

    ty = declarator(ty, nx);

    if (ty->kind == TYPE_VOID)
      error(&tok_type->pos, "void type is not allowed");

    if (ty->kind == TYPE_FUNC)
      error(&tok_type->pos, "func type is not allowed");

    if (ty->kind == TYPE_ARRAY)
      ty = pointer_type(ty->base); // array as a pointer

    vector_push(ft->params, &ty);
  } while (consume(TK_COMMA, nx));
  return;
}

int consume_qualifier(Token **nx) {
  int qualifier = 0;
  Token *tok = NULL;
  while ((tok = consume(TK_CONST, nx)) || (tok = consume(TK_STATIC, nx)) ||
         (tok = consume(TK_EXTERN, nx))) {
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

Type *dectail(Type *base, Token **nx) {
  if ((*nx)->kind == TK_LBRACKET)
    return consume_array_brackets(base, nx);

  if (consume(TK_LPAREN, nx)) {
    Type *ft = func_type(base);
    if (!consume(TK_RPAREN, nx)) {
      funcparam(ft, nx);
      expect(TK_RPAREN, nx);
    }
    return ft;
  }
  return base;
}

Type *declarator(Type *base, Token **nx) {
  Type *type = consume_type_star(base, nx);

  if (consume(TK_LPAREN, nx)) {
    Type dummy;
    dummy.objdec = NULL;
    dummy.kind = TYPE_NONE;
    dummy.size = 0;
    Type *mid = declarator(&dummy, nx);
    expect(TK_RPAREN, nx);
    Type *tail = dectail(base, nx);
    return combine_dectype(mid, tail);
  }

  type->objdec = base->objdec = consume(TK_IDENT, nx);
  return dectail(type, nx);
}

Node *declaration(Token **nx) {
  if (consume(TK_TYPEDEF, nx)) {
    expect_typedef(nx);
    return new_node_nop();
  }

  Token *tok_qualifier = *nx;
  int qualifier = consume_qualifier(nx);

  Token *tk = *nx;
  Type *type = consume_type(nx);
  if (type == NULL) {
    if (current_scope == global_scope)
      error(&tk->pos, "unknown type");
    if (qualifier)
      error(&tok_qualifier->pos, "invalid qualifier");
    return NULL;
  }

  if ((type->kind == TYPE_STRUCT || type->kind == TYPE_UNION ||
       type->kind == TYPE_ENUM) &&
      consume(TK_SEMICOLON, nx)) {
    // type declaration only (qualifier is allowed)
    return new_node_nop();
  }

  type = declarator(type, nx);

  if (type->kind == TYPE_FUNC) {
    if (current_scope != global_scope)
      error(&tk->pos, "function declaration is only allowed in global scope");
    func(type, qualifier, nx);
    return NULL;
  }

  ObjectKind kind = current_scope == global_scope ? OBJ_GVAR : OBJ_LVAR;
  Vector *vars = vardec(type, kind, qualifier, nx);
  if (kind == OBJ_GVAR)
    return NULL;
  if (qualifier & IS_STATIC)
    return new_node_nop();
  Node *node = init_multiple_local_variables(vars);
  if (node == NULL)
    node = new_node_nop();
  return node;
}

VariableInit *varinit(Token **nx) {
  VariableInit *init = calloc(1, sizeof(VariableInit));
  if (consume(TK_LBRACE, nx)) {
    Token *tok = consume(TK_RBRACE, nx);
    if (tok)
      error(&tok->pos, "empty brace initializer is not allowed");
    init->vec = new_vector(0, sizeof(VariableInit *));

    while (true) {
      VariableInit *i = varinit(nx);
      if (i->vec)
        init->nested = true;
      vector_push(init->vec, &i);
      if (consume(TK_COMMA, nx)) {
        if (consume(TK_RBRACE, nx))
          break;
      } else {
        expect(TK_RBRACE, nx);
        break;
      }
    }
  } else {
    init->expr = assign(nx);
  }
  return init;
}

Node *init_local_variable(Variable *var, Node *left, VariableInit *init);
Node *init_local_array(Variable *var, Node *left, VariableInit *init);
Node *init_local_struct(Variable *var, Node *left, VariableInit *init);
Node *init_local_union(Variable *var, Node *left, VariableInit *init);
Node *fill_zero(Node *left, int start_index);
Node *fill_array_zero(Node *left, int start_index);
Node *fill_struct_union_zero(Node *left, Member *member);

Node *fill_struct_union_zero(Node *left, Member *member) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  while (member) {
    Node *s = NULL;
    Node *member_access = new_node_member(NULL, left, member);
    if (is_scalar(member->type))
      s = new_node_assign_ignore_const(NULL, member_access,
                                       new_node_num_zero(NULL, member->type));
    else if (member->type->kind == TYPE_ARRAY)
      s = fill_array_zero(member_access, 0);
    else if (is_struct_union(member->type))
      s = fill_struct_union_zero(member_access, member->type->member);
    else
      assert(false);
    vector_push(node->blk_stmts, &s);
    member = member->next;
  }
  return node;
}

Node *fill_array_zero(Node *left, int start_index) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  if (is_scalar(left->type->base)) {
    Node *zero = new_node_num_zero(NULL, base_type(TYPE_INT));
    for (int i = start_index; i < left->type->array_size; ++i) {
      Node *s = new_node_assign_ignore_const(
          NULL, new_node_array_access(NULL, left, i), zero);
      vector_push(node->blk_stmts, &s);
    }
    return node;
  }

  if (left->type->base->kind == TYPE_ARRAY) {
    for (int i = start_index; i < left->type->array_size; ++i) {
      Node *s = fill_array_zero(new_node_array_access(NULL, left, i), 0);
      vector_push(node->blk_stmts, &s);
    }
    return node;
  }

  if (is_struct_union(left->type->base)) {
    for (int i = start_index; i < left->type->array_size; ++i) {
      Node *s = fill_struct_union_zero(new_node_array_access(NULL, left, i),
                                       left->type->base->member);
      vector_push(node->blk_stmts, &s);
    }
    return node;
  }

  assert(false);
  return NULL;
}

Node *init_local_struct(Variable *var, Node *left, VariableInit *init) {
  assert(left->type->kind == TYPE_STRUCT);
  if (init->vec) {
    // init struct members recursively
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->blk_stmts = new_vector(0, sizeof(Node *));
    Member *m = left->type->member;
    for (int i = 0; i < init->vec->size && m; i++, m = m->next) {
      Node *s = init_local_variable(var, new_node_member(var->token, left, m),
                                    *(VariableInit **)vector_get(init->vec, i));
      vector_push(node->blk_stmts, &s);
    }
    // fill zero
    Node *s = fill_struct_union_zero(left, m);
    vector_push(node->blk_stmts, &s);
    return node;
  }

  assert(init->expr);
  if (same_type(init->expr->type, left->type))
    return new_node_assign_ignore_const(var->token, left, init->expr);

  // when init->expr is given to a struct,
  // init first member by init->expr and fill other members with zero
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));
  Member *m = left->type->member;
  Node *s =
      init_local_variable(var, new_node_member(var->token, left, m), init);
  vector_push(node->blk_stmts, &s);
  // fill zero
  s = fill_struct_union_zero(left, m->next);
  vector_push(node->blk_stmts, &s);
  return node;
}

Node *init_local_union(Variable *var, Node *left, VariableInit *init) {
  assert(left->type->kind == TYPE_UNION);
  if (init->vec) {
    // init first member only
    Member *m = left->type->member;
    return init_local_variable(var, new_node_member(var->token, left, m),
                               *(VariableInit **)vector_get(init->vec, 0));
  }

  assert(init->expr);
  if (same_type(init->expr->type, left->type))
    return new_node_assign_ignore_const(var->token, left, init->expr);

  return init_local_variable(
      var, new_node_member(var->token, left, left->type->member), init);
}

Node *init_local_array(Variable *var, Node *left, VariableInit *init) {
  assert(left->type->kind == TYPE_ARRAY);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  if (init->expr) {
    if (left->type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR &&
        init->expr->variable->kind == OBJ_STRLIT) {
      // initilize the array as a string
      char *lit = init->expr->variable->string_literal;
      if (left->type->array_size != (int)strlen(lit) + 1)
        error(&var->token->pos,
              "miss-match between array-size and string-length");

      for (int i = 0; i < left->type->array_size; ++i) {
        Node *c = new_node_num_char(NULL, lit[i]);
        Node *s = new_node_assign_ignore_const(
            var->token, new_node_array_access(var->token, left, i), c);
        vector_push(node->blk_stmts, &s);
      }

      return node;
    }

    // When init->expr is given for an array,
    // only the first element will be initialized.
    Type *ty = left->type;
    while (ty->kind == TYPE_ARRAY)
      ty = ty->base;
    Node *s =
        init_local_variable(var, new_node_array_access(NULL, left, 0), init);
    vector_push(node->blk_stmts, &s);

    // fill zeros
    s = fill_array_zero(left, 1);
    vector_push(node->blk_stmts, &s);

    return node;
  }

  if (init->vec) {
    assert(init->vec->size > 0);

    Type *base = left->type;
    while (base->kind == TYPE_ARRAY)
      base = base->base;

    if (init->nested) {
      // init arrays recursively
      int len = left->type->array_size < init->vec->size
                    ? left->type->array_size
                    : init->vec->size;
      for (int i = 0; i < len; i++) {
        Node *s =
            init_local_variable(var, new_node_array_access(var->token, left, i),
                                *(VariableInit **)vector_get(init->vec, i));
        vector_push(node->blk_stmts, &s);
      }
      // fill zero
      if (len < left->type->array_size) {
        Node *s = fill_array_zero(left, len);
        vector_push(node->blk_stmts, &s);
      }
      return node;
    }

    // init as a one-dimensional array
    int len1d = left->type->size / base->size;
    int len = len1d < init->vec->size ? len1d : init->vec->size;
    for (int i = 0; i < len; i++) {
      VariableInit *vi = *(VariableInit **)vector_get(init->vec, i);
      assert(vi->expr);
      Node *s = new_node_assign_ignore_const(
          var->token, new_node_array_access_as_1D(var->token, left, i),
          vi->expr);
      vector_push(node->blk_stmts, &s);
    }
    // fill zero
    if (is_scalar(base)) {
      for (int i = len; i < len1d; i++) {
        Node *s = new_node_assign_ignore_const(
            var->token, new_node_array_access_as_1D(var->token, left, i),
            new_node_num_zero(var->token, base_type(TYPE_INT)));
        vector_push(node->blk_stmts, &s);
      }
    } else if (is_struct_union(base)) {
      for (int i = len; i < len1d; i++) {
        Node *s = fill_struct_union_zero(
            new_node_array_access_as_1D(var->token, left, i), base->member);
        vector_push(node->blk_stmts, &s);
      }
    } else
      assert(false);
    return node;
  }

  assert(false);
  return NULL;
}

Node *init_local_variable(Variable *var, Node *left, VariableInit *init) {
  if (init == NULL)
    return NULL;

  if (left->type->kind == TYPE_ARRAY)
    return init_local_array(var, left, init);

  if (left->type->kind == TYPE_STRUCT)
    return init_local_struct(var, left, init);

  if (left->type->kind == TYPE_UNION)
    return init_local_union(var, left, init);

  if (is_scalar(left->type)) {
    while (init->vec) { // for non-array primitive types, only the first element
                        // in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    return new_node_assign_ignore_const(var->token, left, init->expr);
  }

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
    return init_local_variable(var, new_node_var(var->token, var), var->init);
  }

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));

  for (int i = 0; i < variables->size; ++i) {
    Variable *var = *(Variable **)vector_get(variables, i);
    Node *s =
        init_local_variable(var, new_node_var(var->token, var), var->init);
    if (s)
      vector_push(node->blk_stmts, &s);
  }
  return node;
}

Node *stmt(Token **nx) {
  Token *tok = *nx;

  if (consume(TK_SEMICOLON, nx))
    return NULL;

  if (consume(TK_LBRACE, nx)) {
    new_scope();
    Node *node = stmt_block(nx);
    restore_scope();
    return node;
  }

  if (consume(TK_IF, nx))
    return stmt_if(nx);

  if (consume(TK_DO, nx))
    return stmt_do(nx);

  if (consume(TK_WHILE, nx))
    return stmt_while(nx);

  if (consume(TK_FOR, nx))
    return stmt_for(nx);

  if (consume(TK_SWITCH, nx))
    return stmt_switch(nx);

  if (consume(TK_CASE, nx)) {
    if (switch_nodes->size == 0)
      error(&tok->pos, "invalid case");
    Node *sw = *(Node **)vector_last(switch_nodes);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CASE;
    node->token = tok;
    node->label_index = label_index++;

    Node *e = expr(nx);
    if (!e->is_constant_expr)
      error(&(*nx)->pos, "case label must be a constant");
    if (!is_integer(e->num->type))
      error(&(*nx)->pos, "case label must be an integer");
    Node *c = sw->next_case;
    while (c) {
      assert(c->condition->kind == ND_NUM);
      if (number2bool(number_eq(c->condition->num, e->num)))
        error(&tok->pos, "duplicated case value detected");
      c = c->next_case;
    }
    node->condition = new_node_num(NULL, e->num);
    expect(TK_COLON, nx);
    node->body = stmt(nx);
    node->next_case = sw->next_case;
    sw->next_case = node;
    return node;
  }

  if (consume(TK_DEFAULT, nx)) {
    expect(TK_COLON, nx);
    if (switch_nodes->size == 0)
      error(&tok->pos, "invalid default");
    Node *sw = *(Node **)vector_last(switch_nodes);
    if (sw->default_)
      error(&tok->pos, "multiple default for one switch");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_DEFAULT;
    node->token = tok;
    node->label_index = label_index++;
    node->body = stmt(nx);
    sw->default_ = node;
    return node;
  }

  if (consume(TK_RETURN, nx)) {
    Node *node = calloc(1, sizeof(Node));
    node->token = tok;
    node->kind = ND_RETURN;
    node->return_buffer_address = current_function->return_buffer_address;
    if (!consume(TK_SEMICOLON, nx)) {
      node->lhs = expr(nx);
      if (!castable(node->lhs->type, current_function->type->return_type))
        error(&tok->pos, "invalid return type");
      node->lhs =
          new_node_cast(tok, current_function->type->return_type, node->lhs);
      expect(TK_SEMICOLON, nx);
    }
    return node;
  }

  if (consume(TK_CONTINUE, nx)) {
    if (continue_label->size == 0)
      error(&tok->pos, "invalid continue");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_CONTINUE;
    node->label_index = vector_lasti(continue_label);
    node->token = tok;
    expect(TK_SEMICOLON, nx);
    return node;
  }

  if (consume(TK_BREAK, nx)) {
    if (break_label->size == 0)
      error(&tok->pos, "invalid break");
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BREAK;
    node->label_index = vector_lasti(break_label);
    node->token = tok;
    expect(TK_SEMICOLON, nx);
    return node;
  }

  if (consume(TK_GOTO, nx)) {
    Token *label_name = expect(TK_IDENT, nx);
    expect(TK_SEMICOLON, nx);
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_GOTO;
    node->token = label_name;
    vector_push(goto_in_current_function, &node);
    return node;
  }

  if ((*nx)->kind == TK_IDENT && (*nx)->next->kind == TK_COLON) {
    Token *label_name = expect(TK_IDENT, nx);
    expect(TK_COLON, nx);

    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_LABEL;
    node->token = label_name;
    node->body = stmt(nx);

    for (int i = 0; i < label_in_current_function->size; ++i) {
      Node *prev = *(Node **)vector_get(label_in_current_function, i);
      if (same_string(label_name->str, prev->token->str))
        error(&label_name->pos, "duplicated label");
    }

    vector_push(label_in_current_function, &node);
    return node;
  }

  Node *node = expr(nx);
  expect(TK_SEMICOLON, nx);
  return node;
}

Node *stmt_block(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_BLOCK;
  node->blk_stmts = new_vector(0, sizeof(Node *));
  node->token = NULL;

  while (!consume(TK_RBRACE, nx)) {
    Node *s;
    if ((s = declaration(nx))) {
      if (s->kind != ND_NOP)
        vector_push(node->blk_stmts, &s);
    } else if ((s = stmt(nx))) {
      vector_push(node->blk_stmts, &s);
    }
  }

  return node;
}

Node *stmt_if(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_IF;
  node->label_index = label_index++;

  Token *lp = expect(TK_LPAREN, nx);
  Node *cond = expr(nx);
  if (!castable(cond->type, base_type(TYPE_BOOL)))
    error(&lp->pos, "boolean compatible type required");
  node->condition = new_node_cast(lp, base_type(TYPE_BOOL), cond);
  expect(TK_RPAREN, nx);

  new_scope();
  node->body = stmt(nx);
  restore_scope();

  if (consume(TK_ELSE, nx))
    node->else_ = stmt(nx);
  return node;
}

Node *stmt_do(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_DO;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  new_scope();
  node->body = stmt(nx);
  restore_scope();

  expect(TK_WHILE, nx);
  Token *lp = expect(TK_LPAREN, nx);
  Node *cond = expr(nx);
  if (!castable(cond->type, base_type(TYPE_BOOL)))
    error(&lp->pos, "boolean compatible type required");
  node->condition = new_node_cast(lp, base_type(TYPE_BOOL), cond);
  expect(TK_RPAREN, nx);
  expect(TK_SEMICOLON, nx);

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_while(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_WHILE;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  Token *lp = expect(TK_LPAREN, nx);
  Node *cond = expr(nx);
  if (!castable(cond->type, base_type(TYPE_BOOL)))
    error(&lp->pos, "boolean compatible type required");
  node->condition = new_node_cast(lp, base_type(TYPE_BOOL), cond);
  expect(TK_RPAREN, nx);

  new_scope();
  node->body = stmt(nx);
  restore_scope();

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_for(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_FOR;
  node->label_index = label_index++;
  vector_pushi(continue_label, node->label_index);
  vector_pushi(break_label, node->label_index);

  expect(TK_LPAREN, nx);

  new_scope();
  if (!consume(TK_SEMICOLON, nx)) {
    Token *tok = *nx;
    Type *type = consume_type(nx);
    if (type) {
      if (type->size < 0)
        error(&tok->pos, "incomplete type");

      type = declarator(type, nx);
      Vector *vars = vardec(type, OBJ_LVAR, 0, nx);
      node->init = init_multiple_local_variables(vars);
    } else {
      node->init = expr(nx);
      expect(TK_SEMICOLON, nx);
    }
  }

  if (!consume(TK_SEMICOLON, nx)) {
    Token *tok = *nx;
    Node *cond = expr(nx);
    if (!castable(cond->type, base_type(TYPE_BOOL)))
      error(&tok->pos, "boolean compatible type required");
    node->condition = new_node_cast(tok, base_type(TYPE_BOOL), cond);
    expect(TK_SEMICOLON, nx);
  }

  if (!consume(TK_RPAREN, nx)) {
    node->update = expr(nx);
    expect(TK_RPAREN, nx);
  }

  node->body = stmt(nx);
  restore_scope();

  vector_pop(continue_label);
  vector_pop(break_label);

  return node;
}

Node *stmt_switch(Token **nx) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_SWITCH;
  node->label_index = label_index++;
  vector_pushi(break_label, node->label_index);
  vector_push(switch_nodes, &node);

  expect(TK_LPAREN, nx);
  node->condition = expr(nx);
  if (!is_integer(node->condition->type))
    error(&(*nx)->pos, "switch quantity must be an integer");
  expect(TK_RPAREN, nx);

  new_scope();
  node->body = stmt(nx);
  restore_scope();

  vector_pop(break_label);
  vector_pop(switch_nodes);

  return node;
}

Node *expr(Token **nx) { return comma(nx); }

Node *comma(Token **nx) {
  Node *node = assign(nx);
  Token *tok = *nx;
  if (consume(TK_COMMA, nx))
    return new_node_comma(tok, node, comma(nx));
  return node;
}

Node *assign(Token **nx) {
  Node *node = conditional(nx);
  Token *tok = *nx;
  if (consume(TK_ASSIGN, nx))
    node = new_node_assign(tok, node, assign(nx));
  if (consume(TK_ADDEQ, nx))
    node = new_node_assign(tok, node, new_node_add(NULL, node, assign(nx)));
  if (consume(TK_SUBEQ, nx))
    node = new_node_assign(tok, node, new_node_sub(NULL, node, assign(nx)));
  if (consume(TK_MULEQ, nx))
    node = new_node_assign(tok, node, new_node_mul(NULL, node, assign(nx)));
  if (consume(TK_DIVEQ, nx))
    node = new_node_assign(tok, node, new_node_div(NULL, node, assign(nx)));
  if (consume(TK_MODEQ, nx))
    node = new_node_assign(tok, node, new_node_mod(NULL, node, assign(nx)));
  if (consume(TK_XOREQ, nx))
    node = new_node_assign(tok, node, new_node_bitxor(NULL, node, assign(nx)));
  if (consume(TK_ANDEQ, nx))
    node = new_node_assign(tok, node, new_node_bitand(NULL, node, assign(nx)));
  if (consume(TK_OREQ, nx))
    node = new_node_assign(tok, node, new_node_bitor(NULL, node, assign(nx)));
  if (consume(TK_LSHIFTEQ, nx))
    node = new_node_assign(tok, node, new_node_lshift(NULL, node, assign(nx)));
  if (consume(TK_RSHIFTEQ, nx))
    node = new_node_assign(tok, node, new_node_rshift(NULL, node, assign(nx)));
  return node;
}

Node *conditional(Token **nx) {
  Node *node = logor(nx);
  Token *tok = consume(TK_QUESTION, nx);
  if (!tok)
    return node;
  Node *lhs = expr(nx);
  expect(TK_COLON, nx);
  return new_node_conditional(tok, node, lhs, conditional(nx), label_index++);
}

Node *logor(Token **nx) {
  Node *node = logand(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_LOGOR, nx)) {
      int l = label_index++;
      node = new_node_logor(tok, node, logand(nx), l);
    } else {
      return node;
    }
  }
}

Node *logand(Token **nx) {
  Node *node = bitor (nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_LOGAND, nx)) {
      int l = label_index++;
      node = new_node_logand(tok, node, bitor (nx), l);
    } else {
      return node;
    }
  }
}

Node * bitor (Token * *nx) {
  Node *node = bitxor(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_BAR, nx))
      node = new_node_bitor(tok, node, bitxor(nx));
    else
      return node;
  }
}

Node *bitxor(Token **nx) {
  Node *node = bitand(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_HAT, nx))
      node = new_node_bitxor(tok, node, bitand(nx));
    else
      return node;
  }
}

Node *bitand(Token **nx) {
  Node *node = equality(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_AMP, nx))
      node = new_node_bitand(tok, node, equality(nx));
    else
      return node;
  }
}

Node *equality(Token **nx) {
  Node *node = relational(nx);
  while (true) {
    Token *tok;
    if ((tok = consume(TK_EQ, nx)))
      node = new_node_eq(tok, node, relational(nx));
    else if ((tok = consume(TK_NE, nx)))
      node = new_node_ne(tok, node, relational(nx));
    else
      return node;
  }
}

Node *relational(Token **nx) {
  Node *node = bitshift(nx);
  while (true) {
    Token *tok;
    if ((tok = consume(TK_LT, nx)))
      node = new_node_lt(tok, node, bitshift(nx));
    else if ((tok = consume(TK_LE, nx)))
      node = new_node_le(tok, node, bitshift(nx));
    if ((tok = consume(TK_GT, nx)))
      node = new_node_lt(tok, bitshift(nx), node);
    else if ((tok = consume(TK_GE, nx)))
      node = new_node_le(tok, bitshift(nx), node);
    else
      return node;
  }
}

Node *bitshift(Token **nx) {
  Node *node = add(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_LSHIFT, nx)) {
      node = new_node_lshift(tok, node, add(nx));
    } else if (consume(TK_RSHIFT, nx)) {
      node = new_node_rshift(tok, node, add(nx));
    } else
      return node;
  }
}

Node *add(Token **nx) {
  Node *node = mul(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_PLUS, nx)) {
      node = new_node_add(tok, node, mul(nx));
    } else if (consume(TK_MINUS, nx)) {
      node = new_node_sub(tok, node, mul(nx));
    } else
      return node;
  }
}

Node *mul(Token **nx) {
  Node *node = unary(nx);
  while (true) {
    Token *tok = *nx;
    if (consume(TK_STAR, nx))
      node = new_node_mul(tok, node, unary(nx));
    else if (consume(TK_SLASH, nx))
      node = new_node_div(tok, node, unary(nx));
    else if (consume(TK_PERCENT, nx))
      node = new_node_mod(tok, node, unary(nx));
    else
      return node;
  }
}

Node *unary(Token **nx) {
  Token *tok = *nx;
  if (consume(TK_SIZEOF, nx)) {
    Token *keep = *nx;
    Token *left_paren = consume(TK_LPAREN, nx);
    Type *type = consume_type(nx);
    if (left_paren && type) {
      // sizeof(type)
      type = consume_array_brackets(consume_type_star(type, nx), nx);
      if (type->size < 0)
        error(&tok->pos, "invalid array size");
      expect(TK_RPAREN, nx);
    } else {
      // sizeof unary
      *nx = keep; // rollback token
      type = unary(nx)->type;
    }
    if (type->kind == TYPE_VOID)
      error(&tok->pos, "invalid sizeof operation for void type");
    if (type->size < 0)
      error(&tok->pos, "invalid sizeof operation for incomplete type");
    return new_node_num_ulong(tok, type->size);
  }
  if (consume(TK_INC, nx)) {
    Node *u = unary(nx);
    return new_node_assign(NULL, u,
                           new_node_add(NULL, u, new_node_num_int(NULL, 1)));
  }
  if (consume(TK_DEC, nx)) {
    Node *u = unary(nx);
    return new_node_assign(NULL, u,
                           new_node_sub(NULL, u, new_node_num_int(NULL, 1)));
  }

  if (consume(TK_PLUS, nx))
    return unary(nx);

  if (consume(TK_MINUS, nx)) {
    Node *u = unary(nx);
    return new_node_sub(tok, new_node_num_zero(NULL, u->type), u);
  }

  if (consume(TK_AMP, nx))
    return new_node_addr(tok, unary(nx));
  if (consume(TK_STAR, nx))
    return new_node_deref(tok, unary(nx));
  if (consume(TK_LOGNOT, nx))
    return new_node_lognot(tok, unary(nx));
  if (consume(TK_TILDE, nx))
    return new_node_bitnot(tok, unary(nx));

  if (consume(TK_LPAREN, nx)) {
    Type *type = consume_type(nx);
    if (type) {
      type = consume_type_star(type, nx);
      if (consume(TK_RPAREN, nx)) {
        Node *operand = unary(nx);
        if (!castable(operand->type, type))
          error(&tok->pos, "invalid type casting");
        return new_node_cast(tok, type, operand);
      }
    }
    *nx = tok; // rollback
  }
  return postfix(nx);
}

Node *primary(Token **nx) {
  if (consume(TK_LPAREN, nx)) {
    Node *node = expr(nx);
    expect(TK_RPAREN, nx);
    return node;
  }

  Token *tok;
  if ((tok = consume(TK_IDENT, nx))) {
    int *enum_val = find_enum_element(tok->str);
    if (enum_val)
      return new_node_num_int(tok, *enum_val);

    Variable *var = find_object(tok);
    if (!var)
      error(&tok->pos, "undefined identifier: '%.*s'", tok->str->len,
            tok->str->str);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_STR, nx))) {
    Variable *var = new_string_literal(tok);
    return new_node_var(tok, var);
  }

  if ((tok = consume(TK_NUM, nx)))
    return new_node_num(tok, tok->num);

  error(&(*nx)->pos, "primary expected but not found");
  return NULL;
}

Node *tail(Node *x, Token **nx) {
  if (consume(TK_LBRACKET, nx)) {
    // x[y] --> *(x+y)
    Node *y = expr(nx);
    Token *tok = expect(TK_RBRACKET, nx);
    return new_node_deref(tok, new_node_add(tok, x, y));
  }

  Token *op = *nx;

  if (consume(TK_DOT, nx)) {
    // struct member access (x.y)
    if (x->type == NULL || !is_struct_union(x->type))
      error(&op->pos, "not a struct/union");
    Token *y = expect(TK_IDENT, nx);
    Member *member = find_member(x->type, y);
    if (member == NULL)
      error(&y->pos, "unknown struct member");
    return new_node_member(op, x, member);
  }

  if (consume(TK_ARROW, nx)) {
    // struct member access
    // x->y is (*x).y
    if (x->type == NULL || x->type->kind != TYPE_PTR ||
        !is_struct_union(x->type->base))
      error(&op->pos, "not a struct/union pointer");
    Token *y = expect(TK_IDENT, nx);
    Member *member = find_member(x->type->base, y);
    if (member == NULL)
      error(&y->pos, "unknown struct member");
    return new_node_member(op, new_node_deref(NULL, x), member);
  }

  if (consume(TK_INC, nx)) {
    // (x = x + 1) - 1
    return new_node_sub(
        NULL,
        new_node_assign(NULL, x,
                        new_node_add(NULL, x, new_node_num_int(NULL, 1))),
        new_node_num_int(NULL, 1));
  }

  if (consume(TK_DEC, nx)) {
    // (x = x - 1) + 1
    return new_node_add(
        NULL,
        new_node_assign(NULL, x,
                        new_node_sub(NULL, x, new_node_num_int(NULL, 1))),
        new_node_num_int(NULL, 1));
  }

  if (consume(TK_LPAREN, nx)) {
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
    node->caller = current_function;

    if (pass_on_memory(f->return_type) &&
        current_function->return_buffer_offset < f->return_type->size)
      current_function->return_buffer_offset = f->return_type->size;

    if (consume(TK_RPAREN, nx))
      return node;

    do {
      Node *e = assign(nx);

      if (!f->is_variadic && f->params->size == node->args->size)
        error(&op->pos, "too many arguments");

      if (f->params->size > node->args->size) {
        // argument type conversion
        Type *t = *(Type **)vector_get(f->params, node->args->size);
        if (!castable(e->type, t))
          error(&op->pos, "invalid argument type");
        e = new_node_cast(op, t, e);
      } else if (e->type->kind == TYPE_ARRAY) {
        // pass an array as a pointer
        e = new_node_cast(op, pointer_type(e->type->base), e);
      } else {
        // default argument promotions
        switch (e->type->kind) {
        case TYPE_FLOAT:
          e = new_node_cast(op, base_type(TYPE_DOUBLE), e);
          break;
        case TYPE_CHAR:
        case TYPE_SHORT:
          e = new_node_cast(op, base_type(TYPE_INT), e);
          break;
        case TYPE_UCHAR:
        case TYPE_USHORT:
          e = new_node_cast(op, base_type(TYPE_UINT), e);
          break;
        default:
          break;
        }
      }

      vector_push(node->args, &e);
    } while (consume(TK_COMMA, nx));

    expect(TK_RPAREN, nx);
    return node;
  }
  return x;
}

Node *postfix(Token **nx) {
  Node *p = primary(nx);
  Node *q = tail(p, nx);
  while (p != q) {
    p = q;
    q = tail(p, nx);
  }
  return p;
}

void register_builtin_functions() {
  {
    // __builtin_va_arg
    Type *vp = pointer_type(base_type(TYPE_VOID));
    Function *f = calloc(1, sizeof(Function));
    f->type = func_type(vp);
    f->kind = OBJ_FUNC;
    f->ident = new_string("__builtin_va_arg", 0);
    vector_push(f->type->params, &vp);
    vector_push(f->type->params, &vp);
    map_push(global_scope->objects, f->ident, f);
  }
}

void parse(Token *input) {
  assert(input);
  Token *cur = input;
  strings = new_map();
  static_local_variables = new_vector(0, sizeof(Variable *));
  break_label = new_vector(0, sizeof(int));
  continue_label = new_vector(0, sizeof(int));
  switch_nodes = new_vector(0, sizeof(Node *));
  assert(current_scope == NULL);
  new_scope();
  global_scope = current_scope;
  register_builtin_functions();

  while (cur->kind != TK_EOF)
    declaration(&cur);
}
