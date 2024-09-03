#include <stdbool.h>
#define HASH_RANGE 1024

typedef struct HashMapItem HashMapItem;
struct HashMapItem {
  char *key;
  void *value;
  int hash;
  HashMapItem *next; // single linked-list for hash collision

  // doubly linked-list for iterators
  // this list preserves insertion order
  HashMapItem *iter_prev;
  HashMapItem *iter_next;
};

typedef struct {
  // each element of table[] is a dummay head of linked-list
  HashMapItem table[HASH_RANGE];
  int size;
  HashMapItem iter_head;
  HashMapItem *iter_tail;
} HashMap;

HashMap *new_hashmap();
bool hashmap_contains(HashMap *map, char *key);
void *hashmap_get(HashMap *map, char *key);
void hashmap_set(HashMap *map, char *key, void *value);
void hashmap_erase(HashMap *map, char *key);

HashMapItem *hashmap_begin(HashMap *map);
HashMapItem *hashmap_next(HashMapItem *item);
