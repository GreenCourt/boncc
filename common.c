#define _POSIX_C_SOURCE 200809L // for open_memstream
#include <stdio.h>
#undef _POSIX_C_SOURCE
#include "boncc.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

char *source_file_name;
char *source_code;
Token *token;
Vector *functions;
Vector *globals;
Vector *strings;

const char *token_str[] = {
    // corresponding to TokenKind
    "+",
    "-",
    "*",
    "/",
    "&",
    "==",
    "!=",
    "<",
    "<=",
    ">",
    ">=",
    "=",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    ";",
    ",",
    "return",
    "if",
    "else",
    "while",
    "for",
    "int",
    "char",
    "sizeof",
    "str",
    "identifier",
    "number",
};

void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  char *line_start = loc;
  while (source_code < line_start && line_start[-1] != '\n')
    line_start--;

  char *line_end = loc;
  while (*line_end != '\n')
    line_end++;

  int line_number = 1;
  for (char *p = source_code; p < line_start; p++)
    line_number += (*p == '\n');

  int column = loc - line_start + 1;

  int indent = fprintf(stderr, "%s:%d:%d: ", source_file_name, line_number, column);
  fprintf(stderr, "%.*s\n", (int)(line_end - line_start), line_start);

  int pos = loc - line_start + indent;
  fprintf(stderr, "%*s", pos, ""); // print spaces
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  fprintf(stderr, "%s: ", source_file_name);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

bool is_alphabet(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z'); }

bool is_alphanumeric_or_underscore(char c) { return ('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || isdigit(c) || c == '_'; }

char *read_file(char *path) {
  FILE *fp;
  if (strcmp(path, "-") == 0) {
    fp = stdin;
  } else {
    fp = fopen(path, "r");
    if (!fp)
      error("cannot open %s: %s", path, strerror(errno));
  }

  char *content;
  size_t size;
  FILE *stream = open_memstream(&content, &size);

  while (true) {
    char buf[4096];
    int n = fread(buf, 1, sizeof(buf), fp);
    if (n == 0)
      break;
    fwrite(buf, 1, n, stream);
  }

  if (fp != stdin)
    fclose(fp);
  fflush(stream);

  if (size == 0 || content[size - 1] != '\n')
    fputc('\n', stream);
  fputc('\0', stream);
  fclose(stream);
  return content;
}
