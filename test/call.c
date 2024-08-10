#include "common.h"
#include "func.h"

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
  {
    S1 s = return_struct1(5);
    verify(5, s.x, __FILE__, __LINE__);
  }
  {
    S2 s = return_struct2(6);
    verify(6, s.x, __FILE__, __LINE__);
  }
  {
    S3 s = return_struct3(7);
    verify(7, s.x, __FILE__, __LINE__);
  }
  {
    S4 s = return_struct4(8.8f);
    verify(1, s.x == 8.8f, __FILE__, __LINE__);
  }
  {
    S5 s = return_struct5(1.0);
    verify(1, s.x == 1.0, __FILE__, __LINE__);
  }
  {
    S6 s = return_struct6(2.2f, 3.3f);
    verify(1, s.x == 2.2f, __FILE__, __LINE__);
    verify(1, s.y == 3.3f, __FILE__, __LINE__);
  }
  {
    S7 s = return_struct7(4.4f, 5);
    verify(1, s.x == 4.4f, __FILE__, __LINE__);
    verify(5, s.y, __FILE__, __LINE__);
  }
  {
    S8 s = return_struct8(6, 7.7f);
    verify(6, s.x, __FILE__, __LINE__);
    verify(1, s.y == 7.7f, __FILE__, __LINE__);
  }
  {
    S9 s = return_struct9(8, 9.9f);
    verify(8, s.x, __FILE__, __LINE__);
    verify(1, s.y == 9.9f, __FILE__, __LINE__);
  }
  {
    S10 s = return_struct10(1.1f, 3);
    verify(1, s.x == 1.1f, __FILE__, __LINE__);
    verify(3, s.y, __FILE__, __LINE__);
  }
  {
    S11 s = return_struct11(2.2, 4);
    verify(1, s.x == 2.2, __FILE__, __LINE__);
    verify(4, s.y, __FILE__, __LINE__);
  }
  {
    S12 s = return_struct12(3.3f, 4, 5.5f);
    verify(1, s.x == 3.3f, __FILE__, __LINE__);
    verify(4, s.y, __FILE__, __LINE__);
    verify(1, s.z == 5.5f, __FILE__, __LINE__);
  }
  {
    S13 s = return_struct13(1, 2, 3, 4, 5);
    verify(1, s.x[0], __FILE__, __LINE__);
    verify(2, s.x[1], __FILE__, __LINE__);
    verify(3, s.x[2], __FILE__, __LINE__);
    verify(4, s.y, __FILE__, __LINE__);
    verify(5, s.z, __FILE__, __LINE__);
  }
  {
    S14 s = return_struct14(1.1f, 2.2f, 3.3f, 4, 5);
    verify(1, s.x[0] == 1.1f, __FILE__, __LINE__);
    verify(1, s.x[1] == 2.2f, __FILE__, __LINE__);
    verify(1, s.x[2] == 3.3f, __FILE__, __LINE__);
    verify(4, s.y, __FILE__, __LINE__);
    verify(5, s.z, __FILE__, __LINE__);
  }
  {
    S15 s = return_struct15(5, 6, 7.7f, 8.8f, 9.9f);
    verify(5, s.x, __FILE__, __LINE__);
    verify(6, s.y, __FILE__, __LINE__);
    verify(1, s.z[0] == 7.7f, __FILE__, __LINE__);
    verify(1, s.z[1] == 8.8f, __FILE__, __LINE__);
    verify(1, s.z[2] == 9.9f, __FILE__, __LINE__);
  }
  {
    S16 s = return_struct16(4.4f, 5.5f, 6.6f, 7.7f);
    verify(1, s.x[0] == 4.4f, __FILE__, __LINE__);
    verify(1, s.x[1] == 5.5f, __FILE__, __LINE__);
    verify(1, s.x[2] == 6.6f, __FILE__, __LINE__);
    verify(1, s.x[3] == 7.7f, __FILE__, __LINE__);
  }
  {
    S17 s = return_struct17(8.8, 9.9);
    verify(1, s.x[0] == 8.8, __FILE__, __LINE__);
    verify(1, s.x[1] == 9.9, __FILE__, __LINE__);
  }
  {
    S18 s;
    s = return_struct18x(2, 4, 6);
    verify(2, s.u.x[0], __FILE__, __LINE__);
    verify(4, s.u.x[1], __FILE__, __LINE__);
    verify(6, s.a, __FILE__, __LINE__);
    s = return_struct18y(3.3, 5);
    verify(1, s.u.y == 3.3, __FILE__, __LINE__);
    verify(5, s.a, __FILE__, __LINE__);
    s = return_struct18z(7, 8);
    verify(7, s.u.z, __FILE__, __LINE__);
    verify(8, s.a, __FILE__, __LINE__);
  }
  {
    S19 s = return_struct19(6, 9);
    verify(6, s.x[0], __FILE__, __LINE__);
    verify(9, s.x[1], __FILE__, __LINE__);
  }
  {
    S20 s = return_struct20(5, 2, 9, 3);
    verify(5, s.x[0], __FILE__, __LINE__);
    verify(2, s.x[1], __FILE__, __LINE__);
    verify(9, s.x[2], __FILE__, __LINE__);
    verify(3, s.x[3], __FILE__, __LINE__);
  }
  {
    S21 s = return_struct21(7.7, 6.6, 3, 5);
    verify(1, s.x[0] == 7.7, __FILE__, __LINE__);
    verify(1, s.x[1] == 6.6, __FILE__, __LINE__);
    verify(3, s.y, __FILE__, __LINE__);
    verify(5, s.z, __FILE__, __LINE__);
  }
  {
    U1 u;
    u = return_union1x(3, 2, 4);
    verify(3, u.x[0], __FILE__, __LINE__);
    verify(2, u.x[1], __FILE__, __LINE__);
    verify(4, u.x[2], __FILE__, __LINE__);
    u = return_union1y(5.5);
    verify(1, u.y == 5.5, __FILE__, __LINE__);
    u = return_union1z(6);
    verify(6, u.z, __FILE__, __LINE__);
  }
  {
    U2 u;
    u = return_union2x(1.1f, 2.2f, 3.3f);
    verify(1, u.x[0] == 1.1f, __FILE__, __LINE__);
    verify(1, u.x[1] == 2.2f, __FILE__, __LINE__);
    verify(1, u.x[2] == 3.3f, __FILE__, __LINE__);
    u = return_union2y(4.4f, 5.5f);
    verify(1, u.y[0] == 4.4f, __FILE__, __LINE__);
    verify(1, u.y[1] == 5.5f, __FILE__, __LINE__);
    u = return_union2z(6.6f);
    verify(1, u.z == 6.6f, __FILE__, __LINE__);
  }
  {
    U3 u;
    u = return_union3x(2, 3);
    verify(2, u.x[0], __FILE__, __LINE__);
    verify(3, u.x[1], __FILE__, __LINE__);
    u = return_union3y(4);
    verify(4, u.y, __FILE__, __LINE__);
    u = return_union3z(5.5f, 6.6f);
    verify(1, u.z[0] == 5.5f, __FILE__, __LINE__);
    verify(1, u.z[1] == 6.6f, __FILE__, __LINE__);
  }
  {
    U4 u;
    u = return_union4x(2.2f, 3.3f);
    verify(1, u.x[0] == 2.2f, __FILE__, __LINE__);
    verify(1, u.x[1] == 3.3f, __FILE__, __LINE__);
    u = return_union4y(4.4);
    verify(1, u.y == 4.4, __FILE__, __LINE__);
    u = return_union4z(5.5f);
    verify(1, u.z == 5.5f, __FILE__, __LINE__);
  }
  {
    U5 u;
    u = return_union5x(6.6);
    verify(1, u.x == 6.6, __FILE__, __LINE__);
    u = return_union5y(7);
    verify(7, u.y, __FILE__, __LINE__);
  }
  {
    U6 u;
    u = return_union6x(8);
    verify(8, u.x, __FILE__, __LINE__);
    u = return_union6y(9.9);
    verify(1, u.y == 9.9, __FILE__, __LINE__);
  }
  {
    U7 u;
    u = return_union7x(8.8f);
    verify(1, u.x == 8.8f, __FILE__, __LINE__);
    u = return_union7y(3);
    verify(3, u.y, __FILE__, __LINE__);
    u = return_union7z(5, 6, 7, 8);
    verify(5, u.z[0], __FILE__, __LINE__);
    verify(6, u.z[1], __FILE__, __LINE__);
    verify(7, u.z[2], __FILE__, __LINE__);
    verify(8, u.z[3], __FILE__, __LINE__);
  }

  return 0;
}
