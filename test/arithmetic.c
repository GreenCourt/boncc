void verify(int expected, int actual, char *file_name, int line_number);

int main() {
  verify(0, 0, __FILE__, __LINE__);
  verify(42, 42, __FILE__, __LINE__);
  verify(21, 5 + 20 - 4, __FILE__, __LINE__);
  verify(41, 12 + 34 - 5, __FILE__, __LINE__);
  verify(47, 5 + 6 * 7, __FILE__, __LINE__);
  verify(15, 5 * (9 - 6), __FILE__, __LINE__);
  verify(4, (3 + 5) / 2, __FILE__, __LINE__);
  verify(24, -2 * (9 - 6) + 30, __FILE__, __LINE__);
  verify(24, -2 * (+9 - 6) + 30, __FILE__, __LINE__);
  verify(36, +2 * (+9 - 6) + 30, __FILE__, __LINE__);
  verify(12, +2 * +6, __FILE__, __LINE__);
  verify(10, -(-10), __FILE__, __LINE__);
  verify(10, -(-+10), __FILE__, __LINE__);
  verify(1, -3 < 2, __FILE__, __LINE__);
  verify(0, 2 < 1, __FILE__, __LINE__);
  verify(0, -3 > 2, __FILE__, __LINE__);
  verify(1, 2 > 1, __FILE__, __LINE__);
  verify(1, -3 == -3, __FILE__, __LINE__);
  verify(0, -3 != -3, __FILE__, __LINE__);
  verify(1, -3 <= 2, __FILE__, __LINE__);
  verify(1, 2 <= 2, __FILE__, __LINE__);
  verify(0, 2 <= 1, __FILE__, __LINE__);
  verify(0, -3 >= 2, __FILE__, __LINE__);
  verify(1, 2 >= 2, __FILE__, __LINE__);
  verify(1, 2 >= 1, __FILE__, __LINE__);
  verify(1, 12 ? 1 : 2, __FILE__, __LINE__);
  verify(1, 8 - 4 ? 1 : 2, __FILE__, __LINE__);
  verify(2, 8 - 8 ? 1 : 2, __FILE__, __LINE__);
  verify(0, 12 % 4, __FILE__, __LINE__);
  verify(0, 9 % 3, __FILE__, __LINE__);
  verify(3, 7 % 4, __FILE__, __LINE__);
  verify(-4, -9 % 5, __FILE__, __LINE__);
  verify(3, 7 % -4, __FILE__, __LINE__);
  {
    int x = 0;
    int y;
    int *p = &x;
    y = (*p)++;
    verify(1, x, __FILE__, __LINE__);
    verify(0, y, __FILE__, __LINE__);
    y = ++(*p);
    verify(2, x, __FILE__, __LINE__);
    verify(2, y, __FILE__, __LINE__);
    y = (*p) += 2;
    verify(4, x, __FILE__, __LINE__);
    verify(4, x, __FILE__, __LINE__);
    y = (*p) -= 5;
    verify(-1, x, __FILE__, __LINE__);
    verify(-1, y, __FILE__, __LINE__);
    y = (*p)--;
    verify(-2, x, __FILE__, __LINE__);
    verify(-1, y, __FILE__, __LINE__);
    y = --(*p);
    verify(-3, x, __FILE__, __LINE__);
    verify(-3, y, __FILE__, __LINE__);
    y = (*p) *= -8;
    verify(24, x, __FILE__, __LINE__);
    verify(24, y, __FILE__, __LINE__);
    y = (*p) /= 4;
    verify(6, x, __FILE__, __LINE__);
    verify(6, y, __FILE__, __LINE__);
    y = (*p) %= 4;
    verify(2, x, __FILE__, __LINE__);
    verify(2, y, __FILE__, __LINE__);
  }
  {
    int a[3];
    int *p;
    verify(1, !0, __FILE__, __LINE__);
    verify(0, !1, __FILE__, __LINE__);
    verify(0, !57, __FILE__, __LINE__);
    verify(0, !p, __FILE__, __LINE__);
    verify(0, !a, __FILE__, __LINE__);

    verify(1, 3 && 6, __FILE__, __LINE__);
    verify(0, 0 && 9, __FILE__, __LINE__);
    verify(0, 5 && 0, __FILE__, __LINE__);
    verify(0, 0 && 0, __FILE__, __LINE__);
    verify(1, a && p, __FILE__, __LINE__);

    verify(1, 4 || 9, __FILE__, __LINE__);
    verify(1, 0 || 7, __FILE__, __LINE__);
    verify(1, 2 || 0, __FILE__, __LINE__);
    verify(0, 0 || 0, __FILE__, __LINE__);
    verify(1, a || 0, __FILE__, __LINE__);
    verify(1, 0 || p, __FILE__, __LINE__);
  }
  {
    verify(2, 4 >> 1, __FILE__, __LINE__);
    verify(1, 4 >> 2, __FILE__, __LINE__);
    verify(0, 4 >> 3, __FILE__, __LINE__);
    verify(2, 5 >> 1, __FILE__, __LINE__);
    verify(10, 5 << 1, __FILE__, __LINE__);
    verify(20, 5 << 2, __FILE__, __LINE__);
    verify(-456790, ~456789, __FILE__, __LINE__);
    verify(10000, ~~10000, __FILE__, __LINE__);
    verify(465429, 123456 ^ 456789, __FILE__, __LINE__);
    verify(522837, 123456 | 456789, __FILE__, __LINE__);
    verify(57408, 123456 & 456789, __FILE__, __LINE__);
    int x = 123456;
    int y;
    y = (x <<= 4);
    verify(1975296, x, __FILE__, __LINE__);
    verify(1975296, y, __FILE__, __LINE__);
    x = 123456;
    y = (x >>= 4);
    verify(7716, x, __FILE__, __LINE__);
    verify(7716, y, __FILE__, __LINE__);
    x = 123456;
    y = (x ^= 671389);
    verify(777437, x, __FILE__, __LINE__);
    verify(777437, y, __FILE__, __LINE__);
    x = 123456;
    y = (x &= 671389);
    verify(8704, x, __FILE__, __LINE__);
    verify(8704, y, __FILE__, __LINE__);
    x = 123456;
    y = (x |= 671389);
    verify(786141, x, __FILE__, __LINE__);
    verify(786141, y, __FILE__, __LINE__);
  }
  {
    verify(-1661445099, (int)11223456789, __FILE__, __LINE__);
    verify(513, (short)8590066177, __FILE__, __LINE__);
    verify(1, (char)8590066177, __FILE__, __LINE__);
    verify(1, (long)1, __FILE__, __LINE__);
    verify(0, (long)&*(int *)0, __FILE__, __LINE__);
    {
      int x = 512;
      *(char *)&x = 1;
      verify(513, x, __FILE__, __LINE__);
    }
    {
      int x = 5;
      long y = (long)&x;
      verify(5, *(int *)y, __FILE__, __LINE__);
    }
  }
  return 0;
}
