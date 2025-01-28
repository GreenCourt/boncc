#include "common.h"

int one() { return 1; }
int zero() { return 0; }
int f(int a[]) {
  a[1] = 12;
  return 0;
}
int g(int a[1]) {
  a[0] = 12;
  return 0;
}

int a = 12;
int *p = &a;
int *p_addr_num = &a + 1;
int *p_num_addr = 3 + &a;
int *p_num = 1;
int *p_array[][2] = {{&a + 1, &a + 2}, {&a + 3, &a + 4}};
int **pp_array[][2] = {{&p + 1, &p + 2}, {&p + 3, &p + 4}};
long long global_pointer_sub = &a - &a;

char arr[2] = {11, 22};
char *p_arr = arr; // initialize a global pointer to a global array

int main() {
  verify(&a, p);
  verify(&a + 1, p_addr_num);
  verify(&a + 3, p_num_addr);
  verify(1, p_num);
  verify(12, *p);
  verify(12, *(p_addr_num - 1));

  verify(32, sizeof(p_array));
  verify(&a + 1, p_array[0][0]);
  verify(&a + 2, p_array[0][1]);
  verify(&a + 3, p_array[1][0]);
  verify(&a + 4, p_array[1][1]);
  verify(12, *(p_array[0][0] - 1));
  verify(12, *(p_array[0][1] - 2));
  verify(12, *(p_array[1][0] - 3));
  verify(12, *(p_array[1][1] - 4));

  verify(32, sizeof(pp_array));
  verify(&p + 1, pp_array[0][0]);
  verify(&p + 2, pp_array[0][1]);
  verify(&p + 3, pp_array[1][0]);
  verify(&p + 4, pp_array[1][1]);
  verify(12, **(pp_array[0][0] - 1));
  verify(12, **(pp_array[0][1] - 2));
  verify(12, **(pp_array[1][0] - 3));
  verify(12, **(pp_array[1][1] - 4));

  verify(0, global_pointer_sub);
  verify(11, arr[0]);
  verify(22, arr[1]);
  verify(11, p_arr[0]);
  verify(22, p_arr[1]);

  {
    int x = 3;
    int *y = &x;
    int **z = &y;
    verify(3, *y);
    verify(3, **z);
    *y = 4;
    verify(4, x);
    **z = 2;
    verify(2, x);
  }
  {
    int x = 12;
    int *p = &x;
    int **q = &p;
    verify(12, *&*&**q);
  }
  {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p = a;
    verify(3, *p + *(p + 1));
    verify(1, &a[1] - &a[0]);
  }
  {
    char a[2];
    verify(1, &a[1] - &a[0]);
  }
  {
    int a[2];
    int *x = a;
    int *y = &a[1];
    verify(1, y - x);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    int *p = a;
    verify(3, p[0] + p[1]);
  }
  {
    int a[2];
    0 [a] = 1;
    (2 - 1)[a] = 2;
    int *p = a;
    verify(3, p[0] + p[1]);
  }
  {
    int a[2];
    zero()[a] = 1;
    one()[a] = 2;
    int *p = a;
    verify(3, p[zero()] + p[one()]);
  }
  {
    int x[2];
    int *y = &x;
    *y = 3;
    verify(3, *x);
  }
  {
    int x[3];
    *x = 3;
    *(x + 1) = 4;
    *(x + 2) = 5;
    verify(3, *x);
    verify(4, *(x + 1));
    verify(5, *(x + 2));
    verify(3, x[0]);
    verify(4, x[1]);
    verify(5, x[2]);
  }
  {
    int x[3];
    *x = 3;
    x[1] = 4;
    x[2] = 5;
    verify(3, x[0]);
    verify(4, x[1]);
    verify(5, x[2]);
    verify(3, *(x));
    verify(4, *(x + 1));
    verify(5, *(x + 2));
    2 [x] = 6;
    verify(6, *(x + 2));
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    f(a);
    verify(1, a[0]);
    verify(12, a[1]);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    g(a);
    verify(12, a[0]);
    verify(2, a[1]);
  }
  {
    int x[2][3];
    int *y = x;
    y[0] = 0;
    y[1] = 1;
    y[2] = 2;
    y[3] = 3;
    y[4] = 4;
    y[5] = 5;
    verify(0, **x);
    verify(0, x[0][0]);
    verify(1, x[0][1]);
    verify(2, x[0][2]);
    verify(3, x[1][0]);
    verify(4, x[1][1]);
    verify(5, x[1][2]);
  }
  {
    int x[2][2][2];
    int *y = x;
    y[5] = 5;
    verify(5, x[1][0][1]);
  }
  {
    char a[2];
    *a = 1;
    *(a + 1) = 2;
    char *p = a;
    verify(3, *p + *(p + 1));
  }
  {
    int a[2];
    int *p = a;
    a[0] = 1;
    a[1] = 2;
    verify(1, *p++);
    verify(2, *p--);
    verify(2, *++p);
    verify(1, *--p);
    verify(1, *p);
  }
  return 0;
}
