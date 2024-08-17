#include "common.h"

int global_multdec[];  // array size is not given at first time
int global_multdec[3]; // array size is given at second time
int global_multdec[];  // array size is not given at third time

int global_int1;
int global_int2; // multiple declaration
int global_int2 = 4;
int global_int2; // multiple declaration
int global_int3 = {3};
int global_array3[3] = {2, 3, 4};
int global_array20[20];
int global_2darray[2][2] = {{1, 2}, {(27 / 3 == 9) * 3}};
int global_2darray_zero[2][2] = {(3 > 1) + (2 <= 2) - (5 != 4) - (3 >= 2)};
int global_3darray[2][2][3] = {{{1, 2}, {4, 5, 6}}};
char global_3darray2[2][2][3] = {{1, 2}, {4, 5}};
int global_3darray3[][2][3] = {{1, 2}, {4, 5, 6, 7, 8, 9}};
char global_3darray4[1][2][3] = {1, 2, 3, 4, 5};
int global_array_size_assumption[] = {1, 2, 3};
int global_2darray_size_assumption[][2] = {{1, 2}, {3}};
int *global_int_pointer;
long global_long = 2;
long global_long_array[1][2][3] = {1, 2, 3, 4, 5};
short global_short = 3;
short global_short_array[1][2][3] = {1, 2, 3, 4, 5};
extern short external_short;

int main() {
  // global varinit
  verify(0, global_int1);
  verify(0, global_array20[0]);
  verify(0, global_array20[15]);
  verify(0, global_array20[19]);

  verify(4, global_int2);
  verify(3, global_int3);
  verify(2, global_array3[0]);
  verify(3, global_array3[1]);
  verify(4, global_array3[2]);

  verify(1, global_2darray[0][0]);
  verify(2, global_2darray[0][1]);
  verify(3, global_2darray[1][0]);
  verify(0, global_2darray[1][1]);
  verify(0, global_2darray_zero[0][0]);
  verify(0, global_2darray_zero[0][1]);
  verify(0, global_2darray_zero[1][0]);
  verify(0, global_2darray_zero[1][1]);

  verify(1, global_3darray[0][0][0]);
  verify(2, global_3darray[0][0][1]);
  verify(0, global_3darray[0][0][2]);
  verify(4, global_3darray[0][1][0]);
  verify(5, global_3darray[0][1][1]);
  verify(6, global_3darray[0][1][2]);
  verify(0, global_3darray[1][0][0]);
  verify(0, global_3darray[1][0][1]);
  verify(0, global_3darray[1][0][2]);
  verify(0, global_3darray[1][1][0]);
  verify(0, global_3darray[1][1][1]);
  verify(0, global_3darray[1][1][2]);

  verify(12, sizeof(global_array_size_assumption));
  verify(1, global_array_size_assumption[0]);
  verify(2, global_array_size_assumption[1]);
  verify(3, global_array_size_assumption[2]);

  verify(16, sizeof(global_2darray_size_assumption));
  verify(1, global_2darray_size_assumption[0][0]);
  verify(2, global_2darray_size_assumption[0][1]);
  verify(3, global_2darray_size_assumption[1][0]);
  verify(0, global_2darray_size_assumption[1][1]);

  verify(12, sizeof(global_3darray2));
  verify(1, global_3darray2[0][0][0]);
  verify(2, global_3darray2[0][0][1]);
  verify(0, global_3darray2[0][0][2]);
  verify(0, global_3darray2[0][1][0]);
  verify(0, global_3darray2[0][1][1]);
  verify(0, global_3darray2[0][1][2]);
  verify(4, global_3darray2[1][0][0]);
  verify(5, global_3darray2[1][0][1]);
  verify(0, global_3darray2[1][0][2]);
  verify(0, global_3darray2[1][1][0]);
  verify(0, global_3darray2[1][1][1]);
  verify(0, global_3darray2[1][1][2]);

  verify(48, sizeof(global_3darray3));
  verify(1, global_3darray3[0][0][0]);
  verify(2, global_3darray3[0][0][1]);
  verify(0, global_3darray3[0][0][2]);
  verify(0, global_3darray3[0][1][0]);
  verify(0, global_3darray3[0][1][1]);
  verify(0, global_3darray3[0][1][2]);
  verify(4, global_3darray3[1][0][0]);
  verify(5, global_3darray3[1][0][1]);
  verify(6, global_3darray3[1][0][2]);
  verify(7, global_3darray3[1][1][0]);
  verify(8, global_3darray3[1][1][1]);
  verify(9, global_3darray3[1][1][2]);

  verify(6, sizeof(global_3darray4));
  verify(1, global_3darray4[0][0][0]);
  verify(2, global_3darray4[0][0][1]);
  verify(3, global_3darray4[0][0][2]);
  verify(4, global_3darray4[0][1][0]);
  verify(5, global_3darray4[0][1][1]);
  verify(0, global_3darray4[0][1][2]);

  verify(8, sizeof(global_long));
  verify(2, global_long);

  verify(2, sizeof(global_short));
  verify(3, global_short);

  verify(48, sizeof(global_long_array));
  verify(1, global_long_array[0][0][0]);
  verify(2, global_long_array[0][0][1]);
  verify(3, global_long_array[0][0][2]);
  verify(4, global_long_array[0][1][0]);
  verify(5, global_long_array[0][1][1]);
  verify(0, global_long_array[0][1][2]);

  verify(12, sizeof(global_short_array));
  verify(1, global_short_array[0][0][0]);
  verify(2, global_short_array[0][0][1]);
  verify(3, global_short_array[0][0][2]);
  verify(4, global_short_array[0][1][0]);
  verify(5, global_short_array[0][1][1]);
  verify(0, global_short_array[0][1][2]);

  {
    short a = 3;
    long b = 5 * 6 - 8;
    verify(14, a + b / 2);
  }
  { // block scope
    int a = 7;
    {
      int a = 3;
      int b = 5 * 6 - 8;
      verify(14, a + b / 2);
    }
    verify(7, a);
  }
  {
    int aaa = 3;
    int bb = 5 * 6 - 8;
    verify(14, aaa + bb / 2);
    verify(5, aaa + 2);
  }
  {
    int x;
    verify(4, sizeof(x));
  }
  {
    int *x;
    verify(8, sizeof(x));
  }
  {
    int *x;
    verify(8, sizeof(x + 3));
  }
  {
    int *x;
    verify(4, sizeof(*x));
  }
  verify(8, sizeof(sizeof(1)));
  verify(4, sizeof(1));

  global_int1 = 4;
  global_int2 = 3;
  verify(4, global_int1);
  verify(12, global_int1 * global_int2);
  verify(4, sizeof(global_int1));
  verify(12, sizeof(global_array3));
  global_array20[5] = 12;
  global_array20[11] = 14;
  verify(12, global_array20[5]);
  verify(14, global_array20[11]);

  global_int1 = 12;
  global_int_pointer = &global_int1;
  verify(12, *global_int_pointer);

  {
    int y;
    global_int1 = 12;
    y = global_int1;
    verify(12, y);
  }
  {
    char x = 20;
    verify(20, x);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y = 4;
    verify(3, x[0] + y);
    verify(6, x[1] + y);
    verify(1, x[0] + x[1]);
    char *p = &x[0];
    verify(2, *(p + 1));
  }
  {
    int a = 3 * 2 + 1;
    verify(7, a);
  }
  {
    int x[3] = {1, 2};
    verify(1, x[0]);
    verify(2, x[1]);
    verify(0, x[2]);
  }
  {
    char x[3] = {1, 2};
    verify(1, x[0]);
    verify(2, x[1]);
    verify(0, x[2]);
  }
  {
    int x = {1};
    verify(1, x);
  }
  {
    int a = 3;
    int b = a * a + 4 / 2;
    int x[3] = {1, b - a};
    verify(3, a);
    verify(11, b);
    verify(1, x[0]);
    verify(8, x[1]);
    verify(0, x[2]);
  }
  {
    int x[3] = {1, x[0]};
    verify(1, x[0]);
    verify(1, x[1]);
    verify(0, x[2]);
  }
  {
    int x[3] = {1, 2, 3}, *p[4], a = 2, *t;
    int **d, b;
    verify(12, sizeof(x));
    verify(32, sizeof(p));
    verify(4, sizeof(a));
    verify(8, sizeof(t));
    verify(8, sizeof(d));
    verify(4, sizeof(b));
    verify(1, x[0]);
    verify(2, x[1]);
    verify(3, x[2]);
    verify(2, a);
  }
  {
    int x[] = {1, x[0], 2};
    verify(1, x[0]);
    verify(1, x[1]);
    verify(2, x[2]);
    verify(12, sizeof(x));
  }

  {
    int x[2][2] = {{1, 2}, {(27 / 3 == 9) * 3}};
    verify(1, x[0][0]);
    verify(2, x[0][1]);
    verify(3, x[1][0]);
    verify(0, x[1][1]);
    verify(16, sizeof(x));
  }
  {
    int x[2][2] = {(3 > 1) + (2 <= 2) - (5 != 4) - (3 >= 2)};
    verify(0, x[0][0]);
    verify(0, x[0][1]);
    verify(0, x[0][1]);
    verify(0, x[1][1]);
    verify(16, sizeof(x));
  }
  {
    int x[3][2][3] = {{{1, 2}, {4, 5, 6}}};
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(0, x[0][0][2]);
    verify(4, x[0][1][0]);
    verify(5, x[0][1][1]);
    verify(6, x[0][1][2]);
    verify(0, x[1][0][0]);
    verify(0, x[1][0][1]);
    verify(0, x[1][0][2]);
    verify(0, x[1][1][0]);
    verify(0, x[1][1][1]);
    verify(0, x[1][1][2]);
    verify(72, sizeof(x));
    verify(24, sizeof(x[0]));
    verify(12, sizeof(x[0][0]));
  }
  {
    char x[2][2][3] = {{1, 2}, {4, 5}};
    verify(12, sizeof(x));
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(0, x[0][0][2]);
    verify(0, x[0][1][0]);
    verify(0, x[0][1][1]);
    verify(0, x[0][1][2]);
    verify(4, x[1][0][0]);
    verify(5, x[1][0][1]);
    verify(0, x[1][0][2]);
    verify(0, x[1][1][0]);
    verify(0, x[1][1][1]);
    verify(0, x[1][1][2]);
  }
  {
    int x[][2][3] = {{1, 2}, {4, 5, 6, 7, 8, 9}};
    verify(48, sizeof(x));
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(0, x[0][0][2]);
    verify(0, x[0][1][0]);
    verify(0, x[0][1][1]);
    verify(0, x[0][1][2]);
    verify(4, x[1][0][0]);
    verify(5, x[1][0][1]);
    verify(6, x[1][0][2]);
    verify(7, x[1][1][0]);
    verify(8, x[1][1][1]);
    verify(9, x[1][1][2]);
  }
  {
    char x[1][2][3] = {1, 2, 3, 4, 5};
    verify(6, sizeof(x));
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(3, x[0][0][2]);
    verify(4, x[0][1][0]);
    verify(5, x[0][1][1]);
    verify(0, x[0][1][2]);
  }
  {
    long x[1][2][3] = {1, 2, 3, 4, 5};
    verify(48, sizeof(x));
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(3, x[0][0][2]);
    verify(4, x[0][1][0]);
    verify(5, x[0][1][1]);
    verify(0, x[0][1][2]);
  }
  {
    short x[1][2][3] = {1, 2, 3, 4, 5};
    verify(12, sizeof(x));
    verify(1, x[0][0][0]);
    verify(2, x[0][0][1]);
    verify(3, x[0][0][2]);
    verify(4, x[0][1][0]);
    verify(5, x[0][1][1]);
    verify(0, x[0][1][2]);
  }
  {
    extern char external_char;
    {
      extern int external_int;
      verify(56, external_char);
      verify(1234, external_int);
      external_char = 78;
    }
    verify(78, external_char);
    verify(789, external_short);
    external_short = -455;
    verify(-455, external_short);
  }
  {
    const int x = 12;
    verify(12, x);
  }
  {
    int arr[12 + sizeof(int) * 2];
    verify(80, sizeof(arr));
  }
  return 0;
}
