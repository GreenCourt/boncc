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
  bool is_function_like;
  int nparams; // function-like
  bool flag;   // used in recursive manner
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

static Token *expand_object_like(Token *prev, Macro *macro) {
  // expand prev->next and return the tail of expanded tokens
  assert(!macro->is_predefined);
  assert(!macro->is_function_like);
  Token *token = prev->next;
  Token *nx = token->next;

  if (macro->body == NULL) { // empty macro
    prev->next = nx;
    return prev;
  }

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

  if (tail == &head) { // detect empty macro after expansion
    prev->next = tail->next;
    return prev;
  }

  *token = *head.next;
  return tail;
}

static Token *expand_function_like(Token *prev, Macro *macro) {
  // expand prev->next and return the tail of expanded tokens
  assert(!macro->is_predefined);
  assert(macro->is_function_like);

  Token *lparen = prev->next->next;
  if (lparen->kind != TK_LPAREN)
    error(&lparen->pos, "left-paren expected for a function-like marco");

  Token *rparen = NULL;
  Vector *args = args = new_vector(0, sizeof(Token *));
  if (lparen->next->kind == TK_RPAREN) {
    rparen = lparen->next;
  } else if (lparen->next->kind == TK_COMMA) {
    error(&lparen->next->pos, "invalid macro expansion");
  } else {
    // read args
    Token *p = lparen;
    vector_push(args, &p->next);
    Vector *stack = new_vector(0, sizeof(Token *));
    while (true) {
      if (p->kind == TK_EOF)
        error(&p->pos, "invalid macro expansion");

      if (stack->size == 0 && p->next->kind == TK_COMMA) {
        Token *q = p->next;
        vector_push(args, &p->next->next);
        p->next = NULL;
        p = q;
        continue;
      }

      if (stack->size == 0 && p->next->kind == TK_RPAREN) {
        rparen = p->next;
        p->next = NULL;
        break;
      }

      { // paren, brace, bracket
        if (p->next->kind == TK_LBRACKET || p->next->kind == TK_LBRACE || p->next->kind == TK_LPAREN) {
          vector_push(stack, &(p->next));
          p = p->next;
          continue;
        }
        TokenKind left[] = {TK_LBRACE, TK_LBRACKET, TK_LPAREN};
        TokenKind right[] = {TK_RBRACE, TK_RBRACKET, TK_RPAREN};
        bool cont = false;
        for (int i = 0; i < 3; ++i) {
          if (p->next->kind == right[i]) {
            if (stack->size == 0 || ((*(Token **)vector_last(stack))->kind != left[i]))
              error(&p->next->pos, "invalid macro expansion");
            vector_pop(stack);
            cont = true;
            break;
          }
        }
        if (cont) {
          p = p->next;
          continue;
        }
      }
      p = p->next;
    }
  }

  assert(rparen);
  if (args->size != macro->nparams)
    error(&rparen->pos, "invalid number of arguments for macro expansion");

  Token *nx = rparen->next;

  if (macro->body == NULL) { // empty macro
    prev->next = nx;
    return prev;
  }

  Token head;
  head.next = NULL;
  Token *tail = &head;

  { // replace
    Token *b = macro->body;
    while (b) {
      if (b->idx) { // expand an argument
        Token *a = *(Token **)vector_get(args, b->idx - 1 /* 1-indexed */);
        while (a) {
          tail->next = calloc(1, sizeof(Token));
          tail = tail->next;
          *tail = *a;
          tail->pos = prev->next->pos;
          a = a->next;
        }
      } else {
        tail->next = calloc(1, sizeof(Token));
        tail = tail->next;
        *tail = *b;
        tail->pos = prev->next->pos;
      }
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

  if (tail == &head) { // detect empty macro after expansion
    prev->next = tail->next;
    return prev;
  }

  *(prev->next) = *head.next;
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
      else if (m->is_function_like)
        tail = expand_function_like(prev, m);
      else
        tail = expand_object_like(prev, m);
      m->flag = false;
      return tail;
    }
  }

  return token; // unmodified
}

Token *define_macro(Token *prev) {
  assert(prev->next->kind == TK_HASH);
  Token *directive = prev->next->next;
  assert(same_string_nt(directive->str, "define"));

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

  if (macro_ident->has_right_space || macro_ident->next->kind != TK_LPAREN) {
    // object-like macro
    Token *macro_head = macro_ident->next;
    Token *macro_tail = macro_head;
    while (!macro_tail->at_eol)
      macro_tail = macro_tail->next;

    prev->next = macro_tail->next;
    macro_tail->next = NULL;
    new_macro(macro_ident->str, macro_head);
    return prev;
  }

  // function-like macro
  Token *lparen = macro_ident->next;
  Token *rparen = NULL;
  Vector *params; // Token
  if (lparen->at_eol)
    error(&lparen->pos, "invalid function-like macro");

  if (lparen->next->kind == TK_RPAREN) {
    rparen = lparen->next;
  } else {
    // read params
    params = new_vector(0, sizeof(Token *));
    Token *p = lparen;
    do {
      p = p->next;
      if (p->at_eol || !p->is_identifier)
        error(&p->pos, "invalid function-like macro");
      vector_push(params, &p);
      p = p->next;
    } while (p->kind == TK_COMMA);
    if (p->kind != TK_RPAREN)
      error(&p->pos, "invalid function-like macro");
    rparen = p;
  }

  if (rparen->at_eol) { // empty macro
    Macro *m = new_macro(macro_ident->str, NULL);
    m->is_function_like = true;
    m->nparams = params->size;
    prev->next = rparen->next;
    return prev;
  }

  Token *macro_tail = rparen;
  while (!macro_tail->at_eol) {
    macro_tail = macro_tail->next;
    if (macro_tail->is_identifier) {
      for (int i = 0; i < params->size; i++) {
        Token *p = *(Token **)vector_get(params, i);
        if (same_string(macro_tail->str, p->str)) {
          macro_tail->idx = i + 1; // 1-indexed
          break;
        }
      }
    }
  }
  prev->next = macro_tail->next;
  macro_tail->next = NULL;
  Macro *m = new_macro(macro_ident->str, rparen->next);
  m->is_function_like = true;
  m->nparams = params->size;
  return prev;
}

Token *process_directive(Token *prev) {
  assert(prev->next->kind == TK_HASH);
  Token *directive = prev->next->next;
  if (!directive->is_identifier)
    error(&directive->pos, "invalid directive");

  // #define
  if (same_string_nt(directive->str, "define"))
    return define_macro(prev);

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
