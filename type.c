#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

int size_of(Type *type) {
  if (type->kind == TYPE_PTR || type->kind == TYPE_ARRAY)
    return 8;
  if (type->kind == TYPE_INT)
    return 4;
  assert(false);
  return -1;
}

Type *get_type(Node *node) {
  if (node->type)
    return node->type;

  if (node->kind == ND_ADD || node->kind == ND_SUB || node->kind == ND_MUL ||
      node->kind == ND_DIV) {
    Type *left = get_type(node->lhs);
    Type *right = get_type(node->rhs);
    if (left->kind == TYPE_PTR && right->kind == TYPE_PTR)
      error("invalid operands to binary operator (pointer and pointer)");

    if (node->kind == ND_MUL &&
        (left->kind == TYPE_PTR || right->kind == TYPE_PTR))
      error("invalid operands to binary * operator (pointer)");

    if (node->kind == ND_DIV &&
        (left->kind == TYPE_PTR || right->kind == TYPE_PTR))
      error("invalid operands to binary / operator (pointer)");

    if (node->kind == ND_SUB && right->kind == TYPE_PTR)
      error("invalid operands to binary - operator (int - pointer)");

    if (left->kind == TYPE_PTR)
      return node->type = left;
    else if (right->kind == TYPE_PTR)
      return node->type = right;
    else
      return node->type = left;
  }

  if (node->kind == ND_EQ || node->kind == ND_NE || node->kind == ND_LT ||
      node->kind == ND_LE) {
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
    return node->type;
  }

  if (node->kind == ND_ASSIGN) {
    return node->type = get_type(node->lhs);
  }

  if (node->kind == ND_LVAR) {
    return node->type = node->lvar->type;
  }

  if (node->kind == ND_NUM) {
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_INT;
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
    node->type = calloc(1, sizeof(Type));
    node->type->kind = TYPE_PTR;
    node->type->ptr_to = get_type(node->lhs);
    return node->type;
  }

  if (node->kind == ND_DEREF) {
    Type *left = get_type(node->lhs);
    if (left->kind != TYPE_PTR && left->kind != TYPE_ARRAY)
      error("invalid dereference operation (to non-pointer)");
    return node->type = left->ptr_to;
  }

  return NULL;
}
