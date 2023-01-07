#include <stdio.h>
#include <stdlib.h>

void verify(int expected, int actual, char *file_name, int line_number) {
  if (expected == actual)
    return;
  printf("%s:%d: %d expected but got %d\n", file_name, line_number, expected, actual);
  exit(1);
}

void alloc4(int **pp, int a, int b, int c, int d) {
  *pp = (int *)malloc(sizeof(int) * 4);
  (*pp)[0] = a;
  (*pp)[1] = b;
  (*pp)[2] = c;
  (*pp)[3] = d;
}
