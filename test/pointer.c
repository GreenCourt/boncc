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
  VERIFY(&a, p);
  VERIFY(&a + 1, p_addr_num);
  VERIFY(&a + 3, p_num_addr);
  VERIFY(1, p_num);
  VERIFY(12, *p);
  VERIFY(12, *(p_addr_num - 1));

  VERIFY(32, sizeof(p_array));
  VERIFY(&a + 1, p_array[0][0]);
  VERIFY(&a + 2, p_array[0][1]);
  VERIFY(&a + 3, p_array[1][0]);
  VERIFY(&a + 4, p_array[1][1]);
  VERIFY(12, *(p_array[0][0] - 1));
  VERIFY(12, *(p_array[0][1] - 2));
  VERIFY(12, *(p_array[1][0] - 3));
  VERIFY(12, *(p_array[1][1] - 4));

  VERIFY(32, sizeof(pp_array));
  VERIFY(&p + 1, pp_array[0][0]);
  VERIFY(&p + 2, pp_array[0][1]);
  VERIFY(&p + 3, pp_array[1][0]);
  VERIFY(&p + 4, pp_array[1][1]);
  VERIFY(12, **(pp_array[0][0] - 1));
  VERIFY(12, **(pp_array[0][1] - 2));
  VERIFY(12, **(pp_array[1][0] - 3));
  VERIFY(12, **(pp_array[1][1] - 4));

  {
    int x = 3;
    int *y = &x;
    int **z = &y;
    VERIFY(3, *y);
    VERIFY(3, **z);
    *y = 4;
    VERIFY(4, x);
    **z = 2;
    VERIFY(2, x);
  }
  {
    int x = 12;
    int *p = &x;
    int **q = &p;
    VERIFY(12, *&*&**q);
  }
  {
    int a[2];
    *a = 1;
    *(a + 1) = 2;
    int *p = a;
    VERIFY(3, *p + *(p + 1));
    VERIFY(1, &a[1] - &a[0]);
  }
  {
    char a[2];
    VERIFY(1, &a[1] - &a[0]);
  }
  {
    int a[2];
    int *x = a;
    int *y = &a[1];
    VERIFY(1, y - x);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    int *p = a;
    VERIFY(3, p[0] + p[1]);
  }
  {
    int a[2];
    0 [a] = 1;
    (2 - 1)[a] = 2;
    int *p = a;
    VERIFY(3, p[0] + p[1]);
  }
  {
    int a[2];
    zero()[a] = 1;
    one()[a] = 2;
    int *p = a;
    VERIFY(3, p[zero()] + p[one()]);
  }
  {
    int x[2];
    int *y = &x;
    *y = 3;
    VERIFY(3, *x);
  }
  {
    int x[3];
    *x = 3;
    *(x + 1) = 4;
    *(x + 2) = 5;
    VERIFY(3, *x);
    VERIFY(4, *(x + 1));
    VERIFY(5, *(x + 2));
    VERIFY(3, x[0]);
    VERIFY(4, x[1]);
    VERIFY(5, x[2]);
  }
  {
    int x[3];
    *x = 3;
    x[1] = 4;
    x[2] = 5;
    VERIFY(3, x[0]);
    VERIFY(4, x[1]);
    VERIFY(5, x[2]);
    VERIFY(3, *(x));
    VERIFY(4, *(x + 1));
    VERIFY(5, *(x + 2));
    2 [x] = 6;
    VERIFY(6, *(x + 2));
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    f(a);
    VERIFY(1, a[0]);
    VERIFY(12, a[1]);
  }
  {
    int a[2];
    a[0] = 1;
    a[1] = 2;
    g(a);
    VERIFY(12, a[0]);
    VERIFY(2, a[1]);
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
    VERIFY(0, **x);
    VERIFY(0, x[0][0]);
    VERIFY(1, x[0][1]);
    VERIFY(2, x[0][2]);
    VERIFY(3, x[1][0]);
    VERIFY(4, x[1][1]);
    VERIFY(5, x[1][2]);
  }
  {
    int x[2][2][2];
    int *y = x;
    y[5] = 5;
    VERIFY(5, x[1][0][1]);
  }
  {
    char a[2];
    *a = 1;
    *(a + 1) = 2;
    char *p = a;
    VERIFY(3, *p + *(p + 1));
  }
  {
    int a[2];
    int *p = a;
    a[0] = 1;
    a[1] = 2;
    VERIFY(1, *p++);
    VERIFY(2, *p--);
    VERIFY(2, *++p);
    VERIFY(1, *--p);
    VERIFY(1, *p);
  }
  return 0;
}
