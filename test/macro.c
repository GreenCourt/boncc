int line1 = __LINE__;
int line2 = __LINE__;

#include "common.h"

int sprintf(char *s, char *fmt, ...);
#define SPRINTF(s, fmt, ...) sprintf(s, fmt, __VA_ARGS__)

#define FOO 10
#define FOO 12 // redefine
#define BAR \
  "bar"
#define EMPTY // empty macro
#define EMPTY2 EMPTY
#define EMPTY3(X, Y)
#define EMPTY4 EMPTY3
#define EMPTY5(X) EMPTY2
#define EMPTY6 EMPTY3(2, 3)
#define A0 1
#define A1 2
#define B0 4
#define B1 8
#define A01 A0 + EMPTY A1
#define B01 B0 + EMPTY2 B1
#define SUM_AB A01 + B01
#define MUL(X, Y) ((X) * (Y))
#define SUM(X, Y) ((X) + (Y))
#define NOPARAM() 8
#define JOIN(A, B) A##B
#define VAR12 var##1##2
#define VAR_HASH(...) var##__VA_ARGS__
#define THIRTEEN 1######3

int main() {
  verify(1, line1, __FILE__, __LINE__);
  verify(2, line2, __FILE__, __LINE__);
  verify(12, 5 + EMPTY + 7, EMPTY2 __FILE__, __LINE__);
  verify(12, 5 + EMPTY3(6, 7) + EMPTY5(12) 7, EMPTY4(8, 9) __FILE__, __LINE__);
  verify(12, 5 + EMPTY6 7, __FILE__, __LINE__);
  verify(8, NOPARAM(), __FILE__, __LINE__);
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
  verify(24, MUL(SUM(10, 2), MUL(2, 1)), __FILE__, __LINE__);
  verify(13, THIRTEEN, __FILE__, __LINE__);
  {
    int var12 = 12;
    verify(12, VAR12, __FILE__, __LINE__);
    verify(12, JOIN(var, 12), __FILE__, __LINE__);
    verify(12, JOIN(var1, 2), __FILE__, __LINE__);
  }
  {
    char buf[10];
    int x = SPRINTF(buf, "%d_%c_%d", 12, 't', 3);
    verify(6, x, __FILE__, __LINE__);
    verify('1', buf[0], __FILE__, __LINE__);
    verify('2', buf[1], __FILE__, __LINE__);
    verify('_', buf[2], __FILE__, __LINE__);
    verify('t', buf[3], __FILE__, __LINE__);
    verify('_', buf[4], __FILE__, __LINE__);
    verify('3', buf[5], __FILE__, __LINE__);
    verify('\0', buf[6], __FILE__, __LINE__);
  }
  {
    char buf[10];
    int var1 = 12;
    int x = SPRINTF(buf, "%d_%d_%d", VAR_HASH(1, 2, 3));
    verify(6, x, __FILE__, __LINE__);
    verify('1', buf[0], __FILE__, __LINE__);
    verify('2', buf[1], __FILE__, __LINE__);
    verify('_', buf[2], __FILE__, __LINE__);
    verify('2', buf[3], __FILE__, __LINE__);
    verify('_', buf[4], __FILE__, __LINE__);
    verify('3', buf[5], __FILE__, __LINE__);
    verify('\0', buf[6], __FILE__, __LINE__);
  }
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
#else
    x = 15;
#endif
    verify(16, x, __FILE__, __LINE__);
#if 1
    x = 8;
#else
    x = 7;
#endif
    verify(8, x, __FILE__, __LINE__);
#if 0
    x = 1;
#elif 1
    x = 2;
#endif
    verify(2, x, __FILE__, __LINE__);
#ifdef GGG
    x = 6;
#elif TTTT
    x = 7;
#else
    x = 9;
#endif
    verify(9, x, __FILE__, __LINE__);
#if 5 - 5 ? 1 : 0
    x = 1;
#elif 1
    x = 2;
#endif
    verify(2, x, __FILE__, __LINE__);
#if defined(AAA) && defined BBB
#if 1
    x = 4;
#else
#endif
#elif 1
    x = 6;
#endif
    verify(6, x, __FILE__, __LINE__);
#if defined CCC && defined(BBB)
    x = 9;
#elif 1
    x = 7;
#endif
    verify(9, x, __FILE__, __LINE__);
  }
  return 0;
}
