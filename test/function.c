void verify(int expected, int actual, char *file_name, int line_number);

int foo() { return 12; }
int add2(int x, int y) { return x + y; }
int add3(int x, int y, int z) { return x + add2(y, z); }
int ret5(int x, int y, int z) {
  return 5;
  return x + y + z;
}

int add12(int x, int y) {
  int a;
  a = 12;
  return a + x + y;
}

int fib(int a) {
  int r;
  r = 0;
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
  a = a + 1;
  return a;
}

int static_test2(void) {
  static int a = -2;
  a = a + 2;
  return a;
}

int static_struct() {
  static struct { int a; } x;
  x.a = x.a + 3;
  return x.a;
}

int static_enum() {
  static enum { B } d;
  d = d + 4;
  return d;
}

int main() {
  verify(12, add3(1, 5, 6), __FILE__, __LINE__);
  verify(5, ret5(1, 5, 6), __FILE__, __LINE__);
  verify(18, add12(1, 5), __FILE__, __LINE__);
  verify(5, fib(5), __FILE__, __LINE__);
  verify(8, fib(6), __FILE__, __LINE__);
  verify(13, fib(7), __FILE__, __LINE__);
  voidfunc(4);

  verify(1, static_test1(), __FILE__, __LINE__);
  verify(2, static_test1(), __FILE__, __LINE__);
  verify(3, static_test1(), __FILE__, __LINE__);

  verify(0, static_test2(), __FILE__, __LINE__);
  verify(2, static_test2(), __FILE__, __LINE__);
  verify(4, static_test2(), __FILE__, __LINE__);

  verify(3, static_struct(), __FILE__, __LINE__);
  verify(6, static_struct(), __FILE__, __LINE__);
  verify(9, static_struct(), __FILE__, __LINE__);

  verify(4, static_enum(), __FILE__, __LINE__);
  verify(8, static_enum(), __FILE__, __LINE__);
  verify(12, static_enum(), __FILE__, __LINE__);

  {
    int a;
    a = foo();
    verify(12, a, __FILE__, __LINE__);
  }
  {
    int a;
    a = add2(1, 5);
    verify(6, a, __FILE__, __LINE__);
  }
  {
    int a;
    a = add3(1, 5, 6);
    verify(12, a, __FILE__, __LINE__);
  }
  return 0;
}
