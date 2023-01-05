int main() {
  {
    int a;
    a = 3;
    int r;
    if (a == 3)
      r = a + 2;
    else
      r = a;
    verify(5, r, __FILE__, __LINE__);
  }
  {
    int a;
    a = 3;
    int r;
    if (a < 3)
      r = a + 2;
    else
      r = a;
    verify(3, r, __FILE__, __LINE__);
  }
  {
    int b;
    b = 0;
    int a;
    for (a = 0; a < 10; a = a + 1)
      b = a * 2;
    verify(18, b, __FILE__, __LINE__);
  }
  {
    int a;
    a = 0;
    for (; a < 10;)
      a = a + 1;
    a;
    verify(10, a, __FILE__, __LINE__);
  }
  {
    int a;
    a = 0;
    int b;
    b = 0;
    for (; a < 10;) {
      a = a + 1;
      b = b + 2;
    }
    b;
    verify(20, b, __FILE__, __LINE__);
  }
  {
    int a;
    a = 0;
    int b;
    b = 0;
    while (a < 10) {
      a = a + 1;
      b = b + 2;
    }
    b;
    verify(20, b, __FILE__, __LINE__);
  }
  {
    int s = 0;
    for (int i = 1; i <= 10; i = i + 1)
      s = s + i;
    verify(55, s, __FILE__, __LINE__);
  }
  {
    int s = 0;
    for (int i = 1, *p; i <= 10; i = i + 1)
      s = s + sizeof(p);
    verify(80, s, __FILE__, __LINE__);
  }
  return 0;
}
