#include <stdio.h>
#include <stdlib.h>

int external_int = 1234;
char external_char = 56;
short external_short = 789;

void verify(long long expected, long long actual, char *file_name,
            int line_number) {
  if (expected == actual)
    return;
  printf("%s:%lld: %lld expected but got %d\n", file_name, line_number,
         expected, actual);
  exit(1);
}
