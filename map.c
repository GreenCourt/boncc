#include "boncc.h"
#include <assert.h>
#include <stdlib.h>
#include <string.h>

typedef struct KeyValue KeyValue;
struct KeyValue {
  char *ident;
  void *val;
};

Map *new_map() { return new_vector(0, sizeof(KeyValue *)); }

void *map_geti(Map *map, int idx) {
  assert(0 <= idx && idx < map->size);
  KeyValue *e = *(KeyValue **)vector_get(map, idx);
  assert(e);
  return e->val;
}

void *map_get(Map *map, char *key) {
  int sz = map->size;
  for (int i = 0; i < sz; ++i) {
    KeyValue *e = *(KeyValue **)vector_get(map, i);
    if (strcmp(e->ident, key) == 0)
      return e->val;
  }
  return NULL;
}

void map_push(Map *map, char *key, void *val) {
  assert(map_get(map, key) == NULL);
  KeyValue *e = calloc(1, sizeof(KeyValue));
  e->ident = key;
  e->val = val;
  vector_push(map, &e);
}

void map_erase(Map *map, char *key) {
  int sz = map->size;
  for (int i = 0; i < sz; ++i) {
    KeyValue *e = *(KeyValue **)vector_get(map, i);
    if (strcmp(e->ident, key) == 0) {
      KeyValue *last = *(KeyValue **)vector_last(map);
      vector_set(map, i, &last);
      vector_pop(map);
      return;
    }
  }
  assert(false);
}
