#include "vector.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

Vector *new_vector(int size, size_t data_size) {
  Vector *vec = malloc(sizeof(Vector));
  vec->data_size = data_size;
  vec->size = size;
  vec->reserved = (size * 2 > VECTOR_MINIMUM_RESERVATION) ? size * 2 : VECTOR_MINIMUM_RESERVATION;
  vec->data = calloc(vec->reserved, vec->data_size);
  return vec;
}

void vector_resize(Vector *vec, int size) {
  vec->size = size;
  vec->reserved = (size * 2 > VECTOR_MINIMUM_RESERVATION) ? size * 2 : VECTOR_MINIMUM_RESERVATION;
  vec->data = realloc(vec->data, vec->reserved * vec->data_size);
}

void vector_set(Vector *vec, int index, void *p) {
  assert(index < vec->size);
  memcpy(vec->data + (index * vec->data_size), p, vec->data_size);
}

void *vector_get(Vector *vec, int index) {
  assert(index < vec->size);
  return vec->data + (index * vec->data_size);
}

void vector_push(Vector *vec, void *p) {
  if (vec->size == vec->reserved)
    vector_resize(vec, vec->size);
  vec->size++;
  vector_set(vec, vec->size - 1, p);
}

void vector_pop(Vector *vec) {
  assert(vec->size);
  vec->size--;
  if (vec->size * 4 < vec->reserved)
    vector_resize(vec, vec->size);
}

void *vector_last(Vector *vec) {
  assert(vec->size);
  return vector_get(vec, vec->size - 1);
}

void vector_seti(Vector *vec, int index, int val) {
  assert(index < vec->size);
  ((int *)vec->data)[index] = val;
}

int vector_geti(Vector *vec, int index) {
  assert(index < vec->size);
  return ((int *)vec->data)[index];
}

void vector_pushi(Vector *vec, int val) {
  if (vec->size == vec->reserved)
    vector_resize(vec, vec->size);
  vec->size++;
  vector_seti(vec, vec->size - 1, val);
}

int vector_lasti(Vector *vec) {
  assert(vec->size);
  return vector_geti(vec, vec->size - 1);
}
