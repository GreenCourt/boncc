void verify(int expected, int actual, char *file_name, int line_number) {
  if (expected == actual)
    return;
  printf("%s:%d: %d expected but got %d", file_name, line_number, expected, actual);
  puts("");
  exit(1);
}
