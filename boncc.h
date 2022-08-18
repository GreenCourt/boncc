#include <stdbool.h>

typedef enum {
  TK_ADD,       // +
  TK_SUB,       // -
  TK_MUL,       // *
  TK_DIV,       // /
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_LT,        // <
  TK_LE,        // <=
  TK_GT,        // >
  TK_GE,        // >=
  TK_ASSIGN,    // =
  TK_LPAREN,    // (
  TK_RPAREN,    // )
  TK_SEMICOLON, // ;
  TK_RETURN,    // return
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
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;    // only for ND_NUM
  int offset; // only for ND_LVAR
};

typedef struct LVar LVar;
struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

extern char *user_input;
extern Token *token;
extern Node *code[];
extern LVar *locals;

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);

void gen(Node *node);
Token *tokenize(char *p);
void program();
