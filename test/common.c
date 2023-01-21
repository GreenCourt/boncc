#include <stdio.h>
#include <stdlib.h>

void verify(long long expected, long long actual, char *file_name, int line_number) {
  if (expected == actual)
    return;
  printf("%s:%lld: %lld expected but got %d\n", file_name, line_number, expected, actual);
  exit(1);
}
