#include "common.h"

char global_str[4] = "abc";
char *global_char_ptr = "abc";

char *global_array_of_char_ptr[]; // multiple declaration
char *global_array_of_char_ptr[] = {"abc", "def", "ghi"};
char *global_array_of_char_ptr[]; // multiple declaration

int main() {
  verify(97, 'a');
  verify(98, 'b');
  verify(99, 'c');
  verify(10, '\n');
  verify(0, '\0');
  verify(92, '\\');
  verify(39, '\'');

  verify(5, sizeof("a\nb\n"));
  verify(6, sizeof("a\\\nb\n"));
  verify(4, sizeof("ab\0"));

  // ignore newline by backslash
  verify(7, sizeof("ab\
cdef"));

  verify(97, global_str[0]);
  verify(98, global_str[1]);
  verify(99, global_str[2]);
  verify(0, global_str[3]);
  verify(4, sizeof(global_str));

  verify(97, global_char_ptr[0]);
  verify(98, global_char_ptr[1]);
  verify(99, global_char_ptr[2]);
  verify(0, global_char_ptr[3]);
  verify(8, sizeof(global_char_ptr));

  verify(97, global_array_of_char_ptr[0][0]);
  verify(98, global_array_of_char_ptr[0][1]);
  verify(99, global_array_of_char_ptr[0][2]);
  verify(0, global_array_of_char_ptr[0][3]);
  verify(100, global_array_of_char_ptr[1][0]);
  verify(101, global_array_of_char_ptr[1][1]);
  verify(102, global_array_of_char_ptr[1][2]);
  verify(0, global_array_of_char_ptr[1][3]);
  verify(103, global_array_of_char_ptr[2][0]);
  verify(104, global_array_of_char_ptr[2][1]);
  verify(105, global_array_of_char_ptr[2][2]);
  verify(0, global_array_of_char_ptr[2][3]);
  verify(24, sizeof(global_array_of_char_ptr));

  verify(1, "abc" == "ab"
                     "c");
  verify(1, "abc" == "a"
                     "b"
                     "c");

  verify(0, "a2c" == "ab"
                     "c");

  verify(0, "abc" == "a2"
                     "c");
  verify(0, "abc" == "a"
                     "b"
                     "2");

  {
    char *x = "abc";
    verify(97, x[0]);
    verify(0, x[3]);
    verify(8, sizeof(x));
  }
  {
    char *x;
    x = "abc";
    x = "def";
    verify(90, x[0] - 10);
    verify(0, x[3]);
    verify(8, sizeof(x));
  }
  verify(0, ""[0]);
  verify(1, sizeof(""));
  verify(97, "abc"[0]);
  verify(98, "abc"[1]);
  verify(99, "abc"[2]);
  verify(0, "abc"[3]);
  verify(4, sizeof("abc"));
  {
    char x[4] = "abc";
    verify(97, x[0]);
    verify(98, x[1]);
    verify(99, x[2]);
    verify(0, x[3]);
    verify(4, sizeof(x));
  }
  {
    char *x = "abc";
    verify(97, x[0]);
    verify(98, x[1]);
    verify(99, x[2]);
    verify(0, x[3]);
    verify(8, sizeof(x));
  }
  {
    char *x[3] = {"abc", "def", "ghi"};
    verify(97, x[0][0]);
    verify(98, x[0][1]);
    verify(99, x[0][2]);
    verify(0, x[0][3]);
    verify(100, x[1][0]);
    verify(101, x[1][1]);
    verify(102, x[1][2]);
    verify(0, x[1][3]);
    verify(103, x[2][0]);
    verify(104, x[2][1]);
    verify(105, x[2][2]);
    verify(0, x[2][3]);
    verify(24, sizeof(x));
  }
  {
    char x[] = "abc";
    verify(97, x[0]);
    verify(98, x[1]);
    verify(99, x[2]);
    verify(0, x[3]);
    verify(4, sizeof(x));
  }
  {
    char *x[] = {"abc", "def", "ghi"};
    verify(97, x[0][0]);
    verify(98, x[0][1]);
    verify(99, x[0][2]);
    verify(0, x[0][3]);
    verify(100, x[1][0]);
    verify(101, x[1][1]);
    verify(102, x[1][2]);
    verify(0, x[1][3]);
    verify(103, x[2][0]);
    verify(104, x[2][1]);
    verify(105, x[2][2]);
    verify(0, x[2][3]);
  }
  return 0;
}
