#include <hash_map.h>
#include <math.h>
#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#define LOAD_FACTOR 0.75f
#define MINIMUM_CAPACITY 12
#define GROW_FACTOR 2

struct HashMap *create_hash_map(map_size_t (*hash_f) (void *),
                                bool (*eq_f) (void *, void *),
                                map_size_t initial_capacity) {
  struct HashMap *map = malloc(sizeof(struct HashMap));
  if (map == NULL) {
    return NULL;
  }
  if (initial_capacity < MINIMUM_CAPACITY) {
    initial_capacity = MINIMUM_CAPACITY;
  }
  map->hash_f = hash_f;
  map->eq_f = eq_f;
  map_size_t buckets_number = 16;
  while (buckets_number * LOAD_FACTOR < initial_capacity) {
    buckets_number <<= 1;
  }
  map->capacity = buckets_number * LOAD_FACTOR;
  map->buckets = malloc(buckets_number * sizeof(struct HashMapNode));
  if (map->buckets == NULL) {
    free(map);
    return NULL;
  }
  for (size_t i = 0; i < buckets_number; i++) {
    (map->buckets + i)->node=NULL;
  }
  map->buckets_number = buckets_number;
  map->entries_number = 0;
  return map;
}

void __hash_map_put_node__(struct HashMap *map, struct HashMapNode *node) {
  void *key = node->key;
  map_size_t index = map->hash_f(key) & map->buckets_number - 1;
  struct HashMapBucket *bucket = map->buckets + index;
  struct HashMapNode *next_node = bucket->node;
  node->next = next_node;
  bucket->node = node;
  map->entries_number += 1;
}

void __hash_map_resize__(struct HashMap *map) {
  struct HashMapBucket *current_buckets = map->buckets;
  map_size_t current_buckets_number = map->buckets_number;
  map_size_t new_buckets_number = GROW_FACTOR * current_buckets_number;
  map_size_t new_capacity = new_buckets_number * LOAD_FACTOR;

  map->buckets = malloc(new_buckets_number * sizeof(struct HashMapNode));
  for (size_t i = 0; i < new_buckets_number; i++) {
    (map->buckets + i)->node=NULL;
  }
  map->buckets_number = new_buckets_number;
  map->entries_number = 0;
  map->capacity = new_capacity;
  for (size_t i = 0; i < current_buckets_number; i++) {
    struct HashMapBucket *bucket = current_buckets + i;
    struct HashMapNode *node = bucket->node;
    struct HashMapNode *next_node;
    while (node != NULL) {
      next_node = node->next;
      __hash_map_put_node__(map, node);
      node = next_node;
    }
  }

  free(current_buckets);
}

struct HashMapNode *__hash_map_get_node__(struct HashMap *map, void *key) {
  map_size_t index = map->hash_f(key) & map->buckets_number - 1;
  struct HashMapBucket *bucket = map->buckets + index;
  struct HashMapNode *node = bucket->node;
  while (node != NULL) {
    if (map->eq_f(node->key, key)) {
      return node;
    }
    node = node->next;
  }
  return NULL;
}

void *hash_map_put(struct HashMap *map, void *key, void *value) {
  void *result = NULL;
  struct HashMapNode *node = __hash_map_get_node__(map, key);
  if (node != NULL) {
    result = node->value;
    node->value = value;
    return result;
  } else {
    map_size_t index = map->hash_f(key) & map->buckets_number - 1;
    struct HashMapBucket *bucket = map->buckets + index;
    node = malloc(sizeof(struct HashMapNode));
    node->key = key;
    node->value = value;
    node->next = bucket->node;
    bucket->node = node;
    map->entries_number += 1;
    if (map->entries_number > map->capacity) {
      __hash_map_resize__(map);
    }
    return NULL;
  }
}

bool hash_map_contains(struct HashMap *map, void *key) {
  struct HashMapNode *node = __hash_map_get_node__(map, key);
  return node != NULL;
}

void *hash_map_get(struct HashMap *map, void *key) {
  struct HashMapNode *node = __hash_map_get_node__(map, key);
  return node != NULL ? node->value : NULL;
}

void *hash_map_remove(struct HashMap *map, void *key) {
  map_size_t index = map->hash_f(key) & map->buckets_number - 1;
  struct HashMapBucket *bucket = map->buckets + index;
  struct HashMapNode *prev = NULL;
  struct HashMapNode *node = bucket->node;
  while (node != NULL) {
    if (map->eq_f(node->key, key)) {
      if (prev == NULL) {
        bucket->node = node->next;
      } else {
        prev->next = node->next;
      }
      map->entries_number -= 1;
      void *result = node->value;
      free(node);
      return result;
    }
    prev = node;
    node = node->next;
  }
  return NULL;
}

void free_hash_map(struct HashMap *map) {
  for (size_t i = 0; i < map->buckets_number; i++) {
    struct HashMapNode *node = (map->buckets + i)->node;
    while (node != NULL) {
      struct HashMapNode *next_node = node->next;
      free(node);
      node = next_node;
    }
  }
  free(map->buckets);
  free(map);
}
