#include "boncc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  user_input = argv[1];

  token = tokenize(user_input);
  program();

  printf(".intel_syntax noprefix\n");

  for (int i = 0; i < globals->size; i++) {
    Variable *v = *(Variable **)vector_get(globals, i);
    printf(".data\n");
    printf(".globl %.*s\n", v->len, v->name);
    printf("%.*s:\n", v->len, v->name);
    printf("  .zero %d\n", v->type->size);
  }

  for (int i = 0; i < functions->size; i++) {
    Node *f = *(Node **)vector_get(functions, i);
    gen(f);
  }

  return 0;
}
