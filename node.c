#include "boncc.h"
#include <stdlib.h>

Node *new_node_num(Token *tok, int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  node->type = base_type(TYPE_INT);
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
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr || right_is_ptr)
    error_at(tok->pos, "invalid operands to binary * operator");
  Node *node = new_node(ND_MUL, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_div(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr || right_is_ptr)
    error_at(tok->pos, "invalid operands to binary / operator");
  Node *node = new_node(ND_DIV, lhs, rhs, base_type(TYPE_INT));
  node->token = tok;
  return node;
}

Node *new_node_add(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;
  if (left_is_ptr && right_is_ptr)
    error_at(tok->pos,
             "invalid operands to binary + operator (pointer and pointer)");

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
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size),
                   base_type(TYPE_INT));
  else if (right_is_ptr)
    lhs = new_node(ND_MUL, lhs, new_node_num(NULL, rhs->type->base->size),
                   base_type(TYPE_INT));

  Node *node = new_node(ND_ADD, lhs, rhs, type);
  node->token = tok;
  return node;
}

Node *new_node_sub(Token *tok, Node *lhs, Node *rhs) {
  bool left_is_ptr =
      lhs->type->kind == TYPE_PTR || lhs->type->kind == TYPE_ARRAY;
  bool right_is_ptr =
      rhs->type->kind == TYPE_PTR || rhs->type->kind == TYPE_ARRAY;

  if (!left_is_ptr && right_is_ptr)
    error_at(tok->pos, "pointer is not allowed here");

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
    rhs = new_node(ND_MUL, rhs, new_node_num(NULL, lhs->type->base->size),
                   base_type(TYPE_INT));

  Node *node = new_node(ND_SUB, lhs, rhs, type);
  if (left_is_ptr && right_is_ptr)
    node =
        new_node(ND_DIV, node, new_node_num(NULL, lhs->type->base->size), type);
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
    error_at(tok->pos, "invalid unary * operands to non-pointer");
  Type *type = operand->type->base;
  Node *node = new_node(ND_DEREF, operand, NULL, type);
  node->token = tok;
  return node;
}

Node *new_node_assign(Token *tok, Node *lhs, Node *rhs) {
  Node *node = new_node(ND_ASSIGN, lhs, rhs, lhs->type);
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