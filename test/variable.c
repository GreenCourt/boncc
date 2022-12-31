int global_int1;
int global_int2;
int global_array3[3];
int global_array20[20];
int *global_int_pointer;

int main() {
  // verify(14, ({
  //         int a;
  //         a = 3;
  //         int b;
  //         b = 5 * 6 - 8;
  //         a + b / 2;
  //       }),
  //       __FILE__, __LINE__);
  // verify(14, ({
  //         int aaa;
  //         aaa = 3;
  //         int bb;
  //         bb = 5 * 6 - 8;
  //         aaa + bb / 2;
  //       }),
  //       __FILE__, __LINE__);
  // verify(5, ({
  //         int aaa;
  //         aaa = 3;
  //         int bb;
  //         bb = 5 * 6 - 8;
  //         aaa + bb / 2;
  //         aaa + 2;
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int x;
  //         sizeof(x);
  //       }),
  //       __FILE__, __LINE__);
  // verify(8, ({
  //         int *x;
  //         sizeof(x);
  //       }),
  //       __FILE__, __LINE__);
  // verify(8, ({
  //         int *x;
  //         sizeof(x + 3);
  //       }),
  //       __FILE__, __LINE__);
  // verify(4, ({
  //         int *x;
  //         sizeof(*x);
  //       }),
  //       __FILE__, __LINE__);
  verify(4, sizeof(sizeof(1)), __FILE__, __LINE__);
  verify(4, sizeof(1), __FILE__, __LINE__);
  verify(0, global_int1, __FILE__, __LINE__);
  verify(2, global_int1 + 2, __FILE__, __LINE__);
  global_int1 = 4;
  global_int2 = 3;
  verify(4, global_int1, __FILE__, __LINE__);
  verify(12, global_int1 * global_int2, __FILE__, __LINE__);
  verify(4, sizeof(global_int1), __FILE__, __LINE__);
  verify(12, sizeof(global_array3), __FILE__, __LINE__);
  global_array20[5] = 12;
  global_array20[11] = 14;
  verify(12, global_array20[5], __FILE__, __LINE__);
  verify(14, global_array20[11], __FILE__, __LINE__);

  global_int1 = 12;
  global_int_pointer = &global_int1;
  verify(12, *global_int_pointer, __FILE__, __LINE__);

  // verify(12, ({
  //         int y;
  //         global_int1 = 12;
  //         y = global_int1;
  //         y;
  //       }),
  //       __FILE__, __LINE__);
  // verify(20, ({
  //         char x;
  //         x = 20;
  //         x;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         char x[3];
  //         x[0] = -1;
  //         x[1] = 2;
  //         int y;
  //         y = 4;
  //         x[0] + y;
  //       }),
  //       __FILE__, __LINE__);
  // verify(6, ({
  //         char x[3];
  //         x[0] = -1;
  //         x[1] = 2;
  //         int y;
  //         y = 4;
  //         x[1] + y;
  //       }),
  //       __FILE__, __LINE__);
  // verify(1, ({
  //         char x[3];
  //         x[0] = -1;
  //         x[1] = 2;
  //         x[0] + x[1];
  //       }),
  //       __FILE__, __LINE__);
  return 0;
}
