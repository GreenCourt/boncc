#include "boncc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static FILE *ostream;
static int label = 0;
static const char *reg_args1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *reg_args2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *reg_args4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *reg_args8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// node.c -->
bool is_constant_number(Node *node);
int eval(Node *node);
// <-- node.c

void gen(Node *node);

void writeline(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(ostream, fmt, ap);
  fprintf(ostream, "\n");
}

void comment(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
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
    if (node->variable->kind == VK_LOCAL) {
      if (node->variable->is_static) {
        comment(NULL, "gen_address ND_VAR static-local: %.*s", node->variable->ident->len, node->variable->ident->name);
        writeline("  lea rax, %.*s[rip]", node->variable->internal_ident->len, node->variable->internal_ident->name);
      } else {
        comment(NULL, "gen_address ND_VAR local: %.*s", node->variable->ident->len, node->variable->ident->name);
        writeline("  lea rax, [rbp-%d]", node->variable->offset);
      }
    } else if (node->variable->kind == VK_GLOBAL) {
      comment(NULL, "gen_address ND_VAR global: %.*s", node->variable->ident->len, node->variable->ident->name);
      writeline("  lea rax, %.*s[rip]", node->variable->ident->len, node->variable->ident->name);
    } else if (node->variable->kind == VK_STRLIT) {
      comment(NULL, "gen_address ND_VAR strlit: \"%s\"", node->variable->string_literal);
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
    error_at(&node->token->pos, "left-value must be a variable");
  }
}

void load(Type *type) {
  // src  : rax
  // dest : the address from stack top
  comment(NULL, "load %s", type_text(type->kind), type->size);

  if (type->kind == TYPE_ARRAY)
    return; // nothing todo
  if (type->size == 1)
    writeline("  movsx rax, byte ptr [rax]");
  else if (type->size == 2)
    writeline("  movsx rax, word ptr [rax]");
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

  int l = label++;

  if (node->else_) {
    writeline("  je .Lelse%d", l);
    gen(node->body);
    writeline("  jmp .Lend%d", l);
    writeline(".Lelse%d:", l);
    gen(node->else_);
    writeline(".Lend%d:", l);
  } else {
    writeline("  je .Lend%d", l);
    gen(node->body);
    writeline(".Lend%d:", l);
  }
}

void gen_while(Node *node) {
  int l = label++;
  writeline(".Lwhile%d:", l);

  gen(node->condition);
  writeline("  cmp rax, 0");
  writeline("  je .Lend%d", l);

  gen(node->body);
  writeline("  jmp .Lwhile%d", l);
  writeline(".Lend%d:", l);
}

void gen_for(Node *node) {
  int l = label++;

  if (node->init)
    gen(node->init);

  writeline(".Lfor%d:", l);
  if (node->condition) {
    gen(node->condition);
    writeline("  cmp rax, 0");
    writeline("  je .Lend%d", l);
  }
  gen(node->body);
  if (node->update)
    gen(node->update);
  writeline("  jmp .Lfor%d", l);
  writeline(".Lend%d:", l);
}

void gen_block(Node *node) {
  int sz = node->blk_stmts->size;
  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->blk_stmts, i);
    gen(d);
  }
}

void gen_call(Node *node) {
  int sz = node->args->size;
  if (sz > 6)
    error("maximum number of argument is currently 6");

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
  int l = label++;
  writeline("  mov rax, rsp");
  writeline("  and rax, 15"); // rax % 16 == rax & 0xF
  writeline("  jnz .Lcall%d", l);
  writeline("  mov al, 0");
  writeline("  call %.*s", node->func->ident->len, node->func->ident->name);
  writeline("  jmp .Lend%d", l);
  writeline(".Lcall%d:", l);
  writeline("  sub rsp, 8");
  writeline("  mov al, 0");
  writeline("  call %.*s", node->func->ident->len, node->func->ident->name);
  writeline("  add rsp, 8");
  writeline(".Lend%d:", l);
}

void gen_global_init(VariableInit *init, Type *type) {
  if (init == NULL) {
    writeline("  .zero %d", type->size);
    return;
  }

  if (type->kind == TYPE_ARRAY) {
    if (init->expr) {
      if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
        // initilize the array as a string
        Variable *lit = init->expr->variable;
        if (type->array_size != lit->type->array_size)
          error_at(&init->expr->token->pos, "miss-match between array-size and string-length");
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
    if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
      // initilize the pointer to a string-literal
      writeline("  .quad %.*s", init->expr->variable->ident->len, init->expr->variable->ident->name);
      return;
    } else if (init->expr->kind == ND_ADD) {
      bool left_is_addr = init->expr->lhs->kind == ND_ADDR && init->expr->lhs->lhs->kind == ND_VAR && init->expr->lhs->lhs->variable->kind == VK_GLOBAL;
      bool right_is_addr = init->expr->rhs->kind == ND_ADDR && init->expr->rhs->lhs->kind == ND_VAR && init->expr->rhs->lhs->variable->kind == VK_GLOBAL;

      if (left_is_addr && is_constant_number(init->expr->rhs)) {
        writeline("  .quad %.*s+%d",
                  init->expr->lhs->lhs->variable->ident->len,
                  init->expr->lhs->lhs->variable->ident->name,
                  eval(init->expr->rhs));
      } else if (right_is_addr && is_constant_number(init->expr->lhs)) {
        writeline("  .quad %.*s+%d",
                  init->expr->rhs->lhs->variable->ident->len,
                  init->expr->rhs->lhs->variable->ident->name,
                  eval(init->expr->lhs));
      } else {
        error("unsupported initalization of a global pointer.");
      }
    } else if (init->expr->kind == ND_ADDR && init->expr->lhs->kind == ND_VAR && init->expr->lhs->variable->kind == VK_GLOBAL) {
      writeline("  .quad %.*s",
                init->expr->lhs->variable->ident->len,
                init->expr->lhs->variable->ident->name);
    } else if (is_constant_number(init->expr)) {
      writeline("  .quad %d", eval(init->expr));
    } else {
      error("unsupported initalization of a global pointer.");
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
      writeline("  .quad %d", val);
      break;
    case TYPE_INT:
    case TYPE_ENUM:
      writeline("  .long %d", val);
      break;
    case TYPE_SHORT:
      writeline("  .value %d", val);
      break;
    case TYPE_CHAR:
      writeline("  .byte %d", val);
      break;
    default:
      assert(false);
    }
  } else
    assert(false);
}

void gen_func(Node *node) {
  writeline("  .globl %.*s", node->func->ident->len, node->func->ident->name);
  writeline("%.*s:", node->func->ident->len, node->func->ident->name);

  // prologue
  writeline("  push rbp");
  writeline("  mov rbp, rsp");

  // move args to stack
  comment(NULL, "function arguments to stack");
  for (int i = 0; i < node->func->params->size; ++i) {
    Variable *v = *(Variable **)vector_get(node->func->params, i);
    if (v->type->size == 1)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args1[i]);
    else if (v->type->size == 2)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args2[i]);
    else if (v->type->size == 4)
      writeline("  mov [rbp-%d], %s", v->offset, reg_args4[i]);
    else
      writeline("  mov [rbp-%d], %s", v->offset, reg_args8[i]);
  }

  if (node->func->offset) {
    int ofs = node->func->offset;
    if (ofs % 8)
      ofs += 8 - ofs % 8; // align by 8
    writeline("  sub rsp, %d", ofs);
  }

  gen(node->body);

  // epilogue
  writeline("  mov rsp, rbp");
  writeline("  pop rbp");
  writeline("  ret");
}

void gen(Node *node) {
  switch (node->kind) {
  case ND_FUNC:
    comment(node->token, "ND_FUNC %.*s", node->func->ident->len, node->func->ident->name);
    gen_func(node);
    return;
  case ND_CALL:
    comment(node->token, "ND_CALL %.*s", node->func->ident->len, node->func->ident->name);
    gen_call(node);
    return;
  case ND_BLOCK:
    comment(node->token, "ND_BLOCK");
    gen_block(node);
    return;
  case ND_IF:
    comment(node->token, "ND_IF");
    gen_if(node);
    return;
  case ND_WHILE:
    comment(node->token, "ND_WHILE");
    gen_while(node);
    return;
  case ND_FOR:
    comment(node->token, "ND_FOR");
    gen_for(node);
    return;
  case ND_RETURN:
    comment(node->token, "ND_RETURN");
    if (node->lhs)
      gen(node->lhs);
    writeline("  mov rsp, rbp");
    writeline("  pop rbp");
    writeline("  ret");
    return;
  case ND_NUM:
    comment(node->token, "ND_NUM");
    writeline("  mov rax, %lld", node->val);
    return;
  case ND_VAR:
    comment(node->token, "ND_VAR");
    gen_address(node);
    load(node->type);
    return;
  case ND_ASSIGN:
    comment(node->token, "ND_ASSIGN");
    gen_address(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    store(node->type);
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
  default:
    break;
  }

  switch (node->kind) {
  case ND_ADD:
    comment(node->token, "ND_ADD");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  add rax, rdi");
    break;
  case ND_SUB:
    comment(node->token, "ND_SUB");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  sub rax, rdi");
    break;
  case ND_MUL:
    comment(node->token, "ND_MUL");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  imul rax, rdi");
    break;
  case ND_DIV:
    comment(node->token, "ND_DIV");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cqo");
    writeline("  idiv rdi");
    break;
  case ND_EQ:
    comment(node->token, "ND_EQ");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  cmp rax, rdi");
    writeline("  sete al");
    writeline("  movzb rax, al");
    break;
  case ND_NE:
    comment(node->token, "ND_NE");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  pop rdi");
    writeline("  cmp rax, rdi");
    writeline("  setne al");
    writeline("  movzb rax, al");
    break;
  case ND_LT:
    comment(node->token, "ND_LT");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cmp rax, rdi");
    writeline("  setl al");
    writeline("  movzb rax, al");
    break;
  case ND_LE:
    comment(node->token, "ND_LE");
    gen(node->lhs);
    writeline("  push rax");
    gen(node->rhs);
    writeline("  mov rdi, rax");
    writeline("  pop rax");
    writeline("  cmp rax, rdi");
    writeline("  setle al");
    writeline("  movzb rax, al");
    break;
  default:
    break;
  }
}

void gen_toplevel(FILE *output_stream) {
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
  for (int i = 0; i < global_scope->variables->size; i++) {
    Variable *v = map_geti(global_scope->variables, i);
    writeline(".data");
    writeline(".globl %.*s", v->ident->len, v->ident->name);
    writeline("%.*s:", v->ident->len, v->ident->name);
    gen_global_init(v->init, v->type);
  }

  // static local variables
  for (int i = 0; i < static_local_variables->size; i++) {
    Variable *v = *(Variable **)vector_get(static_local_variables, i);
    assert(v->kind == VK_LOCAL);
    assert(v->is_static);
    assert(v->internal_ident);
    writeline(".data");
    writeline(".local %.*s", v->internal_ident->len, v->internal_ident->name);
    writeline("%.*s:", v->internal_ident->len, v->internal_ident->name);
    gen_global_init(v->init, v->type);
  }

  // functions
  writeline(".text");
  for (int i = 0; i < functions->size; i++) {
    Node *f = map_geti(functions, i);
    gen(f);
  }
  ostream = NULL;
}
