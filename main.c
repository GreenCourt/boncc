#include "boncc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  source_file_name = argv[1];
  char *source_code = read_file(source_file_name);

  token = tokenize(source_code);
  program();

  printf(".intel_syntax noprefix\n");

  for (int i = 0; i < strings->size; i++) {
    Variable *v = *(Variable **)vector_get(strings, i);
    printf("%.*s:\n", v->ident->len, v->ident->name);
    printf("  .string \"%s\"\n", v->string_literal);
  }

  gen_global_variables();

  printf(".text\n");
  for (int i = 0; i < functions->size; i++) {
    Node *f = *(Node **)vector_get(functions, i);
    gen(f);
  }

  return 0;
}
