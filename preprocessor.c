#include "boncc.h"
#include <assert.h>
#include <string.h>

static bool match_ident(Ident *ident, char *text) {
  int len = strlen(text);
  return len == ident->len && strncmp(ident->name, text, len) == 0;
}

Token *preprocess(Token *input) {
  assert(input);
  Token *next_token = input;
  Token head;
  head.next = NULL;
  Token *tail = &head;

  while (next_token->kind != TK_EOF) {
    if (next_token->kind == TK_HASH) {
      while (!next_token->at_eol)
        next_token = next_token->next;
      next_token = next_token->next;
      continue;
    }

    if (next_token->kind == TK_IDENT) {
      if (match_ident(next_token->ident, "__FILE__")) {
        next_token->kind = TK_STR;
        next_token->string_literal = next_token->pos.file_name;
      }
      if (match_ident(next_token->ident, "__LINE__")) {
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
