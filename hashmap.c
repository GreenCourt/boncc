#include "hashmap.h"
#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <string.h>

static int strhash(char *s) {
  int len = strlen(s);
  int hash = 0;
  for (int i = 0; i < len; ++i)
    hash = (/* prime number */ 31 * hash + s[i]) % HASH_RANGE;
  return hash;
}

HashMap *new_hashmap() {
  HashMap *map = calloc(1, sizeof(HashMap));
  map->iter_tail = &map->iter_head;
  return map;
}

static HashMapItem *new_item(char *key, void *value) {
  HashMapItem *item = calloc(1, sizeof(HashMapItem));
  item->key = key;
  item->value = value;
  item->hash = strhash(key);
  item->next = NULL;
  item->iter_prev = NULL;
  item->iter_next = NULL;
  return item;
}

bool hashmap_contains(HashMap *map, char *key) {
  HashMapItem *item = map->table[strhash(key)].next;
  while (item) {
    if (strcmp(key, item->key) == 0)
      return true;
    item = item->next;
  }
  return false;
}

void *hashmap_get(HashMap *map, char *key) {
  int hash = strhash(key);
  assert(map->size);
  HashMapItem *item = map->table[hash].next;
  while (item) {
    if (strcmp(key, item->key) == 0)
      return item->value;
    item = item->next;
  }
  assert(false);
  return NULL;
}

void hashmap_set(HashMap *map, char *key, void *value) {
  int hash = strhash(key);
  HashMapItem *item = &map->table[hash];
  while (item->next) {
    item = item->next;
    if (strcmp(key, item->key) == 0) {
      item->value = value;
      return;
    }
  }

  item->next = new_item(key, value);
  map->size++;
  item = item->next;

  // add the new item to the doubly linked-list for iterators
  map->iter_tail->iter_next = item;
  item->iter_prev = map->iter_tail;
  map->iter_tail = item;
  assert(map->iter_tail->iter_next == NULL);
}

void hashmap_erase(HashMap *map, char *key) {
  int hash = strhash(key);
  assert(map->size);

  HashMapItem *item = &map->table[hash];
  while (item->next) {
    if (strcmp(key, item->next->key) == 0) {
      HashMapItem *removed = item->next;
      item->next = item->next->next;
      map->size--;

      // remove from the doubly linked-list for iterators
      removed->iter_prev->iter_next = removed->iter_next;
      if (removed == map->iter_tail)
        map->iter_tail = removed->iter_prev;
      else
        removed->iter_next->iter_prev = removed->iter_prev;
      assert(map->iter_tail->iter_next == NULL);
      return;
    }
    item = item->next;
  }
  assert(false);
}

HashMapItem *hashmap_begin(HashMap *map) { return map->iter_head.iter_next; }
HashMapItem *hashmap_next(HashMapItem *item) { return item->iter_next; }
