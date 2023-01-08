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

typedef struct Position Position;
struct Position {
  char *file_name;
  char *pos;
  int line_number;
  int column_number;
};

typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  Position pos;
  int token_length;
  int val;              // only for TK_NUM
  char *string_literal; // null terminated, only for TK_STR
};

typedef enum { TYPE_PTR,
               TYPE_ARRAY,
               TYPE_INT,
               TYPE_CHAR } TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  int size;          // sizeof
  struct Type *base; // only for TYPE_PTR and TYPE_ARRAY
  int array_size;    // number of elements for TYPE_ARRAY
};

typedef struct Node Node;
typedef struct VariableInit VariableInit;
struct VariableInit {
  Node *expr;
  Vector *vec;
  bool nested; // true if vec is nested
};

typedef enum { VK_GLOBAL,
               VK_LOCAL,
               VK_STRLIT } VariableKind;
typedef struct Variable Variable;
struct Variable {
  char *name;
  int name_length;
  VariableKind kind;
  Type *type;
  Token *token;         // for error messages
  int offset;           // only for VK_LOCAL
  char *string_literal; // null terminated, only for VK_STRLIT
  VariableInit *init;   // VK_GLOBAL, VK_LOCAL
};

typedef struct Function Function;
struct Function {
  char *name;
  int name_length;
  Type *type;
  Vector *params;
  int offset; // total offset
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
  Token *token; // for error messages
  Node *lhs;
  Node *rhs;
  int val;            // only for ND_NUM
  Variable *variable; // only for ND_VAR
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

  Function *func; // for ND_FUNC or ND_CALL
  Vector *args;   // arguments for ND_CALL
};

extern char *source_file_name;
extern Token *token;
extern Vector *functions;
extern Vector *globals;
extern Vector *strings;

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error_at(Position *pos, char *fmt, ...);
void error(char *fmt, ...);
char *read_file(char *path);

void gen(Node *node);
void gen_global_variables();
Token *tokenize(char *p);
void program();

Type *base_type(TypeKind kind);
Type *pointer_type(Type *base);
Type *array_type(Type *base, int len);
