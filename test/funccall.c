#include "common.h"
#include <stdarg.h>

int foo();
int add2(int x, int y);
int add3(int x, int y, int z);
int ret5(int x, int y, int z);
int add12(int x, int y);
int fib(int a);
void voidfunc(int a);
int static_test1();
int static_test2(void);
int static_struct();
int static_enum();
int array_sum(int a[10]);
_Bool retbool(int a);
int add_double(int x, double y);
void empty();
int vsprintf(char *, char *, va_list);
int sprintf_(char *s, char *fmt, ...);

int main() {
  verify(12, add3(1, 5, 6), __FILE__, __LINE__);
  verify(5, ret5(1, 5, 6), __FILE__, __LINE__);
  verify(18, add12(1, 5), __FILE__, __LINE__);
  verify(5, fib(5), __FILE__, __LINE__);
  verify(8, fib(6), __FILE__, __LINE__);
  verify(13, fib(7), __FILE__, __LINE__);
  voidfunc(4);
  empty();

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
    int a = foo();
    verify(12, a, __FILE__, __LINE__);
  }
  {
    int a = add2(1, 5);
    verify(6, a, __FILE__, __LINE__);
  }
  {
    int a = add3(1, 5, 6);
    verify(12, a, __FILE__, __LINE__);
  }
  {
    int (*p_add2)(int, int) = add2;
    int (*p_add3)(int, int, int) = add3;
    int (*p_fib)(int) = fib;
    int (*p_foo)() = foo;
    verify(6, p_add2(2, 4), __FILE__, __LINE__);
    verify(12, p_add3(2, 4, 6), __FILE__, __LINE__);
    verify(8, p_fib(6), __FILE__, __LINE__);
    verify(12, p_foo(), __FILE__, __LINE__);

    void (*p_voidfunc)(int) = voidfunc;
    p_voidfunc(12);
  }
  {
    verify(1, retbool(12), __FILE__, __LINE__);
    verify(1, retbool(1), __FILE__, __LINE__);
    verify(0, retbool(0), __FILE__, __LINE__);
  }
  {
    char buf[10];
    int x = sprintf_(buf, "%d_%c_%d", 12, 't', 3);
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
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    verify(55, array_sum(arr), __FILE__, __LINE__);
  }
  { verify(4, add_double(1, 3.0), __FILE__, __LINE__); }

  return 0;
}
