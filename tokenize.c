#include "boncc.h"
#include <ctype.h>
#include <stdlib.h>
#include <string.h>

Token *new_token(TokenKind kind, Token *cur, char *str, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  tok->len = len;
  cur->next = tok;
  return tok;
}

Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;
  while (*p) {
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alphanumeric_or_underscore(p[6])) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (strncmp(p, "<=", 2) == 0) {
      cur = new_token(TK_LE, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, ">=", 2) == 0) {
      cur = new_token(TK_GE, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "==", 2) == 0) {
      cur = new_token(TK_EQ, cur, p, 2);
      p += 2;
      continue;
    }

    if (strncmp(p, "!=", 2) == 0) {
      cur = new_token(TK_NE, cur, p, 2);
      p += 2;
      continue;
    }

    switch (*p) {
    case '+':
      cur = new_token(TK_ADD, cur, p++, 1);
      continue;
    case '-':
      cur = new_token(TK_SUB, cur, p++, 1);
      continue;
    case '*':
      cur = new_token(TK_MUL, cur, p++, 1);
      continue;
    case '/':
      cur = new_token(TK_DIV, cur, p++, 1);
      continue;
    case '(':
      cur = new_token(TK_LPAREN, cur, p++, 1);
      continue;
    case ')':
      cur = new_token(TK_RPAREN, cur, p++, 1);
      continue;
    case '<':
      cur = new_token(TK_LT, cur, p++, 1);
      continue;
    case '>':
      cur = new_token(TK_GT, cur, p++, 1);
      continue;
    case '=':
      cur = new_token(TK_ASSIGN, cur, p++, 1);
      continue;
    case ';':
      cur = new_token(TK_SEMICOLON, cur, p++, 1);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p, 1);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    if (is_alphabet(*p) || *p == '_') {
      char *q = p;
      while (is_alphanumeric_or_underscore(*(q + 1)))
        q++;
      cur = new_token(TK_IDENT, cur, p, q - p + 1);
      p = q + 1;
      continue;
    }

    error_at(p, "failed to tokenize");
  }
  new_token(TK_EOF, cur, p, 0);
  return head.next;
}
