#include "boncc.h"
#include <assert.h>
#include <stdlib.h>

typedef struct KeyValue KeyValue;
struct KeyValue {
  Ident *ident;
  void *val;
};

Map *new_map() {
  return new_vector(0, sizeof(KeyValue *));
}

void *map_geti(Map *map, int idx) {
  assert(0 <= idx && idx < map->size);
  KeyValue *e = *(KeyValue **)vector_get(map, idx);
  assert(e);
  return e->val;
}

void *map_get(Map *map, Ident *key) {
  int sz = map->size;
  for (int i = 0; i < sz; ++i) {
    KeyValue *e = *(KeyValue **)vector_get(map, i);
    if (same_ident(e->ident, key))
      return e->val;
  }
  return NULL;
}

void map_push(Map *map, Ident *key, void *val) {
  assert(map_get(map, key) == NULL);
  KeyValue *e = calloc(1, sizeof(KeyValue));
  e->ident = key;
  e->val = val;
  vector_push(map, &e);
}
