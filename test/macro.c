int line1 = __LINE__;
int line2 = __LINE__;

void verify(int expected, int actual, char *file_name, int line_number);

#define FOO 12
#define BAR "bar"
#define A0 1
#define A1 2
#define B0 4
#define B1 8
#define A01 A0 + A1
#define B01 B0 + B1
#define SUM A01 + B01

int main() {
  verify(1, line1, __FILE__, __LINE__);
  verify(2, line2, __FILE__, __LINE__);
  verify('t', __FILE__[0], __FILE__, __LINE__);
  verify('e', __FILE__[1], __FILE__, __LINE__);
  verify('s', __FILE__[2], __FILE__, __LINE__);
  verify('t', __FILE__[3], __FILE__, __LINE__);
  verify('/', __FILE__[4], __FILE__, __LINE__);
  verify('m', __FILE__[5], __FILE__, __LINE__);
  verify('a', __FILE__[6], __FILE__, __LINE__);
  verify('c', __FILE__[7], __FILE__, __LINE__);
  verify('r', __FILE__[8], __FILE__, __LINE__);
  verify('o', __FILE__[9], __FILE__, __LINE__);
  verify('.', __FILE__[10], __FILE__, __LINE__);
  verify('c', __FILE__[11], __FILE__, __LINE__);
  verify('\0', __FILE__[12], __FILE__, __LINE__);
  verify(12, FOO, __FILE__, __LINE__);
  verify('b', BAR[0], __FILE__, __LINE__);
  verify('a', BAR[1], __FILE__, __LINE__);
  verify('r', BAR[2], __FILE__, __LINE__);
  verify(3, A01, __FILE__, __LINE__);
  verify(12, B01, __FILE__, __LINE__);
  verify(15, SUM, __FILE__, __LINE__);
  return 0;
}
