#include "boncc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *ostream;
static const char *reg_args[][6] = {{NULL},
                                    {"dil", "sil", "dl", "cl", "r8b", "r9b"},
                                    {"di", "si", "dx", "cx", "r8w", "r9w"},
                                    {NULL},
                                    {"edi", "esi", "edx", "ecx", "r8d", "r9d"},
                                    {NULL},
                                    {NULL},
                                    {NULL},
                                    {"rdi", "rsi", "rdx", "rcx", "r8", "r9"}};
static const char *reg_ax[] = {NULL, "al", "ax", NULL, "eax",
                               NULL, NULL, NULL, "rax"};
static const char *reg_dx[] = {NULL, "dl", "dx", NULL, "edx",
                               NULL, NULL, NULL, "rdx"};
static const char *reg_r10[] = {NULL, "r10b", "r10w", NULL, "r10d",
                                NULL, NULL,   NULL,   "r10"};

// Check the rsp alignment by the variable rsp_shift.
// In the AST traversal, rsp_shift follows the push and pop operations.
// In any single expression and any statement,
// "push" and "pop" instructions are always balanced and rsp remains aligned.
// We need to care about the rsp alignment when multiple expressions are
// combined, e.g "f() + g()".
// Before evaluating "f()", rsp is aligned to 16byte.
// After "f()", rsp remains aligned.
// Then the result of "f()" is pushed to the stack (rsp is shifted 8byte).
// Before "g()" is called, rsp is not aligned to 16byte.
// Therefore rsp is adjusted by 8byte, "g()" is evaluated, and then rsp
// adjustment is reverted by 8byte. Next, the result of "f()" is popped to a
// register (rsp is shifted 8byte) and "add" instruction is processed.
// After that, rsp is aligned to 16byte.
static int rsp_shift = 0;

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

static void push(const char *reg) {
  assert(rsp_shift >= 0);
  writeline("  push %s", reg);
  rsp_shift += 8; // push only 8 byte register
}

static void pop(const char *reg) {
  writeline("  pop %s", reg);
  rsp_shift -= 8; // pop only 8 byte register
  assert(rsp_shift >= 0);
}

static void push_xmm(int n, TypeKind kind) {
  assert(rsp_shift >= 0);
  switch (kind) {
  case TYPE_FLOAT: // align to 8 even if size is 4
    writeline("  sub rsp, 8        # push %s", type_text(kind));
    writeline("  movss [rsp], xmm%d # push %s", n, type_text(kind));
    rsp_shift += 8;
    break;
  case TYPE_DOUBLE:
    writeline("  sub rsp, 8        # push %s", type_text(kind));
    writeline("  movsd [rsp], xmm%d # push %s", n, type_text(kind));
    rsp_shift += 8;
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    rsp_shift += 16;
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
    rsp_shift -= 8;
    break;
  case TYPE_DOUBLE:
    writeline("  movsd xmm%d, [rsp] # pop %s", n, type_text(kind));
    writeline("  add rsp, 8        # pop %s", type_text(kind));
    rsp_shift -= 8;
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    rsp_shift -= 16;
    break;
  default:
    assert(false);
  }
  assert(rsp_shift >= 0);
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

bool use_xmm(Type *type, int begin, int end) {
  if (is_scalar(type))
    return is_float(type);

  if (type->kind == TYPE_ARRAY) {
    int cur = 0;
    for (int i = 0; i < type->array_size && cur < end;
         ++i, cur += type->base->size) {
      if (cur < begin)
        continue;
      if (!use_xmm(type->base, 0, end - cur))
        return false;
    }
    return true;
  }

  if (type->kind == TYPE_STRUCT || type->kind == TYPE_UNION) {
    for (Member *m = type->member; m && m->offset < end; m = m->next) {
      if (m->offset >= begin) {
        if (!use_xmm(m->type, 0, end - m->offset))
          return false;
      } else if (m->offset + m->type->size >= begin) {
        if (!use_xmm(m->type, begin - m->offset, end - m->offset))
          return false;
      }
    }
    return true;
  }

  assert(false);
  return true;
}

void load(Type *type) {
  // src  : the address where rax is pointing to
  // dest : rax or xmm0
  comment(NULL, "load %s (size:%d)", type_text(type->kind), type->size);

  if (type->kind == TYPE_ARRAY || pass_on_memory(type))
    return; // nothing to do

  if (is_struct_union(type)) {
    writeline("  mov r10, rax");
    int xmm_index = 0;
    const char **reg = reg_ax;
    for (int b = 0; b < type->size; b += 8) {
      // put 8byte onto register
      if (use_xmm(type, b, b + 8)) {
        if (type->size == b + 4)
          writeline("  movss xmm%d, [r10+%d]", xmm_index++, b);
        else
          writeline("  movsd xmm%d, [r10+%d]", xmm_index++, b);
      } else {
        writeline("  mov %s, [r10+%d]",
                  reg[type->size - b > 8 ? 8 : type->size - b], b);
        reg = reg_dx;
      }
    }
    return;
  }

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

  pop("r10");

  if (is_struct_union(type)) {
    if (pass_on_memory(type)) {
      // src  : the struct/union that rax is pointing to
      // dest : the address popped from stack
      for (int i = 0; i < type->size; i++) {
        // copy each bytes like memcpy
        writeline("  mov r11b, [rax+%d]", i);
        writeline("  mov [r10+%d], r11b", i);
      }
      return;
    }

    // src  : the struct/union on registers
    // dest : the struct/union that the address popped from stack is pointing
    // to
    int xmm_index = 0;
    const char **reg = reg_ax;
    for (int b = 0; b < type->size; b += 8) {
      // copy 8byte from register
      if (use_xmm(type, b, b + 8)) {
        if (type->size == b + 4)
          writeline("  movss [r10+%d], xmm%d", b, xmm_index++);
        else
          writeline("  movsd [r10+%d], xmm%d", b, xmm_index++);
      } else {
        writeline("  mov [r10+%d], %s", b,
                  reg[type->size - b > 8 ? 8 : type->size - b]);
        reg = reg_dx;
      }
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

  writeline("  mov [r10], %s", reg_ax[type->size]);
}

void gen_if(Node *node) {
  assert(node->condition->type->kind == TYPE_BOOL);
  gen(node->condition);
  writeline("  cmp al, 0");
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
  assert(node->condition->type->kind == TYPE_BOOL);
  writeline(".Ldo%d:", node->label_index);
  gen(node->body);
  writeline(".Lcontinue%d:", node->label_index);
  gen(node->condition);
  writeline("  cmp al, 0");
  writeline("  jne .Ldo%d", node->label_index);
  writeline(".Lend%d:", node->label_index);
}

void gen_while(Node *node) {
  assert(node->condition->type->kind == TYPE_BOOL);
  writeline(".Lcontinue%d:", node->label_index);
  gen(node->condition);
  writeline("  cmp al, 0");
  writeline("  je .Lend%d", node->label_index);

  gen(node->body);
  writeline("  jmp .Lcontinue%d", node->label_index);
  writeline(".Lend%d:", node->label_index);
}

void gen_for(Node *node) {
  assert(node->condition->type->kind == TYPE_BOOL);
  if (node->init)
    gen(node->init);

  writeline(".Lfor%d:", node->label_index);
  if (node->condition) {
    gen(node->condition);
    writeline("  cmp al, 0");
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
  assert(is_integer(node->condition->type));
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

  int n_fp = 0;
  int alignment = 0;
  int shift_by_args = 0;

  // put arguments into registers or the stack
  {
    int sz = node->args->size;
    int count_gp = pass_on_memory(node->type);
    int count_fp = 0;

    Vector *args_on_register = new_vector(0, sizeof(Node *));
    Vector *args_on_stack = new_vector(0, sizeof(Node *));

    // scan args to see where to place them
    for (int i = 0; i < sz; ++i) {
      Node *d = *(Node **)vector_get(node->args, i);
      if (is_float(d->type)) {
        if (count_fp == 8) {
          vector_push(args_on_stack, &d);
          shift_by_args += iceil(d->type->size, 8);
        } else {
          vector_push(args_on_register, &d);
          count_fp++;
        }
      } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
        if (count_gp == 6) {
          vector_push(args_on_stack, &d);
          shift_by_args += iceil(d->type->size, 8);
        } else {
          vector_push(args_on_register, &d);
          count_gp++;
        }
      } else {
        assert(false);
      }
    }

    // Align rsp before evaluating the arguments.
    // After this alignment, args_on_stack are evaluated and pushed to the
    // stack. Then args_on_register are evaluated, pushed to the stack, and
    // popped to registers. The stack operations for args_on_register are
    // balanced and do not affect the rsp. Here, rsp must be aligned
    // considering the shift by args_on_stack and the current rsp_shift.
    alignment = (rsp_shift + shift_by_args) % 16;
    if (alignment) {
      // shift 8byte because rps is alywas aligned to 8byte
      assert(alignment % 8 == 0);
      writeline("  sub rsp, 8 # alignment");
      rsp_shift += 8;
    }

    n_fp = count_fp;

    if (args_on_stack->size > 0) {
      comment(NULL, "push arguments that will be passed by stack memory");
      int before = rsp_shift;
      for (int i = args_on_stack->size - 1; i >= 0; --i) {
        Node *d = *(Node **)vector_get(args_on_stack, i);
        gen(d);
        if (is_float(d->type)) {
          push_xmm(0, d->type->kind);
        } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
          push("rax");
        } else {
          assert(false);
        }
      }
      assert(rsp_shift == before + shift_by_args);
    }

    if (args_on_register->size > 0) {
      comment(NULL, "push arguments that will be passed by registers");
      for (int i = 0; i < args_on_register->size; ++i) {
        Node *d = *(Node **)vector_get(args_on_register, i);
        gen(d);
        if (is_float(d->type)) {
          push_xmm(0, d->type->kind);
        } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
          push("rax");
        } else {
          assert(false);
        }
      }

      comment(NULL, "pop arguments to registers");
      for (int i = args_on_register->size - 1; i >= 0; --i) {
        Node *d = *(Node **)vector_get(args_on_register, i);
        if (is_float(d->type)) {
          assert(count_fp > 0);
          pop_xmm(--count_fp, d->type->kind);
        } else if (is_integer(d->type) || d->type->kind == TYPE_PTR) {
          assert(count_gp > 0);
          pop(reg_args[8][--count_gp]);
        } else {
          assert(false);
        }
      }
    }

    if (pass_on_memory(node->type)) {
      // hidden argument to pass return_buffer address
      writeline("  mov rdi, rbp # hidden argument of return buffer address");
      writeline("  sub rdi, %d # hidden argument of return buffer address",
                node->caller->return_buffer_offset);
    }
  }

  if (node->lhs->kind == ND_VAR && node->lhs->variable->kind == OBJ_FUNC) {
    writeline("  mov al, %d", n_fp);
    writeline("  call %.*s", node->lhs->variable->ident->len,
              node->lhs->variable->ident->str);
  } else {
    gen(node->lhs); // calculate function address
    writeline("  mov r10, rax");
    writeline("  mov al, %d", n_fp);
    writeline("  call r10");
  }
  if (alignment || shift_by_args) {
    // undo the 16byte alignment and the shift by arguments pushed to stack
    int shift = alignment + shift_by_args;
    writeline("  add rsp, %d", shift);
    rsp_shift -= shift;
  }

  if (!pass_on_memory(node->type) && is_integer(node->type) &&
      node->type->size < 8) {
    // Extend the returned integer to 64bit.
    // This compiler treats all integers on registers as 64bit integers.
    // This extension is required when the callee function is compiled by
    // another compiler.
    if (node->type->kind == TYPE_INT || node->type->kind == TYPE_ENUM)
      writeline("  movsxd rax, eax # extend the returned integer to 64bit");
    else if (node->type->kind == TYPE_UINT)
      writeline("  mov eax, eax # extend the returned integer to 64bit");
    else if (node->type->kind == TYPE_BOOL)
      writeline("  movzx rax, al # extend the returned integer to 64bit");
    else if (is_unsigned(node->type))
      writeline("  movzx rax, %s # extend the returned integer to 64bit",
                reg_ax[node->type->size]);
    else if (is_signed(node->type))
      writeline("  movsx rax, %s # extend the returned integer to 64bit",
                reg_ax[node->type->size]);
    else
      assert(false);
  }
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

// In the functions prologue() and epilogue(),
// no need to adjust the rsp_shift.
// Because they are always paired and do not break the rsp alignment.
void prologue() {
  comment(NULL, "prologue");
  writeline("  push rbp");
  writeline("  mov rbp, rsp");
}

void epilogue() {
  comment(NULL, "epilogue");
  writeline("  mov rsp, rbp");
  writeline("  pop rbp");
  writeline("  ret"); // The "ret" instruction pops the return address that was
                      // pushed by "call" instruction and jump to it.
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

  prologue();

  // After prologue(), rsp is aligned to 16byte as following:
  // 1. rsp is aligned to 16bytes before the "call" instruction.
  // 2. The "call" instruction pushes the return address (8byte).
  // 3. rbp (8byte) is pushed in the prologue.
  rsp_shift = 0;

  {
    int count_gp = 0;
    int count_fp = 0;
    int ofs = 16; // offset to access arguments passed by stack memory.
                  // This is initialized by 16,
                  // because of "call" and "push rbp" in the prologue.

    // save the address to the return buffer given by caller
    if (pass_on_memory(func->type->return_type))
      writeline("  mov [rbp-%d], %s", func->return_buffer_address->offset,
                reg_args[8][count_gp++]);

    // move args to stack
    if (func->params->size > 0)
      comment(NULL, "move function arguments to stack");
    for (int i = 0; i < func->params->size; ++i) {
      Variable *v = *(Variable **)vector_get(func->params, i);
      if (is_float(v->type)) {
        if (count_fp == 8) {
          if (v->type->kind == TYPE_FLOAT) {
            writeline("  movss xmm0, [rbp+%d]", ofs);
            writeline("  movss [rbp-%d], xmm0", v->offset);
          } else if (v->type->kind == TYPE_DOUBLE) {
            writeline("  movsd xmm0, [rbp+%d]", ofs);
            writeline("  movsd [rbp-%d], xmm0", v->offset);
          } else if (v->type->kind == TYPE_LDOUBLE) {
            assert(false);
          } else {
            assert(false);
          }
          ofs += iceil(v->type->size, 8);
        } else {
          if (v->type->kind == TYPE_FLOAT)
            writeline("  movss [rbp-%d], xmm%d", v->offset, count_fp++);
          else if (v->type->kind == TYPE_DOUBLE)
            writeline("  movsd [rbp-%d], xmm%d", v->offset, count_fp++);
          else if (v->type->kind == TYPE_LDOUBLE)
            assert(false);
          else
            assert(false);
        }
      } else if (is_integer(v->type) || v->type->kind == TYPE_PTR) {
        if (count_gp == 6) {
          writeline("  mov r10, [rbp+%d]", ofs);
          writeline("  mov [rbp-%d], %s", v->offset, reg_r10[v->type->size]);
          ofs += iceil(v->type->size, 8);
        } else {
          writeline("  mov [rbp-%d], %s", v->offset,
                    reg_args[v->type->size][count_gp++]);
        }
      } else {
        assert(false);
      }
    }
  }

  int local_var_offset = 0;
  if (func->offset) {
    local_var_offset = iceil(func->offset, 8); // align rsp to 8byte
    writeline("  sub rsp, %d", local_var_offset);
    rsp_shift += local_var_offset;
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
  assert(rsp_shift == local_var_offset);
  epilogue(); // By epilogue, rsp is restored to the value before the "call".
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

  assert((is_integer(from) || from->kind == TYPE_PTR) &&
         (is_integer(to) || to->kind == TYPE_PTR));

  if (from->kind == TYPE_BOOL) {
    writeline("  movzx rax, al");
    return;
  }

  if (to->kind == TYPE_BOOL) {
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
    push("rax");
    gen(node->rhs);
    pop("rdi");
    writeline("  %s rax, rdi", node->kind == ND_ADD ? "add" : "imul");
    return;
  }

  if (node->kind == ND_SUB) {
    gen(node->lhs);
    push("rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    pop("rax");
    writeline("  sub rax, rdi");
    return;
  }

  if (node->kind == ND_DIV || node->kind == ND_MOD) {
    gen(node->lhs);
    push("rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    pop("rax");

    // "div" and "idiv" concatenates rdi and rax,
    // then divide the concatenated 128bit value by the argument.
    if (is_unsigned(operand_type)) {
      writeline("  mov rdx, 0 # clear rdx before div");
      writeline("  div rdi");
    } else {
      writeline("  cqo # sign-extend rax to rdx before idiv");
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
    writeline("  cmp al, 0");
    writeline("  sete al");
    writeline("  movzx rax, al");
    return;
  }

  assert(node->rhs->type->kind == TYPE_BOOL);

  if (node->kind == ND_LOGOR) {
    gen(node->lhs);
    writeline("  cmp al, 0");
    writeline("  jne .Ltrue%d", node->label_index);
    gen(node->rhs);
    writeline("  cmp al, 0");
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
    writeline("  cmp al, 0");
    writeline("  je .Lfalse%d", node->label_index);
    gen(node->rhs);
    writeline("  cmp al, 0");
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
  assert(same_type(node->type, node->lhs->type));

  if (node->kind == ND_BITNOT) {
    gen(node->lhs);
    writeline("  not rax");
    return;
  }

  assert(is_integer(node->rhs->type));

  if (node->kind == ND_LSHIFT || node->kind == ND_RSHIFT) {
    gen(node->lhs);
    push("rax");
    gen(node->rhs);
    writeline("  mov rcx, rax");
    pop("rax");
    if (node->kind == ND_LSHIFT)
      writeline("  shl rax, cl");
    else
      writeline("  s%cr rax, cl", is_unsigned(node->lhs->type) ? 'h' : 'a');
    return;
  }

  assert(same_type(node->type, node->rhs->type));

  gen(node->lhs);
  push("rax");
  gen(node->rhs);
  writeline("  mov rdi, rax");
  pop("rax");

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
    push("rax");
    gen(node->rhs);
    pop("rdi");
    writeline("  cmp rax, rdi");
    writeline("  %s al", node->kind == ND_EQ ? "sete" : "setne");
    writeline("  movzb rax, al");
    return;
  }

  if (node->kind == ND_LT || node->kind == ND_LE) {
    gen(node->lhs);
    push("rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    pop("rax");
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
  int shift_before = rsp_shift;
  switch (node->kind) {
  case ND_CALL:
    if (node->lhs->kind == ND_VAR)
      comment(node->token, "ND_CALL %.*s", node->lhs->variable->ident->len,
              node->lhs->variable->ident->str);
    else
      comment(node->token, "ND_CALL");
    gen_call(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_BLOCK:
    comment(node->token, "ND_BLOCK");
    gen_block(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_IF:
    comment(node->token, "ND_IF %d", node->label_index);
    gen_if(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_DO:
    comment(node->token, "ND_DO %d", node->label_index);
    gen_do(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_WHILE:
    comment(node->token, "ND_WHILE %d", node->label_index);
    gen_while(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_FOR:
    comment(node->token, "ND_FOR %d", node->label_index);
    gen_for(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_SWITCH:
    comment(node->token, "ND_SWITCH %d", node->label_index);
    gen_switch(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_CASE:
    comment(node->token, "ND_CASE %d", node->label_index);
    writeline(".Lcase%d:", node->label_index);
    gen(node->body);
    assert(shift_before == rsp_shift);
    return;
  case ND_DEFAULT:
    writeline(".Ldefault%d:", node->label_index);
    comment(node->token, "ND_DEFAULT %d", node->label_index);
    gen(node->body);
    assert(shift_before == rsp_shift);
    return;
  case ND_RETURN:
    comment(node->token, "ND_RETURN");
    if (node->lhs) {
      gen(node->lhs);
      if (pass_on_memory(node->lhs->type)) {
        assert(node->return_buffer_address);
        writeline("  mov r10, [rbp-%d]", node->return_buffer_address->offset);
        push("r10");
        store(node->lhs->type);
        writeline("  mov rax, [rbp-%d]", node->return_buffer_address->offset);
      }
    }
    assert(shift_before == rsp_shift);
    epilogue();
    return;
  case ND_CONTINUE:
    comment(node->token, "ND_CONTINUE");
    writeline("  jmp .Lcontinue%d", node->label_index);
    assert(shift_before == rsp_shift);
    return;
  case ND_BREAK:
    comment(node->token, "ND_BREAK");
    writeline("  jmp .Lend%d", node->label_index);
    assert(shift_before == rsp_shift);
    return;
  case ND_GOTO:
    comment(node->token, "ND_GOTO %.*s", node->token->str->len,
            node->token->str->str);
    writeline("  jmp .Lgoto%d", node->label_index);
    assert(shift_before == rsp_shift);
    return;
  case ND_LABEL:
    comment(node->token, "ND_LABEL %.*s", node->token->str->len,
            node->token->str->str);
    writeline(".Lgoto%d:", node->label_index);
    if (node->body)
      gen(node->body);
    assert(shift_before == rsp_shift);
    return;
  case ND_NUM:
    comment(node->token, "ND_NUM %s", type_text(node->type->kind));
    gen_number(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_VAR:
    comment(node->token, "ND_VAR");
    gen_address(node);
    if (node->variable->kind != OBJ_FUNC)
      load(node->type);
    assert(shift_before == rsp_shift);
    return;
  case ND_ASSIGN:
    comment(node->token, "ND_ASSIGN");
    gen_address(node->lhs);
    push("rax");
    gen(node->rhs);
    store(node->type);
    assert(shift_before == rsp_shift);
    return;
  case ND_COND:
    comment(node->token, "ND_COND %d", node->label_index);
    assert(node->condition->type->kind == TYPE_BOOL);
    gen(node->condition);
    writeline("  cmp al, 0");
    writeline("  je .Lcond_rhs%d", node->label_index);
    gen(node->lhs);
    writeline("  jmp .Lend%d", node->label_index);
    writeline(".Lcond_rhs%d:", node->label_index);
    gen(node->rhs);
    writeline(".Lend%d:", node->label_index);
    assert(shift_before == rsp_shift);
    return;
  case ND_ADDR:
    comment(node->token, "ND_ADDR");
    gen_address(node->lhs);
    assert(shift_before == rsp_shift);
    return;
  case ND_DEREF:
    comment(node->token, "ND_DEREF");
    gen(node->lhs);
    load(node->type);
    assert(shift_before == rsp_shift);
    return;
  case ND_MEMBER:
    comment(node->token, "ND_MEMBER");
    gen_address(node);
    load(node->type);
    assert(shift_before == rsp_shift);
    return;
  case ND_ADD:
    comment(node->token, "ND_ADD");
    gen_numerical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_SUB:
    comment(node->token, "ND_SUB");
    gen_numerical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_MUL:
    comment(node->token, "ND_MUL");
    gen_numerical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_DIV:
    comment(node->token, "ND_DIV");
    gen_numerical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_MOD:
    comment(node->token, "ND_MOD");
    gen_numerical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LOGNOT:
    comment(node->token, "ND_LOGNOT");
    gen_logical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LOGOR:
    comment(node->token, "ND_LOGOR %d", node->label_index);
    gen_logical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LOGAND:
    comment(node->token, "ND_LOGAND %d", node->label_index);
    gen_logical_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LSHIFT:
    comment(node->token, "ND_LSHIFT");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_RSHIFT:
    comment(node->token, "ND_RSHIFT");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_BITXOR:
    comment(node->token, "ND_BITXOR");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_BITOR:
    comment(node->token, "ND_BITOR");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_BITAND:
    comment(node->token, "ND_BITAND");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_BITNOT:
    comment(node->token, "ND_BITNOT");
    gen_bit_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_EQ:
    comment(node->token, "ND_EQ");
    gen_comparison_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_NE:
    comment(node->token, "ND_NE");
    gen_comparison_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LT:
    comment(node->token, "ND_LT");
    gen_comparison_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_LE:
    comment(node->token, "ND_LE");
    gen_comparison_operator(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_CAST:
    comment(node->token, "ND_CAST %s --> %s", type_text(node->lhs->type->kind),
            type_text(node->type->kind));
    gen_cast(node);
    assert(shift_before == rsp_shift);
    return;
  case ND_COMMA:
    comment(node->token, "ND_COMMA");
    gen(node->lhs);
    gen(node->rhs);
    assert(shift_before == rsp_shift);
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
