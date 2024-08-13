#include "func.h"
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

int minus1() { return -1; }

void empty() {}

int vsprintf(char *, char *, va_list);

int sprintf_(char *s, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  return vsprintf(s, fmt, ap);
}

S1 return_struct1(long long x) {
  S1 s = {x};
  return s;
}

S2 return_struct2(int x) {
  S2 s = {x};
  return s;
}

S3 return_struct3(char x) {
  S3 s = {x};
  return s;
}

S4 return_struct4(float x) {
  S4 s = {x};
  return s;
}

S5 return_struct5(double x) {
  S5 s = {x};
  return s;
}

S6 return_struct6(float x, float y) {
  S6 s = {x, y};
  return s;
}

S7 return_struct7(float x, short y) {
  S7 s = {x, y};
  return s;
}

S8 return_struct8(int x, float y) {
  S8 s = {x, y};
  return s;
}

S9 return_struct9(long x, float y) {
  S9 s = {x, y};
  return s;
}

S10 return_struct10(float x, long long y) {
  S10 s = {x, y};
  return s;
}

S11 return_struct11(double x, int y) {
  S11 s = {x, y};
  return s;
}

S12 return_struct12(float x, char y, float z) {
  S12 s = {x, y, z};
  return s;
}

S13 return_struct13(int x0, int x1, int x2, char y, short z) {
  S13 s = {{x0, x1, x2}, y, z};
  return s;
}

S14 return_struct14(float x0, float x1, float x2, char y, short z) {
  S14 s = {{x0, x1, x2}, y, z};
  return s;
}

S15 return_struct15(short x, short y, float z0, float z1, float z2) {
  S15 s = {x, y, {z0, z1, z2}};
  return s;
}

S16 return_struct16(float x0, float x1, float x2, float x3) {
  S16 s = {{x0, x1, x2, x3}};
  return s;
}

S17 return_struct17(double x0, double x1) {
  S17 s = {{x0, x1}};
  return s;
}

S18 return_struct18x(int x0, int x1, int a) {
  S18 s;
  s.u.x[0] = x0;
  s.u.x[1] = x1;
  s.a = a;
  return s;
}

S18 return_struct18y(double y, int a) {
  S18 s;
  s.u.y = y;
  s.a = a;
  return s;
}

S18 return_struct18z(short z, int a) {
  S18 s;
  s.u.z = z;
  s.a = a;
  return s;
}

S19 return_struct19(long long x0, long long x1) {
  S19 s = {{x0, x1}};
  return s;
}

S20 return_struct20(int x0, int x1, int x2, int x3) {
  S20 s = {{x0, x1, x2, x3}};
  return s;
}

S21 return_struct21(double x0, double x1, short y, long long z) {
  S21 s = {{x0, x1}, y, z};
  return s;
}

U1 return_union1x(int x0, int x1, int x2) {
  U1 u;
  u.x[0] = x0;
  u.x[1] = x1;
  u.x[2] = x2;
  return u;
}

U1 return_union1y(double y) {
  U1 u;
  u.y = y;
  return u;
}

U1 return_union1z(short z) {
  U1 u;
  u.z = z;
  return u;
}

U2 return_union2x(float x0, float x1, float x2) {
  U2 u;
  u.x[0] = x0;
  u.x[1] = x1;
  u.x[2] = x2;
  return u;
}

U2 return_union2y(double y0, double y1) {
  U2 u;
  u.y[0] = y0;
  u.y[1] = y1;
  return u;
}

U2 return_union2z(float z) {
  U2 u;
  u.z = z;
  return u;
}

U3 return_union3x(int x0, int x1) {
  U3 u;
  u.x[0] = x0;
  u.x[1] = x1;
  return u;
}

U3 return_union3y(long long y) {
  U3 u;
  u.y = y;
  return u;
}

U3 return_union3z(float z0, float z1) {
  U3 u;
  u.z[0] = z0;
  u.z[1] = z1;
  return u;
}

U4 return_union4x(float x0, float x1) {
  U4 u;
  u.x[0] = x0;
  u.x[1] = x1;
  return u;
}

U4 return_union4y(double y) {
  U4 u;
  u.y = y;
  return u;
}

U4 return_union4z(float z) {
  U4 u;
  u.z = z;
  return u;
}

U5 return_union5x(double x) {
  U5 u;
  u.x = x;
  return u;
}

U5 return_union5y(int y) {
  U5 u;
  u.y = y;
  return u;
}

U6 return_union6x(int x) {
  U6 u;
  u.x = x;
  return u;
}

U6 return_union6y(double y) {
  U6 u;
  u.y = y;
  return u;
}

U7 return_union7x(float x) {
  U7 u;
  u.x = x;
  return u;
}

U7 return_union7y(short y) {
  U7 u;
  u.y = y;
  return u;
}

U7 return_union7z(long long z0, long long z1, long long z2, long long z3) {
  U7 u;
  u.z[0] = z0;
  u.z[1] = z1;
  u.z[2] = z2;
  u.z[3] = z3;
  return u;
}
