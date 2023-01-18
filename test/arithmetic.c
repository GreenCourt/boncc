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
  verify(10, --10, __FILE__, __LINE__);
  verify(10, --+10, __FILE__, __LINE__);
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
  {
    int x = 0;
    int y;
    int *p = &x;
    y = (*p) += 2;
    verify(2, x, __FILE__, __LINE__);
    verify(2, x, __FILE__, __LINE__);
    y = (*p) -= 5;
    verify(-3, x, __FILE__, __LINE__);
    verify(-3, y, __FILE__, __LINE__);
  }
  return 0;
}
