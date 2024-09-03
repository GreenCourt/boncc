#undef NDEBUG
#include "../vector.h"
#include <assert.h>
#include <stdlib.h>

typedef struct VectorTestStruct {
  char c;
  long long l;
  int i;
} VectorTestStruct;

void vec_test_set_get() {
  Vector *vec = new_vector(3, sizeof(int));

  int val;
  val = 0;
  vector_set(vec, 0, &val);
  val = 2;
  vector_set(vec, 1, &val);
  val = 4;
  vector_set(vec, 2, &val);

  assert(*(int *)vector_get(vec, 0) == 0);
  assert(*(int *)vector_get(vec, 1) == 2);
  assert(*(int *)vector_get(vec, 2) == 4);

  vector_resize(vec, 5);

  val = 6;
  vector_set(vec, 3, &val);
  val = 8;
  vector_set(vec, 4, &val);

  assert(*(int *)vector_get(vec, 0) == 0);
  assert(*(int *)vector_get(vec, 1) == 2);
  assert(*(int *)vector_get(vec, 2) == 4);
  assert(*(int *)vector_get(vec, 3) == 6);
  assert(*(int *)vector_get(vec, 4) == 8);

  free(vec->data);
  free(vec);
}

void vec_test_push_pop() {
  Vector *vec = new_vector(0, sizeof(VectorTestStruct));

  int n = 1000;
  for (int i = 0; i < n; ++i) {
    VectorTestStruct t = {(char)('a' + i % 26), i * 2, i};
    vector_push(vec, &t);

    VectorTestStruct s = *(VectorTestStruct *)vector_last(vec);
    assert(s.c == t.c);
    assert(s.i == t.i);
    assert(s.l == t.l);

    vector_pop(vec);
    vector_push(vec, &t);
  }

  for (int i = n - 1; i >= 0; --i) {
    VectorTestStruct t = {(char)('a' + i % 26), i * 2, i};
    VectorTestStruct s = *(VectorTestStruct *)vector_last(vec);
    assert(s.c == t.c);
    assert(s.i == t.i);
    assert(s.l == t.l);
    vector_pop(vec);
  }

  free(vec->data);
  free(vec);
}

void vec_test_seti_geti() {
  Vector *vec = new_vector(3, sizeof(int));

  vector_seti(vec, 0, 0);
  vector_seti(vec, 1, 2);
  vector_seti(vec, 2, 4);

  assert(vector_geti(vec, 0) == 0);
  assert(vector_geti(vec, 1) == 2);
  assert(vector_geti(vec, 2) == 4);

  vector_resize(vec, 5);

  vector_seti(vec, 3, 6);
  vector_seti(vec, 4, 8);

  assert(vector_geti(vec, 0) == 0);
  assert(vector_geti(vec, 1) == 2);
  assert(vector_geti(vec, 2) == 4);
  assert(vector_geti(vec, 3) == 6);
  assert(vector_geti(vec, 4) == 8);

  free(vec->data);
  free(vec);
}

void vec_test_pushi_pop() {
  Vector *vec = new_vector(0, sizeof(int));

  int n = 1000;
  for (int i = 0; i < n; ++i) {
    int val = i * 2;
    vector_pushi(vec, val);
    assert(vector_lasti(vec) == val);
  }

  for (int i = n - 1; i >= 0; --i) {
    int val = i * 2;
    assert(vector_lasti(vec) == val);
    vector_pop(vec);
    vector_pushi(vec, 12);
    assert(vector_lasti(vec) == 12);
    vector_pop(vec);
  }

  free(vec->data);
  free(vec);
}

int main() {
  vec_test_set_get();
  vec_test_push_pop();
  vec_test_seti_geti();
  vec_test_pushi_pop();
}
