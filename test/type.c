void verify(int expected, int actual, char *file_name, int line_number);

typedef enum global_enum typedef_global_enum;
enum global_enum { X0,
                   X1,
                   X2 } g = X2;

typedef enum { A,
               B,
               C,
} typedef_global_unnamed_enum;
typedef struct {
  short a;
  int b;
} typedef_global_unnamed_struct;

int main() {
  verify(4, sizeof(int), __FILE__, __LINE__);
  verify(1, sizeof(char), __FILE__, __LINE__);
  verify(2, sizeof(short), __FILE__, __LINE__);
  verify(2, sizeof(short int), __FILE__, __LINE__);
  verify(8, sizeof(long), __FILE__, __LINE__);
  verify(8, sizeof(long long), __FILE__, __LINE__);
  verify(8, sizeof(long long int), __FILE__, __LINE__);
  verify(8, sizeof(int *), __FILE__, __LINE__);
  verify(8, sizeof(char *), __FILE__, __LINE__);
  verify(32, sizeof(char *[4]), __FILE__, __LINE__);
  verify(48, sizeof(int *[3][2]), __FILE__, __LINE__);
  verify(1, sizeof(signed char), __FILE__, __LINE__);
  verify(1, sizeof(unsigned char), __FILE__, __LINE__);
  verify(1, sizeof(char signed), __FILE__, __LINE__);
  verify(1, sizeof(char unsigned), __FILE__, __LINE__);
  verify(4, sizeof(unsigned), __FILE__, __LINE__);
  verify(4, sizeof(signed), __FILE__, __LINE__);
  verify(4, sizeof(signed int), __FILE__, __LINE__);
  verify(4, sizeof(unsigned int), __FILE__, __LINE__);
  verify(4, sizeof(int signed), __FILE__, __LINE__);
  verify(4, sizeof(int unsigned), __FILE__, __LINE__);
  verify(2, sizeof(signed short), __FILE__, __LINE__);
  verify(2, sizeof(signed short int), __FILE__, __LINE__);
  verify(2, sizeof(unsigned short), __FILE__, __LINE__);
  verify(2, sizeof(unsigned short int), __FILE__, __LINE__);
  verify(2, sizeof(short signed), __FILE__, __LINE__);
  verify(2, sizeof(short signed int), __FILE__, __LINE__);
  verify(2, sizeof(short unsigned), __FILE__, __LINE__);
  verify(2, sizeof(short unsigned int), __FILE__, __LINE__);
  verify(2, sizeof(int short signed), __FILE__, __LINE__);
  verify(2, sizeof(int short unsigned), __FILE__, __LINE__);
  verify(2, sizeof(int short unsigned), __FILE__, __LINE__);
  verify(2, sizeof(int signed short), __FILE__, __LINE__);
  verify(2, sizeof(int unsigned short), __FILE__, __LINE__);
  verify(2, sizeof(int unsigned short), __FILE__, __LINE__);
  verify(8, sizeof(signed long), __FILE__, __LINE__);
  verify(8, sizeof(signed long long), __FILE__, __LINE__);
  verify(8, sizeof(signed long long int), __FILE__, __LINE__);
  verify(8, sizeof(unsigned long), __FILE__, __LINE__);
  verify(8, sizeof(unsigned long long), __FILE__, __LINE__);
  verify(8, sizeof(unsigned long long int), __FILE__, __LINE__);
  verify(8, sizeof(long signed), __FILE__, __LINE__);
  verify(8, sizeof(long signed long), __FILE__, __LINE__);
  verify(8, sizeof(long signed long int), __FILE__, __LINE__);
  verify(8, sizeof(long unsigned), __FILE__, __LINE__);
  verify(8, sizeof(long unsigned long), __FILE__, __LINE__);
  verify(8, sizeof(long unsigned long int), __FILE__, __LINE__);
  verify(8, sizeof(long long int unsigned), __FILE__, __LINE__);
  verify(8, sizeof(long long int signed), __FILE__, __LINE__);

  verify(4, sizeof(float), __FILE__, __LINE__);
  verify(8, sizeof(double), __FILE__, __LINE__);
  verify(16, sizeof(long double), __FILE__, __LINE__);
  verify(16, sizeof(double long), __FILE__, __LINE__);

  verify(4, sizeof(typedef_global_enum), __FILE__, __LINE__);
  verify(4, sizeof(enum global_enum), __FILE__, __LINE__);
  verify(4, sizeof(g), __FILE__, __LINE__);
  verify(2, g, __FILE__, __LINE__);
  verify(4, sizeof(X0), __FILE__, __LINE__);
  verify(4, sizeof(X1), __FILE__, __LINE__);
  verify(4, sizeof(X2), __FILE__, __LINE__);

  verify(4, sizeof(typedef_global_unnamed_enum), __FILE__, __LINE__);
  verify(4, sizeof(A), __FILE__, __LINE__);
  verify(4, sizeof(B), __FILE__, __LINE__);
  verify(4, sizeof(C), __FILE__, __LINE__);
  verify(0, A, __FILE__, __LINE__);
  verify(1, B, __FILE__, __LINE__);
  verify(2, C, __FILE__, __LINE__);

  verify(8, sizeof(typedef_global_unnamed_struct), __FILE__, __LINE__);

  {
    // named enum
    enum X { X0,
             X1,
             X2 };
    enum X a = X1;
    verify(4, sizeof(a), __FILE__, __LINE__);
    verify(4, sizeof(enum X), __FILE__, __LINE__);
    verify(4, sizeof(X0), __FILE__, __LINE__);
    verify(4, sizeof(X1), __FILE__, __LINE__);
    verify(4, sizeof(X2), __FILE__, __LINE__);
    verify(1, a, __FILE__, __LINE__);
    verify(0, X0, __FILE__, __LINE__);
    verify(1, X1, __FILE__, __LINE__);
    verify(2, X2, __FILE__, __LINE__);
  }
  {
    // named enum and vardec at same time
    enum X { X0,
             X1,
             X2 } a = X1;
    verify(4, sizeof(a), __FILE__, __LINE__);
    verify(4, sizeof(enum X), __FILE__, __LINE__);
    verify(4, sizeof(X0), __FILE__, __LINE__);
    verify(4, sizeof(X1), __FILE__, __LINE__);
    verify(4, sizeof(X2), __FILE__, __LINE__);
    verify(1, a, __FILE__, __LINE__);
    verify(0, X0, __FILE__, __LINE__);
    verify(1, X1, __FILE__, __LINE__);
    verify(2, X2, __FILE__, __LINE__);
  }
  {
    // shadow
    enum X { X0,
             X1 };
    verify(0, X0, __FILE__, __LINE__);
    verify(1, X1, __FILE__, __LINE__);
    {
      enum Y { X1,
               X0 };
      verify(1, X0, __FILE__, __LINE__);
      verify(0, X1, __FILE__, __LINE__);
    }
  }
  {
    // unnamed enum
    enum { X0,
           X1 };
    verify(0, X0, __FILE__, __LINE__);
    verify(1, X1, __FILE__, __LINE__);
  }
  {
    // unnamed enum and vardec at same time
    enum { X0,
           X1 } a = X1;
    verify(0, X0, __FILE__, __LINE__);
    verify(1, X1, __FILE__, __LINE__);
    verify(1, a, __FILE__, __LINE__);
  }
  {
    // forward declaration
    enum E;
    enum E { A,
             B };
    enum E a;
    enum E; // declare twice
    verify(4, sizeof(enum E), __FILE__, __LINE__);
    verify(4, sizeof(a), __FILE__, __LINE__);
    verify(4, sizeof(A), __FILE__, __LINE__);
    verify(4, sizeof(B), __FILE__, __LINE__);
  }
  {
    typedef int T;
    T x;
    verify(4, sizeof(T), __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
  }
  {
    typedef char T;
    T x;
    typedef T S;
    S y;
    verify(1, sizeof(T), __FILE__, __LINE__);
    verify(1, sizeof(x), __FILE__, __LINE__);
    verify(1, sizeof(S), __FILE__, __LINE__);
    verify(1, sizeof(y), __FILE__, __LINE__);
  }
  {
    typedef int *int_p, I, int_array3[3];
    verify(8, sizeof(int_p), __FILE__, __LINE__);
    verify(4, sizeof(I), __FILE__, __LINE__);
    verify(12, sizeof(int_array3), __FILE__, __LINE__);
  }
  {
    typedef char *char_p, C, char_array3[3];
    verify(8, sizeof(char_p), __FILE__, __LINE__);
    verify(1, sizeof(C), __FILE__, __LINE__);
    verify(3, sizeof(char_array3), __FILE__, __LINE__);
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
    verify(8, sizeof(S), __FILE__, __LINE__);
    verify(8, sizeof(struct S), __FILE__, __LINE__);
    verify(8, sizeof(x), __FILE__, __LINE__);
  }
  {
    // typdedef and struct definition at same time
    typedef struct S {
      int a;
    } S;
    S x;
    verify(4, sizeof(S), __FILE__, __LINE__);
    verify(4, sizeof(struct S), __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
  }
  {
    // typdedef enum
    typedef enum T T;
    enum T {
      A,
      B,
      C
    };
    typedef enum T T; // twice
    T x;
    verify(4, sizeof(T), __FILE__, __LINE__);
    verify(4, sizeof(enum T), __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
    verify(4, sizeof(A), __FILE__, __LINE__);
    verify(4, sizeof(B), __FILE__, __LINE__);
    verify(4, sizeof(C), __FILE__, __LINE__);
    verify(0, A, __FILE__, __LINE__);
    verify(1, B, __FILE__, __LINE__);
    verify(2, C, __FILE__, __LINE__);
  }
  {
    // typdedef and enum definition at same time
    typedef enum T {
      A,
      B,
      C
    } T;
    T x;
    verify(4, sizeof(T), __FILE__, __LINE__);
    verify(4, sizeof(enum T), __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
    verify(4, sizeof(A), __FILE__, __LINE__);
    verify(4, sizeof(B), __FILE__, __LINE__);
    verify(4, sizeof(C), __FILE__, __LINE__);
    verify(0, A, __FILE__, __LINE__);
    verify(1, B, __FILE__, __LINE__);
    verify(2, C, __FILE__, __LINE__);
  }
  {
    enum T {
      A,
      B = 5,
      C,
      D = 4,
      E
    };
    verify(0, A, __FILE__, __LINE__);
    verify(5, B, __FILE__, __LINE__);
    verify(6, C, __FILE__, __LINE__);
    verify(4, D, __FILE__, __LINE__);
    verify(5, E, __FILE__, __LINE__);
  }
  return 0;
}
