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
  VERIFY(12, sizeof(g));
  VERIFY(12, sizeof(struct global_struct1));
  VERIFY(12, sizeof(struct global_struct2));

  {
    // shadow
    struct global_struct1 {
      int a;
      char c;
    };
    VERIFY(8, sizeof(struct global_struct1));
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
    VERIFY(12, sizeof(x));
    VERIFY(11, x.a);
    VERIFY(12, x.b);
    VERIFY(13, x.c);
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
    VERIFY(24, sizeof(struct st));
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
    VERIFY(24, sizeof(struct st));
    VERIFY(24, sizeof(x));
    VERIFY(8, sizeof(x.b));
    VERIFY(5, x.a);
    VERIFY(6, x.b);
    VERIFY(7, x.c);
    VERIFY(9, x.d);
  }
  {
    // empty struct;
    struct empty {
    } x;
    VERIFY(0, sizeof(struct empty));
    VERIFY(0, sizeof(x));
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
    VERIFY(12, sizeof(struct st));
    VERIFY(8, sizeof(p));
    VERIFY(4, sizeof(p->b));
    VERIFY(5, p->a);
    VERIFY(6, p->b);
    VERIFY(7, p->c);
    VERIFY(9, p->d);
    VERIFY(5, x.a);
    VERIFY(6, x.b);
    VERIFY(7, x.c);
    VERIFY(9, x.d);
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
    VERIFY(12, x.p->a);
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
    VERIFY(8, sizeof(struct S));
    VERIFY(8, sizeof(a));
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
    VERIFY(3, s.a);
    VERIFY(5, s.b);

    u1.y = 42;
    VERIFY(42, u1.y);
    u1.z[2] = 3;
    VERIFY(3, u1.z[2]);
    u1.y = 100;
    VERIFY(1, u1.z[2] != 3);

    union U2 u2;
    u2.s.a = 14;
    VERIFY(14, u2.s.a);
    VERIFY(14, u2.t.c);
  }
  {
    struct S {
      int x;
      const int y;
    } s;
    s.x = 12;
    VERIFY(12, s.x);
  }
  {
    struct S {
      struct NAMED {
        int a;
        int b;
      } x;
    };
    VERIFY(8, sizeof(struct S));
    struct S s;
    s.x.a = 2;
    s.x.b = 3;
    VERIFY(2, s.x.a);
    VERIFY(3, s.x.b);
  }
  {
    struct S {
      union NAMED {
        int a;
        int b;
      } x;
    };
    VERIFY(4, sizeof(struct S));
    struct S s;
    s.x.a = 2;
    VERIFY(2, s.x.a);
    VERIFY(2, s.x.b);
  }
  {
    union U {
      union NAMED {
        int a;
        int b;
      } x;
    };
    VERIFY(4, sizeof(union U));
    union U u;
    u.x.a = 2;
    VERIFY(2, u.x.a);
    VERIFY(2, u.x.b);
  }
  {
    union U {
      struct NAMED {
        int a;
        int b;
      } x;
    };
    VERIFY(8, sizeof(union U));
    union U u;
    u.x.a = 2;
    u.x.b = 3;
    VERIFY(2, u.x.a);
    VERIFY(3, u.x.b);
  }
  {
    union U {
      int q;
      struct {
        int a;
        int b;
      };
    };
    VERIFY(8, sizeof(union U));
    union U u;
    u.a = 2;
    u.b = 3;
    VERIFY(2, u.a);
    VERIFY(3, u.b);
    VERIFY(2, u.q);
  }

  {
    union U {
      int q;
      union {
        int a;
        int b;
      };
    };
    VERIFY(4, sizeof(union U));
    union U u;
    u.a = 12;
    VERIFY(12, u.b);
    VERIFY(12, u.q);
  }

  {
    struct S {
      long q;
      union {
        int a;
        long b;
      };
    };
    VERIFY(16, sizeof(struct S));
    struct S s;
    s.a = 4;
    VERIFY(4, s.a);
    s.b = 7;
    VERIFY(7, s.b);
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
    VERIFY(16, sizeof(struct S));
    struct S s;
    s.a = 2;
    s.b = 3;
    VERIFY(2, s.a);
    VERIFY(3, s.b);
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
    VERIFY(4, x.a);
    VERIFY(5, x.b);
    VERIFY(6, x.c);
    VERIFY(4, y.a);
    VERIFY(5, y.b);
    VERIFY(6, y.c);
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
    VERIFY(6, y.a);
    x.b = 4;
    y = x;
    VERIFY(4, y.b);
    x.c = 8;
    y = x;
    VERIFY(8, y.c);
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
    VERIFY(4, z.a);
    VERIFY(5, z.b);
    VERIFY(6, z.c);
    z = 0 ? x : y;
    VERIFY(1, z.a);
    VERIFY(2, z.b);
    VERIFY(3, z.c);
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
    VERIFY(28, sizeof(struct S));
  }
  {
    // local initializer
    struct global_struct_to_test_initializer x = {
        1, {2}, {3}, {{5}, {7, {8, 9}}}};
    struct global_struct_to_test_initializer y = {1, 2, {3}, 5};
    VERIFY(36, sizeof(x));
    VERIFY(1, x.a);
    VERIFY(2, x.b.b1);
    VERIFY(3, x.c[0]);
    VERIFY(0, x.c[1]);
    VERIFY(0, x.c[2]);
    VERIFY(5, x.d[0].d1);
    VERIFY(0, x.d[0].d2[0]);
    VERIFY(0, x.d[0].d2[1]);
    VERIFY(0, x.d[0].d2[2]);
    VERIFY(7, x.d[1].d1);
    VERIFY(8, x.d[1].d2[0]);
    VERIFY(9, x.d[1].d2[1]);
    VERIFY(0, x.d[1].d2[2]);

    VERIFY(36, sizeof(y));
    VERIFY(1, y.a);
    VERIFY(2, y.b.b1);
    VERIFY(3, y.c[0]);
    VERIFY(0, y.c[1]);
    VERIFY(0, y.c[2]);
    VERIFY(5, y.d[0].d1);
    VERIFY(0, y.d[0].d2[0]);
    VERIFY(0, y.d[0].d2[1]);
    VERIFY(0, y.d[0].d2[2]);
    VERIFY(0, y.d[1].d1);
    VERIFY(0, y.d[1].d2[0]);
    VERIFY(0, y.d[1].d2[1]);
    VERIFY(0, y.d[1].d2[2]);
  }
  {
    // global initializer
    VERIFY(36, sizeof(g1));
    VERIFY(1, g1.a);
    VERIFY(2, g1.b.b1);
    VERIFY(3, g1.c[0]);
    VERIFY(0, g1.c[1]);
    VERIFY(0, g1.c[2]);
    VERIFY(5, g1.d[0].d1);
    VERIFY(0, g1.d[0].d2[0]);
    VERIFY(0, g1.d[0].d2[1]);
    VERIFY(0, g1.d[0].d2[2]);
    VERIFY(7, g1.d[1].d1);
    VERIFY(8, g1.d[1].d2[0]);
    VERIFY(9, g1.d[1].d2[1]);
    VERIFY(0, g1.d[1].d2[2]);

    VERIFY(36, sizeof(g2));
    VERIFY(1, g2.a);
    VERIFY(2, g2.b.b1);
    VERIFY(3, g2.c[0]);
    VERIFY(0, g2.c[1]);
    VERIFY(0, g2.c[2]);
    VERIFY(5, g2.d[0].d1);
    VERIFY(0, g2.d[0].d2[0]);
    VERIFY(0, g2.d[0].d2[1]);
    VERIFY(0, g2.d[0].d2[2]);
    VERIFY(0, g2.d[1].d1);
    VERIFY(0, g2.d[1].d2[0]);
    VERIFY(0, g2.d[1].d2[1]);
    VERIFY(0, g2.d[1].d2[2]);
  }
  return 0;
}
