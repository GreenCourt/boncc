#include "common.h"

int main() {
  VERIFY(0, 0);
  VERIFY(42, 42);
  VERIFY(21, 5 + 20 - 4);
  VERIFY(41, 12 + 34 - 5);
  VERIFY(47, 5 + 6 * 7);
  VERIFY(15, 5 * (9 - 6));
  VERIFY(4, (3 + 5) / 2);
  VERIFY(24, -2 * (9 - 6) + 30);
  VERIFY(24, -2 * (+9 - 6) + 30);
  VERIFY(36, +2 * (+9 - 6) + 30);
  VERIFY(12, +2 * +6);
  VERIFY(10, -(-10));
  VERIFY(10, -(-+10));
  VERIFY(1, -3 < 2);
  VERIFY(0, 2 < 1);
  VERIFY(0, -3 > 2);
  VERIFY(1, 2 > 1);
  VERIFY(1, -3 == -3);
  VERIFY(0, -3 != -3);
  VERIFY(1, -3 <= 2);
  VERIFY(1, 2 <= 2);
  VERIFY(0, 2 <= 1);
  VERIFY(0, -3 >= 2);
  VERIFY(1, 2 >= 2);
  VERIFY(1, 2 >= 1);
  VERIFY(1, 12 ? 1 : 2);
  VERIFY(1, 8 - 4 ? 1 : 2);
  VERIFY(2, 8 - 8 ? 1 : 2);
  VERIFY(0, 12 % 4);
  VERIFY(0, 9 % 3);
  VERIFY(3, 7 % 4);
  VERIFY(-4, -9 % 5);
  VERIFY(3, 7 % -4);
  {
    int x = 0;
    int y;
    int *p = &x;
    y = (*p)++;
    VERIFY(1, x);
    VERIFY(0, y);
    y = ++(*p);
    VERIFY(2, x);
    VERIFY(2, y);
    y = (*p) += 2;
    VERIFY(4, x);
    VERIFY(4, x);
    y = (*p) -= 5;
    VERIFY(-1, x);
    VERIFY(-1, y);
    y = (*p)--;
    VERIFY(-2, x);
    VERIFY(-1, y);
    y = --(*p);
    VERIFY(-3, x);
    VERIFY(-3, y);
    y = (*p) *= -8;
    VERIFY(24, x);
    VERIFY(24, y);
    y = (*p) /= 4;
    VERIFY(6, x);
    VERIFY(6, y);
    y = (*p) %= 4;
    VERIFY(2, x);
    VERIFY(2, y);
  }
  {
    int a[3];
    int *p = &a;
    VERIFY(1, !0);
    VERIFY(0, !1);
    VERIFY(0, !57);
    VERIFY(0, !p);
    VERIFY(0, !a);

    VERIFY(1, 3 && 6);
    VERIFY(0, 0 && 9);
    VERIFY(0, 5 && 0);
    VERIFY(0, 0 && 0);
    VERIFY(1, a && p);

    VERIFY(1, 4 || 9);
    VERIFY(1, 0 || 7);
    VERIFY(1, 2 || 0);
    VERIFY(0, 0 || 0);
    VERIFY(1, a || 0);
    VERIFY(1, 0 || p);
  }
  {
    VERIFY(2, 4 >> 1);
    VERIFY(1, 4 >> 2);
    VERIFY(0, 4 >> 3);
    VERIFY(2, 5 >> 1);
    VERIFY(10, 5 << 1);
    VERIFY(20, 5 << 2);
    VERIFY(-456790, ~456789);
    VERIFY(10000, ~~10000);
    VERIFY(465429, 123456 ^ 456789);
    VERIFY(522837, 123456 | 456789);
    VERIFY(57408, 123456 & 456789);
    int x = 123456;
    int y;
    y = (x <<= 4);
    VERIFY(1975296, x);
    VERIFY(1975296, y);
    x = 123456;
    y = (x >>= 4);
    VERIFY(7716, x);
    VERIFY(7716, y);
    x = 123456;
    y = (x ^= 671389);
    VERIFY(777437, x);
    VERIFY(777437, y);
    x = 123456;
    y = (x &= 671389);
    VERIFY(8704, x);
    VERIFY(8704, y);
    x = 123456;
    y = (x |= 671389);
    VERIFY(786141, x);
    VERIFY(786141, y);
  }
  {
    VERIFY(-1661445099, (int)11223456789);
    VERIFY(513, (short)8590066177);
    VERIFY(1, (char)8590066177);
    VERIFY(1, (long)1);
    VERIFY(0, (long)&*(int *)0);
    {
      int x = 512;
      *(char *)&x = 1;
      VERIFY(513, x);
    }
    {
      int x = 5;
      long y = (long)&x;
      VERIFY(5, *(int *)y);
    }

    VERIFY(-1, (char)255);
    VERIFY(-1, (signed char)255);
    VERIFY(255, (unsigned char)255);
    VERIFY(-1, (short)65535);
    VERIFY(65535, (unsigned short)65535);
    VERIFY(1, -1 < 1);
    VERIFY(0, -1 < (unsigned)1);
    VERIFY(254, (char)127 + (char)127);
    VERIFY(65534, (short)32767 + (short)32767);
    VERIFY(-1, -1 >> 1);
    VERIFY(-1, (unsigned long)-1);
    VERIFY(2147483647, ((unsigned)-1) >> 1);
    VERIFY(-50, (-100) / 2);
    VERIFY(2147483598, ((unsigned)-100) / 2);
    VERIFY(9223372036854775758, ((unsigned long)-100) / 2);
    VERIFY(0, ((long)-1) / (unsigned)100);
    VERIFY(-2, (-100) % 7);
    VERIFY(2, ((unsigned)-100) % 7);
    VERIFY(6, ((unsigned long)-100) % 9);

    VERIFY(65535, (int)(unsigned short)65535);
    {
      unsigned short x = 65535;
      VERIFY(65535, x);
      VERIFY(65535, (int)x);
    }
    {
      typedef short T;
      T x = 65535;
      VERIFY(-1, (int)x);
    }
    {
      typedef unsigned short T;
      T x = 65535;
      VERIFY(65535, (int)x);
    }
  }
  {
    VERIFY((long)-5, -10 + (long)5);
    VERIFY((long)-15, -10 - (long)5);
    VERIFY((long)-50, -10 * (long)5);
    VERIFY((long)-2, -10 / (long)5);

    VERIFY(1, -2 < (long)-1);
    VERIFY(1, -2 <= (long)-1);
    VERIFY(0, -2 > (long)-1);
    VERIFY(0, -2 >= (long)-1);

    VERIFY(1, (long)-2 < -1);
    VERIFY(1, (long)-2 <= -1);
    VERIFY(0, (long)-2 > -1);
    VERIFY(0, (long)-2 >= -1);

    {
      long x = -1;
      VERIFY((long)-1, x);
    }
    {
      char x[3];
      x[0] = 0;
      x[1] = 1;
      x[2] = 2;
      char *y = x + 1;
      VERIFY(1, y[0]);
    }
    {
      char x[3];
      x[0] = 0;
      x[1] = 1;
      x[2] = 2;
      char *y = x + 1;
      VERIFY(0, y[-1]);
    }
    {
      int x = 5, y = 3;
      VERIFY(9, (7, 2, 3, 9));
      VERIFY(1, (x = 2, y = 1));
      VERIFY(2, x);
      VERIFY(1, y);
      VERIFY(4, (12, y = 8, x = 4));
      VERIFY(4, x);
      VERIFY(8, y);
      x = -1, y = -2;
      VERIFY(-1, x);
      VERIFY(-2, y);
    }
  }
  return 0;
}
