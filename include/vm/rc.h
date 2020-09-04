#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>

typedef void * rcmap_key_t;
typedef void * refcounted_t;

//! https://github.com/petewarden/c_hashmap/blob/master/hashmap.c

#define RC_INITIAL_SIZE (256)
#define RC_MAX_CHAIN_LENGTH (8)
#define RC_MAP_MISSING -3
#define RC_MAP_FULL -2
#define RC_MAP_OMEM -1
#define RC_MAP_OK 0

typedef struct {
  rcmap_key_t key;
  bool used;
  size_t count; // payload
} rcentry_t;

typedef struct {
  size_t tableSize;
  size_t size;
  rcentry_t *data;
} rcmap_t;

uint32_t rcmap_hashInt(rcmap_t *map, rcmap_key_t key);

rcmap_t *rcmap_create();
void rcmap_destroy(rcmap_t *map);

int rcmap_hash(rcmap_t *map, rcmap_key_t key);
int rcmap_rehash(rcmap_t *map);
int rcmap_put(rcmap_t *map, rcmap_key_t key, size_t count);
int rcmap_getPtr(rcmap_t *map, rcmap_key_t key, size_t **out);
int rcmap_get(rcmap_t *map, rcmap_key_t key, size_t *out);
int rcmap_remove(rcmap_t *map, rcmap_key_t key);
