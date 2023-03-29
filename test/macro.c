int line1 = __LINE__;
int line2 = __LINE__;

void verify(int expected, int actual, char *file_name, int line_number);

#define FOO 10
#define FOO 12 // redefine
#define BAR "bar"
#define EMPTY // empty macro
#define EMPTY2 EMPTY
#define EMPTY3(X, Y)
#define EMPTY4 EMPTY3
#define EMPTY5(X) EMPTY2
#define A0 1
#define A1 2
#define B0 4
#define B1 8
#define A01 A0 + EMPTY A1
#define B01 B0 + EMPTY2 B1
#define SUM_AB A01 + B01
#define MUL(X, Y) ((X) * (Y))
#define SUM(X, Y) ((X) + (Y))

int main() {
  verify(1, line1, __FILE__, __LINE__);
  verify(2, line2, __FILE__, __LINE__);
  verify(12, 5 + EMPTY + 7, EMPTY2 __FILE__, __LINE__);
  verify(12, 5 + EMPTY3(6, 7) + EMPTY5(12) 7, EMPTY4(8, 9) __FILE__, __LINE__);
  verify('t', __FILE__[0], __FILE__, __LINE__);
  verify('e', __FILE__[1], __FILE__, __LINE__);
  verify('s', __FILE__[2], __FILE__, __LINE__);
  verify('t', __FILE__[3], __FILE__, __LINE__);
  verify('/', __FILE__[4], __FILE__, __LINE__);
  verify('m', __FILE__[5], __FILE__, __LINE__);
  verify('a', __FILE__[6], __FILE__, __LINE__);
  verify('c', __FILE__[7], __FILE__, __LINE__);
  verify('r', __FILE__[8], __FILE__, __LINE__);
  verify('o', __FILE__[9], __FILE__, __LINE__);
  verify('.', __FILE__[10], __FILE__, __LINE__);
  verify('c', __FILE__[11], __FILE__, __LINE__);
  verify('\0', __FILE__[12], __FILE__, __LINE__);
  verify(12, FOO, __FILE__, __LINE__);
  verify('b', BAR[0], __FILE__, __LINE__);
  verify('a', BAR[1], __FILE__, __LINE__);
  verify('r', BAR[2], __FILE__, __LINE__);
  verify(3, A01, __FILE__, __LINE__);
  verify(12, B01, __FILE__, __LINE__);
  verify(15, SUM_AB, __FILE__, __LINE__);
  verify(24, MUL(12, 2), __FILE__, __LINE__);
  verify(24, MUL(10 + 2, 2), __FILE__, __LINE__);
  verify(24, MUL(SUM(10, 2), SUM(1, 1)), __FILE__, __LINE__);
  {
#define BBB
#define CCC
#define DDD
#undef DDD
#undef EEE // undef for non-existing macro is allowed
    int x;
#ifdef BBB
    x = 12;
#endif
    verify(12, x, __FILE__, __LINE__);
#ifdef CCC
    x = 14;
#endif
    verify(14, x, __FILE__, __LINE__);
#ifdef DDD
#ifdef BBB // nested
    x = 16;
#else
    x = 17
#endif
#endif
    verify(14, x, __FILE__, __LINE__);
#ifndef DDD
    x = 16;
#endif
    verify(16, x, __FILE__, __LINE__);
  }
  return 0;
}
