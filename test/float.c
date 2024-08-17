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
double global_double_constexpr = 3.6 / 2 + 1.2 * 2 - 1;
double global_float_constexpr = 3.6f / 2 + 1.2f * 2 - 1;

int main() {
  verify(0, global_double == global_float);
  verify(1, global_double != global_float);
  verify(1, is_close(global_double_constexpr, 3.2, 1e-9));
  verify(1, is_close_f(global_float_constexpr, 3.2, 1e-6));

  {
    double x = 5.2;
    double y = 3.1;
    double z = y - x;
    verify(1, is_close(-2.1, z, 1e-9));
    verify(1, x > y);
    verify(0, x < y);
    verify(1, x >= y);
    verify(0, x <= y);
    verify(0, x == y);
    verify(1, x != y);
    verify(0, x - y > y);
    verify(0, 2 >= y);
    verify(1, 2 <= y);
    verify(1, x - y > 1);
    verify(1, x - y >= 1);
    verify(0, x - y > y);
    verify(1, x - y >= z);
    verify(1, x - y < y);
    verify(1, x - y <= y);
    verify(1, x + y > y - x);
    verify(0, x + y > y * x);
    verify(1, is_close(4.26, y + 1.16, 1e-9));
    verify(1, is_close(2.97, x - 2.23, 1e-9));
    verify(1, is_close(6.2, y * 2.0, 1e-9));
    verify(1, is_close(2.6, x / 2, 1e-9));

    verify(1, 6e0 == 6.);
    verify(1, 1e-3 == .001);
    verify(1, is_close(0x0.12ap4, 1.1640625, 1e-9));
    verify(1, is_close(0x.12ap4, 1.1640625, 1e-9));

    verify(1, 0 || 1.2);
    verify(0, 0 && 1.2);
    verify(1, 1 && 1.2);
    verify(0, 1 && 0.0);
    verify(0, 0 || 0.0);
    verify(1, 1 || 0.0);
    verify(0, !1.2);
    verify(1, !0.0);

    verify(1, global_double == 1.2);
    verify(1, is_close(1.44, global_double * 1.2, 1e-9));
  }

  {
    float x = 5.2;
    float y = 3.1f;
    float z = y - x;
    verify(1, is_close_f(-2.1f, z, 1e-9));
    verify(1, x > y);
    verify(0, x < y);
    verify(1, x >= y);
    verify(0, x <= y);
    verify(0, x == y);
    verify(1, x != y);
    verify(0, x - y > y);
    verify(0, 2 >= y);
    verify(1, 2 <= y);
    verify(1, x - y > 1);
    verify(1, x - y >= 1);
    verify(0, x - y > y);
    verify(1, x - y >= z);
    verify(1, x - y < y);
    verify(1, x - y <= y);
    verify(1, x + y > y - x);
    verify(0, x + y > y * x);
    verify(1, is_close_f(4.26f, y + 1.16f, 1e-6));
    verify(1, is_close_f(2.97, x - 2.23F, 1e-6F));
    verify(1, is_close_f(6.2f, y * 2.0f, 1e-6));
    verify(1, is_close_f(2.6, x / 2, 1e-6f));

    verify(1, 6e0 == 6.);
    verify(1, 1e-3 == .001);
    verify(1, is_close_f(0x0.12ap4f, 1.1640625F, 1e-9f));
    verify(1, is_close_f(0x.12ap4F, 1.1640625f, 1e-9));

    verify(1, 0 || 1.2f);
    verify(0, 0 && 1.2f);
    verify(1, 1 && 1.2f);
    verify(0, 1 && 0.0f);
    verify(0, 0 || 0.0f);
    verify(1, 1 || 0.0f);
    verify(0, !1.2f);
    verify(1, !0.0f);

    verify(1, global_float == 1.2f);
    verify(1, is_close_f(1.44f, global_float * 1.2f, 1e-9));
  }

  return 0;
}
