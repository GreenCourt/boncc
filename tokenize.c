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

  if (match(p->pos, "__FILE__")) {
    tok->kind = TK_STR;
    tok->string_literal = p->file_name;
  }
  if (match(p->pos, "__LINE__")) {
    tok->kind = TK_NUM;
    tok->val = p->line_number;
  }

  (*tail)->next = tok;
  *tail = tok;
  advance(p, len);
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
        TK_SIZEOF,
        TK_STRUCT,
        TK_ENUM,
        TK_TYPEDEF,
        TK_STATIC,
        TK_ARROW,
        TK_INC,
        TK_DEC,
        TK_PLUSEQ,
        TK_MINUSEQ,
        TK_LE,
        TK_GE,
        TK_EQ,
        TK_NE,
        TK_DOT,
        TK_PLUS,
        TK_MINUS,
        TK_STAR,
        TK_SLASH,
        TK_PERCENT,
        TK_AMP,
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
        TK_COMMA};

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
        error_at(&p, "invalid character literal");

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
          error_at(&p, "unsupported escaped literal");
        }
        new_token(TK_CHARLIT, &tail, &p, 2);
        tail->val = val;
        advance(&p, 1);
        continue;
      }

      if (*(p.pos + 1) != '\'')
        error_at(&p, "invalid character literal");

      new_token(TK_CHARLIT, &tail, &p, 1);
      tail->val = *(tail->pos.pos);
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
            error_at(&p, "missing terminating \" character");
          q += 2;
          continue;
        }

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
