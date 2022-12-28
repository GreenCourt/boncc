#include "boncc.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

static int label = 0;
static const char *reg_args[6] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_lval(Node *node) {
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
    printf("  mov %s, rax\n", reg_args[i]);
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
  for (int i = 0; i < node->nparams; ++i)
    printf("  push %s\n", reg_args[i]);

  if (node->locals->size > node->nparams) {
    // stack memory for local variables
    int ofs = (node->locals->size - node->nparams) * 8;
    printf("  sub rsp, %d\n", ofs);
  }

  gen(node->body);
  printf("  pop rax\n"); // return value

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
}

void multiply_node(Node** pnode, int coef) {
  Node *num = calloc(1, sizeof(Node));
  num->kind = ND_NUM;
  num->val = coef;
  Node *mul = calloc(1, sizeof(Node));
  mul->kind = ND_MUL;
  mul->lhs = *pnode;
  mul->rhs = num;
  *pnode = mul;
}

void gen_add(Node* node) {
  type_detection(node);
  Type *lty = type_detection(node->lhs);
  Type *rty = type_detection(node->rhs);

  if(lty->ptr)
    multiply_node(&node->rhs, pointer_destination_size(lty));
  else if(rty->ptr)
    multiply_node(&node->lhs, pointer_destination_size(rty));

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  add rax, rdi\n");
  printf("  push rax\n");
}

void gen_sub(Node* node) {
  type_detection(node);
  Type *lty = type_detection(node->lhs);
  Type *rty = type_detection(node->rhs);

  if(lty->ptr)
    multiply_node(&node->rhs, pointer_destination_size(lty));
  else if(rty->ptr)
    multiply_node(&node->lhs, pointer_destination_size(rty));

  gen(node->lhs);
  gen(node->rhs);
  printf("  pop rdi\n");
  printf("  pop rax\n");
  printf("  sub rax, rdi\n");
  printf("  push rax\n");
}

void gen(Node *node) {
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
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ASSIGN:
    gen_lval(node->lhs);
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  pop rax\n");
    printf("  mov [rax], rdi\n");
    printf("  push rdi\n");
    return;
  case ND_ADDR:
    gen_lval(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  mov rax, [rax]\n");
    printf("  push rax\n");
    return;
  case ND_ADD:
    gen_add(node);
    return;
  case ND_SUB:
    gen_sub(node);
    return;
  default:
    break;
  }

  gen(node->lhs);
  gen(node->rhs);

  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
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
