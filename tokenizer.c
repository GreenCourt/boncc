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

static void new_token(TokenKind kind, Token **tail, Position *p, int len, bool is_identifier) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->pos = *p;
  tok->str = new_string(p->pos, len);
  tok->is_identifier = is_identifier;
  tok->at_bol = (*tail)->at_eol;

  (*tail)->next = tok;
  *tail = tok;
  advance(p, len);
}

Token *tokenize(char *src, char *input_path) {
  Token head;
  head.next = NULL;
  head.at_eol = true;
  Token *tail = &head;

  Position p;
  p.line_number = 1;
  p.column_number = 1;
  p.file_name = input_path;
  p.pos = src;

  while (*p.pos) {
    if (*p.pos == '\n') {
      tail->at_eol = true;
      advance(&p, 1);
      continue;
    }

    if (isspace(*p.pos)) {
      tail->has_right_space = true;
      advance(&p, 1);
      continue;
    }

    if (*p.pos == '\\' && *(p.pos + 1) == '\n') {
      advance(&p, 2);
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

    {
      static const TokenKind kinds[] = {
          // ordering is important, e.g) "==" must be checked before "=".
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
          TK_2HASH,
          TK_HASH,
      };

      bool cont = false;
      for (int i = 0; i < (int)(sizeof(kinds) / sizeof(TokenKind)); ++i) {
        int k = kinds[i];
        const char *t = token_text[k];
        if (match(p.pos, t)) {
          new_token(k, &tail, &p, strlen(t), false);
          cont = true;
          break;
        }
      }
      if (cont)
        continue;
    }

    {
      static const TokenKind kinds[] = {
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
          TK_GOTO,
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
      };

      bool cont = false;
      for (int i = 0; i < (int)(sizeof(kinds) / sizeof(TokenKind)); ++i) {
        int k = kinds[i];
        const char *t = token_text[k];
        if (match(p.pos, t)) {
          new_token(k, &tail, &p, strlen(t), true);
          cont = true;
          break;
        }
      }
      if (cont)
        continue;
    }

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
        new_token(TK_NUM, &tail, &p, 2, false);
        tail->val = val;
        tail->type = base_type(TYPE_CHAR);
        advance(&p, 1);
        continue;
      }

      if (*(p.pos + 1) != '\'')
        error(&p, "invalid character literal");

      new_token(TK_NUM, &tail, &p, 1, false);
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
      new_token(TK_STR, &tail, &p, len, false);
      tail->string_literal = string_literal;
      advance(&p, 1);
      continue;
    }

    if (isdigit(*p.pos)) {
      char *q = p.pos;
      long long val;

      if (*q == '0' && (*(q + 1) == 'X' || *(q + 1) == 'x'))
        val = strtol(q + 2, &q, 16);
      else if (*q == '0')
        val = strtol(q, &q, 8);
      else
        val = strtol(q, &q, 10);

      bool is_long = !(-2147483648 <= val && val <= 2147483647);
      bool is_unsigned = false;

      if (*q == 'U' || *q == 'u') {
        is_unsigned = true;
        q++;
      }

      if (*q == 'L' || *q == 'l') {
        is_long = true;
        if (*(q + 1) == *q)
          q++;
        q++;
      }

      int len = q - p.pos;
      new_token(TK_NUM, &tail, &p, len, false);
      tail->val = val;

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
      new_token(TK_IDENT, &tail, &p, len, true);
      continue;
    }

    error(&p, "failed to tokenize");
  }
  new_token(TK_EOF, &tail, &p, 0, false);
  return head.next;
}
