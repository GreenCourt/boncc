#include "boncc.h"
#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>

char *user_input;
Token *token;
Vector *functions;

const char *token_str[] = {
    // corresponding to TokenKind
    "+",      "-",  "*",    "/",     "&",   "==",  "!=",         "<",      "<=",
    ">",      ">=", "=",    "(",     ")",   "{",   "}",          ";",      ",",
    "return", "if", "else", "while", "for", "int", "identifier", "number",
};

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " ");
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool is_alphabet(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z');
}

bool is_alphanumeric_or_underscore(char c) {
  return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || isdigit(c) ||
         c == '_';
}
