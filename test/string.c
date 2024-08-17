#include "common.h"

char global_str[4] = "abc";
char *global_char_ptr = "abc";

char *global_array_of_char_ptr[]; // multiple declaration
char *global_array_of_char_ptr[] = {"abc", "def", "ghi"};
char *global_array_of_char_ptr[]; // multiple declaration

int main() {
  VERIFY(97, 'a');
  VERIFY(98, 'b');
  VERIFY(99, 'c');
  VERIFY(10, '\n');
  VERIFY(0, '\0');
  VERIFY(92, '\\');
  VERIFY(39, '\'');

  VERIFY(5, sizeof("a\nb\n"));
  VERIFY(6, sizeof("a\\\nb\n"));
  VERIFY(4, sizeof("ab\0"));

  // ignore newline by backslash
  VERIFY(7, sizeof("ab\
cdef"));

  VERIFY(97, global_str[0]);
  VERIFY(98, global_str[1]);
  VERIFY(99, global_str[2]);
  VERIFY(0, global_str[3]);
  VERIFY(4, sizeof(global_str));

  VERIFY(97, global_char_ptr[0]);
  VERIFY(98, global_char_ptr[1]);
  VERIFY(99, global_char_ptr[2]);
  VERIFY(0, global_char_ptr[3]);
  VERIFY(8, sizeof(global_char_ptr));

  VERIFY(97, global_array_of_char_ptr[0][0]);
  VERIFY(98, global_array_of_char_ptr[0][1]);
  VERIFY(99, global_array_of_char_ptr[0][2]);
  VERIFY(0, global_array_of_char_ptr[0][3]);
  VERIFY(100, global_array_of_char_ptr[1][0]);
  VERIFY(101, global_array_of_char_ptr[1][1]);
  VERIFY(102, global_array_of_char_ptr[1][2]);
  VERIFY(0, global_array_of_char_ptr[1][3]);
  VERIFY(103, global_array_of_char_ptr[2][0]);
  VERIFY(104, global_array_of_char_ptr[2][1]);
  VERIFY(105, global_array_of_char_ptr[2][2]);
  VERIFY(0, global_array_of_char_ptr[2][3]);
  VERIFY(24, sizeof(global_array_of_char_ptr));

  VERIFY(1, "abc" == "ab"
                     "c");
  VERIFY(1, "abc" == "a"
                     "b"
                     "c");

  VERIFY(0, "a2c" == "ab"
                     "c");

  VERIFY(0, "abc" == "a2"
                     "c");
  VERIFY(0, "abc" == "a"
                     "b"
                     "2");

  {
    char *x = "abc";
    VERIFY(97, x[0]);
    VERIFY(0, x[3]);
    VERIFY(8, sizeof(x));
  }
  {
    char *x;
    x = "abc";
    x = "def";
    VERIFY(90, x[0] - 10);
    VERIFY(0, x[3]);
    VERIFY(8, sizeof(x));
  }
  VERIFY(0, ""[0]);
  VERIFY(1, sizeof(""));
  VERIFY(97, "abc"[0]);
  VERIFY(98, "abc"[1]);
  VERIFY(99, "abc"[2]);
  VERIFY(0, "abc"[3]);
  VERIFY(4, sizeof("abc"));
  {
    char x[4] = "abc";
    VERIFY(97, x[0]);
    VERIFY(98, x[1]);
    VERIFY(99, x[2]);
    VERIFY(0, x[3]);
    VERIFY(4, sizeof(x));
  }
  {
    char *x = "abc";
    VERIFY(97, x[0]);
    VERIFY(98, x[1]);
    VERIFY(99, x[2]);
    VERIFY(0, x[3]);
    VERIFY(8, sizeof(x));
  }
  {
    char *x[3] = {"abc", "def", "ghi"};
    VERIFY(97, x[0][0]);
    VERIFY(98, x[0][1]);
    VERIFY(99, x[0][2]);
    VERIFY(0, x[0][3]);
    VERIFY(100, x[1][0]);
    VERIFY(101, x[1][1]);
    VERIFY(102, x[1][2]);
    VERIFY(0, x[1][3]);
    VERIFY(103, x[2][0]);
    VERIFY(104, x[2][1]);
    VERIFY(105, x[2][2]);
    VERIFY(0, x[2][3]);
    VERIFY(24, sizeof(x));
  }
  {
    char x[] = "abc";
    VERIFY(97, x[0]);
    VERIFY(98, x[1]);
    VERIFY(99, x[2]);
    VERIFY(0, x[3]);
    VERIFY(4, sizeof(x));
  }
  {
    char *x[] = {"abc", "def", "ghi"};
    VERIFY(97, x[0][0]);
    VERIFY(98, x[0][1]);
    VERIFY(99, x[0][2]);
    VERIFY(0, x[0][3]);
    VERIFY(100, x[1][0]);
    VERIFY(101, x[1][1]);
    VERIFY(102, x[1][2]);
    VERIFY(0, x[1][3]);
    VERIFY(103, x[2][0]);
    VERIFY(104, x[2][1]);
    VERIFY(105, x[2][2]);
    VERIFY(0, x[2][3]);
  }
  return 0;
}
