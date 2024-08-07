#include "boncc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *ostream;
static const char *reg_args1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *reg_args2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *reg_args4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *reg_args8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

typedef struct FloatLiteral FloatLiteral;
struct FloatLiteral {
  int index;
  Number *num;
};
static int float_literal_index = 0;
static Vector *float_literals = NULL; // FloatValue*

// node.c -->
bool is_constant_number(Node *node);
Variable *is_const_var_addr(Node *node);
Number *eval(Node *node);
// <-- node.c

void gen(Node *node);

__attribute__((format(printf, 1, 2))) void writeline(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(ostream, fmt, ap);
  fprintf(ostream, "\n");
}

__attribute__((format(printf, 2, 3))) void comment(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(ostream, "  # ");
  vfprintf(ostream, fmt, ap);
  if (tok)
    fprintf(ostream, "    %s:%d:%d", tok->pos.file_name, tok->pos.line_number,
            tok->pos.column_number);
  fprintf(ostream, "\n");
}

static void push_xmm(int n, TypeKind kind) {
  switch (kind) {
  case TYPE_FLOAT: // align to 8 even if size is 4
    writeline("  sub rsp, 8        # push %s", type_text(kind));
    writeline("  movss [rsp], xmm%d # push %s", n, type_text(kind));
    break;
  case TYPE_DOUBLE:
    writeline("  sub rsp, 8        # push %s", type_text(kind));
    writeline("  movsd [rsp], xmm%d # push %s", n, type_text(kind));
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
  }
}

static void pop_xmm(int n, TypeKind kind) {
  switch (kind) {
  case TYPE_FLOAT: // align to 8 even if size is 4
    writeline("  movss xmm%d, [rsp] # pop %s", n, type_text(kind));
    writeline("  add rsp, 8        # pop %s", type_text(kind));
    break;
  case TYPE_DOUBLE:
    writeline("  movsd xmm%d, [rsp] # pop %s", n, type_text(kind));
    writeline("  add rsp, 8        # pop %s", type_text(kind));
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
  }
}

void gen_address(Node *node) {
  // write address to rax
  switch (node->kind) {
  case ND_DEREF:
    comment(NULL, "gen_address ND_DEREF");
    gen(node->lhs);
    return;
  case ND_VAR:
    if (node->variable->kind == OBJ_LVAR) {
      if (node->variable->is_static) {
        comment(NULL, "gen_address ND_VAR static-local: %.*s",
                node->variable->ident->len, node->variable->ident->str);
        writeline("  lea rax, %.*s[rip]", node->variable->internal_ident->len,
                  node->variable->internal_ident->str);
      } else if (node->variable->is_extern) {
        comment(NULL, "gen_address ND_VAR extern-local: %.*s",
                node->variable->ident->len, node->variable->ident->str);
        writeline("  lea rax, %.*s[rip]", node->variable->ident->len,
                  node->variable->ident->str);
      } else {
        comment(NULL, "gen_address ND_VAR local: %.*s",
                node->variable->ident->len, node->variable->ident->str);
        writeline("  lea rax, [rbp-%d]", node->variable->offset);
      }
    } else if (node->variable->kind == OBJ_GVAR) {
      comment(NULL, "gen_address ND_VAR global: %.*s",
              node->variable->ident->len, node->variable->ident->str);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len,
                node->variable->ident->str);
    } else if (node->variable->kind == OBJ_STRLIT) {
      comment(NULL, "gen_address ND_VAR strlit: \"%s\"",
              node->variable->string_literal);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len,
                node->variable->ident->str);
    } else if (node->variable->kind == OBJ_FUNC) {
      comment(NULL, "gen_address ND_VAR function: %.*s",
              node->variable->ident->len, node->variable->ident->str);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len,
                node->variable->ident->str);
    } else
      assert(false);
    return;
  case ND_MEMBER:
    comment(NULL, "gen_address ND_MEMBER %.*s", node->member->ident->len,
            node->member->ident->str);
    gen_address(node->lhs);
    writeline("  add rax, %d", node->member->offset);
    return;
  default:
    error(&node->token->pos, "failed to generate address");
  }
}

void load(Type *type) {
  // src  : the address where rax is pointing to
  // dest : rax or xmm0
  comment(NULL, "load %s (size:%d)", type_text(type->kind), type->size);

  if (type->kind == TYPE_ARRAY || is_struct_union(type))
    return; // nothing todo

  if (is_float(type)) {
    if (type->kind == TYPE_FLOAT)
      writeline("  movss xmm0, [rax]");
    else if (type->kind == TYPE_DOUBLE)
      writeline("  movsd xmm0, [rax]");
    else if (type->kind == TYPE_LDOUBLE)
      error(NULL, "long double is currently not supported");
    else
      assert(false);
    return;
  }

  if (type->size == 1)
    writeline("  mov%cx rax, byte ptr [rax]", is_unsigned(type) ? 'z' : 's');
  else if (type->size == 2)
    writeline("  mov%cx rax, word ptr [rax]", is_unsigned(type) ? 'z' : 's');
  else if (type->size == 4)
    writeline("  movsxd rax, dword ptr [rax]");
  else if (type->size == 8)
    writeline("  mov rax, [rax]");
  else
    assert(false);
}

void store(Type *type) {
  // src  : rax or xmm0
  // dest : the address popped from stack
  comment(NULL, "store %s", type_text(type->kind));

  writeline("  pop r10");

  if (is_struct_union(type)) {
    // src  : the struct/union that rax is pointing to
    // dest : the struct/union that the address popped from stack is pointing to
    for (int i = 0; i < type->size; i++) {
      // copy each bytes like memcpy
      writeline("  mov r11b, [rax+%d]", i);
      writeline("  mov [r10+%d], r11b", i);
    }
    return;
  }

  if (is_float(type)) {
    if (type->kind == TYPE_FLOAT)
      writeline("  movss [r10], xmm0");
    else if (type->kind == TYPE_DOUBLE)
      writeline("  movsd [r10], xmm0");
    else if (type->kind == TYPE_LDOUBLE)
      error(NULL, "long double is currently not supported");
    else
      assert(false);
    return;
  }

  if (type->size == 1)
    writeline("  mov [r10], al");
  else if (type->size == 2)
    writeline("  mov [r10], ax");
  else if (type->size == 4)
    writeline("  mov [r10], eax");
  else if (type->size == 8)
    writeline("  mov [r10], rax");
  else
    assert(false);
}

void gen_if(Node *node) {
  gen(node->condition);
  writeline("  cmp rax, 0");
  if (node->else_) {
    writeline("  je .Lelse%d", node->label_index);
    gen(node->body);
    writeline("  jmp .Lend%d", node->label_index);
    writeline(".Lelse%d:", node->label_index);
    gen(node->else_);
    writeline(".Lend%d:", node->label_index);
  } else {
    writeline("  je .Lend%d", node->label_index);
    gen(node->body);
    writeline(".Lend%d:", node->label_index);
  }
}

void gen_do(Node *node) {
  writeline(".Ldo%d:", node->label_index);
  gen(node->body);
  writeline(".Lcontinue%d:", node->label_index);
  gen(node->condition);
  writeline("  cmp rax, 0");
  writeline("  jne .Ldo%d", node->label_index);
  writeline(".Lend%d:", node->label_index);
}

void gen_while(Node *node) {
  writeline(".Lcontinue%d:", node->label_index);
  gen(node->condition);
  writeline("  cmp rax, 0");
  writeline("  je .Lend%d", node->label_index);

  gen(node->body);
  writeline("  jmp .Lcontinue%d", node->label_index);
  writeline(".Lend%d:", node->label_index);
}

void gen_for(Node *node) {
  if (node->init)
    gen(node->init);

  writeline(".Lfor%d:", node->label_index);
  if (node->condition) {
    gen(node->condition);
    writeline("  cmp rax, 0");
    writeline("  je .Lend%d", node->label_index);
  }
  gen(node->body);
  writeline(".Lcontinue%d:", node->label_index);
  if (node->update)
    gen(node->update);
  writeline("  jmp .Lfor%d", node->label_index);
  writeline(".Lend%d:", node->label_index);
}

void gen_switch(Node *node) {
  gen(node->condition);

  Node *c = node->next_case;
  while (c) {
    assert(c->condition);
    assert(c->condition->kind == ND_NUM);
    assert(is_integer(c->condition->type));
    if (is_signed(c->condition->type))
      writeline("  cmp rax, %lld", number2long(c->condition->num));
    else
      writeline("  cmp rax, %llu", number2ulong(c->condition->num));
    writeline("  je .Lcase%d", c->label_index);
    c = c->next_case;
  }
  if (node->default_)
    writeline("  jmp .Ldefault%d", node->default_->label_index);
  else
    writeline("  jmp .Lend%d", node->label_index);
  gen(node->body);
  writeline(".Lend%d:", node->label_index);
}

void gen_block(Node *node) {
  int sz = node->blk_stmts->size;
  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->blk_stmts, i);
    gen(d);
  }
}

void gen_call(Node *node) {
  assert(is_funcptr(node->lhs->type));
  assert(node->args->size <= 6);

  int n_vector_registers = 0;

  // put arguments into registers
  {
    int sz = node->args->size;
    int count_integer = 0;
    int count_float = 0;

    // push args to stack
    for (int i = 0; i < sz; ++i) {
      Node *d = *(Node **)vector_get(node->args, i);
      gen(d);
      if (is_float(d->type)) {
        push_xmm(0, d->type->kind);
        count_float++;
      } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
        writeline("  push rax");
        count_integer++;
      } else {
        assert(false);
      }
    }
    assert(sz == count_integer + count_float);
    n_vector_registers = count_float;

    // pop args to registers
    for (int i = sz - 1; i >= 0; --i) {
      Node *d = *(Node **)vector_get(node->args, i);
      if (is_float(d->type)) {
        pop_xmm(--count_float, d->type->kind);
      } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
        writeline("  pop %s", reg_args8[--count_integer]);
      } else {
        assert(false);
      }
    }
  }

  // align RSP to 16bytes (ABI requirements)
  comment(NULL, "check RSP alignment for call");
  static int l = -1;
  l++;
  writeline("  mov rax, rsp");
  writeline("  and rax, 15");     // rax % 16 == rax & 0xF
  writeline("  jnz .Lcall%d", l); // jump if RSP is not aligned

  // alined case
  comment(NULL, "the case when RSP was already aligned");
  if (node->lhs->kind == ND_VAR && node->lhs->variable->kind == OBJ_FUNC) {
    writeline("  mov al, %d", n_vector_registers);
    writeline("  call %.*s", node->lhs->variable->ident->len,
              node->lhs->variable->ident->str);
  } else {
    gen(node->lhs); // calculate function address
    writeline("  mov r10, rax");
    writeline("  mov al, %d", n_vector_registers);
    writeline("  call r10");
  }
  writeline("  jmp .Lend_call%d", l);

  // not-aligned case
  comment(NULL, "the case when RSP was not aligned");
  writeline(".Lcall%d:", l);
  writeline("  sub rsp, 8"); // RSP is always aligned to 8 by this compiler
  if (node->lhs->kind == ND_VAR && node->lhs->variable->kind == OBJ_FUNC) {
    writeline("  mov al, %d", n_vector_registers);
    writeline("  call %.*s", node->lhs->variable->ident->len,
              node->lhs->variable->ident->str);
  } else {
    gen(node->lhs); // calculate function address
    writeline("  mov r10, rax");
    writeline("  mov al, %d", n_vector_registers);
    writeline("  call r10");
  }
  writeline("  add rsp, 8"); // recover RSP

  // end label for aligned case
  writeline(".Lend_call%d:", l);
}

void gen_global_init(VariableInit *init, Type *type);
void gen_global_array_init(VariableInit *init, Type *type);
void gen_global_struct_init(VariableInit *init, Type *type);
void gen_global_union_init(VariableInit *init, Type *type);
void gen_global_pointer_init(VariableInit *init, Type *type);
void gen_global_integer_init(VariableInit *init, Type *type);
void gen_global_float_init(VariableInit *init, Type *type);

void gen_global_array_init(VariableInit *init, Type *type) {
  assert(type->kind == TYPE_ARRAY);

  if (init->expr) {
    if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR &&
        init->expr->variable->kind == OBJ_STRLIT) {
      // initilize the array as a string
      Variable *lit = init->expr->variable;
      if (type->array_size != lit->type->array_size)
        error(&init->expr->token->pos,
              "miss-match between array-size and string-length");
      writeline("  .ascii \"%s\\0\"", lit->string_literal);
      return;
    }

    // When init->expr is given for an array,
    // only the first element will be initialized.
    Type *ty = type;
    while (ty->kind == TYPE_ARRAY)
      ty = ty->base;
    gen_global_init(init, ty);
    writeline("  .zero %d", type->size - ty->size);
    return;
  }

  assert(init->vec);
  assert(init->vec->size > 0);
  if (init->nested) {
    // init arrays recursively
    int zero_size = type->size;
    int len = type->array_size;
    if (len > init->vec->size)
      len = init->vec->size;
    for (int i = 0; i < len; i++) {
      zero_size -= type->base->size;
      gen_global_init(*(VariableInit **)vector_get(init->vec, i), type->base);
    }
    if (zero_size)
      writeline("  .zero %d", zero_size);
    return;
  }

  // init as a one-dimensional array
  Type *base = type;
  while (base->kind == TYPE_ARRAY)
    base = base->base;
  int len = type->size / base->size;
  if (len > init->vec->size)
    len = init->vec->size;
  for (int i = 0; i < len; i++)
    gen_global_init(*(VariableInit **)vector_get(init->vec, i), base);
  int zero_size = type->size - (base->size * len);
  if (zero_size)
    writeline("  .zero %d", zero_size);
}

void gen_global_struct_init(VariableInit *init, Type *type) {
  assert(type->kind == TYPE_STRUCT);
  if (init->vec) {
    // init struct members recursively
    Node *node = calloc(1, sizeof(Node));
    node->kind = ND_BLOCK;
    node->blk_stmts = new_vector(0, sizeof(Node *));

    int zero_size = type->size;

    Member *m = type->member;
    for (int i = 0; i < init->vec->size && m; i++, m = m->next) {
      gen_global_init(*(VariableInit **)vector_get(init->vec, i), m->type);
      if (m->padding)
        writeline("  .zero %d", m->padding);
      zero_size -= m->type->size + m->padding;
    }
    if (zero_size)
      writeline("  .zero %d", zero_size);
    return;
  }

  assert(init->expr);

  // init first member only
  gen_global_init(init, type->member->type);
  int zero_size = type->size - type->member->type->size;
  if (zero_size)
    writeline("  .zero %d", zero_size);
}

void gen_global_union_init(VariableInit *init, Type *type) {
  assert(type->kind == TYPE_UNION);

  // init first member only
  while (init->vec)
    init = *(VariableInit **)vector_get(init->vec, 0);
  assert(init->expr);

  gen_global_init(init, type->member->type);
  int zero_size = type->size - type->member->type->size;
  if (zero_size)
    writeline("  .zero %d", zero_size);
}

void gen_global_pointer_init(VariableInit *init, Type *type) {
  assert(type->kind == TYPE_PTR);
  while (init->vec) { // for non-array primitive types,
                      // only the first element in the brace will be used
    assert(init->vec->size > 0);
    init = *(VariableInit **)vector_get(init->vec, 0);
  }
  assert(init->expr);
  if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR &&
      init->expr->variable->kind == OBJ_STRLIT) {
    // initilize the pointer to a string-literal
    writeline("  .quad %.*s", init->expr->variable->ident->len,
              init->expr->variable->ident->str);
    return;
  }

  if (init->expr->kind == ND_ADD) {
    Variable *left_addr = is_const_var_addr(init->expr->lhs);
    Variable *right_addr = is_const_var_addr(init->expr->rhs);

    if (left_addr && is_constant_number(init->expr->rhs)) {
      writeline("  .quad %.*s+%lld", left_addr->ident->len,
                left_addr->ident->str, number2ulong(eval(init->expr->rhs)));
    } else if (right_addr && is_constant_number(init->expr->lhs)) {
      writeline("  .quad %.*s+%lld", right_addr->ident->len,
                right_addr->ident->str, number2ulong(eval(init->expr->lhs)));
    } else {
      error(NULL, "unsupported initialization of a global pointer.");
    }
    return;
  }

  if (is_const_var_addr(init->expr)) {
    Variable *var = is_const_var_addr(init->expr);
    writeline("  .quad %.*s", var->ident->len, var->ident->str);
    return;
  }

  if (is_constant_number(init->expr)) {
    assert(is_integer(init->expr->type) || init->expr->type->kind == TYPE_PTR);
    writeline("  .quad %llu", number2ulong(eval(init->expr)));
    return;
  }

  error(NULL, "unsupported initialization of a global pointer.");
}

void gen_global_integer_init(VariableInit *init, Type *type) {
  assert(is_integer(type));
  while (init->vec) { // for non-array primitive types,
                      // only the first element in the brace will be used
    assert(init->vec->size > 0);
    init = *(VariableInit **)vector_get(init->vec, 0);
  }
  assert(init->expr);
  Number *val = eval(init->expr);
  switch (type->kind) {
  case TYPE_LONG:
    writeline("  .quad %lld", number2long(val));
    break;
  case TYPE_ULONG:
    writeline("  .quad %llu", number2ulong(val));
    break;
  case TYPE_INT:
  case TYPE_ENUM:
    writeline("  .long %d", number2int(val));
    break;
  case TYPE_UINT:
    writeline("  .long %u", number2uint(val));
    break;
  case TYPE_SHORT:
    writeline("  .value %d", number2short(val));
    break;
  case TYPE_USHORT:
    writeline("  .value %d", number2ushort(val));
    break;
  case TYPE_CHAR:
    writeline("  .byte %d", number2char(val));
    break;
  case TYPE_BOOL:
    writeline("  .byte %d", number2bool(val));
    break;
  case TYPE_UCHAR:
    writeline("  .byte %u", number2uchar(val));
    break;
  default:
    assert(false);
  }
}

static void write_float(Number *num, Type *type) {
  if (type->kind == TYPE_FLOAT) {
    union {
      float x;
      int y;
    } u;
    u.y = 0;
    u.x = number2float(num);
    writeline("  .long %d", u.y);
  } else if (type->kind == TYPE_DOUBLE) {
    union {
      double x;
      int y[2];
    } u;
    u.y[0] = u.y[1] = 0;
    u.x = number2double(num);
    writeline("  .long %d", u.y[0]);
    writeline("  .long %d", u.y[1]);
  } else if (type->kind == TYPE_LDOUBLE) {
    error(NULL, "long double is currently not supported");
  } else {
    assert(false);
  }
}

void gen_global_float_init(VariableInit *init, Type *type) {
  assert(is_float(type));
  while (init->vec) { // for non-array primitive types,
                      // only the first element in the brace will be used
    assert(init->vec->size > 0);
    init = *(VariableInit **)vector_get(init->vec, 0);
  }
  assert(init->expr);
  write_float(eval(init->expr), type);
}

void gen_global_init(VariableInit *init, Type *type) {
  if (init == NULL) {
    writeline("  .zero %d", type->size);
    return;
  }

  if (type->kind == TYPE_ARRAY) {
    gen_global_array_init(init, type);
    return;
  }

  if (type->kind == TYPE_PTR) {
    gen_global_pointer_init(init, type);
    return;
  }

  if (type->kind == TYPE_STRUCT) {
    gen_global_struct_init(init, type);
    return;
  }

  if (type->kind == TYPE_UNION) {
    gen_global_union_init(init, type);
    return;
  }

  if (is_integer(type)) {
    gen_global_integer_init(init, type);
    return;
  }

  if (is_float(type)) {
    gen_global_float_init(init, type);
    return;
  }

  assert(false);
}

void gen_func(Function *func) {
  if (!func->body)
    return; // declared but not defined
  comment(func->token, "function %.*s", func->ident->len, func->ident->str);
  if (func->is_static)
    writeline("  .local %.*s", func->ident->len, func->ident->str);
  else
    writeline("  .globl %.*s", func->ident->len, func->ident->str);
  writeline("%.*s:", func->ident->len, func->ident->str);

  // prologue
  writeline("  push rbp");
  writeline("  mov rbp, rsp");

  // move args to stack
  {
    int count_int = 0;
    int count_float = 0;
    comment(NULL, "move function arguments to stack");
    for (int i = 0; i < func->params->size; ++i) {
      Variable *v = *(Variable **)vector_get(func->params, i);
      if (is_float(v->type)) {
        if (v->type->kind == TYPE_FLOAT)
          writeline("  movss [rbp-%d], xmm%d", v->offset, count_float++);
        else if (v->type->kind == TYPE_DOUBLE)
          writeline("  movsd [rbp-%d], xmm%d", v->offset, count_float++);
        else if (v->type->kind == TYPE_LDOUBLE)
          assert(false);
        else
          assert(false);
      } else if (is_integer(v->type) || v->type->kind == TYPE_PTR) {
        if (v->type->size == 1)
          writeline("  mov [rbp-%d], %s", v->offset, reg_args1[count_int++]);
        else if (v->type->size == 2)
          writeline("  mov [rbp-%d], %s", v->offset, reg_args2[count_int++]);
        else if (v->type->size == 4)
          writeline("  mov [rbp-%d], %s", v->offset, reg_args4[count_int++]);
        else if (v->type->size == 8)
          writeline("  mov [rbp-%d], %s", v->offset, reg_args8[count_int++]);
        else
          assert(false);
      } else {
        assert(false);
      }
    }
  }

  if (func->offset) {
    int ofs = func->offset;
    if (ofs % 8)
      ofs += 8 - ofs % 8; // align by 8
    writeline("  sub rsp, %d", ofs);
  }

  if (func->hidden_va_area) {
    // write variadic argument information to hidden_va_area
    int num_gp = func->params->size;
    int ofs = func->hidden_va_area->offset;

    // va_list
    comment(NULL, "va_list: %.*s", func->ident->len, func->ident->str);
    writeline("  mov dword ptr [rbp-%d], %d", ofs, num_gp * 8); // gp_offset
    writeline("  mov dword ptr [rbp-%d], 0", ofs - 4); // fp_offset (TODO)
    writeline("  movq [rbp-%d], 0", ofs - 8);    // overflow_arg_area (TODO)
    writeline("  movq [rbp-%d], rbp", ofs - 16); // reg_save_area = rbp
    writeline("  subq [rbp-%d], %d", ofs - 16,
              ofs - 24); // reg_save_area -= ofs - 24

    // register save area
    comment(NULL, "register save area: %.*s", func->ident->len,
            func->ident->str);
    writeline("  movq [rbp-%d], rdi", ofs - 24);
    writeline("  movq [rbp-%d], rsi", ofs - 32);
    writeline("  movq [rbp-%d], rdx", ofs - 40);
    writeline("  movq [rbp-%d], rcx", ofs - 48);
    writeline("  movq [rbp-%d], r8", ofs - 56);
    writeline("  movq [rbp-%d], r9", ofs - 64);
    writeline("  movsd [rbp-%d], xmm0", ofs - 72);
    writeline("  movsd [rbp-%d], xmm1", ofs - 88);
    writeline("  movsd [rbp-%d], xmm2", ofs - 104);
    writeline("  movsd [rbp-%d], xmm3", ofs - 120);
    writeline("  movsd [rbp-%d], xmm4", ofs - 136);
    writeline("  movsd [rbp-%d], xmm5", ofs - 152);
    writeline("  movsd [rbp-%d], xmm6", ofs - 168);
    writeline("  movsd [rbp-%d], xmm7", ofs - 184);
  }

  gen(func->body);

  // epilogue
  writeline("  mov rsp, rbp");
  writeline("  pop rbp");
  writeline("  ret");
}

void gen_cast(Node *node) {
  Type *from = node->lhs->type->kind == TYPE_ARRAY
                   ? pointer_type(node->lhs->type->base)
                   : node->lhs->type;
  Type *to = node->type;

  assert(is_scalar(from));
  assert(is_scalar(to) || to->kind == TYPE_VOID);

  gen(node->lhs);
  if (from->kind == to->kind)
    return;

  if (from->kind == TYPE_INT && to->kind == TYPE_ENUM)
    return;

  if (from->kind == TYPE_ENUM && to->kind == TYPE_INT)
    return;

  if (to->kind == TYPE_VOID)
    return;

  if (is_integer(from) && is_float(to)) {
    // all integers are treated as signed long long
    switch (to->kind) {
    case TYPE_FLOAT:
      writeline("  cvtsi2ss xmm0, rax");
      return;
    case TYPE_DOUBLE:
      writeline("  cvtsi2sd xmm0, rax");
      return;
    case TYPE_LDOUBLE:
      error(NULL, "long double is currently not supported");
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_FLOAT) {
    if (is_integer(to)) {
      writeline("  cvttss2si rax, xmm0"); // first, convert to singed long long
      from = base_type(TYPE_LONG);        // then convert from singed long long
      // no return
    } else if (to->kind == TYPE_DOUBLE) {
      writeline("  cvtss2sd xmm0, xmm0");
      return;
    } else if (to->kind == TYPE_LDOUBLE) {
      error(NULL, "long double is currently not supported");
    } else {
      assert(false);
    }
  }

  if (from->kind == TYPE_DOUBLE) {
    if (is_integer(to)) {
      writeline("  cvttsd2si rax, xmm0"); // first, convert to singed long long
      from = base_type(TYPE_LONG);        // then convert from singed long long
      // no return
    } else if (to->kind == TYPE_FLOAT) {
      writeline("  cvtsd2ss xmm0, xmm0");
      return;
    } else if (to->kind == TYPE_LDOUBLE) {
      error(NULL, "long double is currently not supported");
    } else {
      assert(false);
    }
  }

  if (from->kind == TYPE_LDOUBLE) {
    error(NULL, "long double is currently not supported");
  }

  if (from->kind == TYPE_BOOL && is_integer(to))
    return;

  if ((is_integer(from) || from->kind == TYPE_PTR) && to->kind == TYPE_BOOL) {
    writeline("  cmp rax, 0");
    writeline("  setne al");
    writeline("  movzb rax, al");
    return;
  }

  if (from->kind == TYPE_CHAR) {
    switch (to->kind) {
    case TYPE_CHAR:
    case TYPE_SHORT:
    case TYPE_INT:
    case TYPE_ENUM:
    case TYPE_LONG:
      return;
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
    case TYPE_UINT:
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_UCHAR) {
    switch (to->kind) {
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
    case TYPE_INT:
    case TYPE_ENUM:
    case TYPE_LONG:
    case TYPE_UCHAR:
    case TYPE_USHORT:
    case TYPE_UINT:
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_SHORT) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
      writeline("  movzx rax, ax");
      return;
    case TYPE_UINT:
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
    case TYPE_INT:
    case TYPE_ENUM:
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_USHORT) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
    case TYPE_UINT:
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
      writeline("  movsx rax, ax");
      return;
    case TYPE_INT:
    case TYPE_ENUM:
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_INT || from->kind == TYPE_ENUM) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  mov eax, eax");
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
      writeline("  movzx rax, ax");
      return;
    case TYPE_UINT:
      writeline("  mov eax, eax");
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
      writeline("  movsx rax, ax");
      return;
    case TYPE_INT:
    case TYPE_ENUM:
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_UINT) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
      writeline("  movzx rax, ax");
      return;
    case TYPE_UINT:
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
      writeline("  movsx rax, ax");
      return;
    case TYPE_INT:
      writeline("  movsxd rax, eax");
    case TYPE_ENUM:
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_LONG) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
      writeline("  movzx rax, ax");
      return;
    case TYPE_UINT:
      writeline("  mov eax, eax");
      return;
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
      writeline("  movsx rax, ax");
      return;
    case TYPE_INT:
    case TYPE_ENUM:
      writeline("  movsxd rax, eax");
      return;
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }

  if (from->kind == TYPE_ULONG || from->kind == TYPE_PTR) {
    switch (to->kind) {
    case TYPE_UCHAR:
      writeline("  movzx rax, al");
      return;
    case TYPE_USHORT:
      writeline("  movzx rax, ax");
      return;
    case TYPE_UINT:
      writeline("  mov eax, eax");
      return;
    case TYPE_ULONG:
    case TYPE_PTR:
      return;
    case TYPE_CHAR:
      writeline("  movsx rax, al");
      return;
    case TYPE_SHORT:
      writeline("  movsx rax, ax");
      return;
    case TYPE_INT:
    case TYPE_ENUM:
      writeline("  movsxd rax, eax");
      return;
    case TYPE_LONG:
      return;
    default:
      assert(false);
    }
  }
  assert(false);
}

void gen_number(Node *node) {
  assert(node->kind == ND_NUM);
  assert(node->num);

  if (is_float(node->type)) {
    FloatLiteral *f = calloc(1, sizeof(FloatLiteral));
    f->num = node->num;
    f->index = float_literal_index++;
    vector_push(float_literals, &f);

    if (node->type->kind == TYPE_FLOAT)
      writeline("  movss xmm0, .float%d[rip]", f->index);
    else if (node->type->kind == TYPE_DOUBLE)
      writeline("  movsd xmm0, .float%d[rip]", f->index);
    else if (node->type->kind == TYPE_LDOUBLE)
      error(NULL, "long double is currently not supported");
    else
      assert(false);
    return;
  }

  switch (node->type->kind) {
  case TYPE_LONG:
    writeline("  mov rax, %lld", number2long(node->num));
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    writeline("  mov rax, %llu", number2ulong(node->num));
    break;
  case TYPE_INT:
    writeline("  mov rax, %d", number2int(node->num));
    break;
  case TYPE_UINT:
    writeline("  mov rax, %u", number2uint(node->num));
    break;
  case TYPE_SHORT:
    writeline("  mov rax, %d", number2short(node->num));
    break;
  case TYPE_USHORT:
    writeline("  mov rax, %u", number2ushort(node->num));
    break;
  case TYPE_CHAR:
    writeline("  mov rax, %d", number2char(node->num));
    break;
  case TYPE_UCHAR:
    writeline("  mov rax, %u", number2uchar(node->num));
    break;
  case TYPE_BOOL:
    writeline("  mov rax, %d", number2bool(node->num));
    break;
  default:
    assert(false);
  }
}

void gen_numerical_operator(Node *node) {
  assert(same_type(node->type, node->lhs->type));
  assert(same_type(node->type, node->rhs->type));
  Type *operand_type = node->type;

  if (is_float(operand_type)) {
    if (node->kind == ND_ADD) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      pop_xmm(1, operand_type->kind);

      if (operand_type->kind == TYPE_FLOAT)
        writeline("  addss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  addsd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      return;
    }

    if (node->kind == ND_MUL) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      pop_xmm(1, operand_type->kind);

      if (operand_type->kind == TYPE_FLOAT)
        writeline("  mulss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  mulsd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      return;
    }

    if (node->kind == ND_SUB) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  movss xmm1, xmm0");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  movsd xmm1, xmm0");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      pop_xmm(0, operand_type->kind);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  subss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  subsd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else {
        assert(false);
      }
      return;
    }

    if (node->kind == ND_DIV) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  movss xmm1, xmm0");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  movsd xmm1, xmm0");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      pop_xmm(0, operand_type->kind);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  divss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  divsd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      return;
    }
    assert(false);
  }

  if (node->kind == ND_ADD || node->kind == ND_MUL) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  %s rax, rdi", node->kind == ND_ADD ? "add" : "imul");
    return;
  }

  if (node->kind == ND_SUB) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  sub rax, rdi");
    return;
  }

  if (node->kind == ND_DIV || node->kind == ND_MOD) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");

    if (is_unsigned(operand_type)) {
      writeline("  mov rdx, 0");
      writeline("  div rdi");
    } else {
      writeline("  cqo");
      writeline("  idiv rdi");
    }

    if (node->kind == ND_MOD) {
      assert(is_integer(operand_type));
      writeline("  mov rax, rdx");
    }
    return;
  }
  assert(false);
}

void gen_logical_operator(Node *node) {
  assert(node->type->kind == TYPE_BOOL);
  assert(node->lhs->type->kind == TYPE_BOOL);

  if (node->kind == ND_LOGNOT) {
    gen(node->lhs);
    writeline("  cmp rax, 0");
    writeline("  sete al");
    writeline("  movzx rax, al");
    return;
  }

  assert(node->rhs->type->kind == TYPE_BOOL);

  if (node->kind == ND_LOGOR) {
    gen(node->lhs);
    writeline("  cmp rax, 0");
    writeline("  jne .Ltrue%d", node->label_index);
    gen(node->rhs);
    writeline("  cmp rax, 0");
    writeline("  jne .Ltrue%d", node->label_index);
    writeline("  mov rax, 0");
    writeline("  jmp .Lend%d", node->label_index);
    writeline(".Ltrue%d:", node->label_index);
    writeline("  mov rax, 1");
    writeline(".Lend%d:", node->label_index);
    return;
  }

  if (node->kind == ND_LOGAND) {
    gen(node->lhs);
    writeline("  cmp rax, 0");
    writeline("  je .Lfalse%d", node->label_index);
    gen(node->rhs);
    writeline("  cmp rax, 0");
    writeline("  je .Lfalse%d", node->label_index);
    writeline("  mov rax, 1");
    writeline("  jmp .Lend%d", node->label_index);
    writeline(".Lfalse%d:", node->label_index);
    writeline("  mov rax, 0");
    writeline(".Lend%d:", node->label_index);
    return;
  }

  assert(false);
}

void gen_bit_operator(Node *node) {
  assert(is_integer(node->type));
  assert(is_integer(node->lhs->type));

  if (node->kind == ND_BITNOT) {
    gen(node->lhs);
    writeline("  not rax");
    return;
  }

  assert(is_integer(node->rhs->type));

  if (node->kind == ND_LSHIFT || node->kind == ND_RSHIFT) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rcx, rax");
    writeline("  pop rax");
    if (node->kind == ND_LSHIFT)
      writeline("  shl rax, cl");
    else
      writeline("  s%cr rax, cl", is_unsigned(node->lhs->type) ? 'h' : 'a');
    return;
  }

  assert(same_type(node->type, node->lhs->type));
  assert(same_type(node->type, node->rhs->type));

  gen(node->lhs);
  writeline("  push rax");
  gen(node->rhs);
  writeline("  mov rdi, rax");
  writeline("  pop rax");

  if (node->kind == ND_BITXOR)
    writeline("  xor rax, rdi");
  else if (node->kind == ND_BITOR)
    writeline("  or rax, rdi");
  else if (node->kind == ND_BITAND)
    writeline("  and rax, rdi");
  else
    assert(false);
}

void gen_comparison_operator(Node *node) {
  assert(node->type->kind == TYPE_BOOL);
  assert(same_type(node->lhs->type, node->rhs->type));
  Type *operand_type = node->lhs->type;

  if (is_float(operand_type)) {
    if (node->kind == ND_EQ || node->kind == ND_NE) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      pop_xmm(1, operand_type->kind);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  ucomiss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  ucomisd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);

      // nan check by PF
      if (node->kind == ND_EQ)
        writeline("  setnp al"); // al = !isnan(lhs) and !isnan(rhs)
      else
        writeline("  setp al"); // al = isnan(lhs) or isnan(rhs)

      // compare lhs and rhs again because setnp/setp clears eflags
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  ucomiss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  ucomisd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);

      if (node->kind == ND_EQ) {
        // cmovne can not be used with immediate values
        writeline("  mov rdi, 0");
        writeline("  cmovne rax, rdi"); // if(ZF==1) rax = 0
      } else {
        // cmovne can not be used with immediate values
        writeline("  mov rdi, 1");
        writeline("  cmovne rax, rdi"); // if(ZF==1) rax = 1
      }

      writeline("  movzb rax, al");
      return;
    }

    if (node->kind == ND_LT || node->kind == ND_LE) {
      gen(node->lhs);
      push_xmm(0, operand_type->kind);
      gen(node->rhs);
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  movss xmm1, xmm0");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  movsd xmm1, xmm0");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);
      pop_xmm(0, operand_type->kind);

      // nan check is done by comisd and comiss
      if (operand_type->kind == TYPE_FLOAT)
        writeline("  comiss xmm0, xmm1");
      else if (operand_type->kind == TYPE_DOUBLE)
        writeline("  comisd xmm0, xmm1");
      else if (operand_type->kind == TYPE_LDOUBLE)
        error(NULL, "long double is currently not supported");
      else
        assert(false);

      if (node->kind == ND_LT)
        writeline("  setb al");
      else
        writeline("  setbe al");

      writeline("  movzb rax, al");
      return;
    }

    assert(false);
  }

  if (node->kind == ND_EQ || node->kind == ND_NE) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  cmp rax, rdi");
    writeline("  %s al", node->kind == ND_EQ ? "sete" : "setne");
    writeline("  movzb rax, al");
    return;
  }

  if (node->kind == ND_LT || node->kind == ND_LE) {
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cmp rax, rdi");
    if (node->kind == ND_LT)
      writeline("  set%c al", is_unsigned(operand_type) ? 'b' : 'l');
    else
      writeline("  set%ce al", is_unsigned(operand_type) ? 'b' : 'l');
    writeline("  movzb rax, al");
    return;
  }
  assert(false);
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_CALL:
    if (node->lhs->kind == ND_VAR)
      comment(node->token, "ND_CALL %.*s", node->lhs->variable->ident->len,
              node->lhs->variable->ident->str);
    else
      comment(node->token, "ND_CALL");
    gen_call(node);
    return;
  case ND_BLOCK:
    comment(node->token, "ND_BLOCK");
    gen_block(node);
    return;
  case ND_IF:
    comment(node->token, "ND_IF %d", node->label_index);
    gen_if(node);
    return;
  case ND_DO:
    comment(node->token, "ND_DO %d", node->label_index);
    gen_do(node);
    return;
  case ND_WHILE:
    comment(node->token, "ND_WHILE %d", node->label_index);
    gen_while(node);
    return;
  case ND_FOR:
    comment(node->token, "ND_FOR %d", node->label_index);
    gen_for(node);
    return;
  case ND_SWITCH:
    comment(node->token, "ND_SWITCH %d", node->label_index);
    gen_switch(node);
    return;
  case ND_CASE:
    comment(node->token, "ND_CASE %d", node->label_index);
    writeline(".Lcase%d:", node->label_index);
    gen(node->body);
    return;
  case ND_DEFAULT:
    writeline(".Ldefault%d:", node->label_index);
    comment(node->token, "ND_DEFAULT %d", node->label_index);
    gen(node->body);
    return;
  case ND_RETURN:
    comment(node->token, "ND_RETURN");
    if (node->lhs)
      gen(node->lhs);
    writeline("  mov rsp, rbp");
    writeline("  pop rbp");
    writeline("  ret");
    return;
  case ND_CONTINUE:
    comment(node->token, "ND_CONTINUE");
    writeline("  jmp .Lcontinue%d", node->label_index);
    return;
  case ND_BREAK:
    comment(node->token, "ND_BREAK");
    writeline("  jmp .Lend%d", node->label_index);
    return;
  case ND_GOTO:
    comment(node->token, "ND_GOTO %.*s", node->token->str->len,
            node->token->str->str);
    writeline("  jmp .Lgoto%d", node->label_index);
    return;
  case ND_LABEL:
    comment(node->token, "ND_LABEL %.*s", node->token->str->len,
            node->token->str->str);
    writeline(".Lgoto%d:", node->label_index);
    if (node->body)
      gen(node->body);
    return;
  case ND_NUM:
    comment(node->token, "ND_NUM %s", type_text(node->type->kind));
    gen_number(node);
    return;
  case ND_VAR:
    comment(node->token, "ND_VAR");
    gen_address(node);
    if (node->variable->kind != OBJ_FUNC)
      load(node->type);
    return;
  case ND_ASSIGN:
    comment(node->token, "ND_ASSIGN");
    gen_address(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    store(node->type);
    return;
  case ND_COND:
    comment(node->token, "ND_COND %d", node->label_index);
    gen(node->condition);
    writeline("  cmp rax, 0");
    writeline("  je .Lcond_rhs%d", node->label_index);
    gen(node->lhs);
    writeline("  jmp .Lend%d", node->label_index);
    writeline(".Lcond_rhs%d:", node->label_index);
    gen(node->rhs);
    writeline(".Lend%d:", node->label_index);
    return;
  case ND_ADDR:
    comment(node->token, "ND_ADDR");
    gen_address(node->lhs);
    return;
  case ND_DEREF:
    comment(node->token, "ND_DEREF");
    gen(node->lhs);
    load(node->type);
    return;
  case ND_MEMBER:
    comment(node->token, "ND_MEMBER");
    gen_address(node);
    load(node->type);
    return;
  case ND_ADD:
    comment(node->token, "ND_ADD");
    gen_numerical_operator(node);
    return;
  case ND_SUB:
    comment(node->token, "ND_SUB");
    gen_numerical_operator(node);
    return;
  case ND_MUL:
    comment(node->token, "ND_MUL");
    gen_numerical_operator(node);
    return;
  case ND_DIV:
    comment(node->token, "ND_DIV");
    gen_numerical_operator(node);
    return;
  case ND_MOD:
    comment(node->token, "ND_MOD");
    gen_numerical_operator(node);
    return;
  case ND_LOGNOT:
    comment(node->token, "ND_LOGNOT");
    gen_logical_operator(node);
    return;
  case ND_LOGOR:
    comment(node->token, "ND_LOGOR %d", node->label_index);
    gen_logical_operator(node);
    return;
  case ND_LOGAND:
    comment(node->token, "ND_LOGAND %d", node->label_index);
    gen_logical_operator(node);
    return;
  case ND_LSHIFT:
    comment(node->token, "ND_LSHIFT");
    gen_bit_operator(node);
    return;
  case ND_RSHIFT:
    comment(node->token, "ND_RSHIFT");
    gen_bit_operator(node);
    return;
  case ND_BITXOR:
    comment(node->token, "ND_BITXOR");
    gen_bit_operator(node);
    return;
  case ND_BITOR:
    comment(node->token, "ND_BITOR");
    gen_bit_operator(node);
    return;
  case ND_BITAND:
    comment(node->token, "ND_BITAND");
    gen_bit_operator(node);
    return;
  case ND_BITNOT:
    comment(node->token, "ND_BITNOT");
    gen_bit_operator(node);
    return;
  case ND_EQ:
    comment(node->token, "ND_EQ");
    gen_comparison_operator(node);
    return;
  case ND_NE:
    comment(node->token, "ND_NE");
    gen_comparison_operator(node);
    return;
  case ND_LT:
    comment(node->token, "ND_LT");
    gen_comparison_operator(node);
    return;
  case ND_LE:
    comment(node->token, "ND_LE");
    gen_comparison_operator(node);
    return;
  case ND_CAST:
    comment(node->token, "ND_CAST %s --> %s", type_text(node->lhs->type->kind),
            type_text(node->type->kind));
    gen_cast(node);
    return;
  case ND_COMMA:
    comment(node->token, "ND_COMMA");
    gen(node->lhs);
    gen(node->rhs);
    return;
  default:
    assert(false);
  }
}

void generate_code(FILE *output_stream) {
  assert(output_stream);
  ostream = output_stream;
  float_literals = new_vector(0, sizeof(FloatLiteral *));
  writeline(".intel_syntax noprefix");

  // string literals
  for (int i = 0; i < strings->size; i++) {
    Variable *v = map_geti(strings, i);
    writeline("%.*s:", v->ident->len, v->ident->str);
    writeline("  .string \"%s\"", v->string_literal);
  }

  // global variables
  for (int i = 0; i < global_scope->objects->size; i++) {
    Variable *v = map_geti(global_scope->objects, i);
    if (v->kind != OBJ_GVAR)
      continue;
    if (v->is_extern)
      continue;
    if (v->type->kind == TYPE_ARRAY && v->type->size < 0)
      error(v->token ? &v->token->pos : NULL, "unknown array size");
    writeline(".data");
    if (v->is_static)
      writeline(".local %.*s", v->ident->len, v->ident->str);
    else
      writeline(".globl %.*s", v->ident->len, v->ident->str);
    writeline("%.*s:", v->ident->len, v->ident->str);
    gen_global_init(v->init, v->type);
  }

  // static local variables
  for (int i = 0; i < static_local_variables->size; i++) {
    Variable *v = *(Variable **)vector_get(static_local_variables, i);
    assert(v->kind == OBJ_LVAR);
    assert(v->is_static);
    assert(v->internal_ident);
    writeline(".data");
    writeline(".local %.*s", v->internal_ident->len, v->internal_ident->str);
    writeline("%.*s:", v->internal_ident->len, v->internal_ident->str);
    gen_global_init(v->init, v->type);
  }

  // functions
  writeline(".text");
  for (int i = 0; i < global_scope->objects->size; i++) {
    Function *f = map_geti(global_scope->objects, i);
    if (f->kind != OBJ_FUNC)
      continue;
    gen_func(f);
  }

  // float literals
  if (float_literals->size) {
    for (int i = 0; i < float_literals->size; i++) {
      FloatLiteral *f = *(FloatLiteral **)vector_get(float_literals, i);
      writeline(".data");
      writeline(".local .float%d", f->index);
      writeline(".float%d:", f->index);
      write_float(f->num, f->num->type);
    }
  }

  ostream = NULL;
  float_literals = NULL;
}
