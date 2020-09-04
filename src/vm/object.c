#include <vm/object.h>
#include <vm/util.h>

#include <stdlib.h>

object_t *object_create() {
  object_t *object = (object_t*)malloc(sizeof(object_t));
  object->members = (object_member_t*)calloc(OBJECT_INITIAL_SIZE, sizeof(object_member_t));
  object->tableSize = OBJECT_INITIAL_SIZE;
  object->size = 0;
  return object;
}

void object_destroy(object_t *object) {
  free(object->members);
  free(object);
}

void object_destructor(runtime_t *rt, args_t *args) {
  void *ptr = args->_rawData;

  if (ptr == NULL) {
    return; // 'this' object not assigned
  }

  object_t *object = (object_t*)ptr;

  // check if a %destruct member exists on object,
  // if it exists, then attempt to invoke it
  value_t *destructor_ptr = NULL;

  if (object_getPtr(object, "%destruct", &destructor_ptr) == OBJECT_OK) {
    puts("destructors not implemented");
    exit(1);
  }

  object_destroy(object);
}

uint32_t object_hashInt(object_t *object, object_key_t key) {
  return hash6432shift((uint64_t)key) % object->tableSize;
}

int object_hash(object_t *object, object_key_t key) {
  int curr, i;

  if (object->size >= (object->tableSize / 2)) {
    return OBJECT_FULL;
  }

  curr = object_hashInt(object, key);

  for (i = 0; i < OBJECT_MAX_CHAIN_LENGTH; i++) {
    if (!object->members[curr].used) {
      return curr;
    }

    if (object->members[curr].used && object->members[curr].key == key) {
      return curr;
    }

    curr = (curr + 1) % object->tableSize;
  }

  return OBJECT_FULL;
}

int object_rehash(object_t *object) {
  int i, oldSize;
  object_member_t *curr, *tmp;

  tmp = (object_member_t*)calloc(2 * object->tableSize, sizeof(object_member_t));
  curr = object->members;
  object->members = tmp;

  oldSize = object->tableSize;
  object->tableSize *= 2;
  object->size = 0;

  for (i = 0; i < oldSize; i++) {
    int status;

    if (!curr[i].used) {
      continue;
    }

    status = object_put(object, curr[i].key, &curr[i].value);

    if (status != OBJECT_OK) {
      return status;
    }
  }

  free(curr);

  return OBJECT_OK;
}

int object_put(object_t *object, object_key_t key, value_t *value) {
  int index = object_hash(object, key);

  while (index == OBJECT_FULL) {
    if (object_rehash(object) == OBJECT_OMEM) {
      return OBJECT_OMEM;
    }

    index = object_hash(object, key);
  }

  object->members[index].key = key;
  object->members[index].value = *value; // TODO: use value_copyValue maybe?
  object->members[index].used = true;
  object->size++;

  return OBJECT_OK;
}

int object_getPtr(object_t *object, object_key_t key, value_t **out) {
  int curr, i;

  curr = object_hashInt(object, key);

  for (i = 0; i < OBJECT_MAX_CHAIN_LENGTH; i++) {
    if (object->members[curr].used) {
      if (object->members[curr].key == key) {
        *out = &object->members[curr].value;

        return OBJECT_OK;
      }
    }

    curr = (curr + 1) % object->tableSize;
  }

  *out = NULL;

  return OBJECT_MISSING;
}

int object_get(object_t *object, object_key_t key, value_t *out) {
  value_t *ptr = NULL;
  int result = object_getPtr(object, key, &ptr);

  if (result == OBJECT_OK) {
    *out = *ptr;
  }

  return result;
}

int object_remove(object_t *object, object_key_t key) {
  int curr, i;

  curr = object_hashInt(object, key);

  for (i = 0; i < OBJECT_MAX_CHAIN_LENGTH; i++) {
    if (object->members[curr].used) {
      if (object->members[curr].key == key) {
        memset(&object->members[curr], 0, sizeof(object_member_t));
        --object->size;

        return OBJECT_OK;
      }
    }

    curr = (curr + 1) % object->tableSize;
  }

  return OBJECT_MISSING;
}
