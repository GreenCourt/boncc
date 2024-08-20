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
int minus1();
void empty();
int vsprintf(char *, const char *, va_list);
int sprintf_(char *s, char *fmt, ...);

typedef struct {
  long long x;
} S1;
S1 return_struct1(long long x);

typedef struct {
  int x;
} S2;
S2 return_struct2(int x);

typedef struct {
  char x;
} S3;
S3 return_struct3(char x);

typedef struct {
  float x;
} S4;
S4 return_struct4(float x);

typedef struct {
  double x;
} S5;
S5 return_struct5(double x);

typedef struct {
  float x;
  float y;
} S6;
S6 return_struct6(float x, float y);

typedef struct {
  float x;
  short y;
} S7;
S7 return_struct7(float x, short y);

typedef struct {
  int x;
  float y;
} S8;
S8 return_struct8(int x, float y);

typedef struct {
  long x;
  float y;
} S9;
S9 return_struct9(long x, float y);

typedef struct {
  float x;
  long long y;
} S10;
S10 return_struct10(float x, long long y);

typedef struct {
  double x;
  int y;
} S11;
S11 return_struct11(double x, int y);

typedef struct {
  float x;
  char y;
  float z;
} S12;
S12 return_struct12(float x, char y, float z);

typedef struct {
  int x[3];
  char y;
  short z;
} S13;
S13 return_struct13(int x0, int x1, int x2, char y, short z);

typedef struct {
  float x[3];
  char y;
  short z;
} S14;
S14 return_struct14(float x0, float x1, float x2, char y, short z);

typedef struct {
  short x;
  short y;
  float z[3];
} S15;
S15 return_struct15(short x, short y, float z0, float z1, float z2);

typedef struct {
  float x[4];
} S16;
S16 return_struct16(float x0, float x1, float x2, float x3);

typedef struct {
  double x[2];
} S17;
S17 return_struct17(double x0, double x1);

typedef struct {
  union {
    int x[2];
    double y;
    short z;
  } u;
  int a;
} S18;
S18 return_struct18x(int x0, int x1, int a);
S18 return_struct18y(double y, int a);
S18 return_struct18z(short z, int a);

typedef struct {
  long long x[2];
} S19;
S19 return_struct19(long long x0, long long x1);

typedef struct {
  int x[4];
} S20;
S20 return_struct20(int x0, int x1, int x2, int x3);

typedef struct {
  double x[2];
  short y;
  long long z;
} S21;
S21 return_struct21(double x0, double x1, short y, long long z);

typedef union {
  int x[3];
  double y;
  short z;
} U1;
U1 return_union1x(int x0, int x1, int x2);
U1 return_union1y(double y);
U1 return_union1z(short z);

typedef union {
  float x[3];
  double y[2];
  float z;
} U2;
U2 return_union2x(float x0, float x1, float x2);
U2 return_union2y(double y0, double y1);
U2 return_union2z(float z);

typedef union {
  int x[2];
  long long y;
  float z[2];
} U3;
U3 return_union3x(int x0, int x1);
U3 return_union3y(long long y);
U3 return_union3z(float z0, float z1);

typedef union {
  float x[2];
  double y;
  float z;
} U4;
U4 return_union4x(float x0, float x1);
U4 return_union4y(double y);
U4 return_union4z(float z);

typedef union {
  double x;
  int y;
} U5;
U5 return_union5x(double x);
U5 return_union5y(int y);

typedef union {
  int x;
  double y;
} U6;
U6 return_union6x(int x);
U6 return_union6y(double y);

typedef union {
  float x;
  short y;
  long long z[4];
} U7;
U7 return_union7x(float x);
U7 return_union7y(short y);
U7 return_union7z(long long z0, long long z1, long long z2, long long z3);

int pass_by_stack1(int a, int b, int c, int d, int e, int f, char g, short h,
                   int i);
int pass_by_stack2(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k);
int pass_by_stack3(int a, int b, int c, int d, int e, int f, char g, short h,
                   int i, double j, float k);
int pass_by_stack4(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k, double l, float m);
int pass_by_stack5(int a, int b, float c, double d, int e, int f, char g,
                   short h, int i, long long j, int k, double l, float m,
                   float n, double o, double p, float q, float r, double s,
                   int t);

long long pass_struct1(S1 s);
long long pass_struct2(int a, int b, int c, S1 s);
long long pass_struct3(int a, int b, int c, S1 s1, S4 s4);
long long pass_struct4(int a, int b, int c, S1 s1, double d, S7 s7);
long long pass_struct5(int a, int b, int c, double d, S6 s6, S7 s7);
long long pass_struct6(S13 s13);
long long pass_struct7(S9 s9);
long long pass_struct8(int a, int b, int c, double d, float e, S9 s9);
long long pass_struct9(S10 s10);
long long pass_struct10(int a, int b, int c, double d, float e, S10 s10);
long long pass_struct11(S19 s19);
long long pass_struct12(S16 s16);
S10 pass_struct13(S9 s9);
S16 pass_struct14(S16 s16);
S20 pass_struct15(S20 s20);
long long pass_struct16(S21 s21);
S21 pass_struct17(S21 s21);
S17 pass_struct18(S17 s17);
S17 pass_struct19(double a, double b, double c, double d, double e, double f,
                  double g, S17 s17);
S17 pass_struct20(double a, double b, double c, double d, double e, double f,
                  double g, double h, S17 s17);
S19 pass_struct21(int a, int b, int c, int d, int e, S19 s19);
S19 pass_struct22(int a, int b, int c, int d, int e, int f, S19 s19);
int pass_union1x(U1 u1);
double pass_union1y(U1 u1);
short pass_union1z(U1 u1);
double pass_union5x(U5 u5);
int pass_union5y(U5 u5);

int va_arg_int(int n, ...);
long va_arg_long(int n, ...);
short va_arg_short(int n, ...);
unsigned short va_arg_ushort(int n, ...);
int va_copy_int(int n, ...);
int va_arg_double(int n, ...);
int va_arg_float(int n, ...);
int va_arg_int_double(int n, ...);
