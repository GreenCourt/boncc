#include "boncc.h"
#include <libgen.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

int main(int argc, char **argv) {
  if (argc == 1) {
    fprintf(stderr, "arguments required\n");
    return 1;
  }

  char *default_include_path[] = {
      "/usr/include",
      "/usr/local/include",
      "/usr/include/x86_64-linux-gnu",
      BONCC_INCLUDE_PATH,
  };

  include_path = new_vector(0, sizeof(char *));
  for (int i = 0; i < (int)(sizeof(default_include_path) / sizeof(char *)); ++i)
    vector_push(include_path, &default_include_path[i]);

  char *outpath = NULL;
  char *input_path = NULL;
  bool assemble = true;

  for (int i = 1; i < argc; ++i) {
    if (strcmp(argv[i], "-o") == 0) {
      if (i == argc - 1) {
        fprintf(stderr, "invalid -o option\n");
        return 1;
      }
      outpath = argv[i + 1];
      ++i;
    } else if (strncmp(argv[i], "-D", 2) == 0) {
      int len = strlen(argv[i]);
      if (len == 2 && i == argc - 1) {
        fprintf(stderr, "macro name required for -D option\n");
        return 1;
      }
      if (len == 2) {
        define_macro_from_command_line(argv[i + 1]);
        ++i;
      } else {
        define_macro_from_command_line(argv[i] + 2);
      }
    } else if (strncmp(argv[i], "-c", 2) == 0) {
      assemble = true;
    } else if (strncmp(argv[i], "-S", 2) == 0) {
      assemble = false;
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

  if (input_path[strlen(input_path) - 1] == '/') {
    fprintf(stderr, "%s is a directory\n", input_path);
    return 1;
  }

  if (outpath && outpath[strlen(outpath) - 1] == '/') {
    fprintf(stderr, "output path must not be a directory.\n");
    return 1;
  }

  if (assemble && outpath && strcmp(outpath, "-") == 0) {
    fprintf(stderr,
            "output path must not be stdout for a binary object file.\n");
    return 1;
  }

  char *assembly_path = NULL;
  char *object_path = NULL;

  if (assemble) {
    object_path = outpath ? outpath : basename(replace_ext(input_path, "o"));
    assembly_path = replace_ext(object_path, "s");
  } else {
    assembly_path = outpath ? outpath : basename(replace_ext(input_path, "s"));
  }

  Token *tokens = tokenize(read_file(input_path), input_path);
  tokens = preprocess(tokens);
  parse(tokens);

  FILE *ostream =
      strcmp(assembly_path, "-") == 0 ? stdout : fopen(assembly_path, "w");
  generate_code(ostream);
  if (ostream != stdout)
    fclose(ostream);

  if (assemble) {
    char *cmd[] = {"as", "-g", "-o", object_path, assembly_path, NULL};
    if (execvp(cmd[0], cmd) != 0) {
      fprintf(stderr, "failed to execute assembler\n");
      return 1;
    }
  }
  return 0;
}
