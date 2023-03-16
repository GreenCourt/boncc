#include "boncc.h"
#include <assert.h>
#include <string.h>

static Map *user_macros;

Token *directive(Token *next_token, Token **tail) {
  assert(next_token->kind == TK_HASH);
  next_token = next_token->next;
  if (!next_token->is_identifier)
    error(&next_token->pos, "invalid directive");

  if (same_string_nt(next_token->str, "define")) {
    Token *macro_name = next_token->next;
    if (!macro_name->is_identifier)
      error(&macro_name->pos, "identifier expected but not found.");
    Token *macro_head = macro_name->next;
    next_token = macro_head;
    while (!next_token->at_eol)
      next_token = next_token->next;
    Token *macro_tail = next_token;
    next_token = macro_tail->next;
    macro_tail->next = NULL;
    map_push(user_macros, macro_name->str, macro_head);
    (*tail)->next = next_token;
    return next_token;
  }

  // currently, ignore unknown directives
  while (!next_token->at_eol)
    next_token = next_token->next;
  next_token = next_token->next;
  (*tail)->next = next_token;
  return next_token;

  // error(&next_token->pos, "unknown directive");
  // return NULL;
}

Token *preprocess(Token *input) {
  assert(input);
  if (user_macros == NULL)
    user_macros = new_map();

  Token *next_token = input;
  Token head;
  head.next = NULL;
  Token *tail = &head;

  while (next_token->kind != TK_EOF) {
    if (next_token->kind == TK_HASH) {
      if (!next_token->at_bol)
        error(&next_token->pos, "invalid # here");
      next_token = directive(next_token, &tail);
      continue;
    }

    if (next_token->kind == TK_IDENT) {
      if (same_string_nt(next_token->str, "__FILE__")) {
        next_token->kind = TK_STR;
        next_token->string_literal = next_token->pos.file_name;
      }
      if (same_string_nt(next_token->str, "__LINE__")) {
        next_token->kind = TK_NUM;
        next_token->val = next_token->pos.line_number;
        next_token->type = base_type(TYPE_INT);
      }
    }

    tail->next = next_token;
    tail = next_token;
    next_token = next_token->next;
  }
  return head.next;
}
