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

bool compare(char *p, const char *keyword) {
  int len = strlen(keyword);
  return strncmp(p, keyword, len) == 0 &&
         !is_alphanumeric_or_underscore(p[len]);
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

    if (compare(p, "return")) {
      cur = new_token(TK_RETURN, cur, p, 6);
      p += 6;
      continue;
    }

    if (compare(p, "if")) {
      cur = new_token(TK_IF, cur, p, 2);
      p += 2;
      continue;
    }

    if (compare(p, "else")) {
      cur = new_token(TK_ELSE, cur, p, 4);
      p += 4;
      continue;
    }

    if (compare(p, "while")) {
      cur = new_token(TK_WHILE, cur, p, 5);
      p += 5;
      continue;
    }

    if (compare(p, "for")) {
      cur = new_token(TK_FOR, cur, p, 3);
      p += 3;
      continue;
    }

    if (compare(p, "int")) {
      cur = new_token(TK_INT, cur, p, 3);
      p += 3;
      continue;
    }

    if (compare(p, "sizeof")) {
      cur = new_token(TK_SIZEOF, cur, p, 6);
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
      cur = new_token(TK_PLUS, cur, p++, 1);
      continue;
    case '-':
      cur = new_token(TK_MINUS, cur, p++, 1);
      continue;
    case '*':
      cur = new_token(TK_STAR, cur, p++, 1);
      continue;
    case '/':
      cur = new_token(TK_SLASH, cur, p++, 1);
      continue;
    case '&':
      cur = new_token(TK_AMP, cur, p++, 1);
      continue;
    case '(':
      cur = new_token(TK_LPAREN, cur, p++, 1);
      continue;
    case ')':
      cur = new_token(TK_RPAREN, cur, p++, 1);
      continue;
    case '{':
      cur = new_token(TK_LBRACE, cur, p++, 1);
      continue;
    case '}':
      cur = new_token(TK_RBRACE, cur, p++, 1);
      continue;
    case '[':
      cur = new_token(TK_LBRACKET, cur, p++, 1);
      continue;
    case ']':
      cur = new_token(TK_RBRACKET, cur, p++, 1);
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
    case ',':
      cur = new_token(TK_COMMA, cur, p++, 1);
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
