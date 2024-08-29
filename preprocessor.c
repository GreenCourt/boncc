#include "boncc.h"
#include <assert.h>
#include <libgen.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

// builtin-headers.o -->
extern char stdarg_h[];
extern char stddef_h[];
extern char stdbool_h[];
// <-- builtin-headers.o

Node *expr(Token **nx);   // parse.c
Number *eval(Node *node); // node.c

static Map *macros;
static Vector *included_files; // *char

typedef struct Macro Macro;
struct Macro {
  String *ident;
  Token *body; // NULL-terminated linked list
  bool is_dynamic;
  bool is_function_like;
  int nparams; // function-like
  bool is_variadic;
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

static void init_macro() {
  if (macros != NULL)
    return;

  macros = new_map();
  {
    static String idents[] = {
        {"__LINE__", 8},
        {"__FILE__", 8},
    };
    for (int i = 0; i < (int)(sizeof(idents) / sizeof(String)); ++i) {
      Macro *m = new_macro(&idents[i], NULL);
      m->is_dynamic = true;
    }
  }

  {
    static String idents[] = {
        {"__x86_64__", 10},
        {"__LP64__", 8},
    };
    for (int i = 0; i < (int)(sizeof(idents) / sizeof(String)); ++i)
      new_macro(&idents[i], NULL);
  }
}

static Token *expand(Token *prev, Token **stop);

static Token *expand_dynamic(Token *prev, Macro *macro) {
  // expand prev->next and return the tail of expanded tokens
  assert(macro->is_dynamic);

  Token *token = prev->next;

  if (same_string_nt(macro->ident, "__FILE__")) {
    token->kind = TK_STR;
    token->string_literal = token->pos.file_name;
    return token;
  }

  if (same_string_nt(macro->ident, "__LINE__")) {
    token->kind = TK_NUM;
    token->num = new_number_int(token->pos.line_number);
    token->type = token->num->type;
    return token;
  }

  assert(false);
  return NULL;
}

Token *operator_double_hash(Token const *left, Token const *right) {
  assert(left->next->kind == TK_2HASH);
  assert(left->next->next == right);
  int len = left->str->len + right->str->len;
  char *p = calloc(len + 2, sizeof(char));
  char *q = p + snprintf(p, left->str->len + 1, "%.*s", left->str->len,
                         left->str->str);
  snprintf(q, right->str->len + 1, "%.*s", right->str->len, right->str->str);
  p[len] = '\n';
  p[len + 1] = '\0';
  Token *t = tokenize(p, left->pos.file_name);
  t->pos = left->pos;
  Token *tail = t;
  while (tail->next->kind != TK_EOF) {
    tail = tail->next;
    tail->pos = left->pos;
  }
  tail->pos = left->pos;
  tail->next = right->next;
  return t;
}

static Token *expand_object_like(Token *prev, Macro *macro, Token **stop) {
  // expand prev->next and return the tail of expanded tokens
  assert(!macro->is_dynamic);
  assert(!macro->is_function_like);
  Token *expanded = prev->next;

  if (macro->body == NULL) { // empty macro
    prev->next = expanded->next;
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
      tail->pos = expanded->pos;
      b = b->next;
    }
    assert(tail->next == NULL);
    assert(&head != tail);
  }

  { // ## operator
    tail = &head;
    while (tail->next) {
      if (tail->next->kind != TK_2HASH) {
        tail = tail->next;
        continue;
      }
      if (tail == &head || tail->next->next == NULL)
        error(&expanded->pos, "## operator is not allowed here");

      *tail = *operator_double_hash(tail, tail->next->next);
    }
    assert(tail->next == NULL);
    assert(&head != tail);
  }

  tail->next = expanded->next;

  if (stop == NULL)
    stop = &expanded->next; // maybe modified by recursive expansion

  { // recursive expansion
    macro->flag = true;
    Token *t = &head;
    while (t->next != expanded->next && t->next != *stop)
      t = expand(t, stop);
    tail = t;
    macro->flag = false;
  }

  if (head.next == expanded->next) { // detect empty macro after expansion
    prev->next = expanded->next;
    return prev;
  }

  if (head.next == *stop) { // detect empty macro after expansion
    prev->next = *stop;
    return prev;
  }

  assert(head.next);
  prev->next = head.next;
  return tail;
}

static Token *clone_macro_argument(Token *head, Token *arg, Position *pos) {
  // clone a macro argument, link it to head->next
  // and return the tail of the cloned argument
  assert(arg != NULL);
  Token *tail = head;
  while (arg) {
    tail->next = calloc(1, sizeof(Token));
    tail = tail->next;
    *tail = *arg;
    tail->pos = *pos;
    arg = arg->next;
  }
  assert(tail != head);
  assert(tail->next == NULL);
  return tail;
}

static Token *clone_va_args(Token *head, Macro *macro, Vector *args,
                            Position *pos) {
  // clone arguments of variadic part, link them to head->next
  // and return the tail of the cloned arguments
  Token *tail = head;
  assert(macro->is_variadic);

  int nargs = (macro->nparams == 0 && args->size == 1 &&
               (*(Token **)vector_get(args, 0))->kind == TK_EMPTY)
                  ? 0
                  : args->size;
  if (macro->nparams == nargs) {
    // arguments of variadic part are not given.
    return head;
  }

  for (int i = macro->nparams; i < args->size; ++i) {
    tail = clone_macro_argument(tail, *(Token **)vector_get(args, i), pos);
    if (i != args->size - 1) {
      tail->next = calloc(1, sizeof(Token));
      tail = tail->next;
      tail->pos = *pos;
      tail->kind = TK_COMMA;
    }
  }
  assert(tail != head);
  assert(tail->next == NULL);
  return tail;
}

static Token *operator_hash(Token *operand) {
  assert(operand != NULL);
  size_t len = 0;
  for (Token *tk = operand; tk != NULL; tk = tk->next) {
    if (tk->kind == TK_EMPTY)
      continue;

    len += tk->has_right_space && (tk->next != NULL);

    if (tk->kind == TK_STR) {
      char *strlit = tk->string_literal;
      int slen = strlen(strlit);
      len += slen;
      for (int i = 0; i < slen; ++i)
        len += (strlit[i] == '\\' || strlit[i] == '"');
      len += 4; // backslash + double quote at left and right
    } else if (tk->kind == TK_NUM && tk->num->type->kind == TYPE_CHAR) {
      // char literal
      assert(tk->str->len <= 2);
      if (tk->str->len == 2) { // escaped character
        assert(tk->str->str[0] == '\\');
        len += 5 + (tk->str->str[1] == '\\');
      } else if (number2char(tk->num) == '"') {
        len += 4;
      } else {
        len += 3;
      }
    } else {
      len += tk->str->len;
    }
  }

  char *string = calloc(len + 1, sizeof(char));
  int ofs = 0;
  for (Token *tk = operand; tk != NULL; tk = tk->next) {
    if (tk->kind == TK_EMPTY)
      continue;

    if (tk->kind == TK_STR) {
      char *strlit = tk->string_literal;
      int slen = strlen(strlit);
      string[ofs++] = '\\';
      string[ofs++] = '"';

      for (int i = 0; i < slen; ++i) {
        if (strlit[i] == '\\' || strlit[i] == '"')
          string[ofs++] = '\\';
        string[ofs++] = strlit[i];
      }

      string[ofs++] = '\\';
      string[ofs++] = '"';
    } else if (tk->kind == TK_NUM && tk->num->type->kind == TYPE_CHAR) {
      char *str = tk->str->str;
      int slen = tk->str->len;
      assert(slen <= 2);

      string[ofs++] = '\'';
      if (slen == 2) {
        assert(str[0] == '\\');
        string[ofs++] = '\\';
        string[ofs++] = '\\';
        if (str[1] == '\\')
          string[ofs++] = '\\';
        string[ofs++] = str[1];
      } else if (number2char(tk->num) == '"') {
        string[ofs++] = '\\';
        string[ofs++] = '"';
      } else {
        string[ofs++] = str[0];
      }
      string[ofs++] = '\'';
    } else {
      strncpy(string + ofs, tk->str->str, tk->str->len);
      ofs += tk->str->len;
    }

    if (tk->has_right_space && (tk->next != NULL))
      string[ofs++] = ' ';
  }
  assert(strlen(string) == len);

  Token *tk = calloc(1, sizeof(Token));
  tk->kind = TK_STR;
  tk->is_identifier = false;
  tk->pos = operand->pos;
  tk->next = NULL;
  tk->str = new_string(string, len);
  tk->string_literal = string;
  return tk;
}

static Token *expand_function_like(Token *prev, Macro *macro, Token **stop) {
  // expand prev->next and return the tail of expanded tokens
  assert(!macro->is_dynamic);
  assert(macro->is_function_like);

  Token *expanded = prev->next;
  Token *lparen = expanded->next;
  if (lparen->kind != TK_LPAREN)
    error(&lparen->pos, "left-paren expected for a function-like marco");

  bool need_to_extend_stop = stop == NULL || lparen == *stop;

  Token *rparen = NULL;
  Vector *args = new_vector(0, sizeof(Token *));
  {
    // read args
    Token *prev_sep = lparen;
    Token *arg_tail = lparen;
    Vector *stack = new_vector(0, sizeof(int));
    while (true) {
      if (stop && arg_tail->next == *stop)
        need_to_extend_stop = true;

      if (arg_tail->kind == TK_EOF)
        error(&arg_tail->pos, "invalid macro expansion");

      if (stack->size == 0 && arg_tail->next->kind == TK_COMMA) {
        Token *commna = arg_tail->next;
        if (prev_sep == arg_tail) { // empty argument
          Token *empty = calloc(1, sizeof(Token));
          empty->kind = TK_EMPTY;
          vector_push(args, &empty);
        } else {
          vector_push(args, &prev_sep->next);
          arg_tail->next = NULL;
        }
        prev_sep = commna;
        arg_tail = commna;
        continue;
      }

      if (stack->size == 0 && arg_tail->next->kind == TK_RPAREN) {
        rparen = arg_tail->next;
        if (prev_sep == arg_tail) { // emtpty argument
          Token *empty = calloc(1, sizeof(Token));
          empty->kind = TK_EMPTY;
          vector_push(args, &empty);
        } else {
          vector_push(args, &prev_sep->next);
          arg_tail->next = NULL;
        }
        break;
      }

      { // paren, brace, bracket
        if (arg_tail->next->kind == TK_LBRACKET ||
            arg_tail->next->kind == TK_LBRACE ||
            arg_tail->next->kind == TK_LPAREN) {
          vector_pushi(stack, arg_tail->next->kind);
          arg_tail = arg_tail->next;
          continue;
        }
        TokenKind left[] = {TK_LBRACE, TK_LBRACKET, TK_LPAREN};
        TokenKind right[] = {TK_RBRACE, TK_RBRACKET, TK_RPAREN};
        bool cont = false;
        for (int i = 0; i < 3; ++i) {
          if (arg_tail->next->kind == right[i]) {
            if (stack->size == 0 || (TokenKind)vector_lasti(stack) != left[i])
              error(&arg_tail->next->pos, "invalid macro expansion");
            vector_pop(stack);
            cont = true;
            break;
          }
        }
        if (cont) {
          arg_tail = arg_tail->next;
          continue;
        }
      }
      arg_tail = arg_tail->next;
    }
  }

  assert(rparen);
  assert(rparen->kind == TK_RPAREN);
  int nargs = (macro->nparams == 0 && args->size == 1 &&
               (*(Token **)vector_get(args, 0))->kind == TK_EMPTY)
                  ? 0
                  : args->size;
  if ((!macro->is_variadic && nargs != macro->nparams) ||
      (macro->is_variadic && nargs < macro->nparams))
    error(&rparen->pos, "invalid number of arguments for macro expansion");

  // argument prescan
  {
    Token head;
    for (int i = 0; i < args->size; ++i) {
      Token **orig = (Token **)vector_get(args, i);
      head.next = *orig;
      Token *a = &head;
      while (a->next)
        a = expand(a, NULL);
      if (head.next == NULL) { // if expansion result == empty
        head.next = calloc(1, sizeof(Token));
        head.next->kind = TK_EMPTY;
      }
      *orig = head.next;
    }
  }

  if (stop == NULL)
    stop = &rparen->next;
  else if (need_to_extend_stop)
    *stop = rparen->next;

  if (macro->body == NULL) { // empty macro
    prev->next = rparen->next;
    return prev;
  }

  Token head;
  head.next = NULL;
  Token *tail = &head;

  { // replace
    Token *b = macro->body;
    while (b) {
      if (b->kind == TK_HASH) {
        // # operator
        Token *operand = b->next;
        if (operand == NULL || operand->idx == 0)
          error(&expanded->pos, "# operator is not allowed here");
        b = operand;
        Token *arg =
            *(Token **)vector_get(args, operand->idx - 1 /* 1-indexed */);
        assert(arg != NULL);
        tail->next = operator_hash(arg);
        tail = tail->next;
      } else if (b->idx) {
        Token *arg = *(Token **)vector_get(args, b->idx - 1 /* 1-indexed */);
        assert(arg != NULL);
        tail = clone_macro_argument(tail, arg, &expanded->pos);
      } else if (b->is_identifier && same_string_nt(b->str, "__VA_ARGS__")) {
        tail = clone_va_args(tail, macro, args, &expanded->pos);
      } else {
        tail->next = calloc(1, sizeof(Token));
        tail = tail->next;
        *tail = *b;
        tail->pos = expanded->pos;
        tail->next = NULL;
      }
      b = b->next;
    }
    assert(tail->next == NULL);
  }

  { // ## operator
    tail = &head;
    while (tail->next) {
      if (tail->next->kind != TK_2HASH) {
        tail = tail->next;
        continue;
      }
      Token *lhs = tail;
      Token *rhs = tail->next->next;

      if (lhs == &head || rhs == NULL)
        error(&expanded->pos, "## operator is not allowed here");

      if (rhs->kind == TK_EMPTY) {
        lhs->next = rhs->next;
        continue;
      }

      if (lhs->kind == TK_EMPTY) {
        lhs->next = rhs;
        continue;
      }

      *tail = *operator_double_hash(tail, tail->next->next);
    }
    assert(tail->next == NULL);
  }

  { // remove TK_EMPTY
    tail = &head;
    while (tail->next) {
      if (tail->next->kind == TK_EMPTY) {
        tail->next = tail->next->next;
        continue;
      }
      tail = tail->next;
    }
    assert(tail->next == NULL);
  }

  tail->next = rparen->next;

  { // recursive expansion
    macro->flag = true;
    Token *t = &head;
    while (t->next != rparen->next && t->next != *stop)
      t = expand(t, stop);
    tail = t;
    macro->flag = false;
  }

  if (head.next == rparen->next) { // detect empty macro after expansion
    prev->next = rparen->next;
    return prev;
  }

  if (head.next == *stop) { // detect empty macro after expansion
    prev->next = *stop;
    return prev;
  }

  assert(head.next);
  prev->next = head.next;
  return tail;
}

static Token *expand(Token *prev, Token **stop) {
  // expand prev->next and return the tail of expanded tokens
  Token *token = prev->next;
  if (token->kind == TK_IDENT) {
    Macro *m = map_get(macros, token->str);
    if (m && !m->flag) {
      if (m->is_dynamic)
        return expand_dynamic(prev, m);
      else if (m->is_function_like)
        return expand_function_like(prev, m, stop);
      else
        return expand_object_like(prev, m, stop);
    }
  }

  return token; // unmodified
}

bool match_directive(Token *t, char *directive) {
  return t->at_bol && !t->at_eol && t->kind == TK_HASH &&
         t->next->is_identifier && same_string_nt(t->next->str, directive);
}

Token *get_eol(Token *t) {
  while (!t->at_eol)
    t = t->next;
  return t;
}

Token *find_elif_or_else_or_endif(Token *prev) {
  // return the token just before #elif or #else or #endif
  assert(prev->kind != TK_EOF);
  Token *t = prev;
  int nest = 0;
  while (true) {
    if (t->next->kind == TK_EOF)
      error(NULL, "#endif expected but not found");

    if (match_directive(t->next, "endif")) {
      if (nest)
        nest--;
      else
        return t;
    } else if (match_directive(t->next, "elif") && nest == 0) {
      return t;
    } else if (match_directive(t->next, "else") && nest == 0) {
      return t;
    } else if (match_directive(t->next, "if") ||
               match_directive(t->next, "ifdef") ||
               match_directive(t->next, "ifndef")) {
      nest++;
    }
    t = t->next;
  }
  assert(false);
  return NULL;
}

Token *process_if(Token *prev) { // #if and #elif
  assert(match_directive(prev->next, "if") ||
         match_directive(prev->next, "elif"));
  Token *directive = prev->next->next;

  if (directive->at_eol)
    error(&directive->pos, "expr required after the directive but not found.");

  { // crop expr
    Token *eol = get_eol(directive->next);
    prev->next = eol->next;
    Token *eof = calloc(1, sizeof(Token));
    eof->kind = TK_EOF;
    eol->next = eof;
  }

  // expand defined operator
  for (Token *t = directive; t->next->kind != TK_EOF;) {
    if (!t->next->is_identifier || !same_string_nt(t->next->str, "defined")) {
      t = t->next;
      continue;
    }
    Token *op_defined = t->next;
    Token *ident = NULL;
    Token *nx = NULL;
    if (op_defined->next->kind == TK_LPAREN) {
      ident = op_defined->next->next;
      if (!ident->is_identifier)
        error(&op_defined->pos, "identifier expected after defined operator");
      if (ident->next->kind != TK_RPAREN)
        error(&ident->pos, ") expected after identifier");
      nx = ident->next->next;
    } else {
      ident = op_defined->next;
      if (!ident->is_identifier)
        error(&op_defined->pos, "identifier expected after defined operator");
      nx = ident->next;
    }

    Token *e = calloc(1, sizeof(Token));
    e->kind = TK_NUM;
    e->pos = t->pos;
    e->num = new_number_int(map_get(macros, ident->str) != NULL);
    e->type = e->num->type;
    e->next = nx;
    t->next = e;
    t = e;
  }

  // expand expr
  for (Token *t = directive; t->next->kind != TK_EOF;)
    t = expand(t, NULL);

  // replace unknown identifier to 0
  for (Token *t = directive; t->next->kind != TK_EOF; t = t->next) {
    if (!t->next->is_identifier)
      continue;
    Token *zero = calloc(1, sizeof(Token));
    zero->kind = TK_NUM;
    zero->pos = t->pos;
    zero->num = new_number_int(0);
    zero->type = zero->num->type;
    zero->next = t->next->next;
    t->next = zero;
  }

  Node *ex = expr(&directive->next);
  if (!ex->is_constant_expr)
    error(&ex->token->pos, "expr must be a constant.");
  Token *before_hash = find_elif_or_else_or_endif(prev);

  if (number2bool(ex->num)) {
    Token *last = before_hash;
    while (!match_directive(before_hash->next, "endif"))
      before_hash = find_elif_or_else_or_endif(before_hash->next);
    last->next = get_eol(before_hash->next)->next;
    return prev;
  }

  if (match_directive(before_hash->next, "elif")) {
    prev->next = before_hash->next;
    return process_if(prev);
  }

  if (match_directive(before_hash->next, "else")) {
    prev->next = get_eol(before_hash->next)->next;
    before_hash = find_elif_or_else_or_endif(prev);
    if (!match_directive(before_hash->next, "endif"))
      error(&before_hash->next->pos, "#endif expected");
    before_hash->next = get_eol(before_hash->next)->next;
    return prev;
  }

  assert(match_directive(before_hash->next, "endif"));
  prev->next = get_eol(before_hash->next)->next;
  return prev;
}

Token *process_ifdef(Token *prev) { // #ifdef and #ifndef
  assert(match_directive(prev->next, "ifdef") ||
         match_directive(prev->next, "ifndef"));
  Token *directive = prev->next->next;
  bool is_ifndef = match_directive(prev->next, "ifndef");

  if (directive->at_eol)
    error(&directive->pos,
          "identifier required after the directive but not found.");

  Token *macro_ident = directive->next;
  if (!macro_ident->is_identifier)
    error(&macro_ident->pos, "identifier expected but not found.");

  Token *before_hash = find_elif_or_else_or_endif(prev->next);
  Macro *macro = map_get(macros, macro_ident->str);
  if ((!is_ifndef && macro != NULL) || (is_ifndef && macro == NULL)) {
    prev->next = get_eol(macro_ident)->next;
    Token *last = before_hash;
    while (!match_directive(before_hash->next, "endif"))
      before_hash = find_elif_or_else_or_endif(before_hash->next);
    last->next = get_eol(before_hash->next)->next;
    return prev;
  }

  if (match_directive(before_hash->next, "elif")) {
    prev->next = before_hash->next;
    return process_if(prev);
  }

  if (match_directive(before_hash->next, "else")) {
    prev->next = get_eol(before_hash->next)->next;
    before_hash = find_elif_or_else_or_endif(prev);
    if (!match_directive(before_hash->next, "endif"))
      error(&before_hash->next->pos, "#endif expected");
    before_hash->next = get_eol(before_hash->next)->next;
    return prev;
  }

  assert(match_directive(before_hash->next, "endif"));
  prev->next = get_eol(before_hash->next)->next;
  return prev;
}

Token *parse_function_like_macro_param(Token *lparen, Vector *params,
                                       bool *is_variadic) {
  if (lparen->next->kind == TK_RPAREN)
    return lparen->next; // rparen

  Token *p = lparen;
  do {
    p = p->next;
    if (p->at_eol || (!p->is_identifier && p->kind != TK_3DOTS))
      error(&p->pos, "invalid function-like macro");
    if (p->kind == TK_3DOTS) {
      *is_variadic = true;
      p = p->next;
      break;
    }

    vector_push(params, &p);
    p = p->next;
  } while (p->kind == TK_COMMA);
  if (p->kind != TK_RPAREN)
    error(&p->pos, "invalid function-like macro");
  return p; // p-<kind == TK_RPAREN
}

Token *parse_function_like_macro_body(Token *prev, Vector *params) {
  Token *macro_tail = prev;
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
  return macro_tail;
}

Token *define_macro(Token *prev) {
  assert(match_directive(prev->next, "define"));
  Token *directive = prev->next->next;

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
    Token *macro_tail = get_eol(macro_head);
    prev->next = macro_tail->next;
    macro_tail->next = NULL;
    new_macro(macro_ident->str, macro_head);
    return prev;
  }

  // function-like macro
  Token *lparen = macro_ident->next;
  Vector *params = new_vector(0, sizeof(Token *));
  bool is_variadic = false;
  if (lparen->at_eol)
    error(&lparen->pos, "invalid function-like macro");

  Token *rparen = parse_function_like_macro_param(lparen, params, &is_variadic);

  if (rparen->at_eol) { // empty macro
    Macro *m = new_macro(macro_ident->str, NULL);
    m->is_function_like = true;
    m->nparams = params->size;
    m->is_variadic = is_variadic;
    prev->next = rparen->next;
    return prev;
  }

  Token *macro_tail = parse_function_like_macro_body(rparen, params);

  prev->next = macro_tail->next;
  macro_tail->next = NULL;
  Macro *m = new_macro(macro_ident->str, rparen->next);
  m->is_function_like = true;
  m->nparams = params->size;
  m->is_variadic = is_variadic;
  return prev;
}

Token *dummy_token_one() {
  // used for macros defined in command-line arguments
  Token *one = calloc(1, sizeof(Token));
  one->num = new_number_int(1);
  one->type = one->num->type;
  one->kind = TK_NUM;
  char *c = calloc(2, sizeof(char));
  c[0] = '1';
  one->str = new_string(c, 1);
  one->at_eol = true;
  return one;
}

void define_macro_from_command_line(char *arg) {
  init_macro(); // This function is called before preprocess() function.

  Token *macro_ident = tokenize(arg, "command-arg");

  if (macro_ident->kind != TK_IDENT) {
    fprintf(stderr, "invalid macro name for -D option: %s\n", arg);
    exit(1);
  }

  if (macro_ident->at_eol) {
    // When only a macro name is given, the value is 1.
    new_macro(macro_ident->str, dummy_token_one());
    return;
  }

  if (macro_ident->next->kind == TK_ASSIGN) {
    // object-like macro
    Token *tk_assign = macro_ident->next;

    if (tk_assign->at_eol) { // empty macro
      new_macro(macro_ident->str, NULL);
      return;
    }

    Token *macro_head = tk_assign->next;
    get_eol(macro_head)->next = NULL;
    new_macro(macro_ident->str, macro_head);
    return;
  }

  if (macro_ident->next->kind != TK_LPAREN) {
    fprintf(stderr, "invalid macro definition for -D option: %s\n", arg);
    exit(1);
  }

  // function-like macro
  Token *lparen = macro_ident->next;
  Vector *params = new_vector(0, sizeof(Token *));
  bool is_variadic = false;
  if (lparen->at_eol) {
    fprintf(stderr, "invalid macro definition for -D option: %s\n", arg);
    exit(1);
  }

  Token *rparen = parse_function_like_macro_param(lparen, params, &is_variadic);

  if (rparen->at_eol) {
    // When only a macro name is given, the value is 1.
    Macro *m = new_macro(macro_ident->str, dummy_token_one());
    m->is_function_like = true;
    m->nparams = params->size;
    m->is_variadic = is_variadic;
    return;
  }

  Token *tk_assign = rparen->next;
  if (tk_assign->kind != TK_ASSIGN) {
    fprintf(stderr, "invalid macro definition for -D option: %s\n", arg);
    exit(1);
  }

  if (tk_assign->at_eol) { // empty macro
    Macro *m = new_macro(macro_ident->str, NULL);
    m->is_function_like = true;
    m->nparams = params->size;
    m->is_variadic = is_variadic;
    return;
  }

  Token *macro_tail = parse_function_like_macro_body(tk_assign, params);
  macro_tail->next = NULL;

  Macro *m = new_macro(macro_ident->str, tk_assign->next);
  m->is_function_like = true;
  m->nparams = params->size;
  m->is_variadic = is_variadic;
}

Token *undef_macro(Token *prev) {
  assert(match_directive(prev->next, "undef"));
  Token *directive = prev->next->next;

  if (directive->at_eol)
    error(&directive->pos, "identifier required after #undef but not found.");

  Token *macro_ident = directive->next;
  if (!macro_ident->is_identifier)
    error(&macro_ident->pos, "identifier expected but not found.");

  if (map_get(macros, macro_ident->str))
    map_erase(macros, macro_ident->str);

  prev->next = get_eol(macro_ident)->next;
  return prev;
}

Token *builtin_header(char *fname) {
  assert(fname);
  if (strcmp(fname, "stdarg.h") == 0)
    return tokenize(stdarg_h, NULL);

  if (strcmp(fname, "stddef.h") == 0)
    return tokenize(stddef_h, NULL);

  if (strcmp(fname, "stdbool.h") == 0)
    return tokenize(stdbool_h, NULL);

  return NULL;
}

Token *search_include_path(char *fname) {
  for (int i = 0; i < include_path->size; ++i) {
    char *p = path_join(*(char **)vector_get(include_path, i), fname);
    if (access(p, R_OK) == 0) // if file is readable
      return tokenize(read_file(p), p);
  }
  return NULL;
}

Token *process_include(Token *prev) {
  assert(match_directive(prev->next, "include"));
  Token *directive = prev->next->next;

  if (directive->at_eol)
    error(&directive->pos, "filename required after #include but not found.");

  Token *content = NULL;
  if (directive->next->kind == TK_STR) {
    char *p = directive->next->string_literal;

    if (p[0] != '/') {
      // copy string before calling dirname() since it modifies the string
      char *f = calloc(strlen(directive->pos.file_name) + 1, sizeof(char));
      snprintf(f, strlen(directive->pos.file_name) + 1, "%s",
               directive->pos.file_name);
      char *d = dirname(f);
      if (strcmp(d, ".") != 0)
        p = path_join(d, p);
    }

    if (access(p, R_OK) == 0) { // if file is readable
      content = tokenize(read_file(p), p);
      if (included_files)
        vector_push(included_files, &p);
    } else {
      content = builtin_header(directive->next->string_literal);
      if (content == NULL)
        content = search_include_path(directive->next->string_literal);
    }
    if (content == NULL)
      error(&directive->next->pos, "failed to include file: %s",
            directive->next->string_literal);
  } else if (directive->next->kind == TK_LT && !directive->next->at_eol) {
    assert(directive->next->next->kind == TK_STR);
    assert(directive->next->next->next->kind == TK_GT);
    content = builtin_header(directive->next->next->string_literal);
    if (content == NULL)
      content = search_include_path(directive->next->next->string_literal);
    if (content == NULL)
      error(&directive->next->next->pos, "failed to include file: %s",
            directive->next->next->string_literal);
  } else {
    error(&directive->next->pos,
          "filename required after #include but not found.");
  }

  assert(content);
  Token *next_line = get_eol(directive)->next;
  prev->next = content;
  Token *t = prev->next;
  while (t->next->kind != TK_EOF)
    t = t->next;
  t->next = next_line;
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

  // #if
  if (same_string_nt(directive->str, "if"))
    return process_if(prev);

  // #ifdef or #ifndef
  if (same_string_nt(directive->str, "ifdef") ||
      same_string_nt(directive->str, "ifndef"))
    return process_ifdef(prev);

  // #undef
  if (same_string_nt(directive->str, "undef"))
    return undef_macro(prev);

  // #include
  if (same_string_nt(directive->str, "include"))
    return process_include(prev);

  error(&prev->next->pos, "unknown directive");
  return NULL;
}

bool skip_unsupported_keywords(Token *prev) {
  // return true if skipped
  if (prev->next->kind != TK_IDENT)
    return false;

  if (same_string_nt(prev->next->str, "volatile")) {
    prev->next = prev->next->next;
    return true;
  }

  if (same_string_nt(prev->next->str, "__restrict")) {
    prev->next = prev->next->next;
    return true;
  }

  if (same_string_nt(prev->next->str, "__inline")) {
    prev->next = prev->next->next;
    return true;
  }

  if (same_string_nt(prev->next->str, "__extension__")) {
    prev->next = prev->next->next;
    return true;
  }

  if (same_string_nt(prev->next->str, "__attribute__") &&
      prev->next->next->kind == TK_LPAREN &&
      prev->next->next->next->kind == TK_LPAREN) {
    int count = 2;
    Token *t = prev->next->next->next;
    while (count) {
      t = t->next;
      if (t->kind == TK_EOF)
        error(&prev->next->pos,
              "failed to skip __attribute__ (currently __attribute__ is not "
              "supported and will be skipped)");
      if (t->kind == TK_LPAREN)
        count++;
      if (t->kind == TK_RPAREN)
        count--;
    }
    assert(t->kind == TK_RPAREN);
    prev->next = t->next;
    return true;
  }

  if (same_string_nt(prev->next->str, "__asm__")) {
    Token *p = prev->next->next;
    while (p->kind != TK_LPAREN) {
      if (p->kind == TK_EOF)
        error(&prev->next->pos, "failed to skip __asm__ (currently __asm__ is "
                                "not supported and will be skipped)");
      p = p->next;
    }
    assert(p->kind == TK_LPAREN);
    while (p->kind != TK_RPAREN) {
      if (p->kind == TK_EOF)
        error(&prev->next->pos, "failed to skip __asm__ (currently __asm__ is "
                                "not supported and will be skipped)");
      p = p->next;
    }
    assert(p->kind == TK_RPAREN);
    prev->next = p->next;
    return true;
  }

  return false;
}

Token *preprocess(Token *input, Vector *included) {
  assert(input);
  init_macro();
  included_files = included ? included : NULL;

  Token head;
  head.next = input;
  Token *tail = &head;

  while (tail->next->kind != TK_EOF) {
    if (tail->next->kind == TK_HASH) {
      if (!tail->next->at_bol || tail->next->at_eol)
        error(&tail->next->pos, "invalid # here");
      tail = process_directive(tail);
      continue;
    }

    tail = expand(tail, NULL);
  }

  tail = &head;
  while (tail->next->kind != TK_EOF)
    if (!skip_unsupported_keywords(tail))
      tail = tail->next;

  return head.next;
}
