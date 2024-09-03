#include <assert.h>
#include <libgen.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char **argv) {
  /*
   * convert given text files to char arrays of C
   */
  for (int i = 1; i < argc; ++i) {
    char *path = argv[i];
    FILE *fp = fopen(path, "r");
    assert(fp);
    char *name = basename(path); // may modify path
    int len = strlen(name);
    assert(len >= 2 && name[len - 2] == '.' && name[len - 1] == 'h');
    name[len - 2] = '_';

    printf("char %s[] = { ", name);
    while (1) {
      char buf[4096];
      int n = fread(buf, 1, sizeof(buf), fp);
      if (n == 0)
        break;
      for (int j = 0; j < n; ++j)
        printf("0x%x, ", buf[j]);
    }
    printf("0x0 };\n");
    fclose(fp);
  }
}
