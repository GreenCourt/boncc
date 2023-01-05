int global_int1;
int global_int2 = 4;
int global_int3 = {3};
int global_array3[3] = {2, 3, 4};
int global_array20[20];
int global_2darray[2][2] = {{1, 2}, {(27 / 3 == 9) * 3}};
int global_2darray_zero[2][2] = {(3 > 1) + (2 <= 2) - (5 != 4) - (3 >= 2)};
int global_3darray[2][2][3] = {{{1, 2}, {4, 5, 6}}};
int *global_int_pointer;

int main() {
  // global varinit
  verify(0, global_int1, __FILE__, __LINE__);
  verify(0, global_array20[0], __FILE__, __LINE__);
  verify(0, global_array20[15], __FILE__, __LINE__);
  verify(0, global_array20[19], __FILE__, __LINE__);

  verify(4, global_int2, __FILE__, __LINE__);
  verify(3, global_int3, __FILE__, __LINE__);
  verify(2, global_array3[0], __FILE__, __LINE__);
  verify(3, global_array3[1], __FILE__, __LINE__);
  verify(4, global_array3[2], __FILE__, __LINE__);

  verify(1, global_2darray[0][0], __FILE__, __LINE__);
  verify(2, global_2darray[0][1], __FILE__, __LINE__);
  verify(3, global_2darray[1][0], __FILE__, __LINE__);
  verify(0, global_2darray[1][1], __FILE__, __LINE__);
  verify(0, global_2darray_zero[0][0], __FILE__, __LINE__);
  verify(0, global_2darray_zero[0][1], __FILE__, __LINE__);
  verify(0, global_2darray_zero[1][0], __FILE__, __LINE__);
  verify(0, global_2darray_zero[1][1], __FILE__, __LINE__);

  verify(1, global_3darray[0][0][0], __FILE__, __LINE__);
  verify(2, global_3darray[0][0][1], __FILE__, __LINE__);
  verify(0, global_3darray[0][0][2], __FILE__, __LINE__);
  verify(4, global_3darray[0][1][0], __FILE__, __LINE__);
  verify(5, global_3darray[0][1][1], __FILE__, __LINE__);
  verify(6, global_3darray[0][1][2], __FILE__, __LINE__);
  verify(0, global_3darray[1][0][0], __FILE__, __LINE__);
  verify(0, global_3darray[1][0][1], __FILE__, __LINE__);
  verify(0, global_3darray[1][0][2], __FILE__, __LINE__);
  verify(0, global_3darray[1][1][0], __FILE__, __LINE__);
  verify(0, global_3darray[1][1][1], __FILE__, __LINE__);
  verify(0, global_3darray[1][1][2], __FILE__, __LINE__);

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
  {
    int a = 3 * 2 + 1;
    verify(7, a, __FILE__, __LINE__);
  }
  {
    char a = 3 * 2 + 1;
    verify(7, a, __FILE__, __LINE__);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    char *p = &x[0];
    verify(2, *(p + 1), __FILE__, __LINE__);
  }
  {
    int x[3] = {1, 2};
    verify(1, x[0], __FILE__, __LINE__);
    verify(2, x[1], __FILE__, __LINE__);
    verify(0, x[2], __FILE__, __LINE__);
  }
  {
    char x[3] = {1, 2};
    verify(1, x[0], __FILE__, __LINE__);
    verify(2, x[1], __FILE__, __LINE__);
    verify(0, x[2], __FILE__, __LINE__);
  }
  {
    int x = {1};
    verify(1, x, __FILE__, __LINE__);
  }
  {
    int a = 3;
    int b = a * a + 4 / 2;
    int x[3] = {1, b - a};
    verify(3, a, __FILE__, __LINE__);
    verify(11, b, __FILE__, __LINE__);
    verify(1, x[0], __FILE__, __LINE__);
    verify(8, x[1], __FILE__, __LINE__);
    verify(0, x[2], __FILE__, __LINE__);
  }
  {
    int x[3] = {1, x[0], 2};
    verify(1, x[0], __FILE__, __LINE__);
    verify(1, x[1], __FILE__, __LINE__);
    verify(2, x[2], __FILE__, __LINE__);
  }
  {
    int x[3] = {1, 2, 3}, *p[4], a = 2, *t;
    int **d, b;
    verify(12, sizeof(x), __FILE__, __LINE__);
    verify(32, sizeof(p), __FILE__, __LINE__);
    verify(4, sizeof(a), __FILE__, __LINE__);
    verify(8, sizeof(t), __FILE__, __LINE__);
    verify(8, sizeof(d), __FILE__, __LINE__);
    verify(4, sizeof(b), __FILE__, __LINE__);
    verify(1, x[0], __FILE__, __LINE__);
    verify(2, x[1], __FILE__, __LINE__);
    verify(3, x[2], __FILE__, __LINE__);
    verify(2, a, __FILE__, __LINE__);
  }
  return 0;
}
