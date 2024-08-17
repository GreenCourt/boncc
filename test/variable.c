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
  VERIFY(0, global_int1);
  VERIFY(0, global_array20[0]);
  VERIFY(0, global_array20[15]);
  VERIFY(0, global_array20[19]);

  VERIFY(4, global_int2);
  VERIFY(3, global_int3);
  VERIFY(2, global_array3[0]);
  VERIFY(3, global_array3[1]);
  VERIFY(4, global_array3[2]);

  VERIFY(1, global_2darray[0][0]);
  VERIFY(2, global_2darray[0][1]);
  VERIFY(3, global_2darray[1][0]);
  VERIFY(0, global_2darray[1][1]);
  VERIFY(0, global_2darray_zero[0][0]);
  VERIFY(0, global_2darray_zero[0][1]);
  VERIFY(0, global_2darray_zero[1][0]);
  VERIFY(0, global_2darray_zero[1][1]);

  VERIFY(1, global_3darray[0][0][0]);
  VERIFY(2, global_3darray[0][0][1]);
  VERIFY(0, global_3darray[0][0][2]);
  VERIFY(4, global_3darray[0][1][0]);
  VERIFY(5, global_3darray[0][1][1]);
  VERIFY(6, global_3darray[0][1][2]);
  VERIFY(0, global_3darray[1][0][0]);
  VERIFY(0, global_3darray[1][0][1]);
  VERIFY(0, global_3darray[1][0][2]);
  VERIFY(0, global_3darray[1][1][0]);
  VERIFY(0, global_3darray[1][1][1]);
  VERIFY(0, global_3darray[1][1][2]);

  VERIFY(12, sizeof(global_array_size_assumption));
  VERIFY(1, global_array_size_assumption[0]);
  VERIFY(2, global_array_size_assumption[1]);
  VERIFY(3, global_array_size_assumption[2]);

  VERIFY(16, sizeof(global_2darray_size_assumption));
  VERIFY(1, global_2darray_size_assumption[0][0]);
  VERIFY(2, global_2darray_size_assumption[0][1]);
  VERIFY(3, global_2darray_size_assumption[1][0]);
  VERIFY(0, global_2darray_size_assumption[1][1]);

  VERIFY(12, sizeof(global_3darray2));
  VERIFY(1, global_3darray2[0][0][0]);
  VERIFY(2, global_3darray2[0][0][1]);
  VERIFY(0, global_3darray2[0][0][2]);
  VERIFY(0, global_3darray2[0][1][0]);
  VERIFY(0, global_3darray2[0][1][1]);
  VERIFY(0, global_3darray2[0][1][2]);
  VERIFY(4, global_3darray2[1][0][0]);
  VERIFY(5, global_3darray2[1][0][1]);
  VERIFY(0, global_3darray2[1][0][2]);
  VERIFY(0, global_3darray2[1][1][0]);
  VERIFY(0, global_3darray2[1][1][1]);
  VERIFY(0, global_3darray2[1][1][2]);

  VERIFY(48, sizeof(global_3darray3));
  VERIFY(1, global_3darray3[0][0][0]);
  VERIFY(2, global_3darray3[0][0][1]);
  VERIFY(0, global_3darray3[0][0][2]);
  VERIFY(0, global_3darray3[0][1][0]);
  VERIFY(0, global_3darray3[0][1][1]);
  VERIFY(0, global_3darray3[0][1][2]);
  VERIFY(4, global_3darray3[1][0][0]);
  VERIFY(5, global_3darray3[1][0][1]);
  VERIFY(6, global_3darray3[1][0][2]);
  VERIFY(7, global_3darray3[1][1][0]);
  VERIFY(8, global_3darray3[1][1][1]);
  VERIFY(9, global_3darray3[1][1][2]);

  VERIFY(6, sizeof(global_3darray4));
  VERIFY(1, global_3darray4[0][0][0]);
  VERIFY(2, global_3darray4[0][0][1]);
  VERIFY(3, global_3darray4[0][0][2]);
  VERIFY(4, global_3darray4[0][1][0]);
  VERIFY(5, global_3darray4[0][1][1]);
  VERIFY(0, global_3darray4[0][1][2]);

  VERIFY(8, sizeof(global_long));
  VERIFY(2, global_long);

  VERIFY(2, sizeof(global_short));
  VERIFY(3, global_short);

  VERIFY(48, sizeof(global_long_array));
  VERIFY(1, global_long_array[0][0][0]);
  VERIFY(2, global_long_array[0][0][1]);
  VERIFY(3, global_long_array[0][0][2]);
  VERIFY(4, global_long_array[0][1][0]);
  VERIFY(5, global_long_array[0][1][1]);
  VERIFY(0, global_long_array[0][1][2]);

  VERIFY(12, sizeof(global_short_array));
  VERIFY(1, global_short_array[0][0][0]);
  VERIFY(2, global_short_array[0][0][1]);
  VERIFY(3, global_short_array[0][0][2]);
  VERIFY(4, global_short_array[0][1][0]);
  VERIFY(5, global_short_array[0][1][1]);
  VERIFY(0, global_short_array[0][1][2]);

  {
    short a = 3;
    long b = 5 * 6 - 8;
    VERIFY(14, a + b / 2);
  }
  { // block scope
    int a = 7;
    {
      int a = 3;
      int b = 5 * 6 - 8;
      VERIFY(14, a + b / 2);
    }
    VERIFY(7, a);
  }
  {
    int aaa = 3;
    int bb = 5 * 6 - 8;
    VERIFY(14, aaa + bb / 2);
    VERIFY(5, aaa + 2);
  }
  {
    int x;
    VERIFY(4, sizeof(x));
  }
  {
    int *x;
    VERIFY(8, sizeof(x));
  }
  {
    int *x;
    VERIFY(8, sizeof(x + 3));
  }
  {
    int *x;
    VERIFY(4, sizeof(*x));
  }
  VERIFY(8, sizeof(sizeof(1)));
  VERIFY(4, sizeof(1));

  global_int1 = 4;
  global_int2 = 3;
  VERIFY(4, global_int1);
  VERIFY(12, global_int1 * global_int2);
  VERIFY(4, sizeof(global_int1));
  VERIFY(12, sizeof(global_array3));
  global_array20[5] = 12;
  global_array20[11] = 14;
  VERIFY(12, global_array20[5]);
  VERIFY(14, global_array20[11]);

  global_int1 = 12;
  global_int_pointer = &global_int1;
  VERIFY(12, *global_int_pointer);

  {
    int y;
    global_int1 = 12;
    y = global_int1;
    VERIFY(12, y);
  }
  {
    char x = 20;
    VERIFY(20, x);
  }
  {
    char x[3];
    x[0] = -1;
    x[1] = 2;
    int y = 4;
    VERIFY(3, x[0] + y);
    VERIFY(6, x[1] + y);
    VERIFY(1, x[0] + x[1]);
    char *p = &x[0];
    VERIFY(2, *(p + 1));
  }
  {
    int a = 3 * 2 + 1;
    VERIFY(7, a);
  }
  {
    int x[3] = {1, 2};
    VERIFY(1, x[0]);
    VERIFY(2, x[1]);
    VERIFY(0, x[2]);
  }
  {
    char x[3] = {1, 2};
    VERIFY(1, x[0]);
    VERIFY(2, x[1]);
    VERIFY(0, x[2]);
  }
  {
    int x = {1};
    VERIFY(1, x);
  }
  {
    int a = 3;
    int b = a * a + 4 / 2;
    int x[3] = {1, b - a};
    VERIFY(3, a);
    VERIFY(11, b);
    VERIFY(1, x[0]);
    VERIFY(8, x[1]);
    VERIFY(0, x[2]);
  }
  {
    int x[3] = {1, x[0]};
    VERIFY(1, x[0]);
    VERIFY(1, x[1]);
    VERIFY(0, x[2]);
  }
  {
    int x[3] = {1, 2, 3}, *p[4], a = 2, *t;
    int **d, b;
    VERIFY(12, sizeof(x));
    VERIFY(32, sizeof(p));
    VERIFY(4, sizeof(a));
    VERIFY(8, sizeof(t));
    VERIFY(8, sizeof(d));
    VERIFY(4, sizeof(b));
    VERIFY(1, x[0]);
    VERIFY(2, x[1]);
    VERIFY(3, x[2]);
    VERIFY(2, a);
  }
  {
    int x[] = {1, x[0], 2};
    VERIFY(1, x[0]);
    VERIFY(1, x[1]);
    VERIFY(2, x[2]);
    VERIFY(12, sizeof(x));
  }

  {
    int x[2][2] = {{1, 2}, {(27 / 3 == 9) * 3}};
    VERIFY(1, x[0][0]);
    VERIFY(2, x[0][1]);
    VERIFY(3, x[1][0]);
    VERIFY(0, x[1][1]);
    VERIFY(16, sizeof(x));
  }
  {
    int x[2][2] = {(3 > 1) + (2 <= 2) - (5 != 4) - (3 >= 2)};
    VERIFY(0, x[0][0]);
    VERIFY(0, x[0][1]);
    VERIFY(0, x[0][1]);
    VERIFY(0, x[1][1]);
    VERIFY(16, sizeof(x));
  }
  {
    int x[3][2][3] = {{{1, 2}, {4, 5, 6}}};
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(0, x[0][0][2]);
    VERIFY(4, x[0][1][0]);
    VERIFY(5, x[0][1][1]);
    VERIFY(6, x[0][1][2]);
    VERIFY(0, x[1][0][0]);
    VERIFY(0, x[1][0][1]);
    VERIFY(0, x[1][0][2]);
    VERIFY(0, x[1][1][0]);
    VERIFY(0, x[1][1][1]);
    VERIFY(0, x[1][1][2]);
    VERIFY(72, sizeof(x));
    VERIFY(24, sizeof(x[0]));
    VERIFY(12, sizeof(x[0][0]));
  }
  {
    char x[2][2][3] = {{1, 2}, {4, 5}};
    VERIFY(12, sizeof(x));
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(0, x[0][0][2]);
    VERIFY(0, x[0][1][0]);
    VERIFY(0, x[0][1][1]);
    VERIFY(0, x[0][1][2]);
    VERIFY(4, x[1][0][0]);
    VERIFY(5, x[1][0][1]);
    VERIFY(0, x[1][0][2]);
    VERIFY(0, x[1][1][0]);
    VERIFY(0, x[1][1][1]);
    VERIFY(0, x[1][1][2]);
  }
  {
    int x[][2][3] = {{1, 2}, {4, 5, 6, 7, 8, 9}};
    VERIFY(48, sizeof(x));
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(0, x[0][0][2]);
    VERIFY(0, x[0][1][0]);
    VERIFY(0, x[0][1][1]);
    VERIFY(0, x[0][1][2]);
    VERIFY(4, x[1][0][0]);
    VERIFY(5, x[1][0][1]);
    VERIFY(6, x[1][0][2]);
    VERIFY(7, x[1][1][0]);
    VERIFY(8, x[1][1][1]);
    VERIFY(9, x[1][1][2]);
  }
  {
    char x[1][2][3] = {1, 2, 3, 4, 5};
    VERIFY(6, sizeof(x));
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(3, x[0][0][2]);
    VERIFY(4, x[0][1][0]);
    VERIFY(5, x[0][1][1]);
    VERIFY(0, x[0][1][2]);
  }
  {
    long x[1][2][3] = {1, 2, 3, 4, 5};
    VERIFY(48, sizeof(x));
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(3, x[0][0][2]);
    VERIFY(4, x[0][1][0]);
    VERIFY(5, x[0][1][1]);
    VERIFY(0, x[0][1][2]);
  }
  {
    short x[1][2][3] = {1, 2, 3, 4, 5};
    VERIFY(12, sizeof(x));
    VERIFY(1, x[0][0][0]);
    VERIFY(2, x[0][0][1]);
    VERIFY(3, x[0][0][2]);
    VERIFY(4, x[0][1][0]);
    VERIFY(5, x[0][1][1]);
    VERIFY(0, x[0][1][2]);
  }
  {
    extern char external_char;
    {
      extern int external_int;
      VERIFY(56, external_char);
      VERIFY(1234, external_int);
      external_char = 78;
    }
    VERIFY(78, external_char);
    VERIFY(789, external_short);
    external_short = -455;
    VERIFY(-455, external_short);
  }
  {
    const int x = 12;
    VERIFY(12, x);
  }
  {
    int arr[12 + sizeof(int) * 2];
    VERIFY(80, sizeof(arr));
  }
  return 0;
}
