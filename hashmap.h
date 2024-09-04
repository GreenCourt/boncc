#include <stdbool.h>
#define HASH_RANGE 1024

typedef struct HashMapItem HashMapItem;
struct HashMapItem {
  char *key;
  void *value;
  int hash;
  HashMapItem *hash_next; // single linked-list for hash collision

  // doubly linked-list to iterate items
  // this list preserves insertion order
  HashMapItem *iter_prev, *iter_next;
};

typedef struct {
  // each element of table[] is a dummay head of linked-list
  HashMapItem table[HASH_RANGE];
  int size;

  // head and tail of doubly linked-list to iterate items
  HashMapItem *iter_head, *iter_tail;
} HashMap;

HashMap *new_hashmap();
bool hashmap_contains(HashMap *map, char *key);
void *hashmap_get(HashMap *map, char *key);
void hashmap_set(HashMap *map, char *key, void *value);
void hashmap_erase(HashMap *map, char *key);
