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
  case TYPE_BOOL:
    return "_Bool";
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
  case TYPE_NONE:
    return "none";
  }
  assert(false);
  return NULL;
}

Type *base_type(TypeKind kind) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = kind;
  switch (kind) {
  case TYPE_VOID:
  case TYPE_CHAR:
  case TYPE_UCHAR:
  case TYPE_BOOL:
    t->align = t->size = 1;
    break;
  case TYPE_SHORT:
  case TYPE_USHORT:
    t->align = t->size = 2;
    break;
  case TYPE_INT:
  case TYPE_UINT:
  case TYPE_FLOAT:
    t->align = t->size = 4;
    break;
  case TYPE_LONG:
  case TYPE_ULONG:
  case TYPE_DOUBLE:
    t->align = t->size = 8;
    break;
  case TYPE_LDOUBLE:
    t->align = t->size = 16;
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
  t->align = t->size = 8;
  return t;
}

Type *array_type(Type *base, int len) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_ARRAY;
  t->base = base;
  t->objdec = base->objdec;
  t->array_size = len;
  t->size = base->size * len;
  t->align = base->align;
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
  t->align = 4;
  t->is_unnamed = is_unnamed;
  return t;
}

Type *func_type(Type *return_type) {
  Type *t = calloc(1, sizeof(Type));
  t->kind = TYPE_FUNC;
  t->align = t->size = 0;
  t->params = new_vector(0, sizeof(Variable *));
  t->return_type = return_type;
  t->objdec = return_type->objdec;
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
  case TYPE_BOOL:
    return true;
  default:
    return false;
  }
}

bool is_scalar(Type *type) {
  assert(type);
  return is_integer(type) || is_float(type) || type->kind == TYPE_PTR;
}

bool is_numerical(Type *type) {
  assert(type);
  return is_integer(type) || is_float(type);
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

bool is_funcptr(Type *type) {
  return type->kind == TYPE_PTR && type->base->kind == TYPE_FUNC;
}

bool is_struct_union(Type *type) {
  return type->kind == TYPE_STRUCT || type->kind == TYPE_UNION;
}

bool pass_on_memory(Type *type) {
  return is_struct_union(type) && type->size > 16;
}

Type *implicit_type_conversion(Type *l, Type *r) {
  assert(!is_struct_union(l));
  assert(!is_struct_union(r));
  assert(l->kind != TYPE_VOID);
  assert(r->kind != TYPE_VOID);
  assert(!(is_float(l) && (r->kind == TYPE_ARRAY || r->kind == TYPE_PTR)));
  assert(!(is_float(r) && (l->kind == TYPE_ARRAY || l->kind == TYPE_PTR)));

  if (l->kind == TYPE_ARRAY)
    return pointer_type(l->base);
  if (r->kind == TYPE_ARRAY)
    return pointer_type(r->base);

  if (l->kind == TYPE_PTR)
    return l;

  if (r->kind == TYPE_PTR)
    return r;

  if (is_float(l) && is_float(r))
    return l->size > r->size ? l : r;

  if (is_float(l))
    return l;

  if (is_float(r))
    return r;

  if (l->size > r->size)
    return l;

  if (l->size < r->size)
    return r;

  if (is_unsigned(r))
    return r;

  return l;
}

bool castable(Type *from, Type *to) {
  assert(from->kind != TYPE_FUNC);
  assert(to->kind != TYPE_FUNC);
  assert(from->kind != TYPE_NONE);
  assert(to->kind != TYPE_NONE);

  if (same_type(from, to))
    return true;

  if (from->kind == TYPE_ARRAY && is_scalar(to))
    return true;

  if (!is_scalar(from) && to->kind != TYPE_VOID)
    return false;

  if (from->kind == TYPE_VOID && to->kind != TYPE_VOID)
    return false;

  return true;
}
