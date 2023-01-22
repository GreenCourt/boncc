#define _POSIX_C_SOURCE 200809L // for open_memstream
#include <stdio.h>
#undef _POSIX_C_SOURCE
#include "boncc.h"
#include <ctype.h>
#include <errno.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>

Map *functions;
Map *strings;
Scope *global_scope;
Vector *static_local_variables;

const char *token_text[] = {
    // corresponding to TokenKind
    "#",
    "+",
    "-",
    "*",
    "/",
    "%",
    "&",
    "~",
    "^",
    "|",
    "<<",
    ">>",
    "++",
    "--",
    "+=",
    "-=",
    "*=",
    "/=",
    "%=",
    "^=",
    "&=",
    "|=",
    "<<=",
    ">>=",
    "==",
    "!=",
    "<",
    "<=",
    ">",
    ">=",
    "&&",
    "||",
    "!",
    "=",
    "(",
    ")",
    "{",
    "}",
    "[",
    "]",
    "?",
    ":",
    ";",
    ",",
    ".",
    "->",
    "return",
    "if",
    "else",
    "do",
    "while",
    "for",
    "switch",
    "case",
    "default",
    "break",
    "continue",
    "void",
    "int",
    "char",
    "short",
    "long",
    "float",
    "double",
    "sizeof",
    "struct",
    "union",
    "enum",
    "typedef",
    "static",
    "extern",
    "const",
    "signed",
    "unsigned",
    "str",
    "identifier",
    "number",
    "eof",
};

bool same_ident(Ident *a, Ident *b) { return a->len == b->len && strncmp(a->name, b->name, a->len) == 0; }

void error(Position *pos, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  if (pos) {
    char *line_start = pos->pos - pos->column_number + 1;
    char *line_end = pos->pos;
    while (*line_end != '\n')
      line_end++;

    int indent = fprintf(stderr, "%s:%d:%d: ", pos->file_name, pos->line_number, pos->column_number);
    fprintf(stderr, "%.*s\n", (int)(line_end - line_start), line_start);

    int sp = pos->pos - line_start + indent;
    fprintf(stderr, "%*s", sp, ""); // print spaces
    fprintf(stderr, "^ ");
  }

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
      error(NULL, "cannot open %s: %s", path, strerror(errno));
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
