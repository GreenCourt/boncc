#define verify(exp, act) verify_(exp, act, __FILE__, __LINE__)
void verify_(long long expected, long long actual, char *file_name,
             int line_number);
