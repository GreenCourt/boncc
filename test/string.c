char global_str[4] = "abc";
char *global_char_ptr = "abc";

int main() {
  verify(97, global_str[0], __FILE__, __LINE__);
  verify(98, global_str[1], __FILE__, __LINE__);
  verify(99, global_str[2], __FILE__, __LINE__);
  verify(0, global_str[3], __FILE__, __LINE__);

  verify(97, global_char_ptr[0], __FILE__, __LINE__);
  verify(98, global_char_ptr[1], __FILE__, __LINE__);
  verify(99, global_char_ptr[2], __FILE__, __LINE__);
  verify(0, global_char_ptr[3], __FILE__, __LINE__);

  {
    char *x;
    x = "abc";
    verify(97, x[0], __FILE__, __LINE__);
  }
  {
    char *x;
    x = "abc";
    x = "def";
    verify(90, x[0] - 10, __FILE__, __LINE__);
  }
  verify(0, ""[0], __FILE__, __LINE__);
  verify(1, sizeof(""), __FILE__, __LINE__);
  verify(97, "abc"[0], __FILE__, __LINE__);
  verify(98, "abc"[1], __FILE__, __LINE__);
  verify(99, "abc"[2], __FILE__, __LINE__);
  verify(0, "abc"[3], __FILE__, __LINE__);
  verify(4, sizeof("abc"), __FILE__, __LINE__);
  verify(4, sizeof("abc"), __FILE__, __LINE__);
  {
    char x[4] = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(98, x[1], __FILE__, __LINE__);
    verify(99, x[2], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
  }
  {
    char *x = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(98, x[1], __FILE__, __LINE__);
    verify(99, x[2], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
  }
  return 0;
}
