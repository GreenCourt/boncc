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

int main() {
  // verify(3, ({
  //         int x;
  //         x = 3;
  //         int *y;
  //         y = &x;
  //         *y;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x;
  //         x = 3;
  //         int y;
  //         y = 5;
  //         int *z;
  //         z = &y + 1;
  //         *z;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x;
  //         x = 3;
  //         int *y;
  //         y = &x;
  //         int **z;
  //         z = &y;
  //         **z;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x;
  //         int *y;
  //         y = &x;
  //         *y = 3;
  //         x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x;
  //         int *y;
  //         y = &x;
  //         int **z;
  //         z = &y;
  //         **z = 3;
  //         x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(12, ({
  //         int x;
  //         x = 12;
  //         int *p;
  //         p = &x;
  //         int **q;
  //         q = &p;
  //         *&*&**q;
  //       }),
  //       __FILE__, __LINE__);
  // verify(2, ({
  //         int *p;
  //         alloc4(&p, 1, 2, 4, 8);
  //         int *q;
  //         q = p + 1;
  //         *q;
  //       }),
  //       __FILE__, __LINE__);
  // verify(8, ({
  //         int *p;
  //         alloc4(&p, 1, 2, 4, 8);
  //         int *q;
  //         q = p + 3;
  //         *q;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int a[2];
  //         *a = 1;
  //         *(a + 1) = 2;
  //         int *p;
  //         p = a;
  //         *p + *(p + 1);
  //       }),
  //       __FILE__, __LINE__);
  // verify(1, ({
  //         int a[2];
  //         &a[1] - &a[0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(1, ({
  //         char a[2];
  //         &a[1] - &a[0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(1, ({
  //         int a[2];
  //         int *x;
  //         int *y;
  //         x = a;
  //         y = &a[1];
  //         y - x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int a[2];
  //         a[0] = 1;
  //         a[1] = 2;
  //         int *p;
  //         p = a;
  //         p[0] + p[1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int a[2];
  //         0 [a] = 1;
  //         (2 - 1)[a] = 2;
  //         int *p;
  //         p = a;
  //         p[0] + p[1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int a[2];
  //         zero()[a] = 1;
  //         one()[a] = 2;
  //         int *p;
  //         p = a;
  //         p[zero(0)] + p[one()];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x[2];
  //         int *y;
  //         y = &x;
  //         *y = 3;
  //         *x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x[3];
  //         *x = 3;
  //         *(x + 1) = 4;
  //         *(x + 2) = 5;
  //         *x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int x[3];
  //         *x = 3;
  //         *(x + 1) = 4;
  //         *(x + 2) = 5;
  //         *(x + 1);
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[3];
  //         *x = 3;
  //         *(x + 1) = 4;
  //         *(x + 2) = 5;
  //         *(x + 2);
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         x[0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         x[1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         *x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         *(x + 1);
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         *(x + 2);
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         x[2] = 5;
  //         *(x + 2);
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[3];
  //         *x = 3;
  //         x[1] = 4;
  //         2 [x] = 5;
  //         *(x + 2);
  //       }),
  //       __FILE__, __LINE__);
  // verify(12, ({
  //         int a[2];
  //         a[0] = 1;
  //         a[1] = 2;
  //         f(a);
  //         a[1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(12, ({
  //         int a[2];
  //         a[0] = 1;
  //         a[1] = 2;
  //         g(a);
  //         a[0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(0, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[0] = 0;
  //         **x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(0, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[0] = 0;
  //         x[0][0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(1, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[1] = 1;
  //         x[0][1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(2, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[2] = 2;
  //         x[0][2];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[3] = 3;
  //         x[1][0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[4] = 4;
  //         x[1][1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[2][3];
  //         int *y;
  //         y = x;
  //         y[5] = 5;
  //         x[1][2];
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int x[2][2][2];
  //         int *y;
  //         y = x;
  //         y[5] = 5;
  //         x[1][0][1];
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         char a[2];
  //         *a = 1;
  //         *(a + 1) = 2;
  //         char *p;
  //         p = a;
  //         *p + *(p + 1);
  //       }),
  //       __FILE__, __LINE__);
  return 0;
}
