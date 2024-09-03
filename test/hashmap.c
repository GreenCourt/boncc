#include "../hashmap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main() {
  HashMap *map = new_hashmap();

  assert(!hashmap_contains(map, "aaa"));

  hashmap_set(map, "bb", "BB");
  assert(map->size == 1);
  hashmap_set(map, "aa", "AA");
  assert(map->size == 2);
  hashmap_set(map, "CC", "cc");
  assert(map->size == 3);

  assert(strcmp((char *)hashmap_get(map, "aa"), "AA") == 0);
  assert(strcmp((char *)hashmap_get(map, "bb"), "BB") == 0);
  assert(strcmp((char *)hashmap_get(map, "CC"), "cc") == 0);

  assert(hashmap_contains(map, "aa"));
  assert(hashmap_contains(map, "bb"));
  assert(hashmap_contains(map, "CC"));

  assert(!hashmap_contains(map, "AA"));
  assert(!hashmap_contains(map, "BB"));
  assert(!hashmap_contains(map, "cc"));

  hashmap_set(map, "CC", "xx");
  assert(strcmp((char *)hashmap_get(map, "CC"), "xx") == 0);
  assert(map->size == 3);

  hashmap_set(map, "CC", "cc");
  assert(strcmp((char *)hashmap_get(map, "CC"), "cc") == 0);
  assert(map->size == 3);

  {
    int a = 0, b = 0, c = 0;
    for (HashMapItem *i = hashmap_begin(map); i; i = hashmap_next(i)) {
      if (strcmp(i->key, "aa") == 0) {
        a++;
        assert(strcmp(i->value, "AA") == 0);
      } else if (strcmp(i->key, "bb") == 0) {
        b++;
        assert(strcmp(i->value, "BB") == 0);
      } else if (strcmp(i->key, "CC") == 0) {
        c++;
        assert(strcmp(i->value, "cc") == 0);
      } else {
        assert(false);
      }
    }
    assert(a == 1);
    assert(b == 1);
    assert(c == 1);
  }

  hashmap_erase(map, "bb");
  assert(map->size == 2);
  assert(!hashmap_contains(map, "bb"));

  hashmap_set(map, "DD", "dd");
  assert(map->size == 3);
  assert(hashmap_contains(map, "DD"));
  assert(strcmp((char *)hashmap_get(map, "DD"), "dd") == 0);

  {
    int a = 0, c = 0, d = 0;
    for (HashMapItem *i = hashmap_begin(map); i; i = hashmap_next(i)) {
      if (strcmp(i->key, "aa") == 0) {
        a++;
        assert(strcmp(i->value, "AA") == 0);
      } else if (strcmp(i->key, "CC") == 0) {
        c++;
        assert(strcmp(i->value, "cc") == 0);
      } else if (strcmp(i->key, "DD") == 0) {
        d++;
        assert(strcmp(i->value, "dd") == 0);
      } else {
        assert(false);
      }
    }
    assert(a == 1);
    assert(c == 1);
    assert(d == 1);
  }

  hashmap_erase(map, "CC");
  assert(map->size == 2);
  assert(!hashmap_contains(map, "CC"));

  hashmap_erase(map, "DD");
  assert(map->size == 1);
  assert(!hashmap_contains(map, "DD"));

  hashmap_erase(map, "aa");
  assert(map->size == 0);
  assert(!hashmap_contains(map, "aa"));

  assert(hashmap_begin(map) == NULL);

  {
    // register many values to test hash collision
    for (int i = 0; i < 2000; ++i) {
      char *key = calloc(20, sizeof(char));
      int *value = malloc(sizeof(int));
      sprintf(key, "%d", i);
      *value = i;
      hashmap_set(map, key, value);
      assert(map->size == i + 1);
    }

    char str[20];
    for (int i = 0; i < 2000; ++i) {
      sprintf(str, "%d", i);
      assert(hashmap_contains(map, str));
      assert(*(int *)hashmap_get(map, str) == i);
    }

    int *cnt = calloc(2000, sizeof(int));
    for (HashMapItem *i = hashmap_begin(map); i; i = hashmap_next(i)) {
      int value = *(int *)i->value;
      sprintf(str, "%d", value);
      assert(strcmp(i->key, str) == 0);
      cnt[value]++;
    }
    for (int i = 0; i < 2000; ++i)
      assert(cnt[i] == 1);
    free(cnt);

    int size = 2000;
    for (int i = 0; i < 2000; i += 2) {
      sprintf(str, "%d", i);
      hashmap_erase(map, str);
      size--;
      assert(map->size == size);
    }

    {
      // check if the insertion order is kept
      int v = 1;
      for (HashMapItem *i = hashmap_begin(map); i; i = hashmap_next(i)) {
        int value = *(int *)i->value;
        assert(value == v);
        v += 2;
        sprintf(str, "%d", value);
        assert(strcmp(i->key, str) == 0);
      }
    }

    for (int i = 1; i < 2000; i += 2) {
      sprintf(str, "%d", i);
      hashmap_erase(map, str);
      size--;
      assert(map->size == size);
    }
    assert(map->size == 0);
  }
  return 0;
}
