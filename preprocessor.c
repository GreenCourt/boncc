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

static Token *expand(Token *prev);

static Token *expand_predefined(Token *prev, Macro *macro) {
  // expand prev->next and return the tail of expanded tokens
  assert(macro->is_predefined);

  Token *token = prev->next;

  if (same_string_nt(macro->ident, "__FILE__")) {
    token->kind = TK_STR;
    token->string_literal = token->pos.file_name;
    return token;
  }

  if (same_string_nt(macro->ident, "__LINE__")) {
    token->kind = TK_NUM;
    token->val = token->pos.line_number;
    token->type = base_type(TYPE_INT);
    return token;
  }

  assert(false);
  return NULL;
}

static Token *expand_user(Token *prev, Macro *macro) {
  // expand prev->next and return the tail of expanded tokens
  assert(!macro->is_predefined);
  Token *token = prev->next;
  Token *nx = token->next;
  Token head;
  head.next = NULL;
  Token *tail = &head;

  { // replace
    Token *b = macro->body;
    while (b) {
      tail->next = calloc(1, sizeof(Token));
      tail = tail->next;
      *tail = *b;
      tail->pos = token->pos;
      b = b->next;
    }
    tail->next = nx;
  }

  if (tail != &head) { // recusive expansion
    Token *t = &head;
    while (t->next != tail)
      t = expand(t);
    tail = expand(t);
  }

  if (tail == &head) { // empty macro
    prev->next = token->next;
    return prev;
  }

  *token = *head.next;
  return tail;
}

static Token *expand(Token *prev) {
  // expand prev->next and return the tail of expanded tokens
  Token *token = prev->next;
  if (token->kind == TK_IDENT) {
    Macro *m = map_get(macros, token->str);
    if (m && !m->flag) {
      Token *tail = NULL;
      m->flag = true;
      if (m->is_predefined)
        tail = expand_predefined(prev, m);
      else
        tail = expand_user(prev, m);
      m->flag = false;
      return tail;
    }
  }

  return token; // unmodified
}

Token *process_directive(Token *prev) {
  assert(prev->next->kind == TK_HASH);
  Token *directive = prev->next->next;
  if (!directive->is_identifier)
    error(&directive->pos, "invalid directive");

  // #define
  if (same_string_nt(directive->str, "define")) {
    if (directive->at_eol)
      error(&directive->pos, "identifier required after #define but not found.");

    Token *macro_ident = directive->next;
    if (!macro_ident->is_identifier)
      error(&macro_ident->pos, "identifier expected but not found.");

    if (macro_ident->at_eol) { // empty macro
      new_macro(macro_ident->str, NULL);
      prev->next = macro_ident->next;
      return prev;
    }

    Token *macro_head = macro_ident->next;
    Token *macro_tail = macro_head;
    while (!macro_tail->at_eol)
      macro_tail = macro_tail->next;

    prev->next = macro_tail->next;
    macro_tail->next = NULL;
    new_macro(macro_ident->str, macro_head);
    return prev;
  }

  // currently, ignore unknown directives
  Token *nx = directive;
  while (!nx->at_eol)
    nx = nx->next;
  prev->next = nx->next;
  return prev;

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

  Token head;
  head.next = input;
  Token *tail = &head;

  while (tail->next->kind != TK_EOF) {
    if (tail->next->kind == TK_HASH) {
      if (!tail->next->at_bol)
        error(&tail->next->pos, "invalid # here");
      tail = process_directive(tail);
      continue;
    }

    tail = expand(tail);
  }

  return head.next;
}
