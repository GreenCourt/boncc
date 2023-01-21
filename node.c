#include "boncc.h"
#include <assert.h>
#include <stdlib.h>

Node *new_node_nop() {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NOP;
  return node;
}

Node *new_node_num(Token *tok, long long val, Type *type) {
  assert(is_integer(type) || type->kind == TYPE_PTR);
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = type;
  node->token = tok;
  return node;
}

Node *new_node_cast(Token *tok, Type *type, Node *operand) {
  if (type->kind == TYPE_ARRAY)
    error_at(&tok->pos, "invalid type casting to array type");

  if (type->kind != TYPE_PTR && !is_integer(type))
    error_at(&tok->pos, "invalid type casting");

  bool op_is_ptr = operand->type->kind == TYPE_PTR || operand->type->kind == TYPE_ARRAY;
  if (!op_is_ptr && !is_integer(operand->type))
    error_at(&tok->pos, "invalid type casting");

  if (same_type(type, operand->type))
    return operand;

  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_CAST;
  node->token = tok;
  node->type = type;
  node->lhs = operand;
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
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary * operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_MUL, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_div(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary / operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_DIV, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_mod(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary % operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_MOD, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_add(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr && right_is_ptr)
    error_at(&tok->pos, "invalid operands to binary + operator (pointer and pointer)");
  if (!left_is_ptr && !is_integer(lhs->type))
    error_at(&tok->pos, "invalid operands to binary + operator");
  if (!right_is_ptr && !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary + operator");

  if (left_is_ptr) {
    rhs = new_node_cast(NULL, base_type(TYPE_LONG), rhs);
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size, rhs->type), rhs->type);
  } else if (right_is_ptr) {
    lhs = new_node_cast(NULL, base_type(TYPE_LONG), lhs);
    lhs = new_node(ND_MUL, lhs, new_node_num(NULL, rhs->type->base->size, lhs->type), lhs->type);
  }

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_ADD, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_sub(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (!left_is_ptr && right_is_ptr)
    error_at(&tok->pos, "invalid operands to binary - operator");
  if (!left_is_ptr && !is_integer(lhs->type))
    error_at(&tok->pos, "invalid operands to binary - operator");
  if (!right_is_ptr && !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary - operator");

  if (left_is_ptr && !right_is_ptr) {
    rhs = new_node_cast(NULL, base_type(TYPE_PTR), rhs);
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size, rhs->type), rhs->type);
  }

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_SUB, lhs, rhs, type);
  if (left_is_ptr && right_is_ptr)
    node = new_node(ND_DIV, node, new_node_num(NULL, lhs->type->base->size, type), type);
  node->token = tok;
  return node;
}

Node *new_node_eq(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  bool left_is_zero = lhs->kind == ND_NUM && lhs->val == 0;
  bool right_is_zero = rhs->kind == ND_NUM && rhs->val == 0;

  if (!is_integer(lhs->type) && left_is_ptr)
    error_at(&tok->pos, "invalid operands to == operator");

  if (!is_integer(rhs->type) && right_is_ptr)
    error_at(&tok->pos, "invalid operands to == operator");

  if (left_is_ptr && (!right_is_ptr && !right_is_zero))
    error_at(&tok->pos, "invalid operands to == operator (pointer and non-pointer)");

  if (right_is_ptr && (!left_is_ptr && !left_is_zero))
    error_at(&tok->pos, "invalid operands to == operator (pointer and non-pointer)");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_EQ, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_ne(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  bool left_is_zero = lhs->kind == ND_NUM && lhs->val == 0;
  bool right_is_zero = rhs->kind == ND_NUM && rhs->val == 0;

  if (!is_integer(lhs->type) && left_is_ptr)
    error_at(&tok->pos, "invalid operands to == operator");

  if (!is_integer(rhs->type) && right_is_ptr)
    error_at(&tok->pos, "invalid operands to == operator");

  if (left_is_ptr && (!right_is_ptr && !right_is_zero))
    error_at(&tok->pos, "invalid operands to == operator (pointer and non-pointer)");

  if (right_is_ptr && (!left_is_ptr && !left_is_zero))
    error_at(&tok->pos, "invalid operands to == operator (pointer and non-pointer)");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_NE, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_lt(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (!is_integer(lhs->type) && left_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  if (!is_integer(rhs->type) && right_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  if (left_is_ptr != right_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LT, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_le(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (!is_integer(lhs->type) && left_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  if (!is_integer(rhs->type) && right_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  if (left_is_ptr != right_is_ptr)
    error_at(&tok->pos, "invalid operands to relational operator");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LE, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_addr(Token *tok, Node *operand) {
  if (operand->kind == ND_NUM)
    error_at(&tok->pos, "invalid unary & operand");
  Type *type = pointer_type(operand->type);
  Node *node = new_node(ND_ADDR, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_deref(Token *tok, Node *operand) {
  if ((operand->type->kind != TYPE_PTR) && (operand->type->kind != TYPE_ARRAY))
    error_at(&tok->pos, "invalid unary * operand");
  Type *type = operand->type->base;
  Node *node = new_node(ND_DEREF, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_lognot(Token *tok, Node *operand) {
  bool is_ptr = operand->type->kind == TYPE_PTR || operand->type->kind == TYPE_ARRAY;
  if (!is_integer(operand->type) && !is_ptr)
    error_at(&tok->pos, "invalid operand to unary ! operator");
  Node *node = new_node(ND_LOGNOT, operand, NULL, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_logand(Token *tok, Node *lhs, Node *rhs, int label_index) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (!left_is_ptr && !is_integer(lhs->type))
    error_at(&tok->pos, "invalid operands to binary && operator");
  if (!right_is_ptr && !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary && operator");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LOGAND, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  node->label_index = label_index;
  return node;
}

Node *new_node_logor(Token *tok, Node *lhs, Node *rhs, int label_index) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (!left_is_ptr && !is_integer(lhs->type))
    error_at(&tok->pos, "invalid operands to binary && operator");
  if (!right_is_ptr && !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary && operator");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LOGOR, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  node->label_index = label_index;
  return node;
}

Node *new_node_lshift(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary << operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_LSHIFT, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_rshift(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary >> operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_RSHIFT, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_bitand(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary & operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITAND, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_bitor(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary | operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITOR, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_bitxor(Token *tok, Node *lhs, Node *rhs) {
  if (!is_integer(lhs->type) || !is_integer(rhs->type))
    error_at(&tok->pos, "invalid operands to binary ^ operator");
  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_BITXOR, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_bitnot(Token *tok, Node *operand) {
  if (!is_integer(operand->type))
    error_at(&tok->pos, "invalid operands to unary ~ operator");
  Node *node = new_node(ND_BITNOT, operand, NULL, operand->type);
  node->token = tok;
  return node;
}

Node *new_node_assign(Token *tok, Node *lhs, Node *rhs) {
  if (lhs->type->kind == TYPE_STRUCT || lhs->type->kind == TYPE_UNION || rhs->type->kind == TYPE_STRUCT || rhs->type->kind == TYPE_UNION)
    error_at(&tok->pos, "assignning to/from struct/union is currentry not supported");
  Type *type = lhs->type;
  while (type->kind == TYPE_ARRAY)
    type = type->base;
  Node *node = new_node(ND_ASSIGN, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_conditional(Token *tok, Node *cond, Node *lhs, Node *rhs, int label_index) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (left_is_ptr != right_is_ptr)
    error_at(&tok->pos, "invalid operands to conditional operator");

  if (lhs->type->kind == TYPE_STRUCT || lhs->type->kind == TYPE_UNION || rhs->type->kind == TYPE_STRUCT || rhs->type->kind == TYPE_UNION)
    error_at(&tok->pos, "struct/union is currentry not supported for conditional operator");

  Type *type = implicit_type_convertion(lhs->type, rhs->type);
  lhs = new_node_cast(NULL, type, lhs);
  rhs = new_node_cast(NULL, type, rhs);
  Node *node = new_node(ND_COND, lhs, rhs, lhs->type);
  node->condition = cond;
  node->token = tok;
  node->label_index = label_index;
  return node;
}

Node *new_node_member(Token *tok, Node *x, Member *y) {
  // struct member access (x.y)
  assert(x->type->kind == TYPE_STRUCT || x->type->kind == TYPE_UNION);
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
  node->type = var->type;
  node->token = tok;
  return node;
}

Node *new_node_array_set_expr(Variable *var, int idx, Node *expr) {
  assert(var->type->kind == TYPE_ARRAY);
  assert(idx >= 0);
  Type *base = var->type->base;
  while (base->kind == TYPE_ARRAY)
    base = base->base;
  idx *= base->size;
  return new_node_assign(NULL, new_node_deref(NULL, new_node(ND_ADD, new_node_var(NULL, var), new_node_num(NULL, idx, base_type(TYPE_INT)), pointer_type(var->type->base))), expr);
}

Node *new_node_array_set_val(Variable *var, int idx, int val) {
  assert(var->type->kind == TYPE_ARRAY);
  assert(idx >= 0);
  return new_node_array_set_expr(var, idx, new_node_num(NULL, val, base_type(TYPE_INT)));
}

int is_constant_number(Node *node) {
  assert(node);
  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_MOD:
  case ND_BITXOR:
  case ND_BITOR:
  case ND_BITAND:
  case ND_LSHIFT:
  case ND_RSHIFT:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LOGAND:
  case ND_LOGOR:
  case ND_LE:
    return is_constant_number(node->lhs) && is_constant_number(node->rhs);
  case ND_LOGNOT:
  case ND_BITNOT:
  case ND_CAST:
    return is_constant_number(node->lhs);
  case ND_COND:
    return is_constant_number(node->condition) && is_constant_number(node->lhs) && is_constant_number(node->rhs);
  case ND_NUM:
    return true;
  default:
    return false;
  }
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
  case ND_MOD:
    return eval(node->lhs) % eval(node->rhs);
  case ND_BITXOR:
    return eval(node->lhs) ^ eval(node->rhs);
  case ND_BITOR:
    return eval(node->lhs) | eval(node->rhs);
  case ND_BITAND:
    return eval(node->lhs) & eval(node->rhs);
  case ND_BITNOT:
    return ~eval(node->lhs);
  case ND_LSHIFT:
    return eval(node->lhs) << eval(node->rhs);
  case ND_RSHIFT:
    return eval(node->lhs) >> eval(node->rhs);
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_LE:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_COND:
    return eval(node->condition) ? eval(node->lhs) : eval(node->rhs);
  case ND_LOGNOT:
    return !eval(node->lhs);
  case ND_LOGAND:
    return eval(node->lhs) && eval(node->rhs);
  case ND_LOGOR:
    return eval(node->lhs) || eval(node->rhs);
  case ND_CAST:
    return eval(node->lhs);
  case ND_NUM:
    return node->val;
  default:
    error_at(&node->token->pos, "not a constant expr");
    return 0;
  }
}
