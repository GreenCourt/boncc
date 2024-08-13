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
int vsprintf(char *, char *, va_list);
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
