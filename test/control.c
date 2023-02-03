void verify(int expected, int actual, char *file_name, int line_number);
int printf(const char *, ...);

int main() {
  {
    int a = 3;
    int r;
    if (a == 3)
      r = a + 2;
    else
      r = a;
    verify(5, r, __FILE__, __LINE__);
    if (a < 3)
      r = a + 2;
    else
      r = a;
    verify(3, r, __FILE__, __LINE__);
  }
  {
    int b = 0;
    for (int a = 0; a < 10; a++)
      b = a * 2;
    verify(18, b, __FILE__, __LINE__);
  }
  {
    int a = 0;
    for (; a < 10;)
      a = a + 1;
    verify(10, a, __FILE__, __LINE__);
  }
  {
    int a = 0;
    int b = 0;
    for (; a < 10;)
      a = a + 1, b = b + 2;
    verify(20, b, __FILE__, __LINE__);
  }
  {
    int a = 0;
    int b = 0;
    while (a < 10) {
      a = a + 1;
      b = b + 2;
    }
    verify(20, b, __FILE__, __LINE__);
  }
  {
    int s = 0;
    for (int i = 1; i <= 10; i++)
      s = s + i;
    verify(55, s, __FILE__, __LINE__);
  }
  {
    int s = 0;
    for (int i = 1, *p; i <= 10; i = i + 1)
      s = s + sizeof(p);
    verify(80, s, __FILE__, __LINE__);
  }
  {
      // empty block
  }; // empty statement
  {
    // break, continue
    int outer = 0;
    for (int i = 0; i <= 10; ++i) {
      int s = 0;
      for (int j = 0; j < 100; j++) {
        s = s + j;
        if (j == 10)
          break;
      }
      verify(55, s, __FILE__, __LINE__);
      if (i == 5)
        continue;
      outer = outer + i;
    }
    verify(50, outer, __FILE__, __LINE__);
  }
  {
    int x = 2;
    switch (x)
    case 2:
      x = 3;
    verify(3, x, __FILE__, __LINE__);
  }
  {
    int x = 2;
    switch (x)
    case 2: {
      x = 3;
      x = x + 1;
    }
      verify(4, x, __FILE__, __LINE__);
  }
  {
    int x = 2;
    int y = 4;
    switch (x) {
    case 2:
      switch (y) {
      case 4:
        y = 3;
      }
    case 3:
      x = 4;
    }
    verify(4, x, __FILE__, __LINE__);
    verify(3, y, __FILE__, __LINE__);
  }
  {
    int x = 2;
    int y = 3;
    switch (x) {
      {
      case 2:
        if (y == 3) {
          x = 0;
        }
      }
      x = 1;
    case 4:
      y = 6;
    }
    verify(1, x, __FILE__, __LINE__);
    verify(6, y, __FILE__, __LINE__);
  }
  {
    int x = 2;
    int y = 3;
    int z = 4;
    switch (x) {
    default:
      z = 2;
    case 1:
      z = 3;
      {
      case 2:
        if (y == 3) {
          x = 0;
        }
      }
      x = 1;
    case 4:
      y = 6;
    }
    verify(1, x, __FILE__, __LINE__);
    verify(6, y, __FILE__, __LINE__);
    verify(4, z, __FILE__, __LINE__);
  }
  {
    int x = 2;
    int y = 0;
    int z = 4;
    switch (x) {
    case 4 - 2:
      y = 3;
      break;
    case 3:
      y = 4;
      break;
    default:
      y = 5;
      x = 4;
      break;
    }
    verify(2, x, __FILE__, __LINE__);
    verify(3, y, __FILE__, __LINE__);
  }
  {
    int x = 0;
    do {
      x = x + 1;
      if (x == 10)
        continue;
    } while (x < 10);
    verify(10, x, __FILE__, __LINE__);
  }
  {
    int x = 0;
    int y = 0;
    do {
      x = x + 1;
      if (x == 2)
        continue;
      y = y + 3;
    } while (x < 10);
    verify(10, x, __FILE__, __LINE__);
    verify(27, y, __FILE__, __LINE__);
  }
  {
    int x = 0;
    int y = 0;
    do {
      x = x + 1;
      if (x == 5)
        break;
      y = y + 3;
    } while (x < 10);
    verify(5, x, __FILE__, __LINE__);
    verify(12, y, __FILE__, __LINE__);
  }
  {
    int x = -1;
    if (0, 0, 0, 5)
      x = 3;
    verify(3, x, __FILE__, __LINE__);
    if (2, 3, 4, 0)
      x = 4;
    verify(3, x, __FILE__, __LINE__);
  }
  {
    int x = -1;
    while (0, x) {
      verify(-1, x, __FILE__, __LINE__);
      x = 0;
    }
    verify(0, x, __FILE__, __LINE__);
  }
  {
    int x = -4;
    do {
      verify(1, x < 0, __FILE__, __LINE__);
      x++;
    } while (1, 2, 3, x);
    verify(0, x, __FILE__, __LINE__);
  }
  {
    int x = -4;
    do {
      verify(1, x < 0, __FILE__, __LINE__);
      x++;
    } while (1, 2, 3, x);
    verify(0, x, __FILE__, __LINE__);
  }
  {
    int x, y;
    for (x = 2, y = 3; 1, x < 10; x++, y++) {
      verify(1, y - x, __FILE__, __LINE__);
    }
  }
  {
    int r = 0;
  foo:;
    int x = 1;
    printf("r: %d   x: %d\n", r, x);
    if (r) {
      verify(1, x, __FILE__, __LINE__);
      goto end;
    }
    r = 1;
    x = 2;
    goto foo;
  end:;
  }
  return 0;
}
