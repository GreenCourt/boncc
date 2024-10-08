#include "hashmap.h"
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
  TK_EMPTY, // used by preprocessor
} TokenKind;

extern const char *token_text[];

typedef struct Position Position;
struct Position {
  char *file_name;
  char *pos;
  int line_number;
  int column_number;
};

typedef struct Type Type;
typedef struct Number Number;
struct Number {
  Type *type;
  union {
    unsigned long long ulong_value;
    long long long_value;
    float float_value;
    double double_value;
  } value;
};

typedef struct Type Type;
typedef struct Token Token;
struct Token {
  TokenKind kind;
  Token *next;
  Position pos;
  char *str;
  bool is_identifier;   // true iff TK_IDENT or reserved identifiers
  Number *num;          // only for TK_NUM
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
  char *ident;
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
  char *ident;
  Type *type;
  Token *token; // for error messages
  ObjectKind kind;
  bool is_static;
  int offset; // OBJ_LVAR, OBJ_FUNC

  // variable
  char *string_literal; // null terminated, only for OBJ_STRLIT
  VariableInit *init;   // OBJ_GVAR, OBJ_LVAR
  char *internal_ident; // for static local
  bool is_extern;

  // function
  Node *body;
  Vector *params; // vector of Variable*
  Variable *hidden_va_area;
  Variable *return_buffer_address; // only for callee function
  int return_buffer_offset;        // only for caller function
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
  Variable *variable; // only for ND_VAR
  Type *type;

  Number *num; // ND_NUM or constant expr
  bool is_constant_expr;

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

  Vector *args;     // ND_CALL
  Function *caller; // ND_CALL

  Variable *return_buffer_address; // ND_RETURN

  Member *member; // ND_MEMBER
};

typedef struct Scope Scope;
struct Scope {
  Scope *prev;
  HashMap *objects;       // Object*
  HashMap *types;         // Type*
  HashMap *enum_elements; // int*
  HashMap *typedefs;      // Type*
  int offset;             // local variable offset
};

extern Scope *global_scope;
extern HashMap *strings;               // Variable*
extern Vector *static_local_variables; // Variable*
extern Vector *include_path;           // char*

int iceil(int x, int y);
bool is_alphabet(char c);
bool is_alphanumeric_or_underscore(char c);
bool is_hexdigit(char c);
__attribute__((format(printf, 2, 3))) void error(Position *pos, char *fmt, ...);
char *read_file(char *path);
char *path_join(char *dir, char *file);
char *replace_ext(const char *path, const char *ext);

void define_macro_from_command_line(char *arg);

Token *tokenize(char *src, char *input_path);
Token *preprocess(Token *input, Vector *included);
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
bool is_numerical(Type *type);
bool is_unsigned(Type *type);
bool is_signed(Type *type);
bool is_funcptr(Type *type);
bool is_struct_union(Type *type);
bool pass_on_memory(Type *type);
Type *implicit_type_conversion(Type *l, Type *r);
bool castable(Type *from, Type *to);

Number *new_number_int(int val);
bool is_integer_zero(Number *num);
int number2int(Number *num);
unsigned int number2uint(Number *num);
long long number2long(Number *num);
unsigned long long number2ulong(Number *num);
short number2short(Number *num);
unsigned short number2ushort(Number *num);
char number2char(Number *num);
unsigned char number2uchar(Number *num);
bool number2bool(Number *num);
float number2float(Number *num);
double number2double(Number *num);
Number *number_add(Number *l, Number *r);
Number *number_sub(Number *l, Number *r);
Number *number_mul(Number *l, Number *r);
Number *number_div(Number *l, Number *r);
Number *number_mod(Number *l, Number *r);
Number *number_bitxor(Number *l, Number *r);
Number *number_bitor(Number *l, Number *r);
Number *number_bitand(Number *l, Number *r);
Number *number_bitnot(Number *l);
Number *number_lshift(Number *l, Number *r);
Number *number_rshift(Number *l, Number *r);
Number *number_eq(Number *l, Number *r);
Number *number_ne(Number *l, Number *r);
Number *number_lt(Number *l, Number *r);
Number *number_le(Number *l, Number *r);
Number *number_cond(Number *cond, Number *l, Number *r);
Number *number_lognot(Number *l);
Number *number_logand(Number *l, Number *r);
Number *number_logor(Number *l, Number *r);
Number *number_cast(Number *l, Type *type);
