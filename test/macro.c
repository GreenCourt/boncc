int line1 = __LINE__;
int line2 = __LINE__;

#include "common.h"

int sprintf(char *s, const char *fmt, ...);
#define SPRINTF(s, fmt, ...) sprintf(s, fmt, __VA_ARGS__)

#define FOO 10
#define FOO 12 // redefine
// clang-format off
#define BAR \
  "bar"
// clang-format on
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
#define JOIN3(A, B, C) A##B##C
#define VAR12 var##1##2
#define VAR_HASH(...) var##__VA_ARGS__
#define THIRTEEN 1######3
#define VARIADIC_EMPTY(A, B, ...)
#define VARIADIC_PRE(A, B, ...) __VA_ARGS__, (A) + (B)
#define VARIADIC_POST(A, B, ...) (A) + (B), __VA_ARGS__
#define VARIADIC_ARGS_ONLY(...) __VA_ARGS__
#define TO_STRING_LITERAL(x) #x
#define MACRO_VALUE_TO_STRING_LITERAL(x) TO_STRING_LITERAL(x)
#define SAME(x) x
#define THIRD(a, b, c) c
#define THIRD4(a, b, c, d) c
// clang-format off
#define PATH_ABC /a/b/c
#define PATH_ABC_WITH_SPACE /a /b/c
// clang-format on

int main() {
  verify(1, line1);
  verify(2, line2);
  verify(12, 5 + EMPTY + 7 EMPTY2);
  verify(12, 5 + EMPTY3(6, 7) + EMPTY5(12) 7 EMPTY4(8, 9));
  verify(12, 5 + EMPTY6 7);
  verify(8, NOPARAM());
  verify('t', __FILE__[0]);
  verify('e', __FILE__[1]);
  verify('s', __FILE__[2]);
  verify('t', __FILE__[3]);
  verify('/', __FILE__[4]);
  verify('m', __FILE__[5]);
  verify('a', __FILE__[6]);
  verify('c', __FILE__[7]);
  verify('r', __FILE__[8]);
  verify('o', __FILE__[9]);
  verify('.', __FILE__[10]);
  verify('c', __FILE__[11]);
  verify('\0', __FILE__[12]);
  verify(12, FOO);
  verify('b', BAR[0]);
  verify('a', BAR[1]);
  verify('r', BAR[2]);
  verify(3, A01);
  verify(12, B01);
  verify(15, SUM_AB);
  verify(24, MUL(12, 2));
  verify(24, MUL(10 + 2, 2));
  verify(24, MUL(SUM(10, 2), SUM(1, 1)));
  verify(24, MUL(SUM(10, 2), MUL(2, 1)));
  verify(13, THIRTEEN);

  verify(1, COMMAND_ARG_OBJ_LIKE_ONE);
  verify(1, COMMAND_ARG_FUNC_LIKE_ONE(a, b, c, d, e, f));
  verify(1, 4 COMMAND_ARG_OBJ_LIKE_EMPTY - 3);
  verify(1, COMMAND_ARG_FUNC_LIKE_EMPTY(i, j) 2 - 1);
  verify(1, COMMAND_ARG_OBJ_LIKE_ONE2);
  verify(1, COMMAND_ARG_FUNC_LIKE_ONE2(aaaa));
  verify(1, 3 - COMMAND_ARG_OBJ_LIKE_EMPTY2 2);
  verify(1, 5 - 4 COMMAND_ARG_FUNC_LIKE_EMPTY2(ppp));
  verify(22, COMMAND_ARG_OBJ_LIKE_22);
  verify(2345, COMMAND_ARG_FUNC_LIKE_JOIN(23, 45));
  verify(24, COMMAND_ARG_OBJ_LIKE_24);
  verify(-34, COMMAND_ARG_FUNC_LIKE_MINUS(34));

  {
    int var12 = 12;
    verify(12, VAR12);
    verify(12, JOIN(var, 12));
    verify(12, JOIN(var1, 2));
  }
  {
    char buf[10];
    int x = SPRINTF(buf, "%d_%c_%d", 12, 't', 3);
    verify(6, x);
    verify('1', buf[0]);
    verify('2', buf[1]);
    verify('_', buf[2]);
    verify('t', buf[3]);
    verify('_', buf[4]);
    verify('3', buf[5]);
    verify('\0', buf[6]);
  }
  {
    char buf[10];
    int var1 = 12;
    int x = SPRINTF(buf, "%d_%d_%d", VAR_HASH(1, 2, 3));
    verify(6, x);
    verify('1', buf[0]);
    verify('2', buf[1]);
    verify('_', buf[2]);
    verify('2', buf[3]);
    verify('_', buf[4]);
    verify('3', buf[5]);
    verify('\0', buf[6]);
  }
  {
    verify(2, 1 + VARIADIC_EMPTY(aa, bb, cc) 1);
    verify(2, 1 + VARIADIC_EMPTY(aa, bb) 1);
    verify(2, 1 + 1 VARIADIC_ARGS_ONLY());

    {
      char buf[10];
      int x = sprintf(buf, "%d_%d_%d", VARIADIC_PRE(3, 4, 5, 6));
      verify(5, x);
      verify('5', buf[0]);
      verify('_', buf[1]);
      verify('6', buf[2]);
      verify('_', buf[3]);
      verify('7', buf[4]);
      verify('\0', buf[5]);
    }

    {
      char buf[10];
      int x = sprintf(buf, "%d_%d", 4 VARIADIC_PRE(2, 3));
      verify(3, x);
      verify('4', buf[0]);
      verify('_', buf[1]);
      verify('5', buf[2]);
      verify('\0', buf[3]);
    }

    {
      char buf[10];
      int x = sprintf(buf, "%d_%d_%d", VARIADIC_POST(3, 4, 5, 6));
      verify(5, x);
      verify('7', buf[0]);
      verify('_', buf[1]);
      verify('5', buf[2]);
      verify('_', buf[3]);
      verify('6', buf[4]);
      verify('\0', buf[5]);
    }

    {
      char buf[10];
      int x = sprintf(buf, "%d_%d", VARIADIC_POST(2, 3) 4);
      verify(3, x);
      verify('5', buf[0]);
      verify('_', buf[1]);
      verify('4', buf[2]);
      verify('\0', buf[3]);
    }

    {
      char buf[10];
      int x = sprintf(buf, "%d_%d_%d", VARIADIC_ARGS_ONLY(3, 4, 5));
      verify(5, x);
      verify('3', buf[0]);
      verify('_', buf[1]);
      verify('4', buf[2]);
      verify('_', buf[3]);
      verify('5', buf[4]);
      verify('\0', buf[5]);
    }
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL(4567));
    verify(4, x);
    verify('4', buf[0]);
    verify('5', buf[1]);
    verify('6', buf[2]);
    verify('7', buf[3]);
    verify('\0', buf[4]);
  }
  {
    char buf[10];
    int x = sprintf(buf, MACRO_VALUE_TO_STRING_LITERAL(FOO));
    verify(2, x);
    verify('1', buf[0]);
    verify('2', buf[1]);
    verify('\0', buf[2]);
  }
  {
    char buf[10];
    int x = sprintf(buf, MACRO_VALUE_TO_STRING_LITERAL(PATH_ABC));
    verify(6, x);
    verify('/', buf[0]);
    verify('a', buf[1]);
    verify('/', buf[2]);
    verify('b', buf[3]);
    verify('/', buf[4]);
    verify('c', buf[5]);
    verify('\0', buf[6]);
  }
  {
    char buf[10];
    int x = sprintf(buf, MACRO_VALUE_TO_STRING_LITERAL(PATH_ABC_WITH_SPACE));
    verify(7, x);
    verify('/', buf[0]);
    verify('a', buf[1]);
    verify(' ', buf[2]);
    verify('/', buf[3]);
    verify('b', buf[4]);
    verify('/', buf[5]);
    verify('c', buf[6]);
    verify('\0', buf[7]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL('x'));
    verify(3, x);
    verify('\'', buf[0]);
    verify('x', buf[1]);
    verify('\'', buf[2]);
    verify('\0', buf[3]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL('"'));
    verify(3, x);
    verify('\'', buf[0]);
    verify('"', buf[1]);
    verify('\'', buf[2]);
    verify('\0', buf[3]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL("abc"));
    verify(5, x);
    verify('"', buf[0]);
    verify('a', buf[1]);
    verify('b', buf[2]);
    verify('c', buf[3]);
    verify('"', buf[4]);
    verify('\0', buf[5]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL('\n'));
    verify(4, x);
    verify('\'', buf[0]);
    verify('\\', buf[1]);
    verify('n', buf[2]);
    verify('\'', buf[3]);
    verify('\0', buf[4]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL('\\'));
    verify(4, x);
    verify('\'', buf[0]);
    verify('\\', buf[1]);
    verify('\\', buf[2]);
    verify('\'', buf[3]);
    verify('\0', buf[4]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL("ab\\c"));
    verify(7, x);
    verify('"', buf[0]);
    verify('a', buf[1]);
    verify('b', buf[2]);
    verify('\\', buf[3]);
    verify('\\', buf[4]);
    verify('c', buf[5]);
    verify('"', buf[6]);
    verify('\0', buf[7]);
  }
  {
    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL("ab\"c"));
    verify(7, x);
    verify('"', buf[0]);
    verify('a', buf[1]);
    verify('b', buf[2]);
    verify('\\', buf[3]);
    verify('"', buf[4]);
    verify('c', buf[5]);
    verify('"', buf[6]);
    verify('\0', buf[7]);
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
    verify(12, x);
#ifdef CCC
    x = 14;
#endif
    verify(14, x);
#ifdef DDD
#ifdef BBB // nested
    x = 16;
#else
    x = 17
#endif
#endif
    verify(14, x);
#ifndef DDD
    x = 16;
#else
    x = 15;
#endif
    verify(16, x);
#if 1
    x = 8;
#else
    x = 7;
#endif
    verify(8, x);
#if 0
    x = 1;
#elif 1
    x = 2;
#endif
    verify(2, x);
#ifdef GGG
    x = 6;
#elif TTTT
    x = 7;
#else
    x = 9;
#endif
    verify(9, x);
#if 5 - 5 ? 1 : 0
    x = 1;
#elif 1
    x = 2;
#endif
    verify(2, x);
#if defined(AAA) && defined BBB
#if 1
    x = 4;
#else
#endif
#elif 1
    x = 6;
#endif
    verify(6, x);
#if defined CCC && defined(BBB)
    x = 9;
#elif 1
    x = 7;
#endif
    verify(9, x);
  }
  {
    // empty argument for function-like macro
    verify(12, SAME(12));
    verify(12, SAME(/*empty argument*/) 12);
    verify(13, SAME(EMPTY) 13);
    verify(14, SAME(EMPTY5(6)) 14);
    verify(15, SAME(EMPTY6) 15);
    verify(7, THIRD4(1, , 7, 5));
    verify(9, THIRD4(, , 9, ));
    verify(4, 4 THIRD4(, , , ));
    verify(8, 8 THIRD4(, , EMPTY, ));
    verify(62, JOIN(, 62));
    verify(63, JOIN(63, ));
    verify(565, JOIN(, ) 565);

    verify(123, JOIN3(1, 2, 3));
    verify(12, JOIN3(1, 2, ));
    verify(13, JOIN3(1, , 3));
    verify(23, JOIN3(, 2, 3));
    verify(1, JOIN3(1, , ));
    verify(2, JOIN3(, 2, ));
    verify(3, JOIN3(, , 3));
    verify(5, JOIN3(, , ) 5);
    verify(7, JOIN(, ) 7);

    char buf[10];
    int x = sprintf(buf, TO_STRING_LITERAL());
    verify(0, x);
    verify('\0', buf[0]);
  }
  return 0;
}
