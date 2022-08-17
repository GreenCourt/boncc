typedef enum {
  TK_RESERVED,
  TK_NUM,
  TK_EOF,
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind;
  Token *next;
  int val;
  char *str;
  int len;
};

typedef enum {
  ND_ADD, // +
  ND_SUB, // -
  ND_MUL, // *
  ND_DIV, // /
  ND_EQ,  // ==
  ND_NE,  // !=
  ND_LT,  // <
  ND_LE,  // <=
  ND_NUM, // integer
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;
};

extern char *user_input;
extern Token *token;

void gen(Node *node);
Token *tokenize(char *p);
Node *expr();
