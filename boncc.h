#include "Vector.h"
#include <stdbool.h>

typedef enum {
  TK_PLUS,      // +
  TK_MINUS,     // -
  TK_STAR,      // *
  TK_SLASH,     // /
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_LT,        // <
  TK_LE,        // <=
  TK_GT,        // >
  TK_GE,        // >=
  TK_ASSIGN,    // =
  TK_LPAREN,    // (
  TK_RPAREN,    // )
  TK_LBRACE,    // {
  TK_RBRACE,    // }
  TK_SEMICOLON, // ;
  TK_COMMA,     // ,
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

extern const char *token_str[];

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD,    // +
  ND_SUB,    // -
  ND_MUL,    // *
  ND_DIV,    // /
  ND_EQ,     // ==
  ND_NE,     // !=
  ND_LT,     // <
  ND_LE,     // <=
  ND_ASSIGN, // =
  ND_LVAR,   // local variable
  ND_NUM,    // integer
  ND_RETURN, // return
  ND_IF,     // if
  ND_WHILE,  // while
  ND_FOR,    // for
  ND_BLOCK,  // {...}
  ND_CALL,   // call function
  ND_FUNC,   // function
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;    // only for ND_NUM
  int offset; // only for ND_LVAR

  // if(condition) body else else_
  // while(condition) body
  // for(init; condition; update) body
  Node *condition;
  Node *body;
  Node *else_;
  Node *init;
  Node *update;

  Vector *blk_stmts; // statements in ND_BLOCK

  char *name;     // function name for ND_CALL, ND_FUNC
  int len;        // length of name
  Vector *args;   // arguments for ND_CALL
  Vector *locals; // local variables for ND_FUNC
  int nparams;    // number of parameters for ND_FUNC
};

typedef struct LVar LVar;
struct LVar {
  char *name;
  int len;
  int offset;
};

extern char *user_input;
extern Token *token;
extern Vector *functions;

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

void gen(Node *node);
Token *tokenize(char *p);
void program();
