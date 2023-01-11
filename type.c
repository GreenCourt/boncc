#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

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

Type *struct_type(Ident *ident, Member *member, int size) {
  assert(ident != NULL);
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_STRUCT;
  t->ident = ident;
  t->member = member;
  t->size = size;
  return t;
}
