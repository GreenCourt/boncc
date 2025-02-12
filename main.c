#include "boncc.h"
#include <assert.h>
#include <errno.h>
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
  };

  include_path = new_vector(0, sizeof(char *));
  for (int i = 0; i < (int)(sizeof(default_include_path) / sizeof(char *)); ++i)
    vector_push(include_path, &default_include_path[i]);

  char *outpath = NULL;
  char *input_path = NULL;
  bool mmd = false;

  enum {
    ASSEMBLY,
    OBJECT_FILE,
    EXECUTABLE,
  } outformat = EXECUTABLE;

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
    } else if (strcmp(argv[i], "-c") == 0) {
      outformat = OBJECT_FILE;
    } else if (strcmp(argv[i], "-S") == 0) {
      outformat = ASSEMBLY;
    } else if (strcmp(argv[i], "-MMD") == 0) {
      mmd = true;
    } else if (strncmp(argv[i], "-W", 2) == 0) {
      // ignore
    } else if (strncmp(argv[i], "-f", 2) == 0) {
      // ignore
    } else if (strcmp(argv[i], "-w") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-g") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-O0") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-O1") == 0 || strcmp(argv[i], "-O") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-O2") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-O3") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-Og") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-Ofast") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-Os") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-Oz") == 0) {
      // ignore
    } else if (strncmp(argv[i], "-std=", 5) == 0) {
      // ignore
    } else if (strcmp(argv[i], "-pedantic") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-pedantic-errors") == 0) {
      // ignore
    } else if (strcmp(argv[i], "-") != 0 && argv[i][0] == '-') {
      fprintf(stderr, "unknown option %s\n", argv[i]);
      return 1;
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

  if ((outformat == OBJECT_FILE || outformat == EXECUTABLE) && outpath &&
      strcmp(outpath, "-") == 0) {
    fprintf(stderr, "output path must not be stdout for a binary format.\n");
    return 1;
  }

  char *assembly_path = NULL;
  char *elf_path = NULL;
  char *dep_path = NULL;

  switch (outformat) {
  case ASSEMBLY:
    assembly_path = outpath ? outpath : basename(replace_ext(input_path, "s"));
    dep_path = replace_ext(assembly_path, "d");
    outpath = assembly_path;
    break;
  case OBJECT_FILE:
    elf_path = outpath ? outpath : basename(replace_ext(input_path, "o"));
    assembly_path = replace_ext(elf_path, "s");
    dep_path = replace_ext(elf_path, "d");
    outpath = elf_path;
    break;
  case EXECUTABLE:
    elf_path = outpath ? outpath : "a.out";
    assembly_path = outpath ? replace_ext(elf_path, "s") : "a.out.s";
    dep_path = outpath ? replace_ext(elf_path, "d") : "a.out.d";
    outpath = elf_path;
    break;
  default:
    assert(false);
  }

  Token *tokens = tokenize(read_file(input_path), input_path);
  Vector *included = new_vector(0, sizeof(char *)); // *char
  tokens = preprocess(tokens, included);
  parse(tokens);

  if (mmd) {
    FILE *dep = fopen(dep_path, "w");
    if (!dep)
      error(NULL, "failed to open %s: %s", dep_path, strerror(errno));
    fprintf(dep, "%s: ", outpath);
    if (strcmp(input_path, "-") != 0)
      fprintf(dep, "%s", input_path);
    for (int i = 0; i < included->size; i++)
      fprintf(dep, " %s", *(char **)vector_get(included, i));
    fprintf(dep, "\n");
    fclose(dep);
  }

  FILE *ostream =
      strcmp(assembly_path, "-") == 0 ? stdout : fopen(assembly_path, "w");
  if (!ostream)
    error(NULL, "failed to open %s: %s", assembly_path, strerror(errno));
  generate_code(ostream);
  if (ostream != stdout)
    fclose(ostream);

  if (outformat == OBJECT_FILE) {
    char *cmd[] = {"as", "-g", "-o", elf_path, assembly_path, NULL};
    if (execvp(cmd[0], cmd) != 0) {
      fprintf(stderr, "failed to execute assembler\n");
      return 1;
    }
  } else if (outformat == EXECUTABLE) {
    char *cmd[] = {"cc", "-g",     "-x",          "assembler", "-znoexecstack",
                   "-o", elf_path, assembly_path, NULL};
    if (execvp(cmd[0], cmd) != 0) {
      fprintf(stderr, "failed to execute assembler\n");
      return 1;
    }
  }
  return 0;
}
