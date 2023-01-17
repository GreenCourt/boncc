#include "boncc.h"
#include <assert.h>
#include <stdlib.h>

Node *new_node_num(Token *tok, int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = base_type(TYPE_INT);
  node->token = tok;
  return node;
}

Node *new_node_long(Token *tok, long long val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = base_type(TYPE_LONG);
  node->token = tok;
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
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr || right_is_ptr)
    error_at(&tok->pos, "invalid operands to binary * operator");
  Node *node = new_node(ND_MUL, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_div(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr || right_is_ptr)
    error_at(&tok->pos, "invalid operands to binary / operator");
  Node *node = new_node(ND_DIV, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_add(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr && right_is_ptr)
    error_at(&tok->pos, "invalid operands to binary + operator (pointer and pointer)");

  Type *type;
  if (lhs->type->kind == TYPE_PTR)
    type = lhs->type;
  else if (rhs->type->kind == TYPE_PTR)
    type = rhs->type;
  else if (lhs->type->kind == TYPE_ARRAY)
    type = pointer_type(lhs->type->base); // implicitly cast
  else if (rhs->type->kind == TYPE_ARRAY)
    type = pointer_type(rhs->type->base); // implicitly cast
  else
    type = lhs->type;

  if (left_is_ptr)
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size), base_type(TYPE_INT));
  else if (right_is_ptr)
    lhs = new_node(ND_MUL, lhs, new_node_num(NULL, rhs->type->base->size), base_type(TYPE_INT));

  Node *node = new_node(ND_ADD, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_sub(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr = lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr = rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (!left_is_ptr && right_is_ptr)
    error_at(&tok->pos, "pointer is not allowed here");

  Type *type;
  if (left_is_ptr && right_is_ptr)
    type = base_type(TYPE_INT);
  else if (lhs->type->kind == TYPE_PTR)
    type = lhs->type;
  else if (lhs->type->kind == TYPE_ARRAY)
    type = pointer_type(lhs->type->base); // implicitly cast
  else
    type = lhs->type;

  if (left_is_ptr && !right_is_ptr)
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size), base_type(TYPE_INT));

  Node *node = new_node(ND_SUB, lhs, rhs, type);
  if (left_is_ptr && right_is_ptr)
    node = new_node(ND_DIV, node, new_node_num(NULL, lhs->type->base->size), type);
  node->token = tok;
  return node;
}

Node *new_node_eq(Token *tok, Node *lhs, Node *rhs) {
  Node *node = new_node(ND_EQ, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_ne(Token *tok, Node *lhs, Node *rhs) {
  Node *node = new_node(ND_NE, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_lt(Token *tok, Node *lhs, Node *rhs) {
  Node *node = new_node(ND_LT, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_le(Token *tok, Node *lhs, Node *rhs) {
  Node *node = new_node(ND_LE, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_addr(Token *tok, Node *operand) {
  Type *type = pointer_type(operand->type);
  Node *node = new_node(ND_ADDR, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_deref(Token *tok, Node *operand) {
  if ((operand->type->kind != TYPE_PTR) && (operand->type->kind != TYPE_ARRAY))
    error_at(&tok->pos, "invalid unary * operands to non-pointer");
  Type *type = operand->type->base;
  Node *node = new_node(ND_DEREF, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_assign(Token *tok, Node *lhs, Node *rhs) {
  Type *type = lhs->type;
  while (type->kind == TYPE_ARRAY)
    type = type->base;
  Node *node = new_node(ND_ASSIGN, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_conditional(Token *tok, Node *cond, Node *lhs, Node *rhs, int label_index) {
  Node *node = new_node(ND_COND, lhs, rhs, lhs->type);
  node->condition = cond;
  node->token = tok;
  node->label_index = label_index;
  return node;
}

Node *new_node_member(Token *tok, Node *x, Member *y) {
  // struct member access (x.y)
  assert(x->type->kind == TYPE_STRUCT);
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
  return new_node_assign(NULL, new_node_deref(NULL, new_node(ND_ADD, new_node_var(NULL, var), new_node_num(NULL, idx), pointer_type(var->type->base))), expr);
}

Node *new_node_array_set_val(Variable *var, int idx, int val) {
  assert(var->type->kind == TYPE_ARRAY);
  assert(idx >= 0);
  return new_node_array_set_expr(var, idx, new_node_num(NULL, val));
}

int is_constant_number(Node *node) {
  assert(node);
  switch (node->kind) {
  case ND_ADD:
  case ND_SUB:
  case ND_MUL:
  case ND_DIV:
  case ND_EQ:
  case ND_NE:
  case ND_LT:
  case ND_LE:
    return is_constant_number(node->lhs) && is_constant_number(node->rhs);
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
  case ND_EQ:
    return eval(node->lhs) == eval(node->rhs);
  case ND_NE:
    return eval(node->lhs) != eval(node->rhs);
  case ND_LT:
    return eval(node->lhs) < eval(node->rhs);
  case ND_LE:
    return eval(node->lhs) <= eval(node->rhs);
  case ND_NUM:
    return node->val;
  default:
    error_at(&node->token->pos, "not a constant expr");
    return 0;
  }
}
