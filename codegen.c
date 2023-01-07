#include "boncc.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int label = 0;
static const char *reg_args1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *reg_args2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *reg_args4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *reg_args8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

void gen_left_value(Node *node) {
  // push address to stack top
  if (node->kind == ND_DEREF) {
    gen(node->lhs);
  } else if (node->kind == ND_VAR) {
    if (node->variable->kind == VK_LOCAL) {
      printf("  lea rax, [rbp-%d]\n", node->variable->offset);
      printf("  push rax\n");
    } else if (node->variable->kind == VK_GLOBAL || node->variable->kind == VK_STRLIT) {
      printf("  lea rax, %.*s[rip]\n", node->variable->name_length, node->variable->name);
      printf("  push rax\n");
    } else
      assert(false);
  } else {
    error_at(node->token->pos, "left-value must be a variable");
  }
}

void load(Type *type) {
  // pop address from stack
  // push the value of address to stack
  if (type->kind == TYPE_ARRAY)
    return; // nothing todo
  printf("  pop rax\n");
  if (type->size == 1)
    printf("  movsx rax, byte ptr [rax]\n");
  else if (type->size == 2)
    printf("  movsx rax, word ptr [rax]\n");
  else if (type->size == 4)
    printf("  movsxd rax, dword ptr [rax]\n");
  else if (type->size == 8)
    printf("  mov rax, [rax]\n");
  else
    assert(false);
  printf("  push rax\n");
}

void store(Type *type) {
  // pop address from stack
  // store rax value to the address
  printf("  pop rdi\n");
  if (type->size == 1)
    printf("  mov [rdi], al\n");
  else if (type->size == 2)
    printf("  mov [rdi], ax\n");
  else if (type->size == 4)
    printf("  mov [rdi], eax\n");
  else if (type->size == 8)
    printf("  mov [rdi], rax\n");
  else
    assert(false);
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
  printf("  mov al, 0\n");
  printf("  call %.*s\n", node->func->name_length, node->func->name);
  printf("  jmp .Lend%d\n", l);
  printf(".Lcall%d:\n", l);
  printf("  sub rsp, 8\n");
  printf("  mov al, 0\n");
  printf("  call %.*s\n", node->func->name_length, node->func->name);
  printf("  add rsp, 8\n");
  printf(".Lend%d:\n", l);

  // push return value to stack
  printf("  push rax\n");
}

int eval(Node *node) {
  switch (node->kind) {
  case ND_ADD:
    return eval(node->lhs) + eval(node->rhs);
  case ND_SUB:
    return eval(node->lhs) - eval(node->rhs);
  case ND_MUL:
    return eval(node->lhs) * eval(node->rhs);
  case ND_DIV:
    return eval(node->lhs) / eval(node->rhs);
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_LE:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_NUM:
    return node->val;
  default:
    error_at(node->token->pos, "not a constant expr");
    return 0;
  }
}

void gen_global_init(VariableInit *init, Type *type) {
  if (init == NULL) {
    printf("  .zero %d\n", type->size);
    return;
  }

  if (type->kind == TYPE_ARRAY) {
    if (init->expr) {
      if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
        // initilize the array as a string
        char *lit = init->expr->variable->string_literal;
        if (type->array_size != (int)strlen(lit) + 1)
          error_at(init->expr->token->pos, "miss-match between array-size and string-length");
        printf("  .ascii \"%s\\0\"\n", lit);
      } else {
        // When init->expr is given for an array, only the first element will be initialized.
        Type *ty = type;
        while (ty->kind == TYPE_ARRAY)
          ty = ty->base;
        gen_global_init(init, ty);
        printf("  .zero %d\n", type->size - ty->size);
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
          printf("  .zero %d\n", zero_size);
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
          printf("  .zero %d\n", zero_size);
      }
    } else
      assert(false);
  } else if (type->kind == TYPE_PTR) {
    if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
      // initilize the pointer to a string-literal
      printf("  .quad %.*s\n", init->expr->variable->name_length, init->expr->variable->name);
      return;
    }
    error("initilizing a global pointer is not implemented."); // TODO
  } else if (type->kind == TYPE_INT || type->kind == TYPE_CHAR) {
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    int val = eval(init->expr);
    if (type->kind == TYPE_INT) {
      printf("  .long %d\n", val);
    } else if (type->kind == TYPE_CHAR) {
      printf("  .byte %d\n", val);
    } else
      assert(false);
  } else
    assert(false);
}

void gen_global_variables() {
  for (int i = 0; i < globals->size; i++) {
    Variable *v = *(Variable **)vector_get(globals, i);
    printf(".data\n");
    printf(".globl %.*s\n", v->name_length, v->name);
    printf("%.*s:\n", v->name_length, v->name);
    gen_global_init(v->init, v->type);
  }
}

void gen_func(Node *node) {
  if (strncmp(node->func->name, "main", 4) == 0) {
    printf(".globl main\n");
  }
  printf("%.*s:\n", node->func->name_length, node->func->name);

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");

  // move args to stack
  for (int i = 0; i < node->func->params->size; ++i) {
    Variable *v = *(Variable **)vector_get(node->func->params, i);
    if (v->type->size == 1)
      printf("  mov [rbp-%d], %s\n", v->offset, reg_args1[i]);
    else if (v->type->size == 2)
      printf("  mov [rbp-%d], %s\n", v->offset, reg_args2[i]);
    else if (v->type->size == 4)
      printf("  mov [rbp-%d], %s\n", v->offset, reg_args4[i]);
    else
      printf("  mov [rbp-%d], %s\n", v->offset, reg_args8[i]);
  }

  if (node->func->offset) {
    int ofs = node->func->offset;
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
  case ND_VAR:
    gen_left_value(node);
    load(node->type);
    return;
  case ND_ASSIGN:
    gen_left_value(node->lhs);
    gen(node->rhs);
    printf("  pop rax\n");
    store(node->type);
    printf("  push rax\n");
    return;
  case ND_ADDR:
    gen_left_value(node->lhs);
    return;
  case ND_DEREF:
    gen(node->lhs);
    load(node->type);
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
