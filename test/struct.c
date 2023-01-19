void verify(int expected, int actual, char *file_name, int line_number);

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
  return 0;
}
