#include "boncc.h"
#include <stdio.h>

static int label = 0;

void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("left-value of assignment operation must be a variable");
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
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

void gen(Node *node) {
  switch (node->kind) {
  case ND_IF:
    gen_if(node);
    return;
  case ND_WHILE:
    gen_while(node);
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
