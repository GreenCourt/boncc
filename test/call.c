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
  verify(-1, minus1(), __FILE__, __LINE__);

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
    u = return_union7z(5, 13 - 7, 7, 4 + 4);
    verify(5, u.z[0], __FILE__, __LINE__);
    verify(6, u.z[1], __FILE__, __LINE__);
    verify(7, u.z[2], __FILE__, __LINE__);
    verify(8, u.z[3], __FILE__, __LINE__);
  }
  {
    verify(511, pass_by_stack1(1, 2, 4, 8, 16, 32, 64, 128, 256), __FILE__,
           __LINE__);
    verify(-1, pass_by_stack2(1, 2, 1.2f, 2.2, 4, 8, 16, 32, 64, 128, 256),
           __FILE__, __LINE__);
    verify(511, pass_by_stack2(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256),
           __FILE__, __LINE__);
    verify(-1, pass_by_stack2(1, 2, 1.1f, 2.3, 4, 8, 16, 32, 64, 128, 256),
           __FILE__, __LINE__);
    verify(511, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.1, 2.2f),
           __FILE__, __LINE__);
    verify(-1, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.11, 2.2f),
           __FILE__, __LINE__);
    verify(-1, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.1, 2.22f),
           __FILE__, __LINE__);
    verify(
        511,
        pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3, 4.4f),
        __FILE__, __LINE__);
    verify(
        -1,
        pass_by_stack4(1, 2, 1.11f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3, 4.4f),
        __FILE__, __LINE__);
    verify(
        -1,
        pass_by_stack4(1, 2, 1.1f, 2.22, 4, 8, 16, 32, 64, 128, 256, 3.3, 4.4f),
        __FILE__, __LINE__);
    verify(
        -1,
        pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.33, 4.4f),
        __FILE__, __LINE__);
    verify(
        -1,
        pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3, 4.44f),
        __FILE__, __LINE__);
    verify(1023,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.11f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.22, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.33,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.44f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.55f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.66, 7.7, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.77, 8.8f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.88f, 9.9f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.99f, 10.10, 512),
           __FILE__, __LINE__);
    verify(-1,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.101, 512),
           __FILE__, __LINE__);
  }
  {
    S1 s1 = {12};
    verify(12, pass_struct1(s1), __FILE__, __LINE__);
    s1.x = 8;
    verify(15, pass_struct2(1, 2, 4, s1), __FILE__, __LINE__);
    S4 s4 = {1.1f};
    verify(16, pass_struct3(1, 2, 4, s1, s4), __FILE__, __LINE__);
    s4.x = 1.11f;
    verify(14, pass_struct3(1, 2, 4, s1, s4), __FILE__, __LINE__);
    S7 s7 = {1.1f, 4};
    verify(10, pass_struct4(1, 2, 4, s1, 2.2, s7), __FILE__, __LINE__);
    verify(18, pass_struct4(1, 2, 4, s1, 2.22, s7), __FILE__, __LINE__);
    s7.x = 1.11f;
    verify(8, pass_struct4(1, 2, 4, s1, 2.2, s7), __FILE__, __LINE__);
    s7.x = 1.1f;
    S6 s6 = {3.3f, 4.4f};
    verify(9, pass_struct5(1, 2, 4, 2.2, s6, s7), __FILE__, __LINE__);
    verify(17, pass_struct5(1, 2, 4, 2.22, s6, s7), __FILE__, __LINE__);
    s7.x = 1.11f;
    verify(7, pass_struct5(1, 2, 4, 2.2, s6, s7), __FILE__, __LINE__);
    s6.x = 3.33f;
    verify(1, pass_struct5(1, 2, 4, 2.2, s6, s7), __FILE__, __LINE__);
    s6.x = 3.3f;
    s6.y = 4.44f;
    verify(-1, pass_struct5(1, 2, 4, 2.2, s6, s7), __FILE__, __LINE__);
  }
  {
    S13 s13 = {{2, 4, 6}, 8, 10};
    verify(30, pass_struct6(s13), __FILE__, __LINE__);
  }
  {
    S9 s9 = {17, 4.4f};
    verify(19, pass_struct7(s9), __FILE__, __LINE__);
    s9.x = 13;
    s9.y = 3.3;
    verify(12, pass_struct7(s9), __FILE__, __LINE__);
    s9.x = 7;
    s9.y = 4.4f;
    verify(3, pass_struct8(1, 2, 4, 2.2, 3.3f, s9), __FILE__, __LINE__);
    verify(11, pass_struct8(1, 2, 4, 2.3, 3.3f, s9), __FILE__, __LINE__);
    verify(7, pass_struct8(1, 2, 4, 2.2, 3.4f, s9), __FILE__, __LINE__);
    s9.x = 12;
    s9.y = 3.5;
    verify(5, pass_struct8(1, 2, 4, 2.2, 3.3f, s9), __FILE__, __LINE__);
  }
  {
    S10 s10 = {4.4f, 17};
    verify(19, pass_struct9(s10), __FILE__, __LINE__);
    s10.y = 13;
    s10.x = 3.3;
    verify(12, pass_struct9(s10), __FILE__, __LINE__);
    s10.y = 7;
    s10.x = 4.4f;
    verify(3, pass_struct10(1, 2, 4, 2.2, 3.3f, s10), __FILE__, __LINE__);
    verify(11, pass_struct10(1, 2, 4, 2.3, 3.3f, s10), __FILE__, __LINE__);
    verify(7, pass_struct10(1, 2, 4, 2.2, 3.4f, s10), __FILE__, __LINE__);
    s10.y = 12;
    s10.x = 3.5;
    verify(5, pass_struct10(1, 2, 4, 2.2, 3.3f, s10), __FILE__, __LINE__);
  }
  {
    S19 s19;
    s19.x[0] = 6;
    s19.x[1] = 7;
    verify(-1, pass_struct11(s19), __FILE__, __LINE__);
    s19.x[0] = 7;
    s19.x[1] = 6;
    verify(1, pass_struct11(s19), __FILE__, __LINE__);
  }
  {
    S16 s16;
    s16.x[0] = 1.1f;
    s16.x[1] = 2.2f;
    s16.x[2] = 3.3f;
    s16.x[3] = 4.4f;
    verify(15, pass_struct12(s16), __FILE__, __LINE__);
  }
  {
    S9 s9 = {14, 5.5f};
    S10 s10 = pass_struct13(s9);
    verify(15, s10.y, __FILE__, __LINE__);
    verify(1, s10.x == 5.5f, __FILE__, __LINE__);
  }
  {
    S16 s16 = {{7.7f, 8.8f, 3.3f, 1.1f}};
    S16 ret = pass_struct14(s16);
    verify(1, ret.x[0] == 1.1f, __FILE__, __LINE__);
    verify(1, ret.x[1] == 7.7f, __FILE__, __LINE__);
    verify(1, ret.x[2] == 8.8f, __FILE__, __LINE__);
    verify(1, ret.x[3] == 3.3f, __FILE__, __LINE__);
  }
  {
    S20 s20 = {{23, 24, 25, 26}};
    S20 ret = pass_struct15(s20);
    verify(1, ret.x[0] == 26, __FILE__, __LINE__);
    verify(1, ret.x[1] == 23, __FILE__, __LINE__);
    verify(1, ret.x[2] == 24, __FILE__, __LINE__);
    verify(1, ret.x[3] == 25, __FILE__, __LINE__);
  }
  {
    S21 s21;
    s21.x[0] = 6.6;
    s21.x[1] = 7.7;
    s21.y = 21;
    s21.z = -44;
    verify(-13, pass_struct16(s21), __FILE__, __LINE__);
    s21.x[1] = 7.8;
    verify(-21, pass_struct16(s21), __FILE__, __LINE__);
    s21.x[0] = 8.7;
    s21.x[1] = 7.7;
    verify(-20, pass_struct16(s21), __FILE__, __LINE__);
  }
  {
    S21 s21 = {{1.1, 3.3}, 7, 9};
    S21 ret = pass_struct17(s21);
    verify(1, ret.x[0] == 3.3, __FILE__, __LINE__);
    verify(1, ret.x[1] == 1.1, __FILE__, __LINE__);
    verify(9, ret.y, __FILE__, __LINE__);
    verify(7, ret.z, __FILE__, __LINE__);
  }
  {
    U1 u1;
    u1.x[0] = 5;
    u1.x[1] = 13;
    u1.x[2] = 7;
    verify(18, pass_union1x(u1), __FILE__, __LINE__);
    u1.y = 1.34;
    verify(1, pass_union1y(u1) == 1.34, __FILE__, __LINE__);
    u1.z = 8;
    verify(24, pass_union1z(u1), __FILE__, __LINE__);
  }
  {
    U5 u5;
    u5.x = 5.55;
    verify(1, pass_union5x(u5) == 5.55, __FILE__, __LINE__);
    u5.y = 19;
    verify(19, pass_union5y(u5), __FILE__, __LINE__);
  }
  {
    S17 s17;
    s17.x[0] = 3.33;
    s17.x[1] = 4.44;
    S17 ret = pass_struct18(s17);
    verify(1, ret.x[0] == 4.44, __FILE__, __LINE__);
    verify(1, ret.x[1] == 3.33, __FILE__, __LINE__);

    s17.x[0] = 5.55;
    s17.x[1] = 6.66;
    ret = pass_struct19(0, 1, 2, 3, 4, 5, 6, s17);
    verify(1, ret.x[0] == 6.66, __FILE__, __LINE__);
    verify(1, ret.x[1] == 5.55, __FILE__, __LINE__);

    s17.x[0] = 7.77;
    s17.x[1] = 8.88;
    ret = pass_struct20(0, 1, 2, 3, 4, 5, 6, 7, s17);
    verify(1, ret.x[0] == 8.88, __FILE__, __LINE__);
    verify(1, ret.x[1] == 7.77, __FILE__, __LINE__);
  }
  {
    S19 s19;
    s19.x[0] = 3;
    s19.x[1] = 4;
    S19 ret = pass_struct21(1, 2, 3, 4, 5, s19);
    verify(4, ret.x[0], __FILE__, __LINE__);
    verify(3, ret.x[1], __FILE__, __LINE__);

    s19.x[0] = 5;
    s19.x[1] = 6;
    ret = pass_struct22(1, 2, 3, 4, 5, 6, s19);
    verify(6, ret.x[0], __FILE__, __LINE__);
    verify(5, ret.x[1], __FILE__, __LINE__);
  }
  return 0;
}
