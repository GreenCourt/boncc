#include <stdarg.h>

int foo() { return 12; }
int add2(int x, int y) { return x + y; }
int add3(int x, int y, int z) { return x + add2(y, z); }
int ret5(int x, int y, int z) {
  return 5;
  return x + y + z;
}

int add12(int, int);
int add12(int x, int y) {
  int a = 12;
  return a + x + y;
}

int fib(int a) {
  int r = 0;
  if (a == 0)
    return 0;
  else if (a == 1)
    return 1;
  else
    return fib(a - 1) + fib(a - 2);
}

void voidfunc(int a) {
  a = a + 1;
  return;
}

int static_test1() {
  static int a = 0;
  a++;
  return a;
}

int static_test2(void) {
  static int a = -2;
  a += 2;
  return a;
}

int static_struct() {
  static struct {
    int a;
  } x;
  x.a = x.a + 3;
  return x.a;
}

int static_enum() {
  static enum { B } d;
  d = d + 4;
  return d;
}

int array_sum(int a[10]) {
  int s = 0;
  for (int i = 0; i < 10; ++i)
    s += a[i];
  return s;
}

_Bool retbool(int a) { return a; }

int add_double(int x, double y) { return x + y; }

void empty() {}

int vsprintf(char *, char *, va_list);

int sprintf_(char *s, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return vsprintf(s, fmt, ap);
}
