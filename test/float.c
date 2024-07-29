#include "common.h"
#include <stdbool.h>

bool is_close(double expected, double actual, double tolerance) {
  double abs_err = expected > actual ? expected - actual : actual - expected;
  if (abs_err < tolerance)
    return true;
  if ((expected > 0 ? expected : -expected) < tolerance)
    return false;
  double rel_err = abs_err / (expected > 0 ? expected : -expected);
  return rel_err < tolerance;
}

bool is_close_f(float expected, float actual, float tolerance) {
  float abs_err = expected > actual ? expected - actual : actual - expected;
  if (abs_err < tolerance)
    return true;
  if ((expected > 0 ? expected : -expected) < tolerance)
    return false;
  float rel_err = abs_err / (expected > 0 ? expected : -expected);
  return rel_err < tolerance;
}

double global_double = 1.2;
float global_float = 1.2;

int main() {
  verify(0, global_double == global_float, __FILE__, __LINE__);
  verify(1, global_double != global_float, __FILE__, __LINE__);

  {
    double x = 5.2;
    double y = 3.1;
    double z = y - x;
    verify(1, is_close(-2.1, z, 1e-9), __FILE__, __LINE__);
    verify(1, x > y, __FILE__, __LINE__);
    verify(0, x < y, __FILE__, __LINE__);
    verify(1, x >= y, __FILE__, __LINE__);
    verify(0, x <= y, __FILE__, __LINE__);
    verify(0, x == y, __FILE__, __LINE__);
    verify(1, x != y, __FILE__, __LINE__);
    verify(0, x - y > y, __FILE__, __LINE__);
    verify(0, 2 >= y, __FILE__, __LINE__);
    verify(1, 2 <= y, __FILE__, __LINE__);
    verify(1, x - y > 1, __FILE__, __LINE__);
    verify(1, x - y >= 1, __FILE__, __LINE__);
    verify(0, x - y > y, __FILE__, __LINE__);
    verify(1, x - y >= z, __FILE__, __LINE__);
    verify(1, x - y < y, __FILE__, __LINE__);
    verify(1, x - y <= y, __FILE__, __LINE__);
    verify(1, x + y > y - x, __FILE__, __LINE__);
    verify(0, x + y > y * x, __FILE__, __LINE__);
    verify(1, is_close(4.26, y + 1.16, 1e-9), __FILE__, __LINE__);
    verify(1, is_close(2.97, x - 2.23, 1e-9), __FILE__, __LINE__);
    verify(1, is_close(6.2, y * 2.0, 1e-9), __FILE__, __LINE__);
    verify(1, is_close(2.6, x / 2, 1e-9), __FILE__, __LINE__);

    verify(1, 6e0 == 6., __FILE__, __LINE__);
    verify(1, 1e-3 == .001, __FILE__, __LINE__);
    verify(1, is_close(0x0.12ap4, 1.1640625, 1e-9), __FILE__, __LINE__);
    verify(1, is_close(0x.12ap4, 1.1640625, 1e-9), __FILE__, __LINE__);

    verify(1, 0 || 1.2, __FILE__, __LINE__);
    verify(0, 0 && 1.2, __FILE__, __LINE__);
    verify(1, 1 && 1.2, __FILE__, __LINE__);
    verify(0, 1 && 0.0, __FILE__, __LINE__);
    verify(0, 0 || 0.0, __FILE__, __LINE__);
    verify(1, 1 || 0.0, __FILE__, __LINE__);
    verify(0, !1.2, __FILE__, __LINE__);
    verify(1, !0.0, __FILE__, __LINE__);

    verify(1, global_double == 1.2, __FILE__, __LINE__);
    verify(1, is_close(1.44, global_double * 1.2, 1e-9), __FILE__, __LINE__);
  }

  {
    float x = 5.2;
    float y = 3.1f;
    float z = y - x;
    verify(1, is_close_f(-2.1f, z, 1e-9), __FILE__, __LINE__);
    verify(1, x > y, __FILE__, __LINE__);
    verify(0, x < y, __FILE__, __LINE__);
    verify(1, x >= y, __FILE__, __LINE__);
    verify(0, x <= y, __FILE__, __LINE__);
    verify(0, x == y, __FILE__, __LINE__);
    verify(1, x != y, __FILE__, __LINE__);
    verify(0, x - y > y, __FILE__, __LINE__);
    verify(0, 2 >= y, __FILE__, __LINE__);
    verify(1, 2 <= y, __FILE__, __LINE__);
    verify(1, x - y > 1, __FILE__, __LINE__);
    verify(1, x - y >= 1, __FILE__, __LINE__);
    verify(0, x - y > y, __FILE__, __LINE__);
    verify(1, x - y >= z, __FILE__, __LINE__);
    verify(1, x - y < y, __FILE__, __LINE__);
    verify(1, x - y <= y, __FILE__, __LINE__);
    verify(1, x + y > y - x, __FILE__, __LINE__);
    verify(0, x + y > y * x, __FILE__, __LINE__);
    verify(1, is_close_f(4.26f, y + 1.16f, 1e-6), __FILE__, __LINE__);
    verify(1, is_close_f(2.97, x - 2.23F, 1e-6F), __FILE__, __LINE__);
    verify(1, is_close_f(6.2f, y * 2.0f, 1e-6), __FILE__, __LINE__);
    verify(1, is_close_f(2.6, x / 2, 1e-6f), __FILE__, __LINE__);

    verify(1, 6e0 == 6., __FILE__, __LINE__);
    verify(1, 1e-3 == .001, __FILE__, __LINE__);
    verify(1, is_close_f(0x0.12ap4f, 1.1640625F, 1e-9f), __FILE__, __LINE__);
    verify(1, is_close_f(0x.12ap4F, 1.1640625f, 1e-9), __FILE__, __LINE__);

    verify(1, 0 || 1.2f, __FILE__, __LINE__);
    verify(0, 0 && 1.2f, __FILE__, __LINE__);
    verify(1, 1 && 1.2f, __FILE__, __LINE__);
    verify(0, 1 && 0.0f, __FILE__, __LINE__);
    verify(0, 0 || 0.0f, __FILE__, __LINE__);
    verify(1, 1 || 0.0f, __FILE__, __LINE__);
    verify(0, !1.2f, __FILE__, __LINE__);
    verify(1, !0.0f, __FILE__, __LINE__);

    verify(1, global_float == 1.2f, __FILE__, __LINE__);
    verify(1, is_close_f(1.44f, global_float * 1.2f, 1e-9), __FILE__, __LINE__);
  }

  return 0;
}
