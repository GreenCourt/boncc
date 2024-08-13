#include "common.h"

struct global_struct1 {
  int a;
  int b;
  char c;
} g;

struct global_struct2 {
  int a;
  int b;
  int c;
};

struct global_struct_to_test_initializer {
  int a;
  union {
    int b1;
    char b2;
  } b;
  char c[3];
  struct {
    int d1;
    short d2[3];
  } d[2];
};

struct global_struct_to_test_initializer g1 = {1, {2}, {3}, {{5}, {7, {8, 9}}}};
struct global_struct_to_test_initializer g2 = {1, 2, {3}, 5};

int main() {
  verify(12, sizeof(g), __FILE__, __LINE__);
  verify(12, sizeof(struct global_struct1), __FILE__, __LINE__);
  verify(12, sizeof(struct global_struct2), __FILE__, __LINE__);

  {
    // shadow
    struct global_struct1 {
      int a;
      char c;
    };
    verify(8, sizeof(struct global_struct1), __FILE__, __LINE__);
  }
  {
    // declare unnamed struct and variable at same time
    struct {
      int a;
      char b;
      int c;
    } x;
    x.a = 11;
    x.b = 12;
    x.c = 13;
    verify(12, sizeof(x), __FILE__, __LINE__);
    verify(11, x.a, __FILE__, __LINE__);
    verify(12, x.b, __FILE__, __LINE__);
    verify(13, x.c, __FILE__, __LINE__);
  }
  {
    // declare named struct
    struct st {
      short a;
      int b;
      char c;
      char d;
      char *e;
    };
    verify(24, sizeof(struct st), __FILE__, __LINE__);
  }
  {
    // this block contains only struct declaration
    struct {
      int a;
      long b;
      char c;
      char d;
      char *e;
    };
  }
  {
    // declare named struct and variable at same time
    struct st {
      int a;
      long b;
      char c;
      char d;
    } x;
    x.a = 5;
    x.b = 6;
    x.c = 7;
    x.d = 9;
    verify(24, sizeof(struct st), __FILE__, __LINE__);
    verify(24, sizeof(x), __FILE__, __LINE__);
    verify(8, sizeof(x.b), __FILE__, __LINE__);
    verify(5, x.a, __FILE__, __LINE__);
    verify(6, x.b, __FILE__, __LINE__);
    verify(7, x.c, __FILE__, __LINE__);
    verify(9, x.d, __FILE__, __LINE__);
  }
  {
    // empty struct;
    struct empty {
    } x;
    verify(0, sizeof(struct empty), __FILE__, __LINE__);
    verify(0, sizeof(x), __FILE__, __LINE__);
  }
  {
    // pointer
    struct st {
      int a;
      int b;
      char c;
      char d;
    } x, *p;
    p = &x;
    p->a = 5;
    p->b = 6;
    p->c = 7;
    p->d = 9;
    verify(12, sizeof(struct st), __FILE__, __LINE__);
    verify(8, sizeof(p), __FILE__, __LINE__);
    verify(4, sizeof(p->b), __FILE__, __LINE__);
    verify(5, p->a, __FILE__, __LINE__);
    verify(6, p->b, __FILE__, __LINE__);
    verify(7, p->c, __FILE__, __LINE__);
    verify(9, p->d, __FILE__, __LINE__);
    verify(5, x.a, __FILE__, __LINE__);
    verify(6, x.b, __FILE__, __LINE__);
    verify(7, x.c, __FILE__, __LINE__);
    verify(9, x.d, __FILE__, __LINE__);
  }
  {
    // struct pointer in the struct
    struct st {
      struct st *p;
      int a;
    };
    struct st x;
    x.a = 12;
    x.p = &x;
    verify(12, x.p->a, __FILE__, __LINE__);
  }
  {
    // forward declaration
    struct S;
    struct S {
      short a;
      int b;
    };
    struct S a;
    struct S; // declare twice
    verify(8, sizeof(struct S), __FILE__, __LINE__);
    verify(8, sizeof(a), __FILE__, __LINE__);
  }
  {
    union U1 {
      short x;
      int y;
      char z[10];
    };

    union U2 {
      struct S {
        int a;
        long b;
      } s;
      struct T {
        int c;
        short d;
      } t;
    };

    typedef union U1 U1;
    U1 u1;
    struct S s;
    s.a = 3;
    s.b = 5;
    verify(3, s.a, __FILE__, __LINE__);
    verify(5, s.b, __FILE__, __LINE__);

    u1.y = 42;
    verify(42, u1.y, __FILE__, __LINE__);
    u1.z[2] = 3;
    verify(3, u1.z[2], __FILE__, __LINE__);
    u1.y = 100;
    verify(1, u1.z[2] != 3, __FILE__, __LINE__);

    union U2 u2;
    u2.s.a = 14;
    verify(14, u2.s.a, __FILE__, __LINE__);
    verify(14, u2.t.c, __FILE__, __LINE__);
  }
  {
    struct S {
      int x;
      const int y;
    } s;
    s.x = 12;
    verify(12, s.x, __FILE__, __LINE__);
  }
  {
    struct S {
      struct NAMED {
        int a;
        int b;
      } x;
    };
    verify(8, sizeof(struct S), __FILE__, __LINE__);
    struct S s;
    s.x.a = 2;
    s.x.b = 3;
    verify(2, s.x.a, __FILE__, __LINE__);
    verify(3, s.x.b, __FILE__, __LINE__);
  }
  {
    struct S {
      union NAMED {
        int a;
        int b;
      } x;
    };
    verify(4, sizeof(struct S), __FILE__, __LINE__);
    struct S s;
    s.x.a = 2;
    verify(2, s.x.a, __FILE__, __LINE__);
    verify(2, s.x.b, __FILE__, __LINE__);
  }
  {
    union U {
      union NAMED {
        int a;
        int b;
      } x;
    };
    verify(4, sizeof(union U), __FILE__, __LINE__);
    union U u;
    u.x.a = 2;
    verify(2, u.x.a, __FILE__, __LINE__);
    verify(2, u.x.b, __FILE__, __LINE__);
  }
  {
    union U {
      struct NAMED {
        int a;
        int b;
      } x;
    };
    verify(8, sizeof(union U), __FILE__, __LINE__);
    union U u;
    u.x.a = 2;
    u.x.b = 3;
    verify(2, u.x.a, __FILE__, __LINE__);
    verify(3, u.x.b, __FILE__, __LINE__);
  }
  {
    union U {
      int q;
      struct {
        int a;
        int b;
      };
    };
    verify(8, sizeof(union U), __FILE__, __LINE__);
    union U u;
    u.a = 2;
    u.b = 3;
    verify(2, u.a, __FILE__, __LINE__);
    verify(3, u.b, __FILE__, __LINE__);
    verify(2, u.q, __FILE__, __LINE__);
  }

  {
    union U {
      int q;
      union {
        int a;
        int b;
      };
    };
    verify(4, sizeof(union U), __FILE__, __LINE__);
    union U u;
    u.a = 12;
    verify(12, u.b, __FILE__, __LINE__);
    verify(12, u.q, __FILE__, __LINE__);
  }

  {
    struct S {
      long q;
      union {
        int a;
        long b;
      };
    };
    verify(16, sizeof(struct S), __FILE__, __LINE__);
    struct S s;
    s.a = 4;
    verify(4, s.a, __FILE__, __LINE__);
    s.b = 7;
    verify(7, s.b, __FILE__, __LINE__);
  }
  {
    struct S {
      short q;
      struct {
        int a;
        int b;
      };
      int c;
    };
    verify(16, sizeof(struct S), __FILE__, __LINE__);
    struct S s;
    s.a = 2;
    s.b = 3;
    verify(2, s.a, __FILE__, __LINE__);
    verify(3, s.b, __FILE__, __LINE__);
  }
  {
    struct S {
      char a;
      short b;
      int c;
    };
    struct S x, y;
    x.a = 4;
    x.b = 5;
    x.c = 6;
    y = x;
    verify(4, x.a, __FILE__, __LINE__);
    verify(5, x.b, __FILE__, __LINE__);
    verify(6, x.c, __FILE__, __LINE__);
    verify(4, y.a, __FILE__, __LINE__);
    verify(5, y.b, __FILE__, __LINE__);
    verify(6, y.c, __FILE__, __LINE__);
  }
  {
    union U {
      char a;
      short b;
      int c;
    };
    union U x, y;
    x.a = 6;
    y = x;
    verify(6, y.a, __FILE__, __LINE__);
    x.b = 4;
    y = x;
    verify(4, y.b, __FILE__, __LINE__);
    x.c = 8;
    y = x;
    verify(8, y.c, __FILE__, __LINE__);
  }
  {
    struct S {
      char a;
      short b;
      int c;
    };
    struct S x, y;
    x.a = 4;
    x.b = 5;
    x.c = 6;
    y.a = 1;
    y.b = 2;
    y.c = 3;
    struct S z = 1 ? x : y;
    verify(4, z.a, __FILE__, __LINE__);
    verify(5, z.b, __FILE__, __LINE__);
    verify(6, z.c, __FILE__, __LINE__);
    z = 0 ? x : y;
    verify(1, z.a, __FILE__, __LINE__);
    verify(2, z.b, __FILE__, __LINE__);
    verify(3, z.c, __FILE__, __LINE__);
  }
  {
    // test align
    struct S {
      int a;
      union {
        int b1;
        char b2;
      } b;
      char c[3];
      struct {
        int d1;
        short d2;
      } d[2];
    };
    verify(28, sizeof(struct S), __FILE__, __LINE__);
  }
  {
    // local initializer
    struct global_struct_to_test_initializer x = {
        1, {2}, {3}, {{5}, {7, {8, 9}}}};
    struct global_struct_to_test_initializer y = {1, 2, {3}, 5};
    verify(36, sizeof(x), __FILE__, __LINE__);
    verify(1, x.a, __FILE__, __LINE__);
    verify(2, x.b.b1, __FILE__, __LINE__);
    verify(3, x.c[0], __FILE__, __LINE__);
    verify(0, x.c[1], __FILE__, __LINE__);
    verify(0, x.c[2], __FILE__, __LINE__);
    verify(5, x.d[0].d1, __FILE__, __LINE__);
    verify(0, x.d[0].d2[0], __FILE__, __LINE__);
    verify(0, x.d[0].d2[1], __FILE__, __LINE__);
    verify(0, x.d[0].d2[2], __FILE__, __LINE__);
    verify(7, x.d[1].d1, __FILE__, __LINE__);
    verify(8, x.d[1].d2[0], __FILE__, __LINE__);
    verify(9, x.d[1].d2[1], __FILE__, __LINE__);
    verify(0, x.d[1].d2[2], __FILE__, __LINE__);

    verify(36, sizeof(y), __FILE__, __LINE__);
    verify(1, y.a, __FILE__, __LINE__);
    verify(2, y.b.b1, __FILE__, __LINE__);
    verify(3, y.c[0], __FILE__, __LINE__);
    verify(0, y.c[1], __FILE__, __LINE__);
    verify(0, y.c[2], __FILE__, __LINE__);
    verify(5, y.d[0].d1, __FILE__, __LINE__);
    verify(0, y.d[0].d2[0], __FILE__, __LINE__);
    verify(0, y.d[0].d2[1], __FILE__, __LINE__);
    verify(0, y.d[0].d2[2], __FILE__, __LINE__);
    verify(0, y.d[1].d1, __FILE__, __LINE__);
    verify(0, y.d[1].d2[0], __FILE__, __LINE__);
    verify(0, y.d[1].d2[1], __FILE__, __LINE__);
    verify(0, y.d[1].d2[2], __FILE__, __LINE__);
  }
  {
    // global initializer
    verify(36, sizeof(g1), __FILE__, __LINE__);
    verify(1, g1.a, __FILE__, __LINE__);
    verify(2, g1.b.b1, __FILE__, __LINE__);
    verify(3, g1.c[0], __FILE__, __LINE__);
    verify(0, g1.c[1], __FILE__, __LINE__);
    verify(0, g1.c[2], __FILE__, __LINE__);
    verify(5, g1.d[0].d1, __FILE__, __LINE__);
    verify(0, g1.d[0].d2[0], __FILE__, __LINE__);
    verify(0, g1.d[0].d2[1], __FILE__, __LINE__);
    verify(0, g1.d[0].d2[2], __FILE__, __LINE__);
    verify(7, g1.d[1].d1, __FILE__, __LINE__);
    verify(8, g1.d[1].d2[0], __FILE__, __LINE__);
    verify(9, g1.d[1].d2[1], __FILE__, __LINE__);
    verify(0, g1.d[1].d2[2], __FILE__, __LINE__);

    verify(36, sizeof(g2), __FILE__, __LINE__);
    verify(1, g2.a, __FILE__, __LINE__);
    verify(2, g2.b.b1, __FILE__, __LINE__);
    verify(3, g2.c[0], __FILE__, __LINE__);
    verify(0, g2.c[1], __FILE__, __LINE__);
    verify(0, g2.c[2], __FILE__, __LINE__);
    verify(5, g2.d[0].d1, __FILE__, __LINE__);
    verify(0, g2.d[0].d2[0], __FILE__, __LINE__);
    verify(0, g2.d[0].d2[1], __FILE__, __LINE__);
    verify(0, g2.d[0].d2[2], __FILE__, __LINE__);
    verify(0, g2.d[1].d1, __FILE__, __LINE__);
    verify(0, g2.d[1].d2[0], __FILE__, __LINE__);
    verify(0, g2.d[1].d2[1], __FILE__, __LINE__);
    verify(0, g2.d[1].d2[2], __FILE__, __LINE__);
  }
  return 0;
}
