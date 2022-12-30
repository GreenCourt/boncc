#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

// TODO move type checking to parser.c
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
    if (left->kind == TYPE_ARRAY)
      return node->type = pointer_type(left->base); // implicitly cast
    else if (right->kind == TYPE_ARRAY)
      return node->type = pointer_type(right->base); // implicitly cast
    else
      return node->type = left;
  }

  if (node->kind == ND_EQ || node->kind == ND_NE || node->kind == ND_LT ||
      node->kind == ND_LE)
    return node->type = base_type(TYPE_INT);

  if (node->kind == ND_ASSIGN)
    return node->type = get_type(node->lhs);

  if (node->kind == ND_VAR)
    return node->type = node->variable->type;

  if (node->kind == ND_NUM)
    return node->type = base_type(TYPE_INT);

  if (node->kind == ND_CALL) {
    for (int i = 0; i < functions->size; i++) {
      Node *f = *(Node **)vector_get(functions, i);
      if (strncmp(node->name, f->name, node->len) == 0) {
        return node->type = f->type;
      }
    }
  }

  if (node->kind == ND_ADDR)
    return node->type = pointer_type(get_type(node->lhs));

  if (node->kind == ND_DEREF) {
    Type *left = get_type(node->lhs);
    if (left->kind == TYPE_PTR || left->kind == TYPE_ARRAY)
      return node->type = left->base;
    else
      error("invalid dereference operation (to non-pointer)");
  }

  return NULL;
}

Type *base_type(TypeKind kind) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = kind;
  if (kind == TYPE_INT)
    t->size = 4;
  else if (kind == TYPE_CHAR)
    t->size = 1;
  else
    assert(false);
  return t;
}

Type *pointer_type(Type *base) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_PTR;
  t->base = base;
  t->size = 8;
  return t;
}

Type *array_type(Type *base, int len) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_ARRAY;
  t->base = base;
  t->array_size = len;
  t->size = base->size * len;
  return t;
}
