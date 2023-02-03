#include "boncc.h"
#include <assert.h>
#include <ctype.h>
#include <stdio.h>
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

bool match(char *p, const char *keyword) {
  int len = strlen(keyword);
  if (strncmp(p, keyword, len) != 0)
    return false;
  if (is_alphanumeric_or_underscore(p[len - 1]) && is_alphanumeric_or_underscore(p[len]))
    return false;
  return true;
}

static void new_token(TokenKind kind, Token **tail, Position *p, int len) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->pos = *p;
  tok->token_length = len;

  if (tok->kind == TK_IDENT) {
    tok->ident = calloc(1, sizeof(Ident));
    tok->ident->name = p->pos;
    tok->ident->len = len;
  }

  (*tail)->next = tok;
  *tail = tok;
  advance(p, len);
}

Token *tokenize(char *input_path) {
  char *src = read_file(input_path);
  Token head;
  head.next = NULL;
  Token *tail = &head;

  Position p;
  p.line_number = 1;
  p.column_number = 1;
  p.file_name = input_path;
  p.pos = src;

  while (*p.pos) {
    if (*p.pos == '\n')
      tail->at_eol = true;
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
        error(&p, "unclosed block comment");
      advance(&p, (q + 2) - p.pos);
      continue;
    }

    // ignore some keywords
    {
      if (match(p.pos, "volatile")) {
        // my compiler has no optimization mechanisms
        advance(&p, 8);
        continue;
      }
      if (match(p.pos, "__restrict")) {
        advance(&p, 10);
        continue;
      }
      if (match(p.pos, "__inline")) {
        advance(&p, 8);
        continue;
      }
      if (match(p.pos, "__extension__")) {
        advance(&p, 13);
        continue;
      }
      if (match(p.pos, "__attribute__")) {
        advance(&p, 13);
        while (*p.pos != ';')
          advance(&p, 1);
        continue;
      }
      if (match(p.pos, "__asm__")) {
        advance(&p, 7);
        while (*p.pos != ';')
          advance(&p, 1);
        continue;
      }
    }

    static const TokenKind kinds[] = {
        // ordering is important, e.g) "==" must be checked before "=".
        TK_RETURN,
        TK_IF,
        TK_ELSE,
        TK_DO,
        TK_WHILE,
        TK_FOR,
        TK_SWITCH,
        TK_CASE,
        TK_DEFAULT,
        TK_BREAK,
        TK_CONTINUE,
        TK_VOID,
        TK_INT,
        TK_CHAR,
        TK_SHORT,
        TK_LONG,
        TK_FLOAT,
        TK_DOUBLE,
        TK_BOOL,
        TK_SIZEOF,
        TK_STRUCT,
        TK_UNION,
        TK_ENUM,
        TK_TYPEDEF,
        TK_STATIC,
        TK_EXTERN,
        TK_CONST,
        TK_SIGNED,
        TK_UNSIGNED,
        TK_3DOTS,
        TK_ARROW,
        TK_INC,
        TK_DEC,
        TK_ADDEQ,
        TK_SUBEQ,
        TK_MULEQ,
        TK_DIVEQ,
        TK_MODEQ,
        TK_XOREQ,
        TK_ANDEQ,
        TK_OREQ,
        TK_LSHIFTEQ,
        TK_RSHIFTEQ,
        TK_LSHIFT,
        TK_RSHIFT,
        TK_LE,
        TK_GE,
        TK_EQ,
        TK_NE,
        TK_LOGAND,
        TK_LOGOR,
        TK_LOGNOT,
        TK_DOT,
        TK_PLUS,
        TK_MINUS,
        TK_STAR,
        TK_SLASH,
        TK_PERCENT,
        TK_AMP,
        TK_TILDE,
        TK_HAT,
        TK_BAR,
        TK_LPAREN,
        TK_RPAREN,
        TK_LBRACE,
        TK_RBRACE,
        TK_LBRACKET,
        TK_RBRACKET,
        TK_LT,
        TK_GT,
        TK_ASSIGN,
        TK_QUESTION,
        TK_COLON,
        TK_SEMICOLON,
        TK_COMMA,
        TK_HASH,
    };

    bool cont = false;
    for (int i = 0; i < (int)(sizeof(kinds) / sizeof(TokenKind)); ++i) {
      int k = kinds[i];
      const char *t = token_text[k];
      if (match(p.pos, t)) {
        new_token(k, &tail, &p, strlen(t));
        cont = true;
        break;
      }
    }
    if (cont)
      continue;

    if (*p.pos == '\'') { // character literal
      advance(&p, 1);
      if (*p.pos == '\'')
        error(&p, "invalid character literal");

      if (*p.pos == '\\') {
        char val = -1;
        switch (*(p.pos + 1)) {
        case 'a':
          val = '\a';
          break;
        case 'b':
          val = '\b';
          break;
        case 'n':
          val = '\n';
          break;
        case 'r':
          val = '\r';
          break;
        case 'f':
          val = '\f';
          break;
        case 't':
          val = '\t';
          break;
        case 'v':
          val = '\v';
          break;
        case '\\':
          val = '\\';
          break;
        case '?':
          val = '\?';
          break;
        case '\'':
          val = '\'';
          break;
        case '0':
          val = '\0';
          break;
        default:
          error(&p, "unsupported escaped literal");
        }
        new_token(TK_NUM, &tail, &p, 2);
        tail->val = val;
        tail->type = base_type(TYPE_CHAR);
        advance(&p, 1);
        continue;
      }

      if (*(p.pos + 1) != '\'')
        error(&p, "invalid character literal");

      new_token(TK_NUM, &tail, &p, 1);
      tail->val = *(tail->pos.pos);
      tail->type = base_type(TYPE_CHAR);
      advance(&p, 1);
      continue;
    }

    if (*p.pos == '"') { // string literal
      advance(&p, 1);
      char *q = p.pos;
      while (true) {
        char c = *q;
        char d = *(q + 1);
        if (c == '"')
          break;

        if (c == '\\') { // escape
          if (d == '\0')
            error(&p, "missing terminating \" character");
          q += 2;
          continue;
        }

        q++;
        if (*q == '\0' || *q == '\n')
          error(&p, "missing terminating \" character");
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
      long long val;

      if(*p.pos == '0' && *(p.pos+1) == 'x') {
        advance(&p, 2);
        val = strtol(p.pos, &q, 16);
      } else if(*p.pos == '0') {
        val = strtol(p.pos, &q, 8);
      } else {
        val = strtol(p.pos, &q, 10);
      }

      int len = q - p.pos;
      new_token(TK_NUM, &tail, &p, len);
      tail->val = val;

      bool is_long = !(-2147483648 <= val && val <= 2147483647);
      bool is_unsigned = false;

      if (*p.pos == 'U') {
        is_unsigned = true;
        advance(&p, 1);
      }

      if (*p.pos == 'L') {
        is_long = true;
        advance(&p, 1);
        if (*p.pos == 'L')
          advance(&p, 1);
      }

      TypeKind kind;

      if (is_long && is_unsigned)
        kind = TYPE_ULONG;
      else if (is_long)
        kind = TYPE_LONG;
      else if (is_unsigned)
        kind = TYPE_UINT;
      else
        kind = TYPE_INT;

      tail->type = base_type(kind);
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

    error(&p, "failed to tokenize");
  }
  new_token(TK_EOF, &tail, &p, 0);
  return head.next;
}
