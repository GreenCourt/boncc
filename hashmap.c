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
  return map;
}

static HashMapItem *new_item(char *key, void *value) {
  HashMapItem *item = calloc(1, sizeof(HashMapItem));
  item->key = key;
  item->value = value;
  item->hash = strhash(key);
  item->hash_next = NULL;
  item->iter_prev = NULL;
  item->iter_next = NULL;
  return item;
}

bool hashmap_contains(HashMap *map, char *key) {
  HashMapItem *item = map->table[strhash(key)].hash_next;
  while (item) {
    if (strcmp(key, item->key) == 0)
      return true;
    item = item->hash_next;
  }
  return false;
}

void *hashmap_get(HashMap *map, char *key) {
  int hash = strhash(key);
  assert(map->size);
  HashMapItem *item = map->table[hash].hash_next;
  while (item) {
    if (strcmp(key, item->key) == 0)
      return item->value;
    item = item->hash_next;
  }
  assert(false);
  return NULL;
}

void hashmap_set(HashMap *map, char *key, void *value) {
  int hash = strhash(key);
  HashMapItem *item = &map->table[hash];
  while (item->hash_next) {
    item = item->hash_next;
    if (strcmp(key, item->key) == 0) {
      item->value = value;
      return;
    }
  }

  item->hash_next = new_item(key, value);
  map->size++;
  item = item->hash_next;

  // add the new item to the doubly linked-list for iterators
  if (map->size == 1) {
    assert(map->iter_head == NULL);
    assert(map->iter_tail == NULL);
    map->iter_head = map->iter_tail = item;
  } else {
    assert(map->iter_head != NULL);
    assert(map->iter_tail != NULL);
    map->iter_tail->iter_next = item;
    item->iter_prev = map->iter_tail;
    map->iter_tail = item;
  }
  assert(map->iter_head != NULL);
  assert(map->iter_tail != NULL);
  assert(map->iter_head->iter_prev == NULL);
  assert(map->iter_tail->iter_next == NULL);
}

void hashmap_erase(HashMap *map, char *key) {
  int hash = strhash(key);
  assert(map->size);

  HashMapItem *item = &map->table[hash];
  while (item->hash_next) {
    if (strcmp(key, item->hash_next->key) != 0) {
      item = item->hash_next;
      continue;
    }

    HashMapItem *removed = item->hash_next;
    item->hash_next = item->hash_next->hash_next;
    map->size--;

    // remove from the doubly linked-list for iterators
    if (map->size == 0) {
      assert(map->iter_head == removed);
      assert(map->iter_tail == removed);
      map->iter_head = map->iter_tail = NULL;
    } else if (removed == map->iter_head) {
      map->iter_head = removed->iter_next;
      map->iter_head->iter_prev = NULL;
    } else if (removed == map->iter_tail) {
      map->iter_tail = removed->iter_prev;
      map->iter_tail->iter_next = NULL;
    } else {
      assert(map->size >= 2);
      removed->iter_prev->iter_next = removed->iter_next;
      removed->iter_next->iter_prev = removed->iter_prev;
    }

    assert(map->iter_head == NULL || map->iter_head->iter_prev == NULL);
    assert(map->iter_tail == NULL || map->iter_tail->iter_next == NULL);
    return;
  }
  assert(false);
}
