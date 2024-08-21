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

int pass_by_stack1(int a, int b, int c, int d, int e, int f, char g, short h,
                   int i) {
  return a + b + c + d + e + f + g + h + i;
}

int pass_by_stack2(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k) {
  if (c != 1.1f || d != 2.2)
    return -1;
  return a + b + e + f + g + h + i + j + k;
}

int pass_by_stack3(int a, int b, int c, int d, int e, int f, char g, short h,
                   int i, double j, float k) {
  if (j != 1.1 || k != 2.2f)
    return -1;
  return a + b + c + d + e + f + g + h + i;
}

int pass_by_stack4(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k, double l, float m) {
  if (c != 1.1f || d != 2.2 || l != 3.3 || m != 4.4f)
    return -1;
  return a + b + e + f + g + h + i + j + k;
}

int pass_by_stack5(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k, double l, float m,
                   float n, double o, double p, float q, float r, double s,
                   int t) {

  if (c != 1.1f || d != 2.2 || l != 3.3 || m != 4.4f || n != 5.5f || o != 6.6 ||
      p != 7.7 || q != 8.8f || r != 9.9f || s != 10.10)
    return -1;
  return a + b + e + f + g + h + i + j + k + t;
}

long long pass_struct1(S1 s) { return s.x; }

long long pass_struct2(int a, int b, int c, S1 s) { return a + b + c + s.x; }

long long pass_struct3(int a, int b, int c, S1 s1, S4 s4) {
  return a + b + c + s1.x + (s4.x == 1.1f ? 1 : -1);
}

long long pass_struct4(int a, int b, int c, S1 s1, double d, S7 s7) {
  return a + b + c + s1.x + (d == 2.2 ? -10 : -2) + (s7.x == 1.1f ? 1 : -1) +
         s7.y;
}

long long pass_struct5(int a, int b, int c, double d, S6 s6, S7 s7) {
  return a + b + c + (s6.x == 3.3f ? 3 : -3) + (s6.y == 4.4f ? 4 : -4) +
         (d == 2.2 ? -10 : -2) + (s7.x == 1.1f ? 1 : -1) + s7.y;
}

long long pass_struct6(S13 s13) {
  return s13.x[0] + s13.x[1] + s13.x[2] + s13.y + s13.z;
}

long long pass_struct7(S9 s9) { return s9.x + (s9.y == 4.4f ? 2 : -1); }

long long pass_struct8(int a, int b, int c, double d, float e, S9 s9) {
  return a + b + c + (d == 2.2 ? -10 : -2) + (e == 3.3f ? -3 : 1) + s9.x +
         (s9.y == 4.4f ? 2 : -1);
}

long long pass_struct9(S10 s10) { return s10.y + (s10.x == 4.4f ? 2 : -1); }

long long pass_struct10(int a, int b, int c, double d, float e, S10 s10) {
  return a + b + c + (d == 2.2 ? -10 : -2) + (e == 3.3f ? -3 : 1) + s10.y +
         (s10.x == 4.4f ? 2 : -1);
}

long long pass_struct11(S19 s19) { return s19.x[0] - s19.x[1]; }

long long pass_struct12(S16 s16) {
  return (s16.x[0] == 1.1f ? 1 : 0) + (s16.x[1] == 2.2f ? 2 : 0) +
         (s16.x[2] == 3.3f ? 4 : 0) + (s16.x[3] == 4.4f ? 8 : 0);
}

S10 pass_struct13(S9 s9) {
  S10 s10 = {s9.y, s9.x + 1};
  return s10;
}

S16 pass_struct14(S16 s16) {
  S16 ret = {{s16.x[3], s16.x[0], s16.x[1], s16.x[2]}};
  return ret;
}

S20 pass_struct15(S20 s20) {
  S20 ret = {{s20.x[3], s20.x[0], s20.x[1], s20.x[2]}};
  return ret;
}

long long pass_struct16(S21 s21) {
  return (s21.x[0] == 6.6 ? 3 : -4) + (s21.x[1] == 7.7 ? 7 : -1) + s21.y +
         s21.z;
}

S21 pass_struct17(S21 s21) {
  S21 ret = {{s21.x[1], s21.x[0]}, s21.z, s21.y};
  return ret;
}

S17 pass_struct18(S17 s17) {
  S17 ret = {{s17.x[1], s17.x[0]}};
  return ret;
}

S17 pass_struct19(double a, double b, double c, double d, double e, double f,
                  double g, S17 s17) {
  S17 ret = {{s17.x[1], s17.x[0]}};
  return ret;
}

S17 pass_struct20(double a, double b, double c, double d, double e, double f,
                  double g, double h, S17 s17) {
  S17 ret = {{s17.x[1], s17.x[0]}};
  return ret;
}

S19 pass_struct21(int a, int b, int c, int d, int e, S19 s19) {
  S19 ret = {{s19.x[1], s19.x[0]}};
  return ret;
}

S19 pass_struct22(int a, int b, int c, int d, int e, int f, S19 s19) {
  S19 ret = {{s19.x[1], s19.x[0]}};
  return ret;
}

int pass_union1x(U1 u1) { return u1.x[0] * 2 - u1.x[1] + u1.x[2] * 3; }

double pass_union1y(U1 u1) { return u1.y; }

short pass_union1z(U1 u1) { return u1.z * 3; }

double pass_union5x(U5 u5) { return u5.x; }

int pass_union5y(U5 u5) { return u5.y; }

int va_arg_int(int n, ...) {
  va_list ap;
  va_start(ap, n);
  int s = 0;
  for (int i = 0; i < n; ++i)
    s += va_arg(ap, int);
  return s;
}

long va_arg_long(int n, ...) {
  va_list ap;
  va_start(ap, n);
  long s = 0;
  for (int i = 0; i < n; ++i)
    s += va_arg(ap, long);
  return s;
}

short va_arg_short(int n, ...) {
  va_list ap;
  va_start(ap, n);
  short s = 0;
  for (int i = 0; i < n; ++i) {
    // promoted to int by default argument promotion
    s += va_arg(ap, int);
  }
  return s;
}

unsigned short va_arg_ushort(int n, ...) {
  va_list ap;
  va_start(ap, n);
  unsigned short s = 0;
  for (int i = 0; i < n; ++i) {
    // promoted to unsigned int by default argument promotion
    s += va_arg(ap, unsigned int);
  }
  return s;
}

int va_copy_int(int n, ...) {
  va_list ap;
  va_list ap2;
  va_start(ap, n);
  int s = va_arg(ap, int);
  va_copy(ap2, ap);
  for (int i = 1; i < n; ++i)
    s += va_arg(ap, int);
  for (int i = 1; i < n; ++i)
    s += va_arg(ap2, int);
  return s;
}

int va_arg_double(int n, ...) {
  va_list ap;
  va_start(ap, n);
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11};
  int s = 0;
  for (int i = 0; i < n; ++i)
    s += va_arg(ap, double) == r[i];
  return s;
}

int va_arg_float(int n, ...) {
  va_list ap;
  va_start(ap, n);
  float r[] = {1.1f, 2.2f, 3.3f, 4.4f,  5.5f,  6.6f,
               7.7f, 8.8f, 9.9f, 10.1f, 11.11f};
  int s = 0;
  for (int i = 0; i < n; ++i) {
    // promoted to double by default argument promotion
    s += va_arg(ap, double) == (double)r[i];
  }
  return s;
}

int va_arg_int_double(int n, ...) {
  va_list ap;
  va_start(ap, n);
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11};
  int s = 0;
  for (int i = 0; i < n; ++i) {
    if (i % 2)
      s += va_arg(ap, double) == r[i / 2];
    else
      s += va_arg(ap, int);
  }
  return s;
}

long long va_arg_struct1(int n, ...) {
  va_list ap;
  va_start(ap, n);
  long long sum = 0;
  for (int i = 0; i < n; ++i) {
    S1 s = va_arg(ap, S1);
    sum += s.x;
  }
  return sum;
}

int va_arg_struct2(int n, ...) {
  va_list ap;
  va_start(ap, n);
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    S3 s = va_arg(ap, S3);
    sum += s.x;
  }
  return sum;
}

int va_arg_struct3(int n, ...) {
  va_list ap;
  va_start(ap, n);
  float r[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.1f};
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    S4 s = va_arg(ap, S4);
    sum += s.x == r[i];
  }
  return sum;
}

int va_arg_struct4(int n, ...) {
  va_list ap;
  va_start(ap, n);
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1, 11.11};
  int sum = 0;
  for (int i = 0; i < n; ++i) {
    S5 s = va_arg(ap, S5);
    sum += s.x == r[i];
  }
  return sum;
}

S19 va_arg_struct5(int n, ...) {
  va_list ap;
  va_start(ap, n);
  float r[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.1f};
  S19 ret = {{0, 0}};
  for (int i = 0; i < n; ++i) {
    S7 s = va_arg(ap, S7);
    ret.x[0] += s.x == r[i];
    ret.x[1] += s.y;
  }
  return ret;
}

S13 va_arg_struct6(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S13 ret = {{0, 0, 0}, 0, 0};
  for (int i = 0; i < n; ++i) {
    S13 s = va_arg(ap, S13);
    ret.x[0] += s.x[0];
    ret.x[1] += s.x[1];
    ret.x[2] += s.x[2];
    ret.y += s.y;
    ret.z += s.z;
  }
  return ret;
}

S19 va_arg_struct7(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S19 ret = {{0, 0}};
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
  double q[] = {-1.11, -2.22, -3.33, -4.44, -5.55,
                -6.66, -7.77, -8.88, -9.99, -10.101};
  for (int i = 0; i < n; ++i) {
    S17 s = va_arg(ap, S17);
    ret.x[0] += s.x[0] == r[i];
    ret.x[1] += s.x[1] == q[i];
  }
  return ret;
}

S19 va_arg_struct8(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S19 ret = {{0, 0}};
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
  for (int i = 0; i < n; ++i) {
    S11 s = va_arg(ap, S11);
    ret.x[0] += s.x == r[i];
    ret.x[1] += s.y;
  }
  return ret;
}

S19 va_arg_struct9(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S19 ret = {{0, 0}};
  float r[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.1f};
  for (int i = 0; i < n; ++i) {
    S9 s = va_arg(ap, S9);
    ret.x[0] += s.x;
    ret.x[1] += s.y == r[i];
  }
  return ret;
}

S20 va_arg_struct10(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S20 ret = {{0, 0, 0, 0}};
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
  double q[] = {-1.1, -2.2, -3.3, -4.4, -5.5, -6.6, -7.7, -8.8, -9.9, -10.1};
  for (int i = 0; i < n; ++i) {
    S21 s = va_arg(ap, S21);
    ret.x[0] += s.x[0] == r[i];
    ret.x[1] += s.x[1] == q[i];
    ret.x[2] += s.y;
    ret.x[3] += s.z;
  }
  return ret;
}

S20 va_arg_union1x(int n, ...) {
  va_list ap;
  va_start(ap, n);
  S20 ret = {{0, 0, 0, 0}};
  for (int i = 0; i < n; ++i) {
    U1 s = va_arg(ap, U1);
    ret.x[0] += s.x[0];
    ret.x[1] += s.x[1];
    ret.x[2] += s.x[2];
  }
  return ret;
}

int va_arg_union1y(int n, ...) {
  va_list ap;
  va_start(ap, n);
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
  int ret = 0;
  for (int i = 0; i < n; ++i) {
    U1 s = va_arg(ap, U1);
    ret += s.y == r[i];
  }
  return ret;
}

S19 va_arg_union2y(int n, ...) {
  va_list ap;
  va_start(ap, n);
  double r[] = {1.1, 2.2, 3.3, 4.4, 5.5, 6.6, 7.7, 8.8, 9.9, 10.1};
  double q[] = {-1.1, -2.2, -3.3, -4.4, -5.5, -6.6, -7.7, -8.8, -9.9, -10.1};
  S19 ret = {{0, 0}};
  for (int i = 0; i < n; ++i) {
    U2 s = va_arg(ap, U2);
    ret.x[0] += s.y[0] == r[i];
    ret.x[1] += s.y[1] == q[i];
  }
  return ret;
}

int va_arg_union2z(int n, ...) {
  va_list ap;
  va_start(ap, n);
  float r[] = {1.1f, 2.2f, 3.3f, 4.4f, 5.5f, 6.6f, 7.7f, 8.8f, 9.9f, 10.1f};
  int ret = 0;
  for (int i = 0; i < n; ++i) {
    U2 s = va_arg(ap, U2);
    ret += s.z == r[i];
  }
  return ret;
}
