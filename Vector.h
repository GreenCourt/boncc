#ifndef ____VECTOR_H____
#define ____VECTOR_H____

#include <stddef.h>
#define VECTOR_MINIMUM_RESERVATION 16

typedef struct Vector {
  void *data;
  size_t data_size;
  int size;
  int reserved;
} Vector;

Vector *new_vector(int size, size_t data_size);
void vector_resize(Vector *vec, int size);

void vector_set(Vector *vec, int index, void *p);
void *vector_get(Vector *vec, int index);

void vector_push(Vector *vec, void *p);
void vector_pop(Vector *vec);
void *vector_last(Vector *vec);

void vector_seti(Vector *vec, int index, int val);
int vector_geti(Vector *vec, int index);
void vector_pushi(Vector *vec, int val);
int vector_lasti(Vector *vec);

#endif
