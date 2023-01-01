int global_int1;
int global_int2;
int global_array3[3];
int global_array20[20];
int *global_int_pointer;

int main() {
  {
    int a;
    a = 3;
    int b;
    b = 5 * 6 - 8;
    verify(14, a + b / 2, __FILE__, __LINE__);
  }
  { // block scope
    int a;
    a = 7;
    {
      int a;
      a = 3;
      int b;
      b = 5 * 6 - 8;
      verify(14, a + b / 2, __FILE__, __LINE__);
    }
    verify(7, a, __FILE__, __LINE__);
  }
  {
    int aaa;
    aaa = 3;
    int bb;
    bb = 5 * 6 - 8;
    verify(14, aaa + bb / 2, __FILE__, __LINE__);
  }
  {
    int aaa;
    aaa = 3;
    int bb;
    bb = 5 * 6 - 8;
    aaa + bb / 2;
    verify(5, aaa + 2, __FILE__, __LINE__);
  }
  {
    int x;
    verify(4, sizeof(x), __FILE__, __LINE__);
  }
  {
    int *x;
    verify(8, sizeof(x), __FILE__, __LINE__);
  }
  {
    int *x;
    verify(8, sizeof(x + 3), __FILE__, __LINE__);
  }
  {
    int *x;
    verify(4, sizeof(*x), __FILE__, __LINE__);
  }
  verify(4, sizeof(sizeof(1)), __FILE__, __LINE__);
  verify(4, sizeof(1), __FILE__, __LINE__);
  verify(0, global_int1, __FILE__, __LINE__);
  verify(2, global_int1 + 2, __FILE__, __LINE__);
  global_int1 = 4;
  global_int2 = 3;
  verify(4, global_int1, __FILE__, __LINE__);
  verify(12, global_int1 * global_int2, __FILE__, __LINE__);
  verify(4, sizeof(global_int1), __FILE__, __LINE__);
  verify(12, sizeof(global_array3), __FILE__, __LINE__);
  global_array20[5] = 12;
  global_array20[11] = 14;
  verify(12, global_array20[5], __FILE__, __LINE__);
  verify(14, global_array20[11], __FILE__, __LINE__);

  global_int1 = 12;
  global_int_pointer = &global_int1;
  verify(12, *global_int_pointer, __FILE__, __LINE__);

  {
    int y;
    global_int1 = 12;
    y = global_int1;
    verify(12, y, __FILE__, __LINE__);
  }
  {
    char x;
    x = 20;
    verify(20, x, __FILE__, __LINE__);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    verify(3, x[0] + y, __FILE__, __LINE__);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y;
    y = 4;
    verify(6, x[1] + y, __FILE__, __LINE__);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    verify(1, x[0] + x[1], __FILE__, __LINE__);
  }
  return 0;
}
