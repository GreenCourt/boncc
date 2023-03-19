#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

static Map *macros;

typedef struct Macro Macro;
struct Macro {
  String *ident;
  Token *body; // NULL-terminated linked list
  bool is_predefined;
  bool flag; // used in recursive manner
};

static Macro *new_macro(String *ident, Token *body) {
  Macro *m = calloc(1, sizeof(Macro));
  m->ident = ident;
  m->body = body;
  if (map_get(macros, ident))
    map_erase(macros, ident); // overwrite
  map_push(macros, ident, m);
  return m;
}

static Token *expand(Token *token);

static Token *expand_predefined(Token *orig, Macro *macro) {
  // expand orig and return tail of expanded tokens
  assert(macro->is_predefined);

  if (same_string_nt(macro->ident, "__FILE__")) {
    orig->kind = TK_STR;
    orig->string_literal = orig->pos.file_name;
    return orig;
  }

  if (same_string_nt(macro->ident, "__LINE__")) {
    orig->kind = TK_NUM;
    orig->val = orig->pos.line_number;
    orig->type = base_type(TYPE_INT);
    return orig;
  }

  assert(false);
  return NULL;
}

static Token *expand_user(Token *orig, Macro *macro) {
  // expand orig and return tail of expanded tokens
  assert(!macro->is_predefined);
  assert(macro->body);

  Token *nx = orig->next;
  Token head;
  head.next = NULL;
  Token *tail = &head;

  { // replace
    Token *b = macro->body;
    while (b) {
      tail->next = calloc(1, sizeof(Token));
      tail = tail->next;
      *tail = *b;
      tail->pos = orig->pos;
      b = b->next;
    }
    tail->next = nx;
  }

  { // recusive expansion
    Token *cur = head.next;
    assert(cur);
    while (cur != tail)
      cur = expand(cur)->next;
    tail = expand(cur);
  }

  *orig = *head.next;
  return tail;
}

static Token *expand(Token *token) {
  // expand orig and return tail of expanded tokens
  if (token->kind == TK_IDENT) {
    Macro *m = map_get(macros, token->str);
    if (m && !m->flag) {
      Token *tail = NULL;
      m->flag = true;
      if (m->is_predefined)
        tail = expand_predefined(token, m);
      else
        tail = expand_user(token, m);
      m->flag = false;
      return tail;
    }
  }

  return token; // unmodified
}

Token *directive(Token *next_token, Token **tail) {
  assert(next_token->kind == TK_HASH);
  next_token = next_token->next;
  if (!next_token->is_identifier)
    error(&next_token->pos, "invalid directive");

  if (same_string_nt(next_token->str, "define")) {
    if (next_token->at_eol)
      error(&next_token->pos, "identifier required after #define but not found.");

    Token *macro_ident = next_token->next;
    if (!macro_ident->is_identifier)
      error(&macro_ident->pos, "identifier expected but not found.");

    Token *macro_head = macro_ident->next;
    next_token = macro_head;
    while (!next_token->at_eol)
      next_token = next_token->next;
    Token *macro_tail = next_token;
    next_token = macro_tail->next;
    macro_tail->next = NULL;
    new_macro(macro_ident->str, macro_head);
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

  if (macros == NULL) {
    macros = new_map();
    static String idents[] = {
        {"__LINE__", 8},
        {"__FILE__", 8},
    };
    for (int i = 0; i < (int)(sizeof(idents) / sizeof(String)); ++i) {
      Macro *m = new_macro(&idents[i], NULL);
      m->is_predefined = true;
    }
  }

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

    tail->next = next_token;
    tail = expand(next_token);
    next_token = tail->next;
  }

  return head.next;
}
