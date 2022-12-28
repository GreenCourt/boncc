#include "boncc.h"
#include <stdlib.h>
#include <string.h>
#include <assert.h>

int type_size(Type *type) { return type->ptr ? 8 : 8; }

int pointer_destination_size(Type *type) {
  assert(type->ptr);
  if(type->ptr > 1) return 8; // pointer
  else return 8; // int
}

Type *type_detection(Node *node) {
  if (node->type)
    return node->type;

  if (node->kind == ND_ADD || node->kind == ND_SUB || node->kind == ND_MUL ||
      node->kind == ND_DIV) {
    Type *left = type_detection(node->lhs);
    Type *right = type_detection(node->rhs);
    if (left->ptr && right->ptr)
      error("invalid operands to binary operator (pointer and pointer)");

    if (node->kind == ND_MUL && (left->ptr || right->ptr))
      error("invalid operands to binary * operator (pointer)");

    if (node->kind == ND_DIV && (left->ptr || right->ptr))
      error("invalid operands to binary / operator (pointer)");

    if (node->kind == ND_SUB && right->ptr)
      error("invalid operands to binary - operator (int - pointer)");

    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = left->ptr ? left->ptr : right->ptr;
    return node->type;
  }

  if (node->kind == ND_EQ || node->kind == ND_NE || node->kind == ND_LT ||
      node->kind == ND_LE) {
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = 0;
    return node->type;
  }

  if (node->kind == ND_ASSIGN) {
    return node->type = type_detection(node->lhs);
  }

  if (node->kind == ND_LVAR) {
    return node->type = node->lvar->type;
  }

  if (node->kind == ND_NUM) {
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = 0;
    return node->type;
  }

  if (node->kind == ND_NUM) {
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = 0;
    return node->type;
  }

  if (node->kind == ND_CALL) {
    for (int i = 0; i < functions->size; i++) {
      Node *f = *(Node **)vector_get(functions, i);
      if (strncmp(node->name, f->name, node->len) == 0) {
        return node->type = f->type;
      }
    }
  }

  if (node->kind == ND_ADDR) {
    Type *left = type_detection(node->lhs);
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = left->ptr + 1;
    return node->type;
  }

  if (node->kind == ND_DEREF) {
    Type *left = type_detection(node->lhs);
    if (left->ptr == 0)
      error("invalid dereference operation (to non-pointer)");
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    node->type->ptr = left->ptr - 1;
    return node->type;
  }

  return NULL;
}
