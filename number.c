#include "boncc.h"
#include <assert.h>
#include <stdlib.h>

Number *new_number_int(int val) {
  Number *num = calloc(1, sizeof(Number));
  num->value.long_value = val;
  num->type = base_type(TYPE_INT);
  return num;
}

bool is_integer_zero(Number *num) {
  assert(num);
  if (!is_integer(num->type) && num->type->kind != TYPE_PTR)
    return false;

  if (is_signed(num->type))
    return number2long(num) == 0;

  if (is_unsigned(num->type))
    return number2ulong(num) == 0;

  assert(false);
  return 0;
}

int number2int(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (int)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (int)num->value.ulong_value;
  case TYPE_FLOAT:
    return (int)num->value.float_value;
  case TYPE_DOUBLE:
    return (int)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

unsigned int number2uint(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (unsigned int)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (unsigned int)num->value.ulong_value;
  case TYPE_FLOAT:
    return (unsigned int)num->value.float_value;
  case TYPE_DOUBLE:
    return (unsigned int)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

long long number2long(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (long long)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (long long)num->value.ulong_value;
  case TYPE_FLOAT:
    return (long long)num->value.float_value;
  case TYPE_DOUBLE:
    return (long long)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

unsigned long long number2ulong(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (unsigned long long)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (unsigned long long)num->value.ulong_value;
  case TYPE_FLOAT:
    return (unsigned long long)num->value.float_value;
  case TYPE_DOUBLE:
    return (unsigned long long)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

short number2short(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (short)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (short)num->value.ulong_value;
  case TYPE_FLOAT:
    return (short)num->value.float_value;
  case TYPE_DOUBLE:
    return (short)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

unsigned short number2ushort(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (unsigned short)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (unsigned short)num->value.ulong_value;
  case TYPE_FLOAT:
    return (unsigned short)num->value.float_value;
  case TYPE_DOUBLE:
    return (unsigned short)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

char number2char(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (char)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (char)num->value.ulong_value;
  case TYPE_FLOAT:
    return (char)num->value.float_value;
  case TYPE_DOUBLE:
    return (char)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

unsigned char number2uchar(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (unsigned char)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (unsigned char)num->value.ulong_value;
  case TYPE_FLOAT:
    return (unsigned char)num->value.float_value;
  case TYPE_DOUBLE:
    return (unsigned char)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

bool number2bool(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (bool)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (bool)num->value.ulong_value;
  case TYPE_FLOAT:
    return (bool)num->value.float_value;
  case TYPE_DOUBLE:
    return (bool)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

float number2float(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (float)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (float)num->value.ulong_value;
  case TYPE_FLOAT:
    return (float)num->value.float_value;
  case TYPE_DOUBLE:
    return (float)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

double number2double(Number *num) {
  assert(num);
  switch (num->type->kind) {
  case TYPE_BOOL:
  case TYPE_CHAR:
  case TYPE_SHORT:
  case TYPE_INT:
  case TYPE_LONG:
    return (double)num->value.long_value;
  case TYPE_UCHAR:
  case TYPE_USHORT:
  case TYPE_UINT:
  case TYPE_ULONG:
  case TYPE_PTR:
    return (double)num->value.ulong_value;
  case TYPE_FLOAT:
    return (double)num->value.float_value;
  case TYPE_DOUBLE:
    return (double)num->value.double_value;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    return 0;
  default:
    assert(false);
    return 0;
  }
}

Number *number_add(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) + number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) + number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) + number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) + number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) + number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) + number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) + number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR: // base type size is dealt with by node.c
    ret->value.ulong_value = number2ulong(l) + number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.float_value = number2float(l) + number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.double_value = number2double(l) + number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_sub(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = (l->type->kind == TYPE_PTR && r->type->kind == TYPE_PTR)
                  ? base_type(TYPE_ULONG)
                  : implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) - number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) - number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) - number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) - number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) - number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) - number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) - number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR: // base type size is dealt with by node.c
    ret->value.ulong_value = number2ulong(l) - number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.float_value = number2float(l) - number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.double_value = number2double(l) - number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_mul(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) * number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) * number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) * number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) * number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) * number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) * number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) * number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR: // type error check is done by node.c
    ret->value.ulong_value = number2ulong(l) * number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.float_value = number2float(l) * number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.double_value = number2double(l) * number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_div(Number *l, Number *r) {
  assert(l && r);
  assert(l->type->kind != TYPE_PTR && r->type->kind != TYPE_PTR);
  assert(is_float(implicit_type_conversion(l->type, r->type)) ||
         !is_integer_zero(r));

  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) / number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) / number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) / number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) / number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) / number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) / number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) / number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR: // type error check is done by node.c
    ret->value.ulong_value = number2ulong(l) / number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.float_value = number2float(l) / number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.double_value = number2double(l) / number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_mod(Number *l, Number *r) {
  assert(l && r);
  assert(is_integer(l->type) && is_integer(r->type));
  assert(!is_integer_zero(r));

  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) % number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) % number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) % number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) % number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) % number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) % number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) % number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR: // type error check is done by node.c
    ret->value.ulong_value = number2ulong(l) % number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_bitxor(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) ^ number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) ^ number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) ^ number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) ^ number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) ^ number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) ^ number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) ^ number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l) ^ number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_bitor(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) | number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) | number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) | number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) | number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) | number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) | number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) | number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l) | number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_bitand(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) & number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) & number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) & number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) & number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) & number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) & number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) & number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l) & number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_bitnot(Number *l) {
  assert(l);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = l->type;

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = ~number2char(l);
    break;
  case TYPE_SHORT:
    ret->value.long_value = ~number2short(l);
    break;
  case TYPE_INT:
    ret->value.long_value = ~number2int(l);
    break;
  case TYPE_LONG:
    ret->value.long_value = ~number2long(l);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = ~number2uchar(l);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = ~number2ushort(l);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = ~number2uint(l);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = ~number2ulong(l);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_lshift(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) << number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) << number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) << number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) << number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) << number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) << number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) << number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l) << number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_rshift(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = implicit_type_conversion(l->type, r->type);

  switch (ret->type->kind) {
  case TYPE_BOOL: // calculate as char
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) >> number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) >> number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) >> number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) >> number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l) >> number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l) >> number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l) >> number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l) >> number2ulong(r);
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_eq(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);

  assert(same_type(l->type, r->type));
  switch (l->type->kind) {
  case TYPE_BOOL:
    ret->value.long_value = number2bool(l) == number2bool(r);
    break;
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) == number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) == number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) == number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) == number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.long_value = number2uchar(l) == number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.long_value = number2ushort(l) == number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.long_value = number2uint(l) == number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.long_value = number2ulong(l) == number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.long_value = number2float(l) == number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.long_value = number2double(l) == number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_ne(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);

  assert(same_type(l->type, r->type));
  switch (l->type->kind) {
  case TYPE_BOOL:
    ret->value.long_value = number2bool(l) != number2bool(r);
    break;
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) != number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) != number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) != number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) != number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.long_value = number2uchar(l) != number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.long_value = number2ushort(l) != number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.long_value = number2uint(l) != number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.long_value = number2ulong(l) != number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.long_value = number2float(l) != number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.long_value = number2double(l) != number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_lt(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);

  assert(same_type(l->type, r->type));
  switch (l->type->kind) {
  case TYPE_BOOL:
    ret->value.long_value = number2bool(l) < number2bool(r);
    break;
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) < number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) < number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) < number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) < number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.long_value = number2uchar(l) < number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.long_value = number2ushort(l) < number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.long_value = number2uint(l) < number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.long_value = number2ulong(l) < number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.long_value = number2float(l) < number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.long_value = number2double(l) < number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_le(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);

  assert(same_type(l->type, r->type));
  switch (l->type->kind) {
  case TYPE_BOOL:
    ret->value.long_value = number2bool(l) <= number2bool(r);
    break;
  case TYPE_CHAR:
    ret->value.long_value = number2char(l) <= number2char(r);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l) <= number2short(r);
    break;
  case TYPE_INT:
    ret->value.long_value = number2int(l) <= number2int(r);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l) <= number2long(r);
    break;
  case TYPE_UCHAR:
    ret->value.long_value = number2uchar(l) <= number2uchar(r);
    break;
  case TYPE_USHORT:
    ret->value.long_value = number2ushort(l) <= number2ushort(r);
    break;
  case TYPE_UINT:
    ret->value.long_value = number2uint(l) <= number2uint(r);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.long_value = number2ulong(l) <= number2ulong(r);
    break;
  case TYPE_FLOAT:
    ret->value.long_value = number2float(l) <= number2float(r);
    break;
  case TYPE_DOUBLE:
    ret->value.long_value = number2double(l) <= number2double(r);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  default:
    assert(false);
    break;
  }
  return ret;
}

Number *number_cond(Number *cond, Number *l, Number *r) {
  assert(cond && l && r);
  return number2bool(cond) ? l : r;
}

Number *number_lognot(Number *l) {
  assert(l);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);
  ret->value.long_value = !number2bool(l);
  return ret;
}

Number *number_logand(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);
  ret->value.long_value = number2bool(l) && number2bool(r);
  return ret;
}

Number *number_logor(Number *l, Number *r) {
  assert(l && r);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = base_type(TYPE_BOOL);
  ret->value.long_value = number2bool(l) || number2bool(r);
  return ret;
}

Number *number_cast(Number *l, Type *type) {
  assert(l);
  Number *ret = calloc(1, sizeof(Number));
  ret->type = type;

  switch (type->kind) {
  case TYPE_BOOL:
    ret->value.long_value = number2bool(l);
    break;
  case TYPE_CHAR:
    ret->value.long_value = number2char(l);
    break;
  case TYPE_SHORT:
    ret->value.long_value = number2short(l);
    break;
  case TYPE_INT:
  case TYPE_ENUM:
    ret->value.long_value = number2int(l);
    break;
  case TYPE_LONG:
    ret->value.long_value = number2long(l);
    break;
  case TYPE_UCHAR:
    ret->value.ulong_value = number2uchar(l);
    break;
  case TYPE_USHORT:
    ret->value.ulong_value = number2ushort(l);
    break;
  case TYPE_UINT:
    ret->value.ulong_value = number2uint(l);
    break;
  case TYPE_ULONG:
  case TYPE_PTR:
    ret->value.ulong_value = number2ulong(l);
    break;
  case TYPE_FLOAT:
    ret->value.float_value = number2float(l);
    break;
  case TYPE_DOUBLE:
    ret->value.double_value = number2double(l);
    break;
  case TYPE_LDOUBLE:
    error(NULL, "long double is currently not supported");
    break;
  case TYPE_VOID:
    // nothing to do
    break;
  default:
    assert(false);
  }
  return ret;
}
