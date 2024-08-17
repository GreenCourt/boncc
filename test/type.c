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
  VERIFY(4, sizeof(int));
  VERIFY(1, sizeof(char));
  VERIFY(2, sizeof(short));
  VERIFY(2, sizeof(short int));
  VERIFY(8, sizeof(long));
  VERIFY(8, sizeof(long long));
  VERIFY(8, sizeof(long long int));
  VERIFY(8, sizeof(int *));
  VERIFY(8, sizeof(char *));
  VERIFY(32, sizeof(char *[4]));
  VERIFY(48, sizeof(int *[3][2]));
  VERIFY(1, sizeof(signed char));
  VERIFY(1, sizeof(unsigned char));
  VERIFY(1, sizeof(char signed));
  VERIFY(1, sizeof(char unsigned));
  VERIFY(4, sizeof(unsigned));
  VERIFY(4, sizeof(signed));
  VERIFY(4, sizeof(signed int));
  VERIFY(4, sizeof(unsigned int));
  VERIFY(4, sizeof(int signed));
  VERIFY(4, sizeof(int unsigned));
  VERIFY(2, sizeof(signed short));
  VERIFY(2, sizeof(signed short int));
  VERIFY(2, sizeof(unsigned short));
  VERIFY(2, sizeof(unsigned short int));
  VERIFY(2, sizeof(short signed));
  VERIFY(2, sizeof(short signed int));
  VERIFY(2, sizeof(short unsigned));
  VERIFY(2, sizeof(short unsigned int));
  VERIFY(2, sizeof(int short signed));
  VERIFY(2, sizeof(int short unsigned));
  VERIFY(2, sizeof(int short unsigned));
  VERIFY(2, sizeof(int signed short));
  VERIFY(2, sizeof(int unsigned short));
  VERIFY(2, sizeof(int unsigned short));
  VERIFY(8, sizeof(signed long));
  VERIFY(8, sizeof(signed long long));
  VERIFY(8, sizeof(signed long long int));
  VERIFY(8, sizeof(unsigned long));
  VERIFY(8, sizeof(unsigned long long));
  VERIFY(8, sizeof(unsigned long long int));
  VERIFY(8, sizeof(long signed));
  VERIFY(8, sizeof(long signed long));
  VERIFY(8, sizeof(long signed long int));
  VERIFY(8, sizeof(long unsigned));
  VERIFY(8, sizeof(long unsigned long));
  VERIFY(8, sizeof(long unsigned long int));
  VERIFY(8, sizeof(long long int unsigned));
  VERIFY(8, sizeof(long long int signed));

  VERIFY(4, sizeof(float));
  VERIFY(8, sizeof(double));
  VERIFY(16, sizeof(long double));
  VERIFY(16, sizeof(double long));

  VERIFY(4, sizeof(1U));
  VERIFY(8, sizeof(1L));
  VERIFY(8, sizeof(1ULL));
  VERIFY(8, sizeof(1LL));
  VERIFY(4, sizeof(1u));
  VERIFY(8, sizeof(1l));
  VERIFY(8, sizeof(1uLL));
  VERIFY(8, sizeof(1ll));

  VERIFY(255, 0xFF);
  VERIFY(255, 0xFFLL);
  VERIFY(4, sizeof(0xFF));
  VERIFY(8, sizeof(0xFFLL));
  VERIFY(255, 0xFf);
  VERIFY(255, 0xfFll);
  VERIFY(4, sizeof(0xfF));
  VERIFY(8, sizeof(0xffLL));

  VERIFY(10, 012ll);
  VERIFY(4, sizeof(012U));
  VERIFY(8, sizeof(012ull));

  VERIFY(8, sizeof(18446744073709551615U));
  VERIFY(8, sizeof(0xFFFFFFFFFFFFFFFF));
  VERIFY(1, 18446744073709551615U == 0xFFFFFFFFFFFFFFFF);

  VERIFY(4, sizeof(typedef_global_enum));
  VERIFY(4, sizeof(enum global_enum));
  VERIFY(4, sizeof(g));
  VERIFY(2, g);
  VERIFY(4, sizeof(X0));
  VERIFY(4, sizeof(X1));
  VERIFY(4, sizeof(X2));

  VERIFY(4, sizeof(typedef_global_unnamed_enum));
  VERIFY(4, sizeof(A));
  VERIFY(4, sizeof(B));
  VERIFY(4, sizeof(C));
  VERIFY(0, A);
  VERIFY(1, B);
  VERIFY(2, C);

  VERIFY(8, sizeof(typedef_global_unnamed_struct));

  {
    // named enum
    enum X { X0, X1, X2 };
    enum X a = X1;
    VERIFY(4, sizeof(a));
    VERIFY(4, sizeof(enum X));
    VERIFY(4, sizeof(X0));
    VERIFY(4, sizeof(X1));
    VERIFY(4, sizeof(X2));
    VERIFY(1, a);
    VERIFY(0, X0);
    VERIFY(1, X1);
    VERIFY(2, X2);
  }
  {
    // named enum and vardec at same time
    enum X { X0, X1, X2 } a = X1;
    VERIFY(4, sizeof(a));
    VERIFY(4, sizeof(enum X));
    VERIFY(4, sizeof(X0));
    VERIFY(4, sizeof(X1));
    VERIFY(4, sizeof(X2));
    VERIFY(1, a);
    VERIFY(0, X0);
    VERIFY(1, X1);
    VERIFY(2, X2);
  }
  {
    // shadow
    enum X { X0, X1 };
    VERIFY(0, X0);
    VERIFY(1, X1);
    {
      enum Y { X1, X0 };
      VERIFY(1, X0);
      VERIFY(0, X1);
    }
  }
  {
    // unnamed enum
    enum { X0, X1 };
    VERIFY(0, X0);
    VERIFY(1, X1);
  }
  {
    // unnamed enum and vardec at same time
    enum { X0, X1 } a = X1;
    VERIFY(0, X0);
    VERIFY(1, X1);
    VERIFY(1, a);
  }
  {
    // forward declaration
    enum E;
    enum E { A, B };
    enum E a;
    enum E; // declare twice
    VERIFY(4, sizeof(enum E));
    VERIFY(4, sizeof(a));
    VERIFY(4, sizeof(A));
    VERIFY(4, sizeof(B));
  }
  {
    typedef int T;
    T x;
    VERIFY(4, sizeof(T));
    VERIFY(4, sizeof(x));
  }
  {
    typedef char T;
    T x;
    typedef T S;
    S y;
    VERIFY(1, sizeof(T));
    VERIFY(1, sizeof(x));
    VERIFY(1, sizeof(S));
    VERIFY(1, sizeof(y));
  }
  {
    typedef int *int_p, I, int_array3[3];
    VERIFY(8, sizeof(int_p));
    VERIFY(4, sizeof(I));
    VERIFY(12, sizeof(int_array3));
  }
  {
    typedef char *char_p, C, char_array3[3];
    VERIFY(8, sizeof(char_p));
    VERIFY(1, sizeof(C));
    VERIFY(3, sizeof(char_array3));
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
    VERIFY(8, sizeof(S));
    VERIFY(8, sizeof(struct S));
    VERIFY(8, sizeof(x));
  }
  {
    // typdedef and struct definition at same time
    typedef struct S {
      int a;
    } S;
    S x;
    VERIFY(4, sizeof(S));
    VERIFY(4, sizeof(struct S));
    VERIFY(4, sizeof(x));
  }
  {
    // typdedef enum
    typedef enum T T;
    enum T { A, B, C };
    typedef enum T T; // twice
    T x;
    VERIFY(4, sizeof(T));
    VERIFY(4, sizeof(enum T));
    VERIFY(4, sizeof(x));
    VERIFY(4, sizeof(A));
    VERIFY(4, sizeof(B));
    VERIFY(4, sizeof(C));
    VERIFY(0, A);
    VERIFY(1, B);
    VERIFY(2, C);
  }
  {
    // typdedef and enum definition at same time
    typedef enum T { A, B, C } T;
    T x;
    VERIFY(4, sizeof(T));
    VERIFY(4, sizeof(enum T));
    VERIFY(4, sizeof(x));
    VERIFY(4, sizeof(A));
    VERIFY(4, sizeof(B));
    VERIFY(4, sizeof(C));
    VERIFY(0, A);
    VERIFY(1, B);
    VERIFY(2, C);
  }
  {
    enum T { A, B = 5, C, D = 4, E };
    VERIFY(0, A);
    VERIFY(5, B);
    VERIFY(6, C);
    VERIFY(4, D);
    VERIFY(5, E);
  }
  {
    _Bool x = 12;
    VERIFY(1, x);
    x = 0;
    VERIFY(0, x);
    x = 1;
    VERIFY(1, x);
    int a = 123;
    VERIFY(1, (_Bool)a);
    a = 0;
    VERIFY(0, (_Bool)a);
  }
  {
    int a = 12;
    (void)a;
    (void)(void)a;
  }
  {
    VERIFY(0x0, '\x0');
    VERIFY(0xb, '\xb');
    VERIFY(0x7a, '\x7a');
    VERIFY(-1, '\xff');
  }
  return 0;
}
