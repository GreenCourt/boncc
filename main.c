#include "boncc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "arguments required\n");
    return 1;
  }

  char *outpath = NULL;
  char *input_path = NULL;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      if (i == argc - 1) {
        fprintf(stderr, "invalid -o option\n");
        return 1;
      }
      outpath = argv[i + 1];
      ++i;
    } else if (input_path) {
      fprintf(stderr, "invalid number of arguments\n");
      return 1;
    } else {
      input_path = argv[i];
    }
  }

  if (input_path == NULL) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  if (outpath == NULL) {
    int len = strlen(input_path);
    if (len >= 2 && input_path[len - 2] == '.') {
      outpath = calloc(len + 1, sizeof(char));
      strncpy(outpath, input_path, len);
      outpath[len - 1] = 's';
      outpath[len] = '\0';
    } else {
      outpath = calloc(len + 3, sizeof(char));
      strncpy(outpath, input_path, len);
      outpath[len] = '.';
      outpath[len + 1] = 's';
      outpath[len + 2] = '\0';
    }
  }

  Token *tokens = tokenize(input_path);
  tokens = preprocess(tokens);
  parse(tokens);

  FILE *ostream = strcmp(outpath, "-") == 0 ? stdout : fopen(outpath, "w");
  generate_code(ostream);
  if (ostream != stdout)
    fclose(ostream);
  return 0;
}
