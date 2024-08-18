#include "boncc.h"
#include <assert.h>
#include <stdlib.h>

static Number *eval(Node *node);
Variable *is_const_var_addr(Node *node);

Node *new_node_nop() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NOP;
  return node;
}

Node *new_node_num_int(Token *tok, int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  Number *num = calloc(1, sizeof(Number));
  num->value.long_value = val;
  num->type = base_type(TYPE_INT);
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_num_char(Token *tok, char val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  Number *num = calloc(1, sizeof(Number));
  num->value.long_value = val;
  num->type = base_type(TYPE_CHAR);
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_num_ulong(Token *tok, unsigned long long val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  Number *num = calloc(1, sizeof(Number));
  num->value.ulong_value = val;
  num->type = base_type(TYPE_ULONG);
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_num_pointer(Token *tok, unsigned long long val, Type *type) {
  assert(type->kind == TYPE_PTR);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  Number *num = calloc(1, sizeof(Number));
  num->value.ulong_value = val;
  num->type = type;
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_num_zero(Token *tok, Type *type) {
  assert(is_scalar(type));
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  Number *num = calloc(1, sizeof(Number));
  num->type = type;
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_num(Token *tok, Number *num) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->num = num;
  node->type = num->type;
  node->token = tok;
  node->is_constant_expr = true;
  return node;
}

Node *new_node_cast(Token *tok, Type *type, Node *operand) {
  assert(type->kind != TYPE_FUNC);
  assert(operand->type->kind != TYPE_FUNC);
  assert(type->kind != TYPE_NONE);
  assert(operand->type->kind != TYPE_NONE);

  if (same_type(type, operand->type))
    return operand;

  if (!is_scalar(type) && type->kind != TYPE_VOID)
    error(tok ? &tok->pos : NULL, "invalid type casting");

  if (operand->type->kind == TYPE_VOID && type->kind != TYPE_VOID)
    error(tok ? &tok->pos : NULL, "invalid type casting from void");

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CAST;
  node->token = tok;
  node->type = type;
  node->lhs = operand;
  if (operand->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

static Node *new_node(NodeKind kind, Node *lhs, Node *rhs, Type *type) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  node->type = type;
  return node;
}

Node *new_node_mul(Token *tok, Node *lhs, Node *rhs) {
  if (!is_numerical(lhs->type) || !is_numerical(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary * operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_MUL, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_div(Token *tok, Node *lhs, Node *rhs) {
  if (!is_numerical(lhs->type) || !is_numerical(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary / operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_DIV, lhs, rhs, type);
  node->token = tok;
  // integer division by zero is not a constant
  if (lhs->is_constant_expr && rhs->is_constant_expr &&
      !is_integer_zero(eval(node->rhs))) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_mod(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary %% operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_MOD, lhs, rhs, type);
  node->token = tok;
  // integer division by zero is not a constant
  if (lhs->is_constant_expr && rhs->is_constant_expr &&
      !is_integer_zero(eval(node->rhs))) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_add(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if ((left_is_ptr && right_is_ptr) ||
      (!left_is_ptr && !is_scalar(lhs->type)) ||
      (!right_is_ptr && !is_scalar(rhs->type)) ||
      (left_is_ptr && is_float(rhs->type)) ||
      (right_is_ptr && is_float(lhs->type)) ||
      (left_is_ptr && lhs->type->base->size < 0) ||
      (right_is_ptr && rhs->type->base->size < 0) || is_funcptr(lhs->type) ||
      is_funcptr(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary + operator");

  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  if (left_is_ptr) {
    assert(type->kind == TYPE_PTR);
    rhs = new_node_cast(NULL, type, rhs);
    rhs =
        new_node(ND_MUL, rhs,
                 new_node_num_pointer(NULL, lhs->type->base->size, type), type);
    if (rhs->lhs->is_constant_expr && rhs->rhs->is_constant_expr) {
      rhs->is_constant_expr = true;
      eval(rhs);
    }
  } else if (right_is_ptr) {
    assert(type->kind == TYPE_PTR);
    lhs = new_node_cast(NULL, type, lhs);
    lhs =
        new_node(ND_MUL, lhs,
                 new_node_num_pointer(NULL, rhs->type->base->size, type), type);
    if (lhs->lhs->is_constant_expr && lhs->rhs->is_constant_expr) {
      lhs->is_constant_expr = true;
      eval(lhs);
    }
  }

  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_ADD, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_sub(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if ((!left_is_ptr && right_is_ptr) ||
      (!left_is_ptr && !is_scalar(lhs->type)) ||
      (!right_is_ptr && !is_scalar(rhs->type)) ||
      (left_is_ptr && is_float(rhs->type)) ||
      (right_is_ptr && is_float(lhs->type)) ||
      (left_is_ptr && lhs->type->base->size < 0) ||
      (right_is_ptr && rhs->type->base->size < 0) || is_funcptr(lhs->type) ||
      (left_is_ptr && right_is_ptr &&
       !same_type(lhs->type->base, rhs->type->base)) ||
      is_funcptr(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary - operator");

  Type *type = left_is_ptr && right_is_ptr
                   ? base_type(TYPE_ULONG)
                   : implicit_type_conversion(lhs->type, rhs->type);
  if (left_is_ptr && !right_is_ptr) {
    assert(type->kind == TYPE_PTR);
    rhs = new_node_cast(NULL, type, rhs);
    rhs =
        new_node(ND_MUL, rhs,
                 new_node_num_pointer(NULL, lhs->type->base->size, type), type);
    if (rhs->lhs->is_constant_expr && rhs->rhs->is_constant_expr) {
      rhs->is_constant_expr = true;
      eval(rhs);
    }
  }

  Type *base = lhs->type->base;
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_SUB, lhs, rhs, type);
  node->token = tok;

  if (left_is_ptr && right_is_ptr) {
    assert(type->kind == TYPE_ULONG);
    node = new_node(ND_DIV, node, new_node_num_ulong(NULL, base->size), type);
    Variable *left_addr = is_const_var_addr(lhs);
    Variable *right_addr = is_const_var_addr(rhs);

    if (right_addr && left_addr == right_addr) {
      // ND_SUB for a same variable is zero (e.g &a - &a).
      node->is_constant_expr = true;
      node->num = calloc(1, sizeof(Number));
      node->num->type = type;
    } else {
      node->is_constant_expr = false;
    }
    return node;
  }

  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_eq(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  bool left_is_zero = lhs->kind == ND_NUM && is_integer_zero(lhs->num);
  bool right_is_zero = rhs->kind == ND_NUM && is_integer_zero(rhs->num);
  bool left_is_voidptr = left_is_ptr && lhs->type->base->kind == TYPE_VOID;
  bool right_is_voidptr = right_is_ptr && rhs->type->base->kind == TYPE_VOID;

  if ((!is_scalar(lhs->type) && !left_is_ptr) ||
      (!is_scalar(rhs->type) && !right_is_ptr) ||
      (left_is_ptr && is_float(rhs->type)) ||
      (right_is_ptr && is_float(lhs->type)) ||
      (left_is_ptr && (!right_is_ptr && !right_is_zero)) ||
      (right_is_ptr && (!left_is_ptr && !left_is_zero)) ||
      (left_is_ptr && right_is_ptr && !left_is_voidptr && !right_is_voidptr &&
       !same_type(lhs->type->base, rhs->type->base)))
    error(tok ? &tok->pos : NULL, "invalid operands to == operator");

  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_EQ, lhs, rhs, base_type(TYPE_BOOL));
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_ne(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  bool left_is_zero = lhs->kind == ND_NUM && is_integer_zero(lhs->num);
  bool right_is_zero = rhs->kind == ND_NUM && is_integer_zero(rhs->num);
  bool left_is_voidptr = left_is_ptr && lhs->type->base->kind == TYPE_VOID;
  bool right_is_voidptr = right_is_ptr && rhs->type->base->kind == TYPE_VOID;

  if ((!is_scalar(lhs->type) && !left_is_ptr) ||
      (!is_scalar(rhs->type) && !right_is_ptr) ||
      (left_is_ptr && is_float(rhs->type)) ||
      (right_is_ptr && is_float(lhs->type)) ||
      (left_is_ptr && (!right_is_ptr && !right_is_zero)) ||
      (right_is_ptr && (!left_is_ptr && !left_is_zero)) ||
      (left_is_ptr && right_is_ptr && !left_is_voidptr && !right_is_voidptr &&
       !same_type(lhs->type->base, rhs->type->base)))
    error(tok ? &tok->pos : NULL, "invalid operands to != operator");

  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_NE, lhs, rhs, base_type(TYPE_BOOL));
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_lt(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if ((!is_scalar(lhs->type) && !left_is_ptr) ||
      (!is_scalar(rhs->type) && !right_is_ptr) ||
      (left_is_ptr != right_is_ptr) ||
      (left_is_ptr && right_is_ptr &&
       !same_type(lhs->type->base, rhs->type->base)) ||
      is_funcptr(lhs->type) || is_funcptr(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to relational operator");

  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LT, lhs, rhs, base_type(TYPE_BOOL));
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_le(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if ((!is_scalar(lhs->type) && !left_is_ptr) ||
      (!is_scalar(rhs->type) && !right_is_ptr) ||
      (left_is_ptr != right_is_ptr) ||
      (left_is_ptr && right_is_ptr &&
       !same_type(lhs->type->base, rhs->type->base)) ||
      is_funcptr(lhs->type) || is_funcptr(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to relational operator");

  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LE, lhs, rhs, base_type(TYPE_BOOL));
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_addr(Token *tok, Node *operand) {
  if (operand->kind == ND_NUM)
    error(tok ? &tok->pos : NULL, "invalid unary & operand");
  Type *type = pointer_type(operand->type);
  Node *node = new_node(ND_ADDR, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_deref(Token *tok, Node *operand) {
  if ((operand->type->kind != TYPE_PTR) && (operand->type->kind != TYPE_ARRAY))
    error(tok ? &tok->pos : NULL, "invalid unary * operand");

  if (operand->kind == ND_VAR && operand->variable->kind == OBJ_FUNC)
    return operand;

  Type *type = operand->type->base;
  Node *node = new_node(ND_DEREF, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_lognot(Token *tok, Node *operand) {
  bool is_ptr =
      operand->type->kind == TYPE_PTR || operand->type->kind == TYPE_ARRAY;
  if (!is_scalar(operand->type) && !is_ptr)
    error(tok ? &tok->pos : NULL, "invalid operand to unary ! operator");
  operand = new_node_cast(NULL, base_type(TYPE_BOOL), operand);
  Node *node = new_node(ND_LOGNOT, operand, NULL, base_type(TYPE_BOOL));
  node->token = tok;
  if (operand->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_logand(Token *tok, Node *lhs, Node *rhs, int label_index) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if ((!left_is_ptr && !is_scalar(lhs->type)) ||
      (!right_is_ptr && !is_scalar(rhs->type)))
    error(tok ? &tok->pos : NULL, "invalid operands to binary && operator");

  Type *type = base_type(TYPE_BOOL);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LOGAND, lhs, rhs, type);
  node->token = tok;
  node->label_index = label_index;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_logor(Token *tok, Node *lhs, Node *rhs, int label_index) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if ((!left_is_ptr && !is_scalar(lhs->type)) ||
      (!right_is_ptr && !is_scalar(rhs->type)))
    error(tok ? &tok->pos : NULL, "invalid operands to binary && operator");

  Type *type = base_type(TYPE_BOOL);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LOGOR, lhs, rhs, type);
  node->token = tok;
  node->label_index = label_index;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_lshift(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary << operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LSHIFT, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_rshift(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary >> operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_RSHIFT, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_bitand(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary & operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITAND, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_bitor(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary | operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITOR, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_bitxor(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error(tok ? &tok->pos : NULL, "invalid operands to binary ^ operator");
  Type *type = implicit_type_conversion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITXOR, lhs, rhs, type);
  node->token = tok;
  if (lhs->is_constant_expr && rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_bitnot(Token *tok, Node *operand) {
  if (!is_integer(operand->type))
    error(tok ? &tok->pos : NULL, "invalid operands to unary ~ operator");
  Node *node = new_node(ND_BITNOT, operand, NULL, operand->type);
  node->token = tok;
  if (operand->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_comma(Token *tok, Node *lhs, Node *rhs) {
  Type *type = rhs->type;
  Node *node = new_node(ND_COMMA, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_assign_ignore_const(Token *tok, Node *lhs, Node *rhs) {
  if ((is_struct_union(lhs->type) || is_struct_union(rhs->type)) &&
      !same_type(lhs->type, rhs->type))
    error(tok ? &tok->pos : NULL, "unmatched type");

  if (lhs->kind == ND_VAR && lhs->variable->kind == OBJ_FUNC)
    error(tok ? &tok->pos : NULL, "function cannot be a left-value");

  Type *type = lhs->type;
  while (type->kind == TYPE_ARRAY)
    type = type->base;

  if (!is_struct_union(lhs->type))
    rhs = new_node_cast(NULL, type, rhs);

  Node *node = new_node(ND_ASSIGN, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_assign(Token *tok, Node *lhs, Node *rhs) {
  if (lhs->kind == ND_VAR && lhs->variable->type->is_const)
    error(tok ? &tok->pos : NULL, "cannot assignning to a const variable");

  if (lhs->kind == ND_MEMBER && lhs->member->type->is_const)
    error(tok ? &tok->pos : NULL, "cannot assignning to a const member");

  if (lhs->kind == ND_MEMBER && lhs->lhs->kind == ND_VAR &&
      lhs->lhs->variable->type->is_const)
    error(tok ? &tok->pos : NULL, "cannot assignning to a const variable");

  return new_node_assign_ignore_const(tok, lhs, rhs);
}

Node *new_node_conditional(Token *tok, Node *cond, Node *lhs, Node *rhs,
                           int label_index) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  bool left_is_voidptr = left_is_ptr && lhs->type->base->kind == TYPE_VOID;
  bool right_is_voidptr = right_is_ptr && rhs->type->base->kind == TYPE_VOID;

  if ((left_is_ptr != right_is_ptr) ||
      (left_is_ptr && right_is_ptr && !left_is_voidptr && !right_is_voidptr &&
       !same_type(lhs->type->base, rhs->type->base)))
    error(tok ? &tok->pos : NULL, "invalid operands to conditional operator");

  if ((is_struct_union(lhs->type) || is_struct_union(rhs->type)) &&
      !same_type(lhs->type, rhs->type))
    error(tok ? &tok->pos : NULL, "unmatched type");

  if (!is_struct_union(lhs->type)) {
    Type *type = implicit_type_conversion(lhs->type, rhs->type);
    lhs = new_node_cast(NULL, type, lhs);
    rhs = new_node_cast(NULL, type, rhs);
  }

  Node *node = new_node(ND_COND, lhs, rhs, lhs->type);
  node->condition = new_node_cast(tok, base_type(TYPE_BOOL), cond);
  node->token = tok;
  node->label_index = label_index;

  if (cond->is_constant_expr && lhs->is_constant_expr &&
      rhs->is_constant_expr) {
    node->is_constant_expr = true;
    eval(node);
  }
  return node;
}

Node *new_node_member(Token *tok, Node *x, Member *y) {
  // struct member access (x.y)
  assert(is_struct_union(x->type));
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_MEMBER;
  node->type = y->type;
  node->lhs = x;
  node->member = y;
  node->token = tok;
  return node;
}

Node *new_node_var(Token *tok, Variable *var) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_VAR;
  node->variable = var;
  node->type =
      var->type->kind == TYPE_FUNC ? pointer_type(var->type) : var->type;
  node->token = tok;
  return node;
}

Node *new_node_array_access(Token *tok, Node *array, int idx) {
  assert(array->type->kind == TYPE_ARRAY);
  assert(idx >= 0);
  idx *= array->type->base->size;
  Type *type = pointer_type(array->type->base);
  Node *ofs = new_node_num_int(tok, idx);
  array = new_node_cast(NULL, type, array);
  ofs = new_node_cast(NULL, type, ofs);
  return new_node_deref(tok, new_node(ND_ADD, array, ofs, type));
}

Node *new_node_array_access_as_1D(Token *tok, Node *array, int idx) {
  assert(array->type->kind == TYPE_ARRAY);
  assert(idx >= 0);

  // treat as 1D-array even if array->type is multi-dimensional
  Type *base = array->type->base;
  while (base->kind == TYPE_ARRAY)
    base = base->base;
  idx *= base->size;

  Type *type = pointer_type(array->type->base);
  Node *ofs = new_node_num_int(tok, idx);
  array = new_node_cast(NULL, type, array);
  ofs = new_node_cast(NULL, type, ofs);
  return new_node_deref(tok, new_node(ND_ADD, array, ofs, type));
}

Variable *is_const_var_addr(Node *node) {
  assert(node);
  int count = 0;
  while (true) {
    switch (node->kind) {
    case ND_VAR:
      return (count == 1) ? node->variable : NULL;
    case ND_ADDR:
      count++;
      break;
    case ND_DEREF:
      count--;
      break;
    case ND_CAST:
      break;
    default:
      return NULL;
    }
    node = node->lhs;
  }
  return NULL;
}

Number *eval(Node *node) {
  assert(node->is_constant_expr);

  if (node->num)
    return node->num;

  switch (node->kind) {
  case ND_ADD:
    return node->num = number_add(eval(node->lhs), eval(node->rhs));
  case ND_SUB:
    return node->num = number_sub(eval(node->lhs), eval(node->rhs));
  case ND_MUL:
    return node->num = number_mul(eval(node->lhs), eval(node->rhs));
  case ND_DIV:
    return node->num = number_div(eval(node->lhs), eval(node->rhs));
  case ND_MOD:
    return node->num = number_mod(eval(node->lhs), eval(node->rhs));
  case ND_BITXOR:
    return node->num = number_bitxor(eval(node->lhs), eval(node->rhs));
  case ND_BITOR:
    return node->num = number_bitor(eval(node->lhs), eval(node->rhs));
  case ND_BITAND:
    return node->num = number_bitand(eval(node->lhs), eval(node->rhs));
  case ND_BITNOT:
    return node->num = number_bitnot(eval(node->lhs));
  case ND_LSHIFT:
    return node->num = number_lshift(eval(node->lhs), eval(node->rhs));
  case ND_RSHIFT:
    return node->num = number_rshift(eval(node->lhs), eval(node->rhs));
  case ND_EQ:
    return node->num = number_eq(eval(node->lhs), eval(node->rhs));
  case ND_NE:
    return node->num = number_ne(eval(node->lhs), eval(node->rhs));
  case ND_LT:
    return node->num = number_lt(eval(node->lhs), eval(node->rhs));
  case ND_LE:
    return node->num = number_le(eval(node->lhs), eval(node->rhs));
  case ND_COND:
    return node->num = number_cond(eval(node->condition), eval(node->lhs),
                                   eval(node->rhs));
  case ND_LOGNOT:
    return node->num = number_lognot(eval(node->lhs));
  case ND_LOGAND:
    return node->num = number_logand(eval(node->lhs), eval(node->rhs));
  case ND_LOGOR:
    return node->num = number_logor(eval(node->lhs), eval(node->rhs));
  case ND_CAST:
    return node->num = number_cast(eval(node->lhs), node->type);
  case ND_NUM:
    return node->num;
  default:
    assert(false);
  }
}
