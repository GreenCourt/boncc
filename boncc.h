#include "vector.h"
#include <stdbool.h>

typedef enum {
  TK_HASH,      // #
  TK_PLUS,      // +
  TK_MINUS,     // -
  TK_STAR,      // *
  TK_SLASH,     // /
  TK_PERCENT,   // %
  TK_AMP,       // &
  TK_TILDE,     // ~
  TK_HAT,       // ^
  TK_BAR,       // |
  TK_LSHIFT,    // <<
  TK_RSHIFT,    // >>
  TK_INC,       // ++
  TK_DEC,       // --
  TK_ADDEQ,     // +=
  TK_SUBEQ,     // -=
  TK_MULEQ,     // *=
  TK_DIVEQ,     // /=
  TK_MODEQ,     // %=
  TK_XOREQ,     // ^=
  TK_ANDEQ,     // &=
  TK_OREQ,      // |=
  TK_LSHIFTEQ,  // <<=
  TK_RSHIFTEQ,  // >>=
  TK_EQ,        // ==
  TK_NE,        // !=
  TK_LT,        // <
  TK_LE,        // <=
  TK_GT,        // >
  TK_GE,        // >=
  TK_LOGAND,    // &&
  TK_LOGOR,     // ||
  TK_LOGNOT,    // !
  TK_ASSIGN,    // =
  TK_LPAREN,    // (
  TK_RPAREN,    // )
  TK_LBRACE,    // {
  TK_RBRACE,    // }
  TK_LBRACKET,  // [
  TK_RBRACKET,  // ]
  TK_QUESTION,  // ?
  TK_COLON,     // :
  TK_SEMICOLON, // ;
  TK_COMMA,     // ,
  TK_DOT,       // .
  TK_3DOTS,     // ...
  TK_ARROW,     // ->
  TK_RETURN,    // return
  TK_IF,        // if
  TK_ELSE,      // else
  TK_DO,        // do
  TK_WHILE,     // while
  TK_FOR,       // for
  TK_SWITCH,    // switch
  TK_CASE,      // case
  TK_DEFAULT,   // default
  TK_BREAK,     // break
  TK_CONTINUE,  // continue
  TK_VOID,      // void
  TK_INT,       // int
  TK_CHAR,      // char
  TK_SHORT,     // short
  TK_LONG,      // long
  TK_FLOAT,     // float
  TK_DOUBLE,    // double
  TK_SIZEOF,    // sizeof
  TK_STRUCT,    // struct
  TK_UNION,     // union
  TK_ENUM,      // enum
  TK_TYPEDEF,   // typedef
  TK_STATIC,    // static
  TK_EXTERN,    // extern
  TK_CONST,     // const
  TK_SIGNED,    // signed
  TK_UNSIGNED,  // unsigned
  TK_STR,       // string literal
  TK_IDENT,
  TK_NUM,
  TK_EOF,
} TokenKind;

extern const char *token_text[];

typedef struct Position Position;
struct Position {
  char *file_name;
  char *pos;
  int line_number;
  int column_number;
};

typedef struct Ident Ident;
struct Ident {
  char *name;
  int len;
};

typedef struct Type Type;
typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  Position pos;
  int token_length;
  Ident *ident;         // TK_IDENT
  long long val;        // only for TK_NUM
  char *string_literal; // null terminated, only for TK_STR
  Type *type;           // TK_NUM
  bool at_eol;
};

typedef struct Type Type;
typedef struct Member Member;
struct Member {
  Ident *ident;
  Type *type;
  int offset;
  bool is_const;
  Member *next; // liked list
};

typedef enum { TYPE_VOID,
               TYPE_PTR,
               TYPE_ARRAY,
               TYPE_STRUCT,
               TYPE_UNION,
               TYPE_ENUM,
               TYPE_INT,
               TYPE_CHAR,
               TYPE_SHORT,
               TYPE_LONG,
               TYPE_UINT,
               TYPE_UCHAR,
               TYPE_USHORT,
               TYPE_ULONG,
               TYPE_FLOAT,
               TYPE_DOUBLE,
               TYPE_LDOUBLE,
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  int size; // sizeof

  struct Type *base; // only for TYPE_PTR and TYPE_ARRAY
  int array_size;    // number of elements for TYPE_ARRAY

  bool is_unnamed;

  Ident *ident;
  Member *member; // TYPE_STRUCT
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
  Ident *ident;
  VariableKind kind;
  Type *type;
  Token *token;         // for error messages
  int offset;           // only for VK_LOCAL
  char *string_literal; // null terminated, only for VK_STRLIT
  VariableInit *init;   // VK_GLOBAL, VK_LOCAL
  bool is_const;
  bool is_extern;
  bool is_static;
  Ident *internal_ident; // for static local
};

typedef struct Function Function;
struct Function {
  Ident *ident;
  Type *type;
  Vector *params; // Variable*
  Node *body;
  Token *token;
  bool is_static;
  bool is_variadic;
  int offset; // total offset
};

typedef enum {
  ND_ADD,      // +
  ND_SUB,      // -
  ND_MUL,      // *
  ND_DIV,      // /
  ND_MOD,      // %
  ND_BITXOR,   // ^
  ND_BITOR,    // |
  ND_BITAND,   // &
  ND_BITNOT,   // ~
  ND_LSHIFT,   // <<
  ND_RSHIFT,   // >>
  ND_EQ,       // ==
  ND_NE,       // !=
  ND_LT,       // <
  ND_LE,       // <=
  ND_ASSIGN,   // =
  ND_COND,     // ?:
  ND_LOGNOT,   // !
  ND_LOGAND,   // &&
  ND_LOGOR,    // ||
  ND_CAST,     // (type)
  ND_VAR,      // variable
  ND_NUM,      // integer
  ND_RETURN,   // return
  ND_IF,       // if
  ND_DO,       // do-while
  ND_WHILE,    // while
  ND_FOR,      // for
  ND_SWITCH,   // switch
  ND_CASE,     // case
  ND_DEFAULT,  // default
  ND_BREAK,    // break
  ND_CONTINUE, // continue
  ND_BLOCK,    // { }
  ND_CALL,     // call function
  ND_ADDR,     // &val
  ND_DEREF,    // *ptr
  ND_MEMBER,   // struct member access
  ND_NOP,      // dummy node only used in parse.c
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Token *token; // for error messages
  Node *lhs;
  Node *rhs;
  long long val;      // only for ND_NUM
  Variable *variable; // only for ND_VAR
  Type *type;

  int label_index;

  // if(condition) body else else_
  // do body while(condition)
  // while(condition) body
  // for(init; condition; update) body
  // condition ? lhs : rhs
  // switch(condition) body
  // case condition: body
  // default: body
  Node *condition;
  Node *body;
  Node *else_;
  Node *init;
  Node *update;

  Node *next_case; // linked-list for ND_SWITCH, ND_CASE
  Node *default_;  // ND_SWITCH

  Vector *blk_stmts; // statements in ND_BLOCK

  Function *func; // ND_CALL
  Vector *args;   // ND_CALL

  Member *member; // ND_MEMBER
};

typedef struct Vector Map;

typedef struct Scope Scope;
struct Scope {
  Scope *prev;
  Map *variables;     // Variable*
  Map *structs;       // Type*
  Map *enums;         // Type*
  Map *enum_elements; // int*
  Map *typedefs;      // Type*
};

Map *new_map();
void *map_geti(Map *map, int idx);
void *map_get(Map *map, Ident *key);
void map_push(Map *map, Ident *key, void *val);

extern Map *functions; // Function*
extern Map *strings;   // Variable*
extern Scope *global_scope;
extern Vector *static_local_variables; // Variable*

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error(Position *pos, char *fmt, ...);
char *read_file(char *path);
bool same_ident(Ident *a, Ident *b);

Token *tokenize(char *input_path);
Token *preprocess(Token *input);
void parse(Token *input);
void generate_code();

char *type_text(TypeKind kind);
Type *base_type(TypeKind kind);
Type *pointer_type(Type *base);
Type *array_type(Type *base, int len);
Type *struct_type(Ident *ident);
Type *union_type(Ident *ident);
Type *enum_type(Ident *ident);
bool same_type(Type *a, Type *b);
bool is_float(Type *type);
bool is_integer(Type *type);
bool is_unsigned(Type *type);
bool is_signed(Type *type);
Type *implicit_type_conversion(Type *l, Type *r);
