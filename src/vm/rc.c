#include <vm/rc.h>
#include <vm/util.h>

rcmap_t *rcmap_create() {
  rcmap_t *map = (rcmap_t*)malloc(sizeof(rcmap_t));
  map->data = (rcentry_t*)calloc(RC_INITIAL_SIZE, sizeof(rcentry_t));
  map->tableSize = RC_INITIAL_SIZE;
  map->size = 0;
  return map;
}

void rcmap_destroy(rcmap_t *map) {
  free(map->data);
  free(map);
}

uint32_t rcmap_hashInt(rcmap_t *map, rcmap_key_t key) {
  return hash6432shift((uint64_t)key) % map->tableSize;
}

int rcmap_hash(rcmap_t *map, rcmap_key_t key) {
  int curr, i;

  if (map->size >= (map->tableSize / 2)) {
    return RC_MAP_FULL;
  }

  curr = rcmap_hashInt(map, key);

  for (i = 0; i < RC_MAX_CHAIN_LENGTH; i++) {
    if (!map->data[curr].used) {
      return curr;
    }

    if (map->data[curr].used && map->data[curr].key == key) {
      return curr;
    }

    curr = (curr + 1) % map->tableSize;
  }

  return RC_MAP_FULL;
}

int rcmap_rehash(rcmap_t *map) {
  int i, oldSize;
  rcentry_t *curr, *tmp;

  tmp = (rcentry_t*)calloc(2 * map->tableSize, sizeof(rcentry_t));
  curr = map->data;
  map->data = tmp;

  oldSize = map->tableSize;
  map->tableSize *= 2;
  map->size = 0;

  for (i = 0; i < oldSize; i++) {
    int status;

    if (!curr[i].used) {
      continue;
    }

    status = rcmap_put(map, curr[i].key, curr[i].count);

    if (status != RC_MAP_OK) {
      return status;
    }
  }

  free(curr);

  return RC_MAP_OK;
}

int rcmap_put(rcmap_t *map, rcmap_key_t key, size_t count) {
  int index = rcmap_hash(map, key);

  while (index == RC_MAP_FULL) {
    if (rcmap_rehash(map) == RC_MAP_OMEM) {
      return RC_MAP_OMEM;
    }

    index = rcmap_hash(map, key);
  }

  map->data[index].key = key;
  map->data[index].count = count;
  map->data[index].used = true;
  map->size++;

  return RC_MAP_OK;
}

int rcmap_getPtr(rcmap_t *map, rcmap_key_t key, size_t **out) {
  int curr, i;

  curr = rcmap_hashInt(map, key);

  for (i = 0; i < RC_MAX_CHAIN_LENGTH; i++) {
    if (map->data[curr].used) {
      if (map->data[curr].key == key) {
        *out = &map->data[curr].count;

        return RC_MAP_OK;
      }
    }

    curr = (curr + 1) % map->tableSize;
  }

  *out = NULL;

  return RC_MAP_MISSING;
}

int rcmap_get(rcmap_t *map, rcmap_key_t key, size_t *out) {
  size_t *ptr = NULL;
  int result = rcmap_getPtr(map, key, &ptr);

  if (result == RC_MAP_OK) {
    *out = *ptr;
  } else {
    *out = 0;
  }

  return result;
}

int rcmap_remove(rcmap_t *map, rcmap_key_t key) {
  int curr, i;

  curr = rcmap_hashInt(map, key);

  for (i = 0; i < RC_MAX_CHAIN_LENGTH; i++) {
    if (map->data[curr].used) {
      if (map->data[curr].key == key) {
        memset(&map->data[curr], 0, sizeof(rcentry_t));
        --map->size;

        return RC_MAP_OK;
      }
    }

    curr = (curr + 1) % map->tableSize;
  }

  return RC_MAP_MISSING;
}
