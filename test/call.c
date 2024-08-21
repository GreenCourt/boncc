#include "common.h"
#include "func.h"

int main() {
  verify(12, add3(1, 5, 6));
  verify(5, ret5(1, 5, 6));
  verify(18, add12(1, 5));
  verify(5, fib(5));
  verify(8, fib(6));
  verify(13, fib(7));
  voidfunc(4);
  empty();
  verify(-1, minus1());

  verify(1, static_test1());
  verify(2, static_test1());
  verify(3, static_test1());

  verify(0, static_test2());
  verify(2, static_test2());
  verify(4, static_test2());

  verify(3, static_struct());
  verify(6, static_struct());
  verify(9, static_struct());

  verify(4, static_enum());
  verify(8, static_enum());
  verify(12, static_enum());

  {
    int a = foo();
    verify(12, a);
  }
  {
    int a = add2(1, 5);
    verify(6, a);
  }
  {
    int a = add3(1, 5, 6);
    verify(12, a);
  }
  {
    int (*p_add2)(int, int) = add2;
    int (*p_add3)(int, int, int) = add3;
    int (*p_fib)(int) = fib;
    int (*p_foo)() = foo;
    verify(6, p_add2(2, 4));
    verify(12, p_add3(2, 4, 6));
    verify(8, p_fib(6));
    verify(12, p_foo());

    void (*p_voidfunc)(int) = voidfunc;
    p_voidfunc(12);
  }
  {
    verify(1, retbool(12));
    verify(1, retbool(1));
    verify(0, retbool(0));
  }
  {
    char buf[10];
    int x = sprintf_(buf, "%d_%c_%d", 12, 't', 3);
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
    int arr[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    verify(55, array_sum(arr));
  }
  { verify(4, add_double(1, 3.0)); }
  {
    S1 s = return_struct1(5);
    verify(5, s.x);
  }
  {
    S2 s = return_struct2(6);
    verify(6, s.x);
  }
  {
    S3 s = return_struct3(7);
    verify(7, s.x);
  }
  {
    S4 s = return_struct4(8.8f);
    verify(1, s.x == 8.8f);
  }
  {
    S5 s = return_struct5(1.0);
    verify(1, s.x == 1.0);
  }
  {
    S6 s = return_struct6(2.2f, 3.3f);
    verify(1, s.x == 2.2f);
    verify(1, s.y == 3.3f);
  }
  {
    S7 s = return_struct7(4.4f, 5);
    verify(1, s.x == 4.4f);
    verify(5, s.y);
  }
  {
    S8 s = return_struct8(6, 7.7f);
    verify(6, s.x);
    verify(1, s.y == 7.7f);
  }
  {
    S9 s = return_struct9(8, 9.9f);
    verify(8, s.x);
    verify(1, s.y == 9.9f);
  }
  {
    S10 s = return_struct10(1.1f, 3);
    verify(1, s.x == 1.1f);
    verify(3, s.y);
  }
  {
    S11 s = return_struct11(2.2, 4);
    verify(1, s.x == 2.2);
    verify(4, s.y);
  }
  {
    S12 s = return_struct12(3.3f, 4, 5.5f);
    verify(1, s.x == 3.3f);
    verify(4, s.y);
    verify(1, s.z == 5.5f);
  }
  {
    S13 s = return_struct13(1, 2, 3, 4, 5);
    verify(1, s.x[0]);
    verify(2, s.x[1]);
    verify(3, s.x[2]);
    verify(4, s.y);
    verify(5, s.z);
  }
  {
    S14 s = return_struct14(1.1f, 2.2f, 3.3f, 4, 5);
    verify(1, s.x[0] == 1.1f);
    verify(1, s.x[1] == 2.2f);
    verify(1, s.x[2] == 3.3f);
    verify(4, s.y);
    verify(5, s.z);
  }
  {
    S15 s = return_struct15(5, 6, 7.7f, 8.8f, 9.9f);
    verify(5, s.x);
    verify(6, s.y);
    verify(1, s.z[0] == 7.7f);
    verify(1, s.z[1] == 8.8f);
    verify(1, s.z[2] == 9.9f);
  }
  {
    S16 s = return_struct16(4.4f, 5.5f, 6.6f, 7.7f);
    verify(1, s.x[0] == 4.4f);
    verify(1, s.x[1] == 5.5f);
    verify(1, s.x[2] == 6.6f);
    verify(1, s.x[3] == 7.7f);
  }
  {
    S17 s = return_struct17(8.8, 9.9);
    verify(1, s.x[0] == 8.8);
    verify(1, s.x[1] == 9.9);
  }
  {
    S18 s;
    s = return_struct18x(2, 4, 6);
    verify(2, s.u.x[0]);
    verify(4, s.u.x[1]);
    verify(6, s.a);
    s = return_struct18y(3.3, 5);
    verify(1, s.u.y == 3.3);
    verify(5, s.a);
    s = return_struct18z(7, 8);
    verify(7, s.u.z);
    verify(8, s.a);
  }
  {
    S19 s = return_struct19(6, 9);
    verify(6, s.x[0]);
    verify(9, s.x[1]);
  }
  {
    S20 s = return_struct20(5, 2, 9, 3);
    verify(5, s.x[0]);
    verify(2, s.x[1]);
    verify(9, s.x[2]);
    verify(3, s.x[3]);
  }
  {
    S21 s = return_struct21(7.7, 6.6, 3, 5);
    verify(1, s.x[0] == 7.7);
    verify(1, s.x[1] == 6.6);
    verify(3, s.y);
    verify(5, s.z);
  }
  {
    U1 u;
    u = return_union1x(3, 2, 4);
    verify(3, u.x[0]);
    verify(2, u.x[1]);
    verify(4, u.x[2]);
    u = return_union1y(5.5);
    verify(1, u.y == 5.5);
    u = return_union1z(6);
    verify(6, u.z);
  }
  {
    U2 u;
    u = return_union2x(1.1f, 2.2f, 3.3f);
    verify(1, u.x[0] == 1.1f);
    verify(1, u.x[1] == 2.2f);
    verify(1, u.x[2] == 3.3f);
    u = return_union2y(4.4f, 5.5f);
    verify(1, u.y[0] == 4.4f);
    verify(1, u.y[1] == 5.5f);
    u = return_union2z(6.6f);
    verify(1, u.z == 6.6f);
  }
  {
    U3 u;
    u = return_union3x(2, 3);
    verify(2, u.x[0]);
    verify(3, u.x[1]);
    u = return_union3y(4);
    verify(4, u.y);
    u = return_union3z(5.5f, 6.6f);
    verify(1, u.z[0] == 5.5f);
    verify(1, u.z[1] == 6.6f);
  }
  {
    U4 u;
    u = return_union4x(2.2f, 3.3f);
    verify(1, u.x[0] == 2.2f);
    verify(1, u.x[1] == 3.3f);
    u = return_union4y(4.4);
    verify(1, u.y == 4.4);
    u = return_union4z(5.5f);
    verify(1, u.z == 5.5f);
  }
  {
    U5 u;
    u = return_union5x(6.6);
    verify(1, u.x == 6.6);
    u = return_union5y(7);
    verify(7, u.y);
  }
  {
    U6 u;
    u = return_union6x(8);
    verify(8, u.x);
    u = return_union6y(9.9);
    verify(1, u.y == 9.9);
  }
  {
    U7 u;
    u = return_union7x(8.8f);
    verify(1, u.x == 8.8f);
    u = return_union7y(3);
    verify(3, u.y);
    u = return_union7z(5, 13 - 7, 7, 4 + 4);
    verify(5, u.z[0]);
    verify(6, u.z[1]);
    verify(7, u.z[2]);
    verify(8, u.z[3]);
  }
  {
    verify(511, pass_by_stack1(1, 2, 4, 8, 16, 32, 64, 128, 256));
    verify(-1, pass_by_stack2(1, 2, 1.2f, 2.2, 4, 8, 16, 32, 64, 128, 256));
    verify(511, pass_by_stack2(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256));
    verify(-1, pass_by_stack2(1, 2, 1.1f, 2.3, 4, 8, 16, 32, 64, 128, 256));
    verify(511, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.1, 2.2f));
    verify(-1, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.11, 2.2f));
    verify(-1, pass_by_stack3(1, 2, 4, 8, 16, 32, 64, 128, 256, 1.1, 2.22f));
    verify(511, pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                               4.4f));
    verify(-1, pass_by_stack4(1, 2, 1.11f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f));
    verify(-1, pass_by_stack4(1, 2, 1.1f, 2.22, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f));
    verify(-1, pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.33,
                              4.4f));
    verify(-1, pass_by_stack4(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.44f));
    verify(1023,
           pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                          4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.11f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.22, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.33,
                              4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.44f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.55f, 6.6, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.66, 7.7, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.77, 8.8f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.7, 8.88f, 9.9f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.99f, 10.10, 512));
    verify(-1, pass_by_stack5(1, 2, 1.1f, 2.2, 4, 8, 16, 32, 64, 128, 256, 3.3,
                              4.4f, 5.5f, 6.6, 7.7, 8.8f, 9.9f, 10.101, 512));
  }
  {
    S1 s1 = {12};
    verify(12, pass_struct1(s1));
    s1.x = 8;
    verify(15, pass_struct2(1, 2, 4, s1));
    S4 s4 = {1.1f};
    verify(16, pass_struct3(1, 2, 4, s1, s4));
    s4.x = 1.11f;
    verify(14, pass_struct3(1, 2, 4, s1, s4));
    S7 s7 = {1.1f, 4};
    verify(10, pass_struct4(1, 2, 4, s1, 2.2, s7));
    verify(18, pass_struct4(1, 2, 4, s1, 2.22, s7));
    s7.x = 1.11f;
    verify(8, pass_struct4(1, 2, 4, s1, 2.2, s7));
    s7.x = 1.1f;
    S6 s6 = {3.3f, 4.4f};
    verify(9, pass_struct5(1, 2, 4, 2.2, s6, s7));
    verify(17, pass_struct5(1, 2, 4, 2.22, s6, s7));
    s7.x = 1.11f;
    verify(7, pass_struct5(1, 2, 4, 2.2, s6, s7));
    s6.x = 3.33f;
    verify(1, pass_struct5(1, 2, 4, 2.2, s6, s7));
    s6.x = 3.3f;
    s6.y = 4.44f;
    verify(-1, pass_struct5(1, 2, 4, 2.2, s6, s7));
  }
  {
    S13 s13 = {{2, 4, 6}, 8, 10};
    verify(30, pass_struct6(s13));
  }
  {
    S9 s9 = {17, 4.4f};
    verify(19, pass_struct7(s9));
    s9.x = 13;
    s9.y = 3.3;
    verify(12, pass_struct7(s9));
    s9.x = 7;
    s9.y = 4.4f;
    verify(3, pass_struct8(1, 2, 4, 2.2, 3.3f, s9));
    verify(11, pass_struct8(1, 2, 4, 2.3, 3.3f, s9));
    verify(7, pass_struct8(1, 2, 4, 2.2, 3.4f, s9));
    s9.x = 12;
    s9.y = 3.5;
    verify(5, pass_struct8(1, 2, 4, 2.2, 3.3f, s9));
  }
  {
    S10 s10 = {4.4f, 17};
    verify(19, pass_struct9(s10));
    s10.y = 13;
    s10.x = 3.3;
    verify(12, pass_struct9(s10));
    s10.y = 7;
    s10.x = 4.4f;
    verify(3, pass_struct10(1, 2, 4, 2.2, 3.3f, s10));
    verify(11, pass_struct10(1, 2, 4, 2.3, 3.3f, s10));
    verify(7, pass_struct10(1, 2, 4, 2.2, 3.4f, s10));
    s10.y = 12;
    s10.x = 3.5;
    verify(5, pass_struct10(1, 2, 4, 2.2, 3.3f, s10));
  }
  {
    S19 s19;
    s19.x[0] = 6;
    s19.x[1] = 7;
    verify(-1, pass_struct11(s19));
    s19.x[0] = 7;
    s19.x[1] = 6;
    verify(1, pass_struct11(s19));
  }
  {
    S16 s16;
    s16.x[0] = 1.1f;
    s16.x[1] = 2.2f;
    s16.x[2] = 3.3f;
    s16.x[3] = 4.4f;
    verify(15, pass_struct12(s16));
  }
  {
    S9 s9 = {14, 5.5f};
    S10 s10 = pass_struct13(s9);
    verify(15, s10.y);
    verify(1, s10.x == 5.5f);
  }
  {
    S16 s16 = {{7.7f, 8.8f, 3.3f, 1.1f}};
    S16 ret = pass_struct14(s16);
    verify(1, ret.x[0] == 1.1f);
    verify(1, ret.x[1] == 7.7f);
    verify(1, ret.x[2] == 8.8f);
    verify(1, ret.x[3] == 3.3f);
  }
  {
    S20 s20 = {{23, 24, 25, 26}};
    S20 ret = pass_struct15(s20);
    verify(1, ret.x[0] == 26);
    verify(1, ret.x[1] == 23);
    verify(1, ret.x[2] == 24);
    verify(1, ret.x[3] == 25);
  }
  {
    S21 s21;
    s21.x[0] = 6.6;
    s21.x[1] = 7.7;
    s21.y = 21;
    s21.z = -44;
    verify(-13, pass_struct16(s21));
    s21.x[1] = 7.8;
    verify(-21, pass_struct16(s21));
    s21.x[0] = 8.7;
    s21.x[1] = 7.7;
    verify(-20, pass_struct16(s21));
  }
  {
    S21 s21 = {{1.1, 3.3}, 7, 9};
    S21 ret = pass_struct17(s21);
    verify(1, ret.x[0] == 3.3);
    verify(1, ret.x[1] == 1.1);
    verify(9, ret.y);
    verify(7, ret.z);
  }
  {
    U1 u1;
    u1.x[0] = 5;
    u1.x[1] = 13;
    u1.x[2] = 7;
    verify(18, pass_union1x(u1));
    u1.y = 1.34;
    verify(1, pass_union1y(u1) == 1.34);
    u1.z = 8;
    verify(24, pass_union1z(u1));
  }
  {
    U5 u5;
    u5.x = 5.55;
    verify(1, pass_union5x(u5) == 5.55);
    u5.y = 19;
    verify(19, pass_union5y(u5));
  }
  {
    S17 s17;
    s17.x[0] = 3.33;
    s17.x[1] = 4.44;
    S17 ret = pass_struct18(s17);
    verify(1, ret.x[0] == 4.44);
    verify(1, ret.x[1] == 3.33);

    s17.x[0] = 5.55;
    s17.x[1] = 6.66;
    ret = pass_struct19(0, 1, 2, 3, 4, 5, 6, s17);
    verify(1, ret.x[0] == 6.66);
    verify(1, ret.x[1] == 5.55);

    s17.x[0] = 7.77;
    s17.x[1] = 8.88;
    ret = pass_struct20(0, 1, 2, 3, 4, 5, 6, 7, s17);
    verify(1, ret.x[0] == 8.88);
    verify(1, ret.x[1] == 7.77);
  }
  {
    S19 s19;
    s19.x[0] = 3;
    s19.x[1] = 4;
    S19 ret = pass_struct21(1, 2, 3, 4, 5, s19);
    verify(4, ret.x[0]);
    verify(3, ret.x[1]);

    s19.x[0] = 5;
    s19.x[1] = 6;
    ret = pass_struct22(1, 2, 3, 4, 5, 6, s19);
    verify(6, ret.x[0]);
    verify(5, ret.x[1]);
  }

  verify(-5, va_arg_int(1, -5));
  verify(7, va_arg_int(3, 1, 2, 4));
  verify(31, va_arg_int(5, 1, 2, 4, 8, 16));
  verify(63, va_arg_int(6, 1, 2, 4, 8, 16, 32));
  verify(255, va_arg_int(8, 1, 2, 4, 8, 16, 32, 64, 128));

  verify(-5, va_arg_long(1, -5L));
  verify(7, va_arg_long(3, 1L, 2L, 4L));
  verify(31, va_arg_long(5, 1L, 2L, 4L, 8L, 16L));
  verify(63, va_arg_long(6, 1L, 2L, 4L, 8L, 16L, 32L));
  verify(255, va_arg_long(8, 1L, 2L, 4L, 8L, 16L, 32L, 64L, 128L));

  verify(-5, va_arg_short(1, (short)-5));
  verify(7, va_arg_short(3, (short)1, (short)2, (short)4));
  verify(31,
         va_arg_short(5, (short)1, (short)2, (short)4, (short)8, (short)16));
  verify(63, va_arg_short(6, (short)1, (short)2, (short)4, (short)8, (short)16,
                          (short)32));
  verify(255, va_arg_short(8, (short)1, (short)2, (short)4, (short)8, (short)16,
                           (short)32, (short)64, (short)128));

  verify(2, va_arg_ushort(1, (unsigned short)2));
  verify(7, va_arg_ushort(3, (unsigned short)1, (unsigned short)2,
                          (unsigned short)4));
  verify(31, va_arg_ushort(5, (unsigned short)1, (unsigned short)2,
                           (unsigned short)4, (unsigned short)8,
                           (unsigned short)16));
  verify(63, va_arg_ushort(6, (unsigned short)1, (unsigned short)2,
                           (unsigned short)4, (unsigned short)8,
                           (unsigned short)16, (unsigned short)32));
  verify(255, va_arg_ushort(8, (unsigned short)1, (unsigned short)2,
                            (unsigned short)4, (unsigned short)8,
                            (unsigned short)16, (unsigned short)32,
                            (unsigned short)64, (unsigned short)128));

  verify(-5, va_copy_int(1, -5));
  verify(13, va_copy_int(3, 1, 2, 4));
  verify(61, va_copy_int(5, 1, 2, 4, 8, 16));
  verify(125, va_copy_int(6, 1, 2, 4, 8, 16, 32));
  verify(509, va_copy_int(8, 1, 2, 4, 8, 16, 32, 64, 128));

  verify(1, va_arg_float(1, 1.1f));
  verify(4, va_arg_float(4, 1.1f, 2.2f, 3.3f, 4.4f));
  verify(3, va_arg_float(4, 1.2f, 2.2f, 3.3f, 4.4f));
  verify(3, va_arg_float(4, 1.1f, 2.2f, 3.3f, 4.5f));
  verify(8, va_arg_float(8, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f));
  verify(9,
         va_arg_float(9, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f));
  verify(11, va_arg_float(11, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f,
                          9.9f, 10.1f, 11.11f));
  verify(10, va_arg_float(11, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f,
                          9.9f, 10.2f, 11.11f));
  verify(10, va_arg_float(11, 1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f,
                          9.9f, 10.1f, 11.10f));

  verify(1, va_arg_double(1, 1.1));
  verify(4, va_arg_double(4, 1.1, 2.2, 3.3, 4.4));
  verify(3, va_arg_double(4, 1.2, 2.2, 3.3, 4.4));
  verify(3, va_arg_double(4, 1.1, 2.2, 3.3, 4.5));
  verify(8, va_arg_double(8, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8));
  verify(9, va_arg_double(9, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9));
  verify(11, va_arg_double(11, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9,
                           10.1, 11.11));
  verify(10, va_arg_double(11, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9,
                           10.2, 11.11));
  verify(10, va_arg_double(11, 1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9,
                           10.1, 11.10));

  verify(158, va_arg_int_double(6, 5, 1.1, 25, 2.2, 125, 3.3));
  verify(157, va_arg_int_double(6, 5, 1.1, 25, 2.2, 125, 3.2));
  verify(61035166,
         va_arg_int_double(22, 5, 1.1, 25, 2.2, 125, 3.3, 625, 4.4, 3125, 5.5,
                           15625, 6.6, 78125, 7.7, 390625, 8.8, 1953125, 9.9,
                           9765625, 10.10, 48828125, 11.11));
  verify(61035165,
         va_arg_int_double(22, 5, 1.1, 25, 2.2, 125, 3.3, 625, 4.4, 3125, 5.5,
                           15625, 6.6, 78125, 7.7, 390625, 8.8, 1953125, 9.9,
                           9765625, 10.10, 48828125, 11.12));
  verify(61035165,
         va_arg_int_double(22, 5, 1.1, 25, 2.2, 125, 3.4, 625, 4.4, 3125, 5.5,
                           15625, 6.6, 78125, 7.7, 390625, 8.8, 1953125, 9.9,
                           9765625, 10.10, 48828125, 11.11));
  {
    S1 s[] = {
        {1}, {2}, {4}, {8}, {16}, {32}, {64}, {128}, {256}, {512},
    };
    verify(31, va_arg_struct1(5, s[0], s[1], s[2], s[3], s[4]));
    verify(1023, va_arg_struct1(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6],
                                s[7], s[8], s[9]));
  }
  {
    S3 s[] = {{1}, {2}, {3}, {4}, {5}, {6}, {7}, {8}, {9}, {10}};
    verify(15, va_arg_struct2(5, s[0], s[1], s[2], s[3], s[4]));
    verify(55, va_arg_struct2(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6],
                              s[7], s[8], s[9]));
  }
  {
    S4 s[] = {{1.1f}, {2.2f}, {3.3f}, {4.4f}, {5.5f},
              {6.6f}, {7.7f}, {8.8f}, {9.9f}, {10.1f}};
    verify(5, va_arg_struct3(5, s[0], s[1], s[2], s[3], s[4]));
    verify(10, va_arg_struct3(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6],
                              s[7], s[8], s[9]));
    verify(8, va_arg_struct3(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                             s[9], s[8]));
  }
  {
    S5 s[] = {{1.1}, {2.2}, {3.3}, {4.4}, {5.5},
              {6.6}, {7.7}, {8.8}, {9.9}, {10.1}};
    verify(5, va_arg_struct4(5, s[0], s[1], s[2], s[3], s[4]));
    verify(10, va_arg_struct4(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6],
                              s[7], s[8], s[9]));
    verify(8, va_arg_struct4(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                             s[9], s[8]));
  }
  {
    S7 s[] = {{1.1, 1},  {2.2, 2},  {3.3, 4},   {4.4, 8},   {5.5, 16},
              {6.6, 32}, {7.7, 64}, {8.8, 128}, {9.9, 256}, {10.1, 512}};
    S19 ret;
    ret = va_arg_struct5(5, s[0], s[1], s[2], s[3], s[4]);
    verify(5, ret.x[0]);
    verify(31, ret.x[1]);
    ret = va_arg_struct5(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(10, ret.x[0]);
    verify(1023, ret.x[1]);
    ret = va_arg_struct5(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[9], s[8]);
    verify(8, ret.x[0]);
    verify(1023, ret.x[1]);
  }
  {
    S13 s[] = {
        {{5, -1, 3}, 1, 1},
        {{25, -2, 9}, 2, 2},
        {{125, -4, 27}, 3, 4},
        {{625, -8, 81}, 4, 8},
        {{3125, -16, 243}, 5, 16},
        {{15625, -32, 729}, 6, 32},
        {{78125, -64, 2187}, 7, 64},
        {{390625, -128, 6561}, 8, 128},
        {{1953125, -256, 19683}, 9, 256},
        {{9765625, -512, 59049}, 10, 512},
    };
    S13 ret;
    ret = va_arg_struct6(4, s[0], s[1], s[2], s[3], s[4]);
    verify(780, ret.x[0]);
    verify(-15, ret.x[1]);
    verify(120, ret.x[2]);
    verify(10, ret.y);
    verify(15, ret.z);

    ret = va_arg_struct6(5, s[0], s[1], s[2], s[3], s[4]);
    verify(3905, ret.x[0]);
    verify(-31, ret.x[1]);
    verify(363, ret.x[2]);
    verify(15, ret.y);
    verify(31, ret.z);

    ret = va_arg_struct6(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);

    verify(12207030, ret.x[0]);
    verify(-1023, ret.x[1]);
    verify(88572, ret.x[2]);
    verify(55, ret.y);
    verify(1023, ret.z);
  }
  {
    S17 s[] = {
        {{1.1, -2.22}}, {{2.2, -1.11}},    {{3.3, -3.33}}, {{4.4, -4.44}},
        {{5.5, -5.55}}, {{6.6, -6.66}},    {{7.7, -7.77}}, {{8.8, -8.88}},
        {{9.9, -9.99}}, {{10.1, -10.101}},
    };
    S19 ret;
    ret = va_arg_struct7(5, s[0], s[1], s[2], s[3], s[4]);
    verify(5, ret.x[0]);
    verify(3, ret.x[1]);
    ret = va_arg_struct7(5, s[1], s[0], s[2], s[3], s[4]);
    verify(3, ret.x[0]);
    verify(5, ret.x[1]);

    ret = va_arg_struct7(6, s[0], s[1], s[2], s[3], s[4], s[5]);
    verify(6, ret.x[0]);
    verify(4, ret.x[1]);

    ret = va_arg_struct7(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(10, ret.x[0]);
    verify(8, ret.x[1]);
    ret = va_arg_struct7(10, s[1], s[0], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(8, ret.x[0]);
    verify(10, ret.x[1]);
    ret = va_arg_struct7(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[9], s[8]);
    verify(8, ret.x[0]);
    verify(6, ret.x[1]);
    ret = va_arg_struct7(10, s[1], s[0], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[9], s[8]);
    verify(6, ret.x[0]);
    verify(8, ret.x[1]);
  }
  {
    S11 s[] = {
        {1.1, 1},  {2.2, 2},  {3.3, 4},   {4.4, 8},   {5.5, 16},
        {6.6, 32}, {7.7, 64}, {8.8, 128}, {9.9, 256}, {10.1, 512},
    };
    S19 ret;
    ret = va_arg_struct8(5, s[0], s[1], s[2], s[3], s[4]);
    verify(5, ret.x[0]);
    verify(31, ret.x[1]);

    ret = va_arg_struct8(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(10, ret.x[0]);
    verify(1023, ret.x[1]);

    ret = va_arg_struct8(10, s[0], s[2], s[1], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(8, ret.x[0]);
    verify(1023, ret.x[1]);

    ret = va_arg_struct8(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[9], s[8]);
    verify(8, ret.x[0]);
    verify(1023, ret.x[1]);
  }
  {
    S9 s[] = {
        {1, 1.1f},  {2, 2.2f},  {4, 3.3f},   {8, 4.4f},   {16, 5.5f},
        {32, 6.6f}, {64, 7.7f}, {128, 8.8f}, {256, 9.9f}, {512, 10.1f},
    };
    S19 ret;
    ret = va_arg_struct9(5, s[0], s[1], s[2], s[3], s[4]);
    verify(31, ret.x[0]);
    verify(5, ret.x[1]);

    ret = va_arg_struct9(6, s[0], s[1], s[2], s[3], s[4], s[5]);
    verify(63, ret.x[0]);
    verify(6, ret.x[1]);

    ret = va_arg_struct9(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(1023, ret.x[0]);
    verify(10, ret.x[1]);

    ret = va_arg_struct9(10, s[0], s[2], s[1], s[3], s[4], s[5], s[6], s[7],
                         s[8], s[9]);
    verify(1023, ret.x[0]);
    verify(8, ret.x[1]);

    ret = va_arg_struct9(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                         s[9], s[8]);
    verify(1023, ret.x[0]);
    verify(8, ret.x[1]);
  }
  {
    S21 s[] = {
        {{1.1, -1.1}, 1, -1},     {{2.2, -2.2}, 2, -2},
        {{3.3, -3.3}, 4, -4},     {{4.4, -4.4}, 8, -8},
        {{5.5, -5.5}, 16, -16},   {{6.6, -6.6}, 32, -32},
        {{7.7, -7.7}, 64, -64},   {{8.8, -8.8}, 128, -128},
        {{9.9, -9.9}, 256, -256}, {{10.1, -10.1}, 512, -512},
    };
    S20 ret;
    ret = va_arg_struct10(5, s[0], s[1], s[2], s[3], s[4]);
    verify(5, ret.x[0]);
    verify(5, ret.x[1]);
    verify(31, ret.x[2]);
    verify(-31, ret.x[3]);

    ret = va_arg_struct10(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                          s[8], s[9]);
    verify(10, ret.x[0]);
    verify(10, ret.x[1]);
    verify(1023, ret.x[2]);
    verify(-1023, ret.x[3]);

    ret = va_arg_struct10(10, s[0], s[2], s[1], s[3], s[4], s[5], s[6], s[7],
                          s[8], s[9]);
    verify(8, ret.x[0]);
    verify(8, ret.x[1]);
    verify(1023, ret.x[2]);
    verify(-1023, ret.x[3]);

    ret = va_arg_struct10(10, s[0], s[1], s[2], s[3], s[4], s[5], s[6], s[7],
                          s[9], s[8]);
    verify(8, ret.x[0]);
    verify(8, ret.x[1]);
    verify(1023, ret.x[2]);
    verify(-1023, ret.x[3]);
  }
  {
    U1 u[10];
    u[0].x[0] = 1;
    u[1].x[0] = 2;
    u[2].x[0] = 4;
    u[3].x[0] = 8;
    u[4].x[0] = 16;
    u[5].x[0] = 32;
    u[6].x[0] = 64;
    u[7].x[0] = 128;
    u[8].x[0] = 256;
    u[9].x[0] = 512;

    u[0].x[1] = 1;
    u[1].x[1] = 3;
    u[2].x[1] = 9;
    u[3].x[1] = 27;
    u[4].x[1] = 81;
    u[5].x[1] = 243;
    u[6].x[1] = 729;
    u[7].x[1] = 2187;
    u[8].x[1] = 6561;
    u[9].x[1] = 19683;

    u[0].x[2] = 1;
    u[1].x[2] = 5;
    u[2].x[2] = 25;
    u[3].x[2] = 125;
    u[4].x[2] = 625;
    u[5].x[2] = 3125;
    u[6].x[2] = 15625;
    u[7].x[2] = 78125;
    u[8].x[2] = 390625;
    u[9].x[2] = 1953125;

    S20 ret;
    ret = va_arg_union1x(5, u[0], u[1], u[2], u[3], u[4]);
    verify(31, ret.x[0]);
    verify(121, ret.x[1]);
    verify(781, ret.x[2]);
    verify(0, ret.x[3]);

    ret = va_arg_union1x(10, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
                         u[8], u[9]);
    verify(1023, ret.x[0]);
    verify(29524, ret.x[1]);
    verify(2441406, ret.x[2]);
    verify(0, ret.x[3]);
  }
  {
    U1 u[10];
    u[0].y = 1.1;
    u[1].y = 2.2;
    u[2].y = 3.3;
    u[3].y = 4.4;
    u[4].y = 5.5;
    u[5].y = 6.6;
    u[6].y = 7.7;
    u[7].y = 8.8;
    u[8].y = 9.9;
    u[9].y = 10.1;

    verify(5, va_arg_union1y(5, u[0], u[1], u[2], u[3], u[4]));
    verify(10, va_arg_union1y(10, u[0], u[1], u[2], u[3], u[4], u[5], u[6],
                              u[7], u[8], u[9]));
    verify(8, va_arg_union1y(10, u[0], u[1], u[2], u[3], u[4], u[5], u[7], u[6],
                             u[8], u[9]));
  }
  {
    U2 u[10];
    u[0].y[0] = 1.1;
    u[1].y[0] = 2.2;
    u[2].y[0] = 3.3;
    u[3].y[0] = 4.4;
    u[4].y[0] = 5.5;
    u[5].y[0] = 6.6;
    u[6].y[0] = 7.7;
    u[7].y[0] = 8.8;
    u[8].y[0] = 9.9;
    u[9].y[0] = 10.1;

    u[0].y[1] = -1.1;
    u[1].y[1] = -2.2;
    u[2].y[1] = -3.3;
    u[3].y[1] = -4.4;
    u[4].y[1] = -5.5;
    u[5].y[1] = -6.6;
    u[6].y[1] = -7.7;
    u[7].y[1] = -8.8;
    u[8].y[1] = -9.9;
    u[9].y[1] = -10.1;

    S19 ret;
    ret = va_arg_union2y(5, u[0], u[1], u[2], u[3], u[4]);
    verify(5, ret.x[0]);
    verify(5, ret.x[1]);
    ret = va_arg_union2y(10, u[0], u[1], u[2], u[3], u[4], u[5], u[6], u[7],
                         u[8], u[9]);
    verify(10, ret.x[0]);
    verify(10, ret.x[1]);
    ret = va_arg_union2y(10, u[0], u[1], u[3], u[2], u[4], u[5], u[6], u[7],
                         u[8], u[9]);
    verify(8, ret.x[0]);
    verify(8, ret.x[1]);
  }
  {
    U2 u[10];
    u[0].z = 1.1f;
    u[1].z = 2.2f;
    u[2].z = 3.3f;
    u[3].z = 4.4f;
    u[4].z = 5.5f;
    u[5].z = 6.6f;
    u[6].z = 7.7f;
    u[7].z = 8.8f;
    u[8].z = 9.9f;
    u[9].z = 10.1f;

    verify(5, va_arg_union2z(5, u[0], u[1], u[2], u[3], u[4]));
    verify(10, va_arg_union2z(10, u[0], u[1], u[2], u[3], u[4], u[5], u[6],
                              u[7], u[8], u[9]));
    verify(8, va_arg_union2z(10, u[0], u[1], u[2], u[3], u[4], u[5], u[7], u[6],
                             u[8], u[9]));
  }

  return 0;
}
