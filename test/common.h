#define VERIFY(exp, act) verify(exp, act, __FILE__, __LINE__)
void verify(long long expected, long long actual, char *file_name,
            int line_number);
