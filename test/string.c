#include "common.h"

char global_str[4] = "abc";
char *global_char_ptr = "abc";

char *global_array_of_char_ptr[]; // multiple declaration
char *global_array_of_char_ptr[] = {"abc", "def", "ghi"};
char *global_array_of_char_ptr[]; // multiple declaration

int main() {
  verify(97, 'a', __FILE__, __LINE__);
  verify(98, 'b', __FILE__, __LINE__);
  verify(99, 'c', __FILE__, __LINE__);
  verify(10, '\n', __FILE__, __LINE__);
  verify(0, '\0', __FILE__, __LINE__);
  verify(92, '\\', __FILE__, __LINE__);
  verify(39, '\'', __FILE__, __LINE__);

  verify(5, sizeof("a\nb\n"), __FILE__, __LINE__);
  verify(6, sizeof("a\\\nb\n"), __FILE__, __LINE__);
  verify(4, sizeof("ab\0"), __FILE__, __LINE__);

  // ignore newline by backslash
  verify(7, sizeof("ab\
cdef"),
         __FILE__, __LINE__);

  verify(97, global_str[0], __FILE__, __LINE__);
  verify(98, global_str[1], __FILE__, __LINE__);
  verify(99, global_str[2], __FILE__, __LINE__);
  verify(0, global_str[3], __FILE__, __LINE__);
  verify(4, sizeof(global_str), __FILE__, __LINE__);

  verify(97, global_char_ptr[0], __FILE__, __LINE__);
  verify(98, global_char_ptr[1], __FILE__, __LINE__);
  verify(99, global_char_ptr[2], __FILE__, __LINE__);
  verify(0, global_char_ptr[3], __FILE__, __LINE__);
  verify(8, sizeof(global_char_ptr), __FILE__, __LINE__);

  verify(97, global_array_of_char_ptr[0][0], __FILE__, __LINE__);
  verify(98, global_array_of_char_ptr[0][1], __FILE__, __LINE__);
  verify(99, global_array_of_char_ptr[0][2], __FILE__, __LINE__);
  verify(0, global_array_of_char_ptr[0][3], __FILE__, __LINE__);
  verify(100, global_array_of_char_ptr[1][0], __FILE__, __LINE__);
  verify(101, global_array_of_char_ptr[1][1], __FILE__, __LINE__);
  verify(102, global_array_of_char_ptr[1][2], __FILE__, __LINE__);
  verify(0, global_array_of_char_ptr[1][3], __FILE__, __LINE__);
  verify(103, global_array_of_char_ptr[2][0], __FILE__, __LINE__);
  verify(104, global_array_of_char_ptr[2][1], __FILE__, __LINE__);
  verify(105, global_array_of_char_ptr[2][2], __FILE__, __LINE__);
  verify(0, global_array_of_char_ptr[2][3], __FILE__, __LINE__);
  verify(24, sizeof(global_array_of_char_ptr), __FILE__, __LINE__);

  verify(1, "abc" == "ab"
                     "c",
         __FILE__, __LINE__);
  verify(1, "abc" == "a"
                     "b"
                     "c",
         __FILE__, __LINE__);

  verify(0, "a2c" == "ab"
                     "c",
         __FILE__, __LINE__);

  verify(0, "abc" == "a2"
                     "c",
         __FILE__, __LINE__);
  verify(0, "abc" == "a"
                     "b"
                     "2",
         __FILE__, __LINE__);

  {
    char *x = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
    verify(8, sizeof(x), __FILE__, __LINE__);
  }
  {
    char *x;
    x = "abc";
    x = "def";
    verify(90, x[0] - 10, __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
    verify(8, sizeof(x), __FILE__, __LINE__);
  }
  verify(0, ""[0], __FILE__, __LINE__);
  verify(1, sizeof(""), __FILE__, __LINE__);
  verify(97, "abc"[0], __FILE__, __LINE__);
  verify(98, "abc"[1], __FILE__, __LINE__);
  verify(99, "abc"[2], __FILE__, __LINE__);
  verify(0, "abc"[3], __FILE__, __LINE__);
  verify(4, sizeof("abc"), __FILE__, __LINE__);
  {
    char x[4] = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(98, x[1], __FILE__, __LINE__);
    verify(99, x[2], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
  }
  {
    char *x = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(98, x[1], __FILE__, __LINE__);
    verify(99, x[2], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
    verify(8, sizeof(x), __FILE__, __LINE__);
  }
  {
    char *x[3] = {"abc", "def", "ghi"};
    verify(97, x[0][0], __FILE__, __LINE__);
    verify(98, x[0][1], __FILE__, __LINE__);
    verify(99, x[0][2], __FILE__, __LINE__);
    verify(0, x[0][3], __FILE__, __LINE__);
    verify(100, x[1][0], __FILE__, __LINE__);
    verify(101, x[1][1], __FILE__, __LINE__);
    verify(102, x[1][2], __FILE__, __LINE__);
    verify(0, x[1][3], __FILE__, __LINE__);
    verify(103, x[2][0], __FILE__, __LINE__);
    verify(104, x[2][1], __FILE__, __LINE__);
    verify(105, x[2][2], __FILE__, __LINE__);
    verify(0, x[2][3], __FILE__, __LINE__);
    verify(24, sizeof(x), __FILE__, __LINE__);
  }
  {
    char x[] = "abc";
    verify(97, x[0], __FILE__, __LINE__);
    verify(98, x[1], __FILE__, __LINE__);
    verify(99, x[2], __FILE__, __LINE__);
    verify(0, x[3], __FILE__, __LINE__);
    verify(4, sizeof(x), __FILE__, __LINE__);
  }
  {
    char *x[] = {"abc", "def", "ghi"};
    verify(97, x[0][0], __FILE__, __LINE__);
    verify(98, x[0][1], __FILE__, __LINE__);
    verify(99, x[0][2], __FILE__, __LINE__);
    verify(0, x[0][3], __FILE__, __LINE__);
    verify(100, x[1][0], __FILE__, __LINE__);
    verify(101, x[1][1], __FILE__, __LINE__);
    verify(102, x[1][2], __FILE__, __LINE__);
    verify(0, x[1][3], __FILE__, __LINE__);
    verify(103, x[2][0], __FILE__, __LINE__);
    verify(104, x[2][1], __FILE__, __LINE__);
    verify(105, x[2][2], __FILE__, __LINE__);
    verify(0, x[2][3], __FILE__, __LINE__);
  }
  return 0;
}
