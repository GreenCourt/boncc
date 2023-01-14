#include "boncc.h"
#include <assert.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int label = 0;
static const char *reg_args1[] = {"dil", "sil", "dl", "cl", "r8b", "r9b"};
static const char *reg_args2[] = {"di", "si", "dx", "cx", "r8w", "r9w"};
static const char *reg_args4[] = {"edi", "esi", "edx", "ecx", "r8d", "r9d"};
static const char *reg_args8[] = {"rdi", "rsi", "rdx", "rcx", "r8", "r9"};

// node.c -->
bool is_constant_number(Node *node);
int eval(Node *node);
// <-- node.c

void comment(Token *tok, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  printf("  # ");
  vprintf(fmt, ap);
  if (tok)
    printf("    %s:%d:%d", tok->pos.file_name, tok->pos.line_number, tok->pos.column_number);
  printf("\n");
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
      comment(NULL, "gen_address ND_VAR local: %.*s", node->variable->ident->len, node->variable->ident->name);
      printf("  lea rax, [rbp-%d]\n", node->variable->offset);
    } else if (node->variable->kind == VK_GLOBAL) {
      comment(NULL, "gen_address ND_VAR global: %.*s", node->variable->ident->len, node->variable->ident->name);
      printf("  lea rax, %.*s[rip]\n", node->variable->ident->len, node->variable->ident->name);
    } else if (node->variable->kind == VK_STRLIT) {
      comment(NULL, "gen_address ND_VAR strlit: \"%s\"", node->variable->string_literal);
      printf("  lea rax, %.*s[rip]\n", node->variable->ident->len, node->variable->ident->name);
    } else
      assert(false);
    return;
  case ND_MEMBER:
    comment(NULL, "gen_address ND_MEMBER %.*s", node->member->ident->len, node->member->ident->name);
    gen_address(node->lhs);
    printf("  add rax, %d\n", node->member->offset);
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
    printf("  movsx rax, byte ptr [rax]\n");
  else if (type->size == 2)
    printf("  movsx rax, word ptr [rax]\n");
  else if (type->size == 4)
    printf("  movsxd rax, dword ptr [rax]\n");
  else if (type->size == 8)
    printf("  mov rax, [rax]\n");
  else
    assert(false);
}

void store(Type *type) {
  // src  : rax
  // dest : the address popped from stack
  comment(NULL, "store %s", type_text(type->kind));

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
  }
}

void gen_call(Node *node) {
  int sz = node->args->size;
  if (sz > 6)
    error("maximum number of argument is currently 6");

  for (int i = 0; i < sz; ++i) {
    Node *d = *(Node **)vector_get(node->args, i);
    gen(d);
    printf("  push rax\n");
  }
  for (int i = sz - 1; i >= 0; --i) {
    printf("  pop %s\n", reg_args8[i]);
  }

  // align RSP to 16bytes (ABI requirements)
  comment(NULL, "RSP alignment for call");
  int l = label++;
  printf("  mov rax, rsp\n");
  printf("  and rax, 15\n"); // rax % 16 == rax & 0xF
  printf("  jnz .Lcall%d\n", l);
  printf("  mov al, 0\n");
  printf("  call %.*s\n", node->func->ident->len, node->func->ident->name);
  printf("  jmp .Lend%d\n", l);
  printf(".Lcall%d:\n", l);
  printf("  sub rsp, 8\n");
  printf("  mov al, 0\n");
  printf("  call %.*s\n", node->func->ident->len, node->func->ident->name);
  printf("  add rsp, 8\n");
  printf(".Lend%d:\n", l);
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
          error_at(&init->expr->token->pos, "miss-match between array-size and string-length");
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
    while (init->vec) { // for non-array primitive types, only the first element in the brace will be used
      assert(init->vec->size > 0);
      init = *(VariableInit **)vector_get(init->vec, 0);
    }
    assert(init->expr);
    if (type->base->kind == TYPE_CHAR && init->expr->kind == ND_VAR && init->expr->variable->kind == VK_STRLIT) {
      // initilize the pointer to a string-literal
      printf("  .quad %.*s\n", init->expr->variable->ident->len, init->expr->variable->ident->name);
      return;
    } else if (init->expr->kind == ND_ADD) {
      bool left_is_addr = init->expr->lhs->kind == ND_ADDR && init->expr->lhs->lhs->kind == ND_VAR && init->expr->lhs->lhs->variable->kind == VK_GLOBAL;
      bool right_is_addr = init->expr->rhs->kind == ND_ADDR && init->expr->rhs->lhs->kind == ND_VAR && init->expr->rhs->lhs->variable->kind == VK_GLOBAL;

      if (left_is_addr && is_constant_number(init->expr->rhs)) {
        printf("  .quad %.*s+%d\n",
               init->expr->lhs->lhs->variable->ident->len,
               init->expr->lhs->lhs->variable->ident->name,
               eval(init->expr->rhs));
      } else if (right_is_addr && is_constant_number(init->expr->lhs)) {
        printf("  .quad %.*s+%d\n",
               init->expr->rhs->lhs->variable->ident->len,
               init->expr->rhs->lhs->variable->ident->name,
               eval(init->expr->lhs));
      } else {
        error("unsupported initalization of a global pointer.");
      }
    } else if (init->expr->kind == ND_ADDR && init->expr->lhs->kind == ND_VAR && init->expr->lhs->variable->kind == VK_GLOBAL) {
      printf("  .quad %.*s\n",
             init->expr->lhs->variable->ident->len,
             init->expr->lhs->variable->ident->name);
    } else if (is_constant_number(init->expr)) {
      printf("  .quad %d\n", eval(init->expr));
    } else {
      error("unsupported initalization of a global pointer.");
    }
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
    printf(".globl %.*s\n", v->ident->len, v->ident->name);
    printf("%.*s:\n", v->ident->len, v->ident->name);
    gen_global_init(v->init, v->type);
  }
}

void gen_func(Node *node) {
  if (strncmp(node->func->ident->name, "main", 4) == 0) {
    printf(".globl main\n");
  }
  printf("%.*s:\n", node->func->ident->len, node->func->ident->name);

  // prologue
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");

  // move args to stack
  comment(NULL, "function arguments to stack");
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

  // epilogue
  printf("  mov rsp, rbp\n");
  printf("  pop rbp\n");
  printf("  ret\n");
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
    printf("  mov rsp, rbp\n");
    printf("  pop rbp\n");
    printf("  ret\n");
    return;
  case ND_NUM:
    comment(node->token, "ND_NUM");
    printf("  mov rax, %lld\n", node->val);
    return;
  case ND_VAR:
    comment(node->token, "ND_VAR");
    gen_address(node);
    load(node->type);
    return;
  case ND_ASSIGN:
    comment(node->token, "ND_ASSIGN");
    gen_address(node->lhs);
    printf("  push rax\n");
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
    printf("  push rax\n");
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  add rax, rdi\n");
    break;
  case ND_SUB:
    comment(node->token, "ND_SUB");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  mov rdi, rax\n");
    printf("  pop rax\n");
    printf("  sub rax, rdi\n");
    break;
  case ND_MUL:
    comment(node->token, "ND_MUL");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  imul rax, rdi\n");
    break;
  case ND_DIV:
    comment(node->token, "ND_DIV");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  mov rdi, rax\n");
    printf("  pop rax\n");
    printf("  cqo\n");
    printf("  idiv rdi\n");
    break;
  case ND_EQ:
    comment(node->token, "ND_EQ");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  cmp rax, rdi\n");
    printf("  sete al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_NE:
    comment(node->token, "ND_NE");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  pop rdi\n");
    printf("  cmp rax, rdi\n");
    printf("  setne al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LT:
    comment(node->token, "ND_LT");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  mov rdi, rax\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  setl al\n");
    printf("  movzb rax, al\n");
    break;
  case ND_LE:
    comment(node->token, "ND_LE");
    gen(node->lhs);
    printf("  push rax\n");
    gen(node->rhs);
    printf("  mov rdi, rax\n");
    printf("  pop rax\n");
    printf("  cmp rax, rdi\n");
    printf("  setle al\n");
    printf("  movzb rax, al\n");
    break;
  default:
    break;
  }
}
