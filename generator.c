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

// node.c -->
bool is_constant_number(Node *node);
Variable *is_const_var_addr(Node *node);
int eval(Node *node);
// <-- node.c

void gen(Node *node);

void writeline(char *fmt, ...) {
#ifdef BONCC
  typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
  } __va_list;
  typedef __va_list va_list[1];
  va_list ap;
  *ap = *(__va_list *)__hidden_va_area__;
#else
  va_list ap;
  va_start(ap, fmt);
#endif
  vfprintf(ostream, fmt, ap);
  fprintf(ostream, "\n");
}

void comment(Token *tok, char *fmt, ...) {
#ifdef BONCC
  typedef struct {
    unsigned int gp_offset;
    unsigned int fp_offset;
    void *overflow_arg_area;
    void *reg_save_area;
  } __va_list;
  typedef __va_list va_list[1];
  va_list ap;
  *ap = *(__va_list *)__hidden_va_area__;
#else
  va_list ap;
  va_start(ap, fmt);
#endif
  fprintf(ostream, "  # ");
  vfprintf(ostream, fmt, ap);
  if (tok)
    fprintf(ostream, "    %s:%d:%d", tok->pos.file_name, tok->pos.line_number, tok->pos.column_number);
  fprintf(ostream, "\n");
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
        comment(NULL, "gen_address ND_VAR static-local: %.*s", node->variable->ident->len, node->variable->ident->name);
        writeline("  lea rax, %.*s[rip]", node->variable->internal_ident->len, node->variable->internal_ident->name);
      } else if (node->variable->is_extern) {
        comment(NULL, "gen_address ND_VAR extern-local: %.*s", node->variable->ident->len, node->variable->ident->name);
        writeline("  lea rax, %.*s[rip]", node->variable->ident->len, node->variable->ident->name);
      } else {
        comment(NULL, "gen_address ND_VAR local: %.*s", node->variable->ident->len, node->variable->ident->name);
        writeline("  lea rax, [rbp-%d]", node->variable->offset);
      }
    } else if (node->variable->kind == OBJ_GVAR) {
      comment(NULL, "gen_address ND_VAR global: %.*s", node->variable->ident->len, node->variable->ident->name);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len, node->variable->ident->name);
    } else if (node->variable->kind == OBJ_STRLIT) {
      comment(NULL, "gen_address ND_VAR strlit: \"%s\"", node->variable->string_literal);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len, node->variable->ident->name);
    } else if (node->variable->kind == OBJ_FUNC) {
      comment(NULL, "gen_address ND_VAR function: %.*s", node->variable->ident->len, node->variable->ident->name);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len, node->variable->ident->name);
    } else
      assert(false);
    return;
  case ND_MEMBER:
    comment(NULL, "gen_address ND_MEMBER %.*s", node->member->ident->len, node->member->ident->name);
    gen_address(node->lhs);
    writeline("  add rax, %d", node->member->offset);
    return;
  default:
    error(&node->token->pos, "failed to generate address");
  }
}

void load(Type *type) {
  // src  : rax
  // dest : the address from stack top
  comment(NULL, "load %s", type_text(type->kind), type->size);

  if (type->kind == TYPE_ARRAY || is_struct_union(type))
    return; // nothing todo
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
  // src  : rax
  // dest : the address popped from stack
  comment(NULL, "store %s", type_text(type->kind));

  writeline("  pop rdi");

  if (is_struct_union(type)) {
    // src  : the struct/union that rax is pointing to
    // dest : the struct/union that the address popped from stack is pointing to
    for (int i = 0; i < type->size; i++) {
      // copy each bytes like memcpy
      writeline("  mov r10b, [rax+%d]", i);
      writeline("  mov [rdi+%d], r10b", i);
    }
    return;
  }

  if (type->size == 1)
    writeline("  mov [rdi], al");
  else if (type->size == 2)
    writeline("  mov [rdi], ax");
  else if (type->size == 4)
    writeline("  mov [rdi], eax");
  else if (type->size == 8)
    writeline("  mov [rdi], rax");
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
    writeline("  cmp rax, %ld", c->condition->val);
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
  int sz = node->args->size;
  if (sz > 6)
    error(NULL, "maximum number of argument is currently 6");

  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->args, i);
    gen(d);
    writeline("  push rax");
  }
  for (int i = sz - 1; i >= 0; --i) {
    writeline("  pop %s", reg_args8[i]);
  }

  // align RSP to 16bytes (ABI requirements)
  comment(NULL, "RSP alignment for call");
  static int l = -1;
  l++;
  writeline("  mov rax, rsp");
  writeline("  and rax, 15"); // rax % 16 == rax & 0xF
  writeline("  jnz .Lcall%d", l);
  if (node->lhs->kind == ND_VAR && node->lhs->variable->kind == OBJ_FUNC) {
    writeline("  mov al, 0");
    writeline("  call %.*s", node->lhs->variable->ident->len, node->lhs->variable->ident->name);
  } else {
    gen(node->lhs);
    writeline("  mov r10, rax");
    writeline("  mov al, 0");
    writeline("  call r10");
  }
  writeline("  jmp .Lend_call%d", l);
  writeline(".Lcall%d:", l);
  writeline("  sub rsp, 8");
  if (node->lhs->kind == ND_VAR && node->lhs->variable->kind == OBJ_FUNC) {
    writeline("  mov al, 0");
    writeline("  call %.*s", node->lhs->variable->ident->len, node->lhs->variable->ident->name);
  } else {
    gen(node->lhs);
    writeline("  mov r10, rax");
    writeline("  mov al, 0");
    writeline("  call r10");
  }
  writeline("  add rsp, 8");
  writeline(".Lend_call%d:", l);
}

void gen_global_init(VariableInit *init, Type *type) {
  if (init == NULL) {
    writeline("  .zero %d", type->size);
    return;
  }

  if (type->kind == TYPE_ARRAY) {
    if (init->expr) {
      if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == OBJ_STRLIT) {
        // initilize the array as a string
        Variable *lit = init->expr->variable;
        if (type->array_size != lit->type->array_size)
          error(&init->expr->token->pos, "miss-match between array-size and string-length");
        writeline("  .ascii \"%s\\0\"", lit->string_literal);
      } else {
        // When init->expr is given for an array, only the first element will be initialized.
        Type *ty = type;
        while (ty->kind == TYPE_ARRAY)
          ty = ty->base;
        gen_global_init(init, ty);
        writeline("  .zero %d", type->size - ty->size);
      }
    } else if (init->vec) {
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
      } else {
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
    } else
      assert(false);
  } else if (type->kind == TYPE_PTR) {
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == OBJ_STRLIT) {
      // initilize the pointer to a string-literal
      writeline("  .quad %.*s", init->expr->variable->ident->len, init->expr->variable->ident->name);
      return;
    } else if (init->expr->kind == ND_ADD) {
      Variable *left_addr = is_const_var_addr(init->expr->lhs);
      Variable *right_addr = is_const_var_addr(init->expr->rhs);

      if (left_addr && is_constant_number(init->expr->rhs)) {
        writeline("  .quad %.*s+%d",
                  left_addr->ident->len,
                  left_addr->ident->name,
                  eval(init->expr->rhs));
      } else if (right_addr && is_constant_number(init->expr->lhs)) {
        writeline("  .quad %.*s+%d",
                  right_addr->ident->len,
                  right_addr->ident->name,
                  eval(init->expr->lhs));
      } else {
        error(NULL, "unsupported initalization of a global pointer.");
      }
    } else if (is_const_var_addr(init->expr)) {
      Variable *var = is_const_var_addr(init->expr);
      writeline("  .quad %.*s",
                var->ident->len,
                var->ident->name);
    } else if (is_constant_number(init->expr)) {
      writeline("  .quad %d", eval(init->expr));
    } else {
      error(NULL, "unsupported initalization of a global pointer.");
    }
  } else if (is_integer(type)) {
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    int val = eval(init->expr);
    switch (type->kind) {
    case TYPE_LONG:
    case TYPE_ULONG:
      writeline("  .quad %d", val);
      break;
    case TYPE_INT:
    case TYPE_UINT:
    case TYPE_ENUM:
      writeline("  .long %d", val);
      break;
    case TYPE_SHORT:
    case TYPE_USHORT:
      writeline("  .value %d", val);
      break;
    case TYPE_CHAR:
    case TYPE_BOOL:
      writeline("  .byte %d", val);
      break;
    default:
      assert(false);
    }
  } else
    assert(false);
}

void gen_func(Function *func) {
  if (!func->body)
    return; // declared but not defined
  comment(func->token, "function %.*s", func->ident->len, func->ident->name);
  if (func->is_static)
    writeline("  .local %.*s", func->ident->len, func->ident->name);
  else
    writeline("  .globl %.*s", func->ident->len, func->ident->name);
  writeline("%.*s:", func->ident->len, func->ident->name);

  // prologue
  writeline("  push rbp");
  writeline("  mov rbp, rsp");

  // move args to stack
  comment(NULL, "function arguments to stack");
  for (int i = 0; i < func->params->size; ++i) {
    Variable *v = *(Variable **)vector_get(func->params, i);
    if (v->type->size == 1)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args1[i]);
    else if (v->type->size == 2)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args2[i]);
    else if (v->type->size == 4)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args4[i]);
    else
      writeline("  mov [rbp-%d], %s", v->offset, reg_args8[i]);
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
    comment(NULL, "va_list: %.*s", func->ident->len, func->ident->name);
    writeline("  mov dword ptr [rbp-%d], %d", ofs, num_gp * 8); // gp_offset
    writeline("  mov dword ptr [rbp-%d], 0", ofs - 4);          // fp_offset (TODO)
    writeline("  movq [rbp-%d], 0", ofs - 8);                   // overflow_arg_area (TODO)
    writeline("  movq [rbp-%d], rbp", ofs - 16);                // reg_save_area = rbp
    writeline("  subq [rbp-%d], %d", ofs - 16, ofs - 24);       // reg_save_area -= ofs - 24

    // register save area
    comment(NULL, "register save area: %.*s", func->ident->len, func->ident->name);
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
  Type *from = node->lhs->type->kind == TYPE_ARRAY ? pointer_type(node->lhs->type->base) : node->lhs->type;
  Type *to = node->type;

  gen(node->lhs);
  if (from->kind == to->kind)
    return;

  if (from->kind == TYPE_INT && to->kind == TYPE_ENUM)
    return;

  if (from->kind == TYPE_ENUM && to->kind == TYPE_INT)
    return;

  if (from->kind == TYPE_BOOL)
    return;

  if (to->kind == TYPE_BOOL) {
    writeline("  cmp rax, 0");
    writeline("  setne al");
    writeline("  movzb rax, al");
    return;
  }

  if (to->kind == TYPE_VOID)
    return;

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
    default:
      break;
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
      break;
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
      break;
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
      return;
    default:
      break;
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
      break;
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
      break;
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
      break;
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
      break;
    }
  }
  assert(false);
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_CALL:
    if (node->lhs->kind == ND_VAR)
      comment(node->token, "ND_CALL %.*s", node->lhs->variable->ident->len, node->lhs->variable->ident->name);
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
  case ND_NUM:
    comment(node->token, "ND_NUM");
    writeline("  mov rax, %lld", node->val);
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
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  add rax, rdi");
    return;
  case ND_SUB:
    comment(node->token, "ND_SUB");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  sub rax, rdi");
    return;
  case ND_MUL:
    comment(node->token, "ND_MUL");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  imul rax, rdi");
    return;
  case ND_DIV:
  case ND_MOD:
    comment(node->token, node->kind == ND_DIV ? "ND_DIV" : "ND_MOD");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    if (is_unsigned(node->type)) {
      writeline("  mov rdx, 0");
      writeline("  div rdi");
    } else {
      writeline("  cqo");
      writeline("  idiv rdi");
    }
    if (node->kind == ND_MOD)
      writeline("  mov rax, rdx");
    return;
  case ND_LOGNOT:
    comment(node->token, "ND_LOGNOT");
    gen(node->lhs);
    writeline("  cmp rax, 0");
    writeline("  sete al");
    writeline("  movzx rax, al");
    return;
  case ND_LOGOR:
    comment(node->token, "ND_LOGOR");
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
  case ND_LOGAND:
    comment(node->token, "ND_LOGAND %d", node->label_index);
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
  case ND_LSHIFT:
    comment(node->token, "ND_LSHIFT");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rcx, rax");
    writeline("  pop rax");
    writeline("  shl rax, cl");
    return;
  case ND_RSHIFT:
    comment(node->token, "ND_RSHIFT");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rcx, rax");
    writeline("  pop rax");
    writeline("  s%cr rax, cl", is_unsigned(node->lhs->type) ? 'h' : 'a');
    return;
  case ND_BITXOR:
    comment(node->token, "ND_BITXOR");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  xor rax, rdi");
    return;
  case ND_BITOR:
    comment(node->token, "ND_BITOR");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  or rax, rdi");
    return;
  case ND_BITAND:
    comment(node->token, "ND_BITAND");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  and rax, rdi");
    return;
  case ND_BITNOT:
    comment(node->token, "ND_BITNOT");
    gen(node->lhs);
    writeline("  not rax");
    return;
  case ND_EQ:
    comment(node->token, "ND_EQ");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  cmp rax, rdi");
    writeline("  sete al");
    writeline("  movzb rax, al");
    return;
  case ND_NE:
    comment(node->token, "ND_NE");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  cmp rax, rdi");
    writeline("  setne al");
    writeline("  movzb rax, al");
    return;
  case ND_LT:
    comment(node->token, "ND_LT");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cmp rax, rdi");
    writeline("  set%c al", is_unsigned(node->lhs->type) ? 'b' : 'l');
    writeline("  movzb rax, al");
    return;
  case ND_LE:
    comment(node->token, "ND_LE");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cmp rax, rdi");
    writeline("  set%ce al", is_unsigned(node->lhs->type) ? 'b' : 'l');
    writeline("  movzb rax, al");
    return;
  case ND_CAST:
    comment(node->token, "ND_CAST %s --> %s", type_text(node->lhs->type->kind), type_text(node->type->kind));
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
  writeline(".intel_syntax noprefix");

  // string literals
  for (int i = 0; i < strings->size; i++) {
    Variable *v = map_geti(strings, i);
    writeline("%.*s:", v->ident->len, v->ident->name);
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
      writeline(".local %.*s", v->ident->len, v->ident->name);
    else
      writeline(".globl %.*s", v->ident->len, v->ident->name);
    writeline("%.*s:", v->ident->len, v->ident->name);
    gen_global_init(v->init, v->type);
  }

  // static local variables
  for (int i = 0; i < static_local_variables->size; i++) {
    Variable *v = *(Variable **)vector_get(static_local_variables, i);
    assert(v->kind == OBJ_LVAR);
    assert(v->is_static);
    assert(v->internal_ident);
    writeline(".data");
    writeline(".local %.*s", v->internal_ident->len, v->internal_ident->name);
    writeline("%.*s:", v->internal_ident->len, v->internal_ident->name);
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
  ostream = NULL;
}
