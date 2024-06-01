#include "vector.h"
#include <stdbool.h>
#include <stdio.h>

typedef enum {
  TK_HASH,      // #
  TK_2HASH,     // ##
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
  TK_GOTO,      // goto
  TK_VOID,      // void
  TK_INT,       // int
  TK_CHAR,      // char
  TK_SHORT,     // short
  TK_LONG,      // long
  TK_FLOAT,     // float
  TK_DOUBLE,    // double
  TK_BOOL,      // _Bool
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

typedef struct String String;
struct String {
  // a string that is not null-terminated
  char *str;
  int len;
};

typedef struct Type Type;
typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  Position pos;
  String *str;
  bool is_identifier;   // true iff TK_IDENT or reserved identifiers
  long long val;        // only for TK_NUM
  char *string_literal; // null terminated, only for TK_STR
  Type *type;           // TK_NUM
  bool at_bol;
  bool at_eol;
  bool has_right_space; // to detect function-like macros
  int idx;              // used by function-like macros
};

typedef struct Type Type;
typedef struct Member Member;
struct Member {
  String *ident;
  Type *type;
  int offset;
  int padding;
  Member *next; // liked list
};

typedef enum {
  TYPE_VOID,
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
  TYPE_BOOL,
  TYPE_FUNC,
  TYPE_NONE, // internally used for declarator
} TypeKind;

typedef struct Type Type;
struct Type {
  TypeKind kind;
  int size; // sizeof
  int align;
  bool is_const;

  struct Type *base; // only for TYPE_PTR and TYPE_ARRAY
  int array_size;    // number of elements for TYPE_ARRAY

  bool is_unnamed; // TYPE_STRUCT, TYPE_UNION, TYPE_ENUM
  Member *member;  // TYPE_STRUCT, TYPE_UNION

  Token *objdec; // used for declaration

  Type *return_type; // TYPE_FUNC
  Vector *params;    // vector of Type* for TYPE_FUNC
  bool is_variadic;  // TYPE_FUNC
};

typedef struct Node Node;
typedef struct VariableInit VariableInit;
struct VariableInit {
  Node *expr;
  Vector *vec;
  bool nested; // true if vec is nested
};

typedef enum {
  OBJ_GVAR,
  OBJ_LVAR,
  OBJ_STRLIT,
  OBJ_FUNC,
} ObjectKind;
typedef struct Object Variable;
typedef struct Object Function;
typedef struct Object Object;
struct Object { // variable or function
  String *ident;
  Type *type;
  Token *token; // for error messages
  ObjectKind kind;
  bool is_static;
  int offset; // OBJ_LVAR, OBJ_FUNC

  // variable
  char *string_literal;   // null terminated, only for OBJ_STRLIT
  VariableInit *init;     // OBJ_GVAR, OBJ_LVAR
  String *internal_ident; // for static local
  bool is_extern;

  // function
  Node *body;
  Vector *params; // vector of Variable*
  Variable *hidden_va_area;
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
  ND_COMMA,    // ,
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
  ND_LABEL,    // label:
  ND_GOTO,     // goto
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
  // label: body
  Node *condition;
  Node *body;
  Node *else_;
  Node *init;
  Node *update;

  Node *next_case; // linked-list for ND_SWITCH, ND_CASE
  Node *default_;  // ND_SWITCH

  Vector *blk_stmts; // statements in ND_BLOCK

  Vector *args; // ND_CALL

  Member *member; // ND_MEMBER
};

typedef struct Vector Map;

typedef struct Scope Scope;
struct Scope {
  Scope *prev;
  Map *objects;       // Object*
  Map *types;         // Type*
  Map *enum_elements; // int*
  Map *typedefs;      // Type*
  int offset;         // local variable offset
};

Map *new_map();
void *map_geti(Map *map, int idx);
void *map_get(Map *map, String *key);
void map_push(Map *map, String *key, void *val);
void map_erase(Map *map, String *key);

extern Scope *global_scope;
extern Map *strings;                   // Variable*
extern Vector *static_local_variables; // Variable*
extern Vector *include_path;           // char*

bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
void error(Position *pos, char *fmt, ...);
char *read_file(char *path);
char *path_join(char *dir, char *file);

String *new_string(char *str, int len);
bool same_string(String *a, String *b);
bool same_string_nt(String *s, char *null_terminated);

Token *tokenize(char *src, char *input_path);
Token *preprocess(Token *input);
void parse(Token *input);
void generate_code(FILE *output_stream);

char *type_text(TypeKind kind);
Type *base_type(TypeKind kind);
Type *pointer_type(Type *base);
Type *array_type(Type *base, int len);
Type *struct_type(bool is_unnamed);
Type *union_type(bool is_unnamed);
Type *enum_type(bool is_unnamed);
Type *func_type(Type *return_type);
bool same_type(Type *a, Type *b);
bool is_float(Type *type);
bool is_integer(Type *type);
bool is_scalar(Type *type);
bool is_unsigned(Type *type);
bool is_signed(Type *type);
bool is_funcptr(Type *type);
bool is_struct_union(Type *type);
Type *implicit_type_conversion(Type *l, Type *r);
