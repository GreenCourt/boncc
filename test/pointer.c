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

int main() {
  verify(&a, p, __FILE__, __LINE__);
  verify(&a + 1, p_addr_num, __FILE__, __LINE__);
  verify(&a + 3, p_num_addr, __FILE__, __LINE__);
  verify(1, p_num, __FILE__, __LINE__);
  verify(12, *p, __FILE__, __LINE__);
  verify(12, *(p_addr_num - 1), __FILE__, __LINE__);

  verify(32, sizeof(p_array), __FILE__, __LINE__);
  verify(&a + 1, p_array[0][0], __FILE__, __LINE__);
  verify(&a + 2, p_array[0][1], __FILE__, __LINE__);
  verify(&a + 3, p_array[1][0], __FILE__, __LINE__);
  verify(&a + 4, p_array[1][1], __FILE__, __LINE__);
  verify(12, *(p_array[0][0] - 1), __FILE__, __LINE__);
  verify(12, *(p_array[0][1] - 2), __FILE__, __LINE__);
  verify(12, *(p_array[1][0] - 3), __FILE__, __LINE__);
  verify(12, *(p_array[1][1] - 4), __FILE__, __LINE__);

  verify(32, sizeof(pp_array), __FILE__, __LINE__);
  verify(&p + 1, pp_array[0][0], __FILE__, __LINE__);
  verify(&p + 2, pp_array[0][1], __FILE__, __LINE__);
  verify(&p + 3, pp_array[1][0], __FILE__, __LINE__);
  verify(&p + 4, pp_array[1][1], __FILE__, __LINE__);
  verify(12, **(pp_array[0][0] - 1), __FILE__, __LINE__);
  verify(12, **(pp_array[0][1] - 2), __FILE__, __LINE__);
  verify(12, **(pp_array[1][0] - 3), __FILE__, __LINE__);
  verify(12, **(pp_array[1][1] - 4), __FILE__, __LINE__);

  {
    int x = 3;
    int *y = &x;
    int **z = &y;
    verify(3, *y, __FILE__, __LINE__);
    verify(3, **z, __FILE__, __LINE__);
    *y = 4;
    verify(4, x, __FILE__, __LINE__);
    **z = 2;
    verify(2, x, __FILE__, __LINE__);
  }
  {
    int x = 12;
    int *p = &x;
    int **q = &p;
    verify(12, *&*&**q, __FILE__, __LINE__);
  }
  {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p = a;
    verify(3, *p + *(p + 1), __FILE__, __LINE__);
    verify(1, &a[1] - &a[0], __FILE__, __LINE__);
  }
  {
    char a[2];
    verify(1, &a[1] - &a[0], __FILE__, __LINE__);
  }
  {
    int a[2];
    int *x = a;
    int *y = &a[1];
    verify(1, y - x, __FILE__, __LINE__);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    int *p = a;
    verify(3, p[0] + p[1], __FILE__, __LINE__);
  }
  {
    int a[2];
    0 [a] = 1;
    (2 - 1)[a] = 2;
    int *p = a;
    verify(3, p[0] + p[1], __FILE__, __LINE__);
  }
  {
    int a[2];
    zero()[a] = 1;
    one()[a] = 2;
    int *p = a;
    verify(3, p[zero()] + p[one()], __FILE__, __LINE__);
  }
  {
    int x[2];
    int *y = &x;
    *y = 3;
    verify(3, *x, __FILE__, __LINE__);
  }
  {
    int x[3];
    *x = 3;
    *(x + 1) = 4;
    *(x + 2) = 5;
    verify(3, *x, __FILE__, __LINE__);
    verify(4, *(x + 1), __FILE__, __LINE__);
    verify(5, *(x + 2), __FILE__, __LINE__);
    verify(3, x[0], __FILE__, __LINE__);
    verify(4, x[1], __FILE__, __LINE__);
    verify(5, x[2], __FILE__, __LINE__);
  }
  {
    int x[3];
    *x = 3;
    x[1] = 4;
    x[2] = 5;
    verify(3, x[0], __FILE__, __LINE__);
    verify(4, x[1], __FILE__, __LINE__);
    verify(5, x[2], __FILE__, __LINE__);
    verify(3, *(x), __FILE__, __LINE__);
    verify(4, *(x + 1), __FILE__, __LINE__);
    verify(5, *(x + 2), __FILE__, __LINE__);
    2 [x] = 6;
    verify(6, *(x + 2), __FILE__, __LINE__);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    f(a);
    verify(1, a[0], __FILE__, __LINE__);
    verify(12, a[1], __FILE__, __LINE__);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    g(a);
    verify(12, a[0], __FILE__, __LINE__);
    verify(2, a[1], __FILE__, __LINE__);
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
    verify(0, **x, __FILE__, __LINE__);
    verify(0, x[0][0], __FILE__, __LINE__);
    verify(1, x[0][1], __FILE__, __LINE__);
    verify(2, x[0][2], __FILE__, __LINE__);
    verify(3, x[1][0], __FILE__, __LINE__);
    verify(4, x[1][1], __FILE__, __LINE__);
    verify(5, x[1][2], __FILE__, __LINE__);
  }
  {
    int x[2][2][2];
    int *y = x;
    y[5] = 5;
    verify(5, x[1][0][1], __FILE__, __LINE__);
  }
  {
    char a[2];
    *a = 1;
    *(a + 1) = 2;
    char *p = a;
    verify(3, *p + *(p + 1), __FILE__, __LINE__);
  }
  {
    int a[2];
    int *p = a;
    a[0] = 1;
    a[1] = 2;
    verify(1, *p++, __FILE__, __LINE__);
    verify(2, *p--, __FILE__, __LINE__);
    verify(2, *++p, __FILE__, __LINE__);
    verify(1, *--p, __FILE__, __LINE__);
    verify(1, *p, __FILE__, __LINE__);
  }
  return 0;
}
