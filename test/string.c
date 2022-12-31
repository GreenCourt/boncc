int main() {
  // verify(97, ({
  //         char *x;
  //         x = "abc";
  //         x[0];
  //       }),
  //       __FILE__, __LINE__);
  // verify(90, ({
  //         char *x;
  //         x = "abc";
  //         x = "def";
  //         x[0] - 10;
  //       }),
  //       __FILE__, __LINE__);
  verify(0, ""[0], __FILE__, __LINE__);
  verify(1, sizeof(""), __FILE__, __LINE__);
  verify(97, "abc"[0], __FILE__, __LINE__);
  verify(98, "abc"[1], __FILE__, __LINE__);
  verify(99, "abc"[2], __FILE__, __LINE__);
  verify(0, "abc"[3], __FILE__, __LINE__);
  verify(4, sizeof("abc"), __FILE__, __LINE__);
  verify(4, sizeof("abc"), __FILE__, __LINE__);
  return 0;
}
