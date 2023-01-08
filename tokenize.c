#include "boncc.h"
#include <assert.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

static void advance(Position *p, int n) {
  for (int i = 0; i < n; ++i) {
    assert(*p->pos);
    if (*p->pos == '\n') {
      p->line_number++;
      p->column_number = 1;
    } else {
      p->column_number++;
    }
    p->pos++;
  }
}

static void new_token(TokenKind kind, Token **tail, Position *p, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->pos = *p;
  tok->token_length = len;
  (*tail)->next = tok;
  *tail = tok;
  advance(p, len);
}

bool match(char *p, const char *keyword) {
  int len = strlen(keyword);
  if (strncmp(p, keyword, len) != 0)
    return false;
  if (is_alphanumeric_or_underscore(p[len - 1]) && is_alphanumeric_or_underscore(p[len]))
    return false;
  return true;
}

Token *tokenize(char *src) {
  Token head;
  head.next = NULL;
  Token *tail = &head;

  Position p;
  p.line_number = 1;
  p.column_number = 1;
  p.file_name = source_file_name;
  p.pos = src;

  while (*p.pos) {
    if (isspace(*p.pos)) {
      advance(&p, 1);
      continue;
    }

    if (match(p.pos, "//")) { // line comments
      advance(&p, 2);
      while (*p.pos != '\n')
        advance(&p, 1);
      continue;
    }

    if (match(p.pos, "/*")) { // block comments
      char *q = strstr(p.pos + 2, "*/");
      if (!q)
        error_at(&p, "unclosed block comment");
      advance(&p, (q + 2) - p.pos);
      continue;
    }

    if (match(p.pos, "return")) {
      new_token(TK_RETURN, &tail, &p, 6);
      continue;
    }

    if (match(p.pos, "if")) {
      new_token(TK_IF, &tail, &p, 2);
      continue;
    }

    if (match(p.pos, "else")) {
      new_token(TK_ELSE, &tail, &p, 4);
      continue;
    }

    if (match(p.pos, "while")) {
      new_token(TK_WHILE, &tail, &p, 5);
      continue;
    }

    if (match(p.pos, "for")) {
      new_token(TK_FOR, &tail, &p, 3);
      continue;
    }

    if (match(p.pos, "int")) {
      new_token(TK_INT, &tail, &p, 3);
      continue;
    }

    if (match(p.pos, "char")) {
      new_token(TK_CHAR, &tail, &p, 4);
      continue;
    }

    if (match(p.pos, "sizeof")) {
      new_token(TK_SIZEOF, &tail, &p, 6);
      continue;
    }

    if (match(p.pos, "<=")) {
      new_token(TK_LE, &tail, &p, 2);
      continue;
    }

    if (match(p.pos, ">=")) {
      new_token(TK_GE, &tail, &p, 2);
      continue;
    }

    if (match(p.pos, "==")) {
      new_token(TK_EQ, &tail, &p, 2);
      continue;
    }

    if (match(p.pos, "!=")) {
      new_token(TK_NE, &tail, &p, 2);
      continue;
    }

    switch (*p.pos) {
    case '+':
      new_token(TK_PLUS, &tail, &p, 1);
      continue;
    case '-':
      new_token(TK_MINUS, &tail, &p, 1);
      continue;
    case '*':
      new_token(TK_STAR, &tail, &p, 1);
      continue;
    case '/':
      new_token(TK_SLASH, &tail, &p, 1);
      continue;
    case '&':
      new_token(TK_AMP, &tail, &p, 1);
      continue;
    case '(':
      new_token(TK_LPAREN, &tail, &p, 1);
      continue;
    case ')':
      new_token(TK_RPAREN, &tail, &p, 1);
      continue;
    case '{':
      new_token(TK_LBRACE, &tail, &p, 1);
      continue;
    case '}':
      new_token(TK_RBRACE, &tail, &p, 1);
      continue;
    case '[':
      new_token(TK_LBRACKET, &tail, &p, 1);
      continue;
    case ']':
      new_token(TK_RBRACKET, &tail, &p, 1);
      continue;
    case '<':
      new_token(TK_LT, &tail, &p, 1);
      continue;
    case '>':
      new_token(TK_GT, &tail, &p, 1);
      continue;
    case '=':
      new_token(TK_ASSIGN, &tail, &p, 1);
      continue;
    case ';':
      new_token(TK_SEMICOLON, &tail, &p, 1);
      continue;
    case ',':
      new_token(TK_COMMA, &tail, &p, 1);
      continue;
    }

    if (*p.pos == '"') { // string literal
      advance(&p, 1);
      char *q = p.pos;
      while (*q != '"') {
        q++;
        if (*q == '\0' || *q == '\n')
          error_at(&p, "missing terminating \" character");
      }
      int len = q - p.pos;
      char *string_literal = calloc(len + 1, sizeof(char));
      strncpy(string_literal, p.pos, len);
      new_token(TK_STR, &tail, &p, len);
      tail->string_literal = string_literal;
      advance(&p, 1);
      continue;
    }

    if (isdigit(*p.pos)) {
      char *q;
      int val = strtol(p.pos, &q, 10);
      int len = q - p.pos;
      new_token(TK_NUM, &tail, &p, len);
      tail->val = val;
      continue;
    }

    if (is_alphabet(*p.pos) || *p.pos == '_') {
      char *q = p.pos;
      while (is_alphanumeric_or_underscore(*(q + 1)))
        q++;
      int len = q + 1 - p.pos;
      new_token(TK_IDENT, &tail, &p, len);
      continue;
    }

    error_at(&p, "failed to tokenize");
  }
  new_token(TK_EOF, &tail, &p, 0);
  return head.next;
}
