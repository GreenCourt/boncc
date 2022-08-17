#include "boncc.h"
#include <stdio.h>

char *user_input;
Token *token;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];

  token = tokenize(argv[1]);
  Node *node = expr();

  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  gen(node);

  printf("  pop rax\n");
  printf("  ret\n");
  return 0;
}
