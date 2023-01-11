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
  verify(9, sizeof(g), __FILE__, __LINE__);
  verify(9, sizeof(struct global_struct1), __FILE__, __LINE__);
  verify(12, sizeof(struct global_struct2), __FILE__, __LINE__);

  {
    // shadow
    struct global_struct1 {
      int a;
      char c;
    };
    verify(5, sizeof(struct global_struct1), __FILE__, __LINE__);
  }
  {
    // declare unnamed struct and variable at same time
    struct {
      int a;
      int b;
      char c;
    } x;
    verify(9, sizeof(x), __FILE__, __LINE__);
  }
  {
    // declare named struct
    struct st {
      int a;
      int b;
      char c;
      char d;
    };
    verify(10, sizeof(struct st), __FILE__, __LINE__);
  }
  {
    // declare named struct and variable at same time
    struct st {
      int a;
      int b;
      char c;
      char d;
    } x;
    verify(10, sizeof(struct st), __FILE__, __LINE__);
    verify(10, sizeof(x), __FILE__, __LINE__);
  }
  {
    // empty struct;
    struct empty {
    } x;
    verify(0, sizeof(struct empty), __FILE__, __LINE__);
    verify(0, sizeof(x), __FILE__, __LINE__);
  }
  return 0;
}
