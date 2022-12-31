int main() {
  // verify(5, ({
  //         int a;
  //         a = 3;
  //         if (a == 3)
  //           a + 2;
  //         else
  //           a;
  //       }),
  //       __FILE__, __LINE__);
  // verify(3, ({
  //         int a;
  //         a = 3;
  //         if (a < 3)
  //           a + 2;
  //         else
  //           a;
  //       }),
  //       __FILE__, __LINE__);
  // verify(18, ({
  //         int b;
  //         b = 0;
  //         int a;
  //         for (a = 0; a < 10; a = a + 1)
  //           b = a * 2;
  //         b;
  //       }),
  //       __FILE__, __LINE__);
  // verify(10, ({
  //         int a;
  //         a = 0;
  //         for (; a < 10;)
  //           a = a + 1;
  //         a;
  //       }),
  //       __FILE__, __LINE__);
  // verify(20, ({
  //         int a;
  //         a = 0;
  //         int b;
  //         b = 0;
  //         for (; a < 10;) {
  //           a = a + 1;
  //           b = b + 2;
  //         }
  //         b;
  //       }),
  //       __FILE__, __LINE__);
  return 0;
}
