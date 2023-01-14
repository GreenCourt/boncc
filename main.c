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

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      if (i == argc - 1) {
        fprintf(stderr, "invalid -o option\n");
        return 1;
      }
      outpath = argv[i + 1];
      ++i;
    } else if (source_file_name) {
      fprintf(stderr, "invalid number of arguments\n");
      return 1;
    } else {
      source_file_name = argv[i];
    }
  }

  if (source_file_name == NULL) {
    fprintf(stderr, "invalid number of arguments\n");
    return 1;
  }

  if (outpath == NULL) {
    int len = strlen(source_file_name);
    if (len >= 2 && source_file_name[len - 2] == '.') {
      outpath = calloc(len + 1, sizeof(char));
      strncpy(outpath, source_file_name, len);
      outpath[len - 1] = 's';
      outpath[len] = '\0';
    } else {
      outpath = calloc(len + 3, sizeof(char));
      strncpy(outpath, source_file_name, len);
      outpath[len] = '.';
      outpath[len + 1] = 's';
      outpath[len + 2] = '\0';
    }
  }

  char *source_code = read_file(source_file_name);

  token = tokenize(source_code);
  program();

  FILE *ostream = strcmp(outpath, "-") == 0 ? stdout : fopen(outpath, "w");
  gen_toplevel(ostream);
  if (ostream != stdout)
    fclose(ostream);
  return 0;
}
