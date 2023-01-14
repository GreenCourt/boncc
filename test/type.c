enum global_enum { X0,
                   X1,
                   X2 } g = X2;

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

  verify(4, sizeof(enum global_enum), __FILE__, __LINE__);
  verify(4, sizeof(g), __FILE__, __LINE__);
  verify(2, g, __FILE__, __LINE__);
  verify(4, sizeof(X0), __FILE__, __LINE__);
  verify(4, sizeof(X1), __FILE__, __LINE__);
  verify(4, sizeof(X2), __FILE__, __LINE__);

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
  return 0;
}
