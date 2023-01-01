#include "Vector.h"
#include <stdbool.h>

typedef enum {
  TK_PLUS,      // +
  TK_MINUS,     // -
  TK_STAR,      // *
  TK_SLASH,     // /
  TK_AMP,       // &
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
  TK_LBRACKET,  // [
  TK_RBRACKET,  // ]
  TK_SEMICOLON, // ;
  TK_COMMA,     // ,
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_INT,       // int
  TK_CHAR,      // char
  TK_SIZEOF,    // sizeof
  TK_STR,       // string literal
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

extern const char *token_str[];

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  char *pos;
  int token_length;
  int val;              // only for TK_NUM
  char *string_literal; // null terminated, only for TK_STR
};

typedef enum { TYPE_PTR, TYPE_ARRAY, TYPE_INT, TYPE_CHAR } TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  int size;          // sizeof
  struct Type *base; // only for TYPE_PTR and TYPE_ARRAY
  size_t array_size; // number of elements for TYPE_ARRAY
};

typedef enum { VK_GLOBAL, VK_LOCAL, VK_STRLIT } VariableKind;
typedef struct Variable Variable;
struct Variable {
  char *name;
  int name_length;
  VariableKind kind;
  Type *type;
  int offset;           // only for VK_LOCAL
  char *string_literal; // null terminated, only for VK_STRLIT
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
  ND_VAR,    // variable
  ND_NUM,    // integer
  ND_RETURN, // return
  ND_IF,     // if
  ND_WHILE,  // while
  ND_FOR,    // for
  ND_BLOCK,  // {...}
  ND_CALL,   // call function
  ND_FUNC,   // function
  ND_ADDR,   // &val
  ND_DEREF,  // *ptr
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;            // only for ND_NUM
  Variable *variable; // only for ND_LVAR
  Type *type;

  // if(condition) body else else_
  // while(condition) body
  // for(init; condition; update) body
  Node *condition;
  Node *body;
  Node *else_;
  Node *init;
  Node *update;

  Vector *blk_stmts; // statements in ND_BLOCK

  char *name; // function name for ND_CALL, ND_FUNC
  int name_length;
  Vector *args;   // arguments for ND_CALL
  Vector *params; // parameters for ND_FUNC
  int offset;     // total offset ND_FUNC
};

extern char *source_file_name;
extern char *source_code;
extern Token *token;
extern Vector *functions;
extern Vector *globals;
extern Vector *strings;

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error_at(char *loc, char *fmt, ...);
void error(char *fmt, ...);
char *read_file(char *path);

void gen(Node *node);
Token *tokenize(char *p);
void program();

Type *get_type(Node *node);
Type *base_type(TypeKind kind);
Type *pointer_type(Type *base);
Type *array_type(Type *base, int len);
