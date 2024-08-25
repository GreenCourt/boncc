#include "common.h"

int main() {
  verify(0, 0);
  verify(42, 42);
  verify(21, 5 + 20 - 4);
  verify(41, 12 + 34 - 5);
  verify(47, 5 + 6 * 7);
  verify(15, 5 * (9 - 6));
  verify(4, (3 + 5) / 2);
  verify(24, -2 * (9 - 6) + 30);
  verify(24, -2 * (+9 - 6) + 30);
  verify(36, +2 * (+9 - 6) + 30);
  verify(12, +2 * +6);
  verify(10, -(-10));
  verify(10, -(-+10));
  verify(1, -3 < 2);
  verify(0, 2 < 1);
  verify(0, -3 > 2);
  verify(1, 2 > 1);
  verify(1, -3 == -3);
  verify(0, -3 != -3);
  verify(1, -3 <= 2);
  verify(1, 2 <= 2);
  verify(0, 2 <= 1);
  verify(0, -3 >= 2);
  verify(1, 2 >= 2);
  verify(1, 2 >= 1);
  verify(1, 12 ? 1 : 2);
  verify(1, 8 - 4 ? 1 : 2);
  verify(2, 8 - 8 ? 1 : 2);
  verify(0, 12 % 4);
  verify(0, 9 % 3);
  verify(3, 7 % 4);
  verify(-4, -9 % 5);
  verify(3, 7 % -4);
  {
    int x = 0;
    int y;
    int *p = &x;
    y = (*p)++;
    verify(1, x);
    verify(0, y);
    y = ++(*p);
    verify(2, x);
    verify(2, y);
    y = (*p) += 2;
    verify(4, x);
    verify(4, x);
    y = (*p) -= 5;
    verify(-1, x);
    verify(-1, y);
    y = (*p)--;
    verify(-2, x);
    verify(-1, y);
    y = --(*p);
    verify(-3, x);
    verify(-3, y);
    y = (*p) *= -8;
    verify(24, x);
    verify(24, y);
    y = (*p) /= 4;
    verify(6, x);
    verify(6, y);
    y = (*p) %= 4;
    verify(2, x);
    verify(2, y);
  }
  {
    int a[3];
    int *p = a;
    verify(1, !0);
    verify(0, !1);
    verify(0, !57);
    verify(0, !p);
    verify(0, !a);

    verify(1, 3 && 6);
    verify(0, 0 && 9);
    verify(0, 5 && 0);
    verify(0, 0 && 0);
    verify(1, a && p);

    verify(1, 4 || 9);
    verify(1, 0 || 7);
    verify(1, 2 || 0);
    verify(0, 0 || 0);
    verify(1, a || 0);
    verify(1, 0 || p);
  }
  {
    verify(2, 4 >> 1);
    verify(1, 4 >> 2);
    verify(0, 4 >> 3);
    verify(2, 5 >> 1);
    verify(10, 5 << 1);
    verify(20, 5 << 2);
    verify(-456790, ~456789);
    verify(10000, ~~10000);
    verify(465429, 123456 ^ 456789);
    verify(522837, 123456 | 456789);
    verify(57408, 123456 & 456789);
    int x = 123456;
    int y;
    y = (x <<= 4);
    verify(1975296, x);
    verify(1975296, y);
    x = 123456;
    y = (x >>= 4);
    verify(7716, x);
    verify(7716, y);
    x = 123456;
    y = (x ^= 671389);
    verify(777437, x);
    verify(777437, y);
    x = 123456;
    y = (x &= 671389);
    verify(8704, x);
    verify(8704, y);
    x = 123456;
    y = (x |= 671389);
    verify(786141, x);
    verify(786141, y);
  }
  {
    verify(-1661445099, (int)11223456789);
    verify(513, (short)8590066177);
    verify(1, (char)8590066177);
    verify(1, (long)1);
    verify(0, (long)&*(int *)0);
    {
      int x = 512;
      *(char *)&x = 1;
      verify(513, x);
    }
    {
      int x = 5;
      long y = (long)&x;
      verify(5, *(int *)y);
    }

    verify(-1, (char)255);
    verify(-1, (signed char)255);
    verify(255, (unsigned char)255);
    verify(-1, (short)65535);
    verify(65535, (unsigned short)65535);
    verify(1, -1 < 1);
    verify(0, -1 < (unsigned)1);
    verify(254, (char)127 + (char)127);
    verify(65534, (short)32767 + (short)32767);
    verify(-1, -1 >> 1);
    verify(-1, (unsigned long)-1);
    verify(2147483647, ((unsigned)-1) >> 1);
    verify(-50, (-100) / 2);
    verify(2147483598, ((unsigned)-100) / 2);
    verify(9223372036854775758, ((unsigned long)-100) / 2);
    verify(0, ((long)-1) / (unsigned)100);
    verify(-2, (-100) % 7);
    verify(2, ((unsigned)-100) % 7);
    verify(6, ((unsigned long)-100) % 9);

    verify(65535, (int)(unsigned short)65535);
    {
      unsigned short x = 65535;
      verify(65535, x);
      verify(65535, (int)x);
    }
    {
      typedef short T;
      T x = 65535;
      verify(-1, (int)x);
    }
    {
      typedef unsigned short T;
      T x = 65535;
      verify(65535, (int)x);
    }
  }
  {
    verify((long)-5, -10 + (long)5);
    verify((long)-15, -10 - (long)5);
    verify((long)-50, -10 * (long)5);
    verify((long)-2, -10 / (long)5);

    verify(1, -2 < (long)-1);
    verify(1, -2 <= (long)-1);
    verify(0, -2 > (long)-1);
    verify(0, -2 >= (long)-1);

    verify(1, (long)-2 < -1);
    verify(1, (long)-2 <= -1);
    verify(0, (long)-2 > -1);
    verify(0, (long)-2 >= -1);

    {
      long x = -1;
      verify((long)-1, x);
    }
    {
      char x[3];
      x[0] = 0;
      x[1] = 1;
      x[2] = 2;
      char *y = x + 1;
      verify(1, y[0]);
    }
    {
      char x[3];
      x[0] = 0;
      x[1] = 1;
      x[2] = 2;
      char *y = x + 1;
      verify(0, y[-1]);
    }
    {
      int x = 5, y = 3;
      verify(9, (7, 2, 3, 9));
      verify(1, (x = 2, y = 1));
      verify(2, x);
      verify(1, y);
      verify(4, (12, y = 8, x = 4));
      verify(4, x);
      verify(8, y);
      x = -1, y = -2;
      verify(-1, x);
      verify(-2, y);
    }
  }
  {
    int x = 7, y = 2;
    verify(14, x * y);
    verify(9, x + y);
    verify(5, x - y);
    verify(3, x / y);
    verify(1, x % y);
    verify(28, x << y);
    verify(1, x >> y);
    verify(-8, ~x);
    verify(2, x & y);
    verify(7, x | y);
    verify(15, x | 8);
    verify(5, x ^ y);
    verify(0, !x);
    verify(1, x && y);
    verify(0, !x && y);
    verify(1, x || y);
    verify(1, !x || y);
    verify(0, !x || !y);
  }
  {
    short x = 7, y = 2;
    verify(14, x * y);
    verify(9, x + y);
    verify(5, x - y);
    verify(3, x / y);
    verify(1, x % y);
    verify(28, x << y);
    verify(1, x >> y);
    verify(-8, ~x);
    verify(2, x & y);
    verify(7, x | y);
    verify(15, x | 8);
    verify(5, x ^ y);
    verify(0, !x);
    verify(1, x && y);
    verify(0, !x && y);
    verify(1, x || y);
    verify(1, !x || y);
    verify(0, !x || !y);
  }

  // void for conditional operator(used in macro)
  1 ? (void)0 : (void)2;
  1 ? (int)0 : (void)2;
  1 ? (void)0 : (int)2;

  // statement expressions
  verify(14, ({
           int x = 4;
           x + 10;
         }));
  verify(1, 14.45 == ({
              double x = 14.45, y = 12.12;
              x;
            }));
  verify(1, 12.12 == ({
              double x = 14.45, y = 12.12;
              3 < 5 ? y : x;
            }));
  verify(1, 14.45 == ({
              double x = 14.45, y = 12.12;
              3 < -5 ? y : x;
            }));
  ({});
  return 0;
}
