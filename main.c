#include "boncc.h"
#include <stdio.h>

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  source_file_name = argv[1];
  source_code = read_file(source_file_name);

  token = tokenize(source_code);
  program();

  printf(".intel_syntax noprefix\n");

  for (int i = 0; i < strings->size; i++) {
    Variable *v = *(Variable **)vector_get(strings, i);
    printf("%.*s:\n", v->name_length, v->name);
    printf("  .string \"%s\"\n", v->string_literal);
  }

  for (int i = 0; i < globals->size; i++) {
    Variable *v = *(Variable **)vector_get(globals, i);
    printf(".data\n");
    printf(".globl %.*s\n", v->name_length, v->name);
    printf("%.*s:\n", v->name_length, v->name);
    printf("  .zero %d\n", v->type->size);
  }

  for (int i = 0; i < functions->size; i++) {
    Node *f = *(Node **)vector_get(functions, i);
    gen(f);
  }

  return 0;
}
