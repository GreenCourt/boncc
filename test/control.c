#include "common.h"

int main() {
  {
    int a = 3;
    int r;
    if (a == 3)
      r = a + 2;
    else
      r = a;
    VERIFY(5, r);
    if (a < 3)
      r = a + 2;
    else
      r = a;
    VERIFY(3, r);
  }
  {
    int b = 0;
    for (int a = 0; a < 10; a++)
      b = a * 2;
    VERIFY(18, b);
  }
  {
    int a = 0;
    for (; a < 10;)
      a = a + 1;
    VERIFY(10, a);
  }
  {
    int a = 0;
    int b = 0;
    for (; a < 10;)
      a = a + 1, b = b + 2;
    VERIFY(20, b);
  }
  {
    int a = 0;
    int b = 0;
    while (a < 10) {
      a = a + 1;
      b = b + 2;
    }
    VERIFY(20, b);
  }
  {
    int s = 0;
    for (int i = 1; i <= 10; i++)
      s = s + i;
    VERIFY(55, s);
  }
  {
    int s = 0;
    for (int i = 1, *p; i <= 10; i = i + 1)
      s = s + sizeof(p);
    VERIFY(80, s);
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
      VERIFY(55, s);
      if (i == 5)
        continue;
      outer = outer + i;
    }
    VERIFY(50, outer);
  }
  {
    int x = 2;
    switch (x)
    case 2:
      x = 3;
    VERIFY(3, x);
  }
  {
    int x = 2;
    switch (x)
    case 2: {
      x = 3;
      x = x + 1;
    }
      VERIFY(4, x);
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
    VERIFY(4, x);
    VERIFY(3, y);
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
    VERIFY(1, x);
    VERIFY(6, y);
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
    VERIFY(1, x);
    VERIFY(6, y);
    VERIFY(4, z);
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
    VERIFY(2, x);
    VERIFY(3, y);
  }
  {
    int x = 0;
    do {
      x = x + 1;
      if (x == 10)
        continue;
    } while (x < 10);
    VERIFY(10, x);
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
    VERIFY(10, x);
    VERIFY(27, y);
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
    VERIFY(5, x);
    VERIFY(12, y);
  }
  {
    int x = -1;
    if (0, 0, 0, 5)
      x = 3;
    VERIFY(3, x);
    if (2, 3, 4, 0)
      x = 4;
    VERIFY(3, x);
  }
  {
    int x = -1;
    while (0, x) {
      VERIFY(-1, x);
      x = 0;
    }
    VERIFY(0, x);
  }
  {
    int x = -4;
    do {
      VERIFY(1, x < 0);
      x++;
    } while (1, 2, 3, x);
    VERIFY(0, x);
  }
  {
    int x = -4;
    do {
      VERIFY(1, x < 0);
      x++;
    } while (1, 2, 3, x);
    VERIFY(0, x);
  }
  {
    int x, y;
    for (x = 2, y = 3; 1, x < 10; x++, y++) {
      VERIFY(1, y - x);
    }
  }
  {
    int r = 0;
  foo:;
    int x = 1;
    if (r) {
      VERIFY(1, x);
      goto end;
    }
    r = 1;
    x = 2;
    goto foo;
  end:;
  }
  return 0;
}
