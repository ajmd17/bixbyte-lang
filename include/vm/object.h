#pragma once

#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

#include <vm/types.h>
#include <vm/value.h>

typedef char * object_key_t;

#define OBJECT_INITIAL_SIZE (8)
#define OBJECT_MAX_CHAIN_LENGTH (8)
#define OBJECT_MISSING -3
#define OBJECT_FULL -2
#define OBJECT_OMEM -1
#define OBJECT_OK 0

typedef struct {
  object_key_t key;
  bool used;
  value_t value;
} object_member_t;

typedef struct {
  size_t tableSize;
  size_t size;
  object_member_t *members;
} object_t;

uint32_t object_hashInt(object_t *object, object_key_t key);

object_t *object_create();
void object_destroy(object_t *object);

// a native_function_t used as the dtor_ptr on heap node
void object_destructor(runtime_t *rt, args_t *args);

int object_hash(object_t *object, object_key_t key);
int object_rehash(object_t *object);
int object_put(object_t *object, object_key_t key, value_t *value);
int object_getPtr(object_t *object, object_key_t key, value_t **out);
int object_get(object_t *object, object_key_t key, value_t *out);
int object_remove(object_t *object, object_key_t key);

