#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

char *type_text(TypeKind kind) {
  switch (kind) {
  case TYPE_VOID:
    return "void";
  case TYPE_INT:
    return "int";
  case TYPE_CHAR:
    return "char";
  case TYPE_SHORT:
    return "short";
  case TYPE_LONG:
    return "long";
  case TYPE_UINT:
    return "unsigned int";
  case TYPE_UCHAR:
    return "unsigned char";
  case TYPE_USHORT:
    return "unsigned short";
  case TYPE_ULONG:
    return "unsigned long";
  case TYPE_FLOAT:
    return "float";
  case TYPE_DOUBLE:
    return "double";
  case TYPE_LDOUBLE:
    return "long double";
  case TYPE_PTR:
    return "ptr";
  case TYPE_ARRAY:
    return "array";
  case TYPE_STRUCT:
    return "struct";
  case TYPE_UNION:
    return "union";
  case TYPE_ENUM:
    return "enum";
  case TYPE_FUNC:
    return "function";
  }
  assert(false);
  return NULL;
};

Type *base_type(TypeKind kind) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = kind;
  switch (kind) {
  case TYPE_VOID:
    t->size = 1;
    break;
  case TYPE_CHAR:
  case TYPE_UCHAR:
    t->size = 1;
    break;
  case TYPE_SHORT:
  case TYPE_USHORT:
    t->size = 2;
    break;
  case TYPE_INT:
  case TYPE_UINT:
  case TYPE_FLOAT:
    t->size = 4;
    break;
  case TYPE_LONG:
  case TYPE_ULONG:
  case TYPE_DOUBLE:
    t->size = 8;
    break;
  case TYPE_LDOUBLE:
    t->size = 16;
    break;
  default:
    assert(false);
  }
  return t;
}

Type *pointer_type(Type *base) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_PTR;
  t->base = base;
  t->objdec = base->objdec;
  t->size = 8;
  return t;
}

Type *array_type(Type *base, int len) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_ARRAY;
  t->base = base;
  t->objdec = base->objdec;
  t->array_size = len;
  t->size = base->size * len;
  return t;
}

Type *struct_type(bool is_unnamed) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_STRUCT;
  t->size = -1;
  t->is_unnamed = is_unnamed;
  return t;
}

Type *union_type(bool is_unnamed) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_UNION;
  t->size = -1;
  t->is_unnamed = is_unnamed;
  return t;
}

Type *enum_type(bool is_unnamed) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_ENUM;
  t->size = -1;
  t->is_unnamed = is_unnamed;
  return t;
}

Type *func_type() {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_FUNC;
  t->size = -1;
  t->params = new_vector(0, sizeof(Variable *));
  return t;
}

bool same_type(Type *a, Type *b) {
  assert(a);
  assert(b);
  if (a == b)
    return true;

  if (a->kind != b->kind)
    return false;

  if (a->kind == TYPE_STRUCT || a->kind == TYPE_UNION || a->kind == TYPE_ENUM)
    return a == b; // for user defined types, compare as pointers

  if (a->kind == TYPE_ARRAY || a->kind == TYPE_PTR)
    return same_type(a->base, b->base);

  if (a->kind == TYPE_FUNC) {
    if (!same_type(a->return_type, b->return_type))
      return false;

    if (a->params->size != b->params->size || a->is_variadic != b->is_variadic)
      return false;

    for (int i = 0; i < a->params->size; ++i) {
      Type *ap = *(Type **)vector_get(a->params, i);
      Type *bp = *(Type **)vector_get(b->params, i);
      if (!same_type(ap, bp))
        return false;
    }
  }

  return true;
}

bool is_float(Type *type) {
  assert(type);
  switch (type->kind) {
  case TYPE_FLOAT:
  case TYPE_DOUBLE:
  case TYPE_LDOUBLE:
    return true;
  default:
    return false;
  }
}

bool is_integer(Type *type) {
  assert(type);
  switch (type->kind) {
  case TYPE_INT:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_LONG:
  case TYPE_UINT:
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_ULONG:
  case TYPE_ENUM:
    return true;
  default:
    return false;
  }
}

bool is_signed(Type *type) {
  assert(type);
  switch (type->kind) {
  case TYPE_INT:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_LONG:
  case TYPE_ENUM:
    return true;
  default:
    return false;
  }
}

bool is_unsigned(Type *type) {
  assert(type);
  switch (type->kind) {
  case TYPE_UINT:
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return true;
  default:
    return false;
  }
}

Type *implicit_type_conversion(Type *l, Type *r) {
  if (l->kind == TYPE_ARRAY)
    return pointer_type(l->base);
  if (r->kind == TYPE_ARRAY)
    return pointer_type(r->base);

  if (l->kind == TYPE_PTR || r->kind == TYPE_PTR)
    return l;

  if (l->size > r->size)
    return l;

  if (l->size < r->size)
    return r;

  if (is_unsigned(r))
    return r;

  return l;
}
