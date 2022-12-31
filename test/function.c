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

int main() {
  // verify(12, ({
  //         int a;
  //         a = foo();
  //         a;
  //       }),
  //       __FILE__, __LINE__);
  // verify(6, ({
  //         int a;
  //         a = add2(1, 5);
  //         a;
  //       }),
  //       __FILE__, __LINE__);
  // verify(12, ({
  //         int a;
  //         a = add3(1, 5, 6);
  //         a;
  //       }),
  //       __FILE__, __LINE__);
  verify(12, add3(1, 5, 6), __FILE__, __LINE__);
  verify(5, ret5(1, 5, 6), __FILE__, __LINE__);
  verify(18, add12(1, 5), __FILE__, __LINE__);
  verify(5, fib(5), __FILE__, __LINE__);
  verify(8, fib(6), __FILE__, __LINE__);
  verify(13, fib(7), __FILE__, __LINE__);
  return 0;
}
