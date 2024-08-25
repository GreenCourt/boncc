#include "common.h"

typedef enum global_enum typedef_global_enum;
enum global_enum { X0, X1, X2 } g = X2;

typedef enum {
  A,
  B,
  C,
} typedef_global_unnamed_enum;
typedef struct {
  short a;
  int b;
} typedef_global_unnamed_struct;

int main() {
  verify(4, sizeof(int));
  verify(1, sizeof(char));
  verify(2, sizeof(short));
  verify(2, sizeof(short int));
  verify(8, sizeof(long));
  verify(8, sizeof(long long));
  verify(8, sizeof(long long int));
  verify(8, sizeof(int *));
  verify(8, sizeof(char *));
  verify(32, sizeof(char *[4]));
  verify(48, sizeof(int *[3][2]));
  verify(1, sizeof(signed char));
  verify(1, sizeof(unsigned char));
  verify(1, sizeof(char signed));
  verify(1, sizeof(char unsigned));
  verify(4, sizeof(unsigned));
  verify(4, sizeof(signed));
  verify(4, sizeof(signed int));
  verify(4, sizeof(unsigned int));
  verify(4, sizeof(int signed));
  verify(4, sizeof(int unsigned));
  verify(2, sizeof(signed short));
  verify(2, sizeof(signed short int));
  verify(2, sizeof(unsigned short));
  verify(2, sizeof(unsigned short int));
  verify(2, sizeof(short signed));
  verify(2, sizeof(short signed int));
  verify(2, sizeof(short unsigned));
  verify(2, sizeof(short unsigned int));
  verify(2, sizeof(int short signed));
  verify(2, sizeof(int short unsigned));
  verify(2, sizeof(int short unsigned));
  verify(2, sizeof(int signed short));
  verify(2, sizeof(int unsigned short));
  verify(2, sizeof(int unsigned short));
  verify(8, sizeof(signed long));
  verify(8, sizeof(signed long long));
  verify(8, sizeof(signed long long int));
  verify(8, sizeof(unsigned long));
  verify(8, sizeof(unsigned long long));
  verify(8, sizeof(unsigned long long int));
  verify(8, sizeof(long signed));
  verify(8, sizeof(long signed long));
  verify(8, sizeof(long signed long int));
  verify(8, sizeof(long unsigned));
  verify(8, sizeof(long unsigned long));
  verify(8, sizeof(long unsigned long int));
  verify(8, sizeof(long long int unsigned));
  verify(8, sizeof(long long int signed));

  verify(4, sizeof(float));
  verify(8, sizeof(double));
  verify(16, sizeof(long double));
  verify(16, sizeof(double long));

  verify(4, sizeof(1U));
  verify(8, sizeof(1L));
  verify(8, sizeof(1ULL));
  verify(8, sizeof(1LL));
  verify(4, sizeof(1u));
  verify(8, sizeof(1l));
  verify(8, sizeof(1uLL));
  verify(8, sizeof(1ll));

  verify(255, 0xFF);
  verify(255, 0xFFLL);
  verify(4, sizeof(0xFF));
  verify(8, sizeof(0xFFLL));
  verify(255, 0xFf);
  verify(255, 0xfFll);
  verify(4, sizeof(0xfF));
  verify(8, sizeof(0xffLL));

  verify(10, 012ll);
  verify(4, sizeof(012U));
  verify(8, sizeof(012ull));

  verify(8, sizeof(18446744073709551615U));
  verify(8, sizeof(0xFFFFFFFFFFFFFFFF));
  verify(1, 18446744073709551615U == 0xFFFFFFFFFFFFFFFF);

  verify(4, sizeof(typedef_global_enum));
  verify(4, sizeof(enum global_enum));
  verify(4, sizeof(g));
  verify(2, g);
  verify(4, sizeof(X0));
  verify(4, sizeof(X1));
  verify(4, sizeof(X2));

  verify(4, sizeof(typedef_global_unnamed_enum));
  verify(4, sizeof(A));
  verify(4, sizeof(B));
  verify(4, sizeof(C));
  verify(0, A);
  verify(1, B);
  verify(2, C);

  verify(8, sizeof(typedef_global_unnamed_struct));

  verify(8, sizeof((const char *)0));
  verify(2, sizeof((const short)0));

  {
    // named enum
    enum X { X0, X1, X2 };
    enum X a = X1;
    verify(4, sizeof(a));
    verify(4, sizeof(enum X));
    verify(4, sizeof(X0));
    verify(4, sizeof(X1));
    verify(4, sizeof(X2));
    verify(1, a);
    verify(0, X0);
    verify(1, X1);
    verify(2, X2);
  }
  {
    // named enum and vardec at same time
    enum X { X0, X1, X2 } a = X1;
    verify(4, sizeof(a));
    verify(4, sizeof(enum X));
    verify(4, sizeof(X0));
    verify(4, sizeof(X1));
    verify(4, sizeof(X2));
    verify(1, a);
    verify(0, X0);
    verify(1, X1);
    verify(2, X2);
  }
  {
    // shadow
    enum X { X0, X1 };
    verify(0, X0);
    verify(1, X1);
    {
      enum Y { X1, X0 };
      verify(1, X0);
      verify(0, X1);
    }
  }
  {
    // unnamed enum
    enum { X0, X1 };
    verify(0, X0);
    verify(1, X1);
  }
  {
    // unnamed enum and vardec at same time
    enum { X0, X1 } a = X1;
    verify(0, X0);
    verify(1, X1);
    verify(1, a);
  }
  {
    // forward declaration
    enum E;
    enum E { A, B };
    enum E a;
    enum E; // declare twice
    verify(4, sizeof(enum E));
    verify(4, sizeof(a));
    verify(4, sizeof(A));
    verify(4, sizeof(B));
  }
  {
    typedef int T;
    T x;
    verify(4, sizeof(T));
    verify(4, sizeof(x));
  }
  {
    typedef char T;
    T x;
    typedef T S;
    S y;
    verify(1, sizeof(T));
    verify(1, sizeof(x));
    verify(1, sizeof(S));
    verify(1, sizeof(y));
  }
  {
    typedef int *int_p, I, int_array3[3];
    verify(8, sizeof(int_p));
    verify(4, sizeof(I));
    verify(12, sizeof(int_array3));
  }
  {
    typedef char *char_p, C, char_array3[3];
    verify(8, sizeof(char_p));
    verify(1, sizeof(C));
    verify(3, sizeof(char_array3));
  }
  {
    // typdedef struct
    typedef struct S S;
    struct S {
      int a;
      char b;
    };
    typedef struct S S; // twice
    S x;
    verify(8, sizeof(S));
    verify(8, sizeof(struct S));
    verify(8, sizeof(x));
  }
  {
    // typdedef and struct definition at same time
    typedef struct S {
      int a;
    } S;
    S x;
    verify(4, sizeof(S));
    verify(4, sizeof(struct S));
    verify(4, sizeof(x));
  }
  {
    // typdedef enum
    typedef enum T T;
    enum T { A, B, C };
    typedef enum T T; // twice
    T x;
    verify(4, sizeof(T));
    verify(4, sizeof(enum T));
    verify(4, sizeof(x));
    verify(4, sizeof(A));
    verify(4, sizeof(B));
    verify(4, sizeof(C));
    verify(0, A);
    verify(1, B);
    verify(2, C);
  }
  {
    // typdedef and enum definition at same time
    typedef enum T { A, B, C } T;
    T x;
    verify(4, sizeof(T));
    verify(4, sizeof(enum T));
    verify(4, sizeof(x));
    verify(4, sizeof(A));
    verify(4, sizeof(B));
    verify(4, sizeof(C));
    verify(0, A);
    verify(1, B);
    verify(2, C);
  }
  {
    enum T { A, B = 5, C, D = 4, E };
    verify(0, A);
    verify(5, B);
    verify(6, C);
    verify(4, D);
    verify(5, E);
  }
  {
    _Bool x = 12;
    verify(1, x);
    x = 0;
    verify(0, x);
    x = 1;
    verify(1, x);
    int a = 123;
    verify(1, (_Bool)a);
    a = 0;
    verify(0, (_Bool)a);
  }
  {
    int a = 12;
    (void)a;
    (void)(void)a;
  }
  {
    verify(0x0, '\x0');
    verify(0xb, '\xb');
    verify(0x7a, '\x7a');
    verify(-1, '\xff');
  }
  return 0;
}
