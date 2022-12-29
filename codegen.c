#include "boncc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int label = 0;
static const char *reg_args1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *reg_args2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *reg_args4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *reg_args8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
  // push address to stack top
  if (node->kind == ND_DEREF) {
    gen(node->lhs);
  } else if (node->kind == ND_LVAR) {
    printf("  mov rax, rbp\n");
    printf("  sub rax, %d\n", node->lvar->offset);
    printf("  push rax\n");
  } else {
    error("left-value must be a variable");
  }
}

void load(Type *type) {
  // pop address from stack
  // push the value of address to stack
  printf("  pop rax\n");
  if (size_of(type) == 1)
    printf("  movsx rax, byte ptr [rax]\n");
  else if (size_of(type) == 2)
    printf("  movsx rax, word ptr [rax]\n");
  else if (size_of(type) == 4)
    printf("  movsxd rax, dword ptr [rax]\n");
  else if (size_of(type) == 8)
    printf("  mov rax, [rax]\n");
  printf("  push rax\n");
}

void store(Type *type) {
  // pop address from stack
  // store rax value to the address
  printf("  pop rdi\n");
  if (size_of(type) == 1)
    printf("  mov [rdi], al\n");
  else if (size_of(type) == 2)
    printf("  mov [rdi], ax\n");
  else if (size_of(type) == 4)
    printf("  mov [rdi], eax\n");
  else
    printf("  mov [rdi], rax\n");
}

void gen_if(Node *node) {
  gen(node->condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");

  int l = label++;

  if (node->else_) {
    printf("  je .Lelse%d\n", l);
    gen(node->body);
    printf("  jmp .Lend%d\n", l);
    printf(".Lelse%d:\n", l);
    gen(node->else_);
    printf(".Lend%d:\n", l);
  } else {
    printf("  je .Lend%d\n", l);
    gen(node->body);
    printf(".Lend%d:\n", l);
  }
}

void gen_while(Node *node) {
  int l = label++;
  printf(".Lwhile%d:\n", l);

  gen(node->condition);
  printf("  pop rax\n");
  printf("  cmp rax, 0\n");
  printf("  je .Lend%d\n", l);

  gen(node->body);
  printf("  jmp .Lwhile%d\n", l);
  printf(".Lend%d:\n", l);
}

void gen_for(Node *node) {
  int l = label++;

  if (node->init)
    gen(node->init);

  printf(".Lfor%d:\n", l);
  if (node->condition) {
    gen(node->condition);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", l);
  }
  gen(node->body);
  if (node->update)
    gen(node->update);
  printf("  jmp .Lfor%d\n", l);
  printf(".Lend%d:\n", l);
}

void gen_block(Node *node) {
  int sz = node->blk_stmts->size;
  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->blk_stmts, i);
    gen(d);
    if (i != sz - 1)
      printf("  pop rax\n");
  }
}

void gen_call(Node *node) {
  int sz = node->args->size;
  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->args, i);
    gen(d);
  }
  for (int i = sz - 1; i >= 0; --i) {
    printf("  pop rax\n");
    printf("  mov %s, rax\n", reg_args8[i]);
  }

  // align RSP to 16bytes (ABI requirements)
  int l = label++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n"); // rax % 16 == rax & 0xF
  printf("  jnz .Lcall%d\n", l);
  printf("  call %.*s\n", node->len, node->name);
  printf("  jmp .Lend%d\n", l);
  printf(".Lcall%d:\n", l);
  printf("  sub rsp, 8\n");
  printf("  call %.*s\n", node->len, node->name);
  printf("  add rsp, 8\n");
  printf(".Lend%d:\n", l);

  // push return value to stack
  printf("  push rax\n");
}

void gen_func(Node *node) {
  if (strncmp(node->name, "main", 4) == 0) {
    printf(".globl main\n");
  }
  printf("%.*s:\n", node->len, node->name);

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");

  // push args to stack
  for (int i = 0; i < node->nparams; ++i) {
    LVar *lv = *(LVar **)vector_get(node->locals, i);
    if (size_of(lv->type) == 1)
      printf("  mov [rbp-%d], %s\n", lv->offset, reg_args1[i]);
    else if (size_of(lv->type) == 2)
      printf("  mov [rbp-%d], %s\n", lv->offset, reg_args2[i]);
    else if (size_of(lv->type) == 4)
      printf("  mov [rbp-%d], %s\n", lv->offset, reg_args4[i]);
    else
      printf("  mov [rbp-%d], %s\n", lv->offset, reg_args8[i]);
  }

  if (node->locals->size) {
    LVar *last = *(LVar **)vector_last(node->locals);
    int ofs = last->offset;
    if (ofs % 8)
      ofs += 8 - ofs % 8; // align by 8
    printf("  sub rsp, %d\n", ofs);
  }

  gen(node->body);
  printf("  pop rax\n"); // return value

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void gen(Node *node) {
  get_type(node);
  switch (node->kind) {
  case ND_FUNC:
    gen_func(node);
    return;
  case ND_CALL:
    gen_call(node);
    return;
  case ND_BLOCK:
    gen_block(node);
    return;
  case ND_IF:
    gen_if(node);
    return;
  case ND_WHILE:
    gen_while(node);
    return;
  case ND_FOR:
    gen_for(node);
    return;
  case ND_RETURN:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    printf("  push %d\n", node->val);
    return;
  case ND_LVAR:
    gen_lval(node);
    if (get_type(node)->kind != TYPE_ARRAY)
      load(get_type(node));
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rax\n");
    store(get_type(node->lhs));
    printf("  push rax\n");
    return;
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    load(get_type(node->lhs));
    return;
  default:
    break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
  case ND_ADD:
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    break;
  }
  printf("  push rax\n");
}
