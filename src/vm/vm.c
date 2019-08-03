#include <stdint.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>

#include <pthread.h>
pthread_mutex_t heapMutex = PTHREAD_MUTEX_INITIALIZER;

// ===== Instructions =====

typedef enum {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
} ARCHETYPE;

typedef uint32_t obj_loc_t;
typedef uint8_t archtype_t;
typedef uint32_t loc_28_t;

obj_loc_t obj_loc_make(loc_28_t loc, archtype_t at) {
  obj_loc_t payload = loc;
  payload <<= 4;
  payload |= at;
  return payload;
}

void obj_loc_parse(obj_loc_t payload, loc_28_t *loc, archtype_t *at) {
  *loc = payload >> 4;
  *at = payload & 0xF;
}

typedef void * rcmap_key_t;
typedef void * refcounted_t;

struct runtime;
typedef struct runtime runtime_t;
refcounted_t runtime_claim(runtime_t *rt, rcmap_key_t ptr);
void runtime_release(runtime_t *rt, refcounted_t rc);

struct value;
typedef struct value value_t;
value_t value_invoke(runtime_t *r, value_t *value);
// enum _VALUE_FLAGS;
// typedef enum _VALUE_FLAGS VALUE_FLAGS;
// value_t value_fromRawPointer(void*, VALUE_FLAGS);
// void value_destroy(runtime_t *rt, value_t *value);

struct args;
typedef struct args args_t;

typedef uint32_t metadata_t;
typedef value_t (*native_function_t)(runtime_t*, args_t*);

typedef enum {
  TYPE_NONE = 0x0,
  TYPE_INT = 0x1,
  TYPE_UINT = 0x2,
  TYPE_DOUBLE = 0x3,
  TYPE_BOOLEAN = 0x4,
  TYPE_POINTER = 0x5,
  TYPE_FUNCTION = 0x6
} VALUE_TYPE;

typedef enum {
  FLAG_NONE = 0x0,
  FLAG_MARKED = 0x1,
  FLAG_OBJECT = 0x2,
  FLAG_EXCEPTION = 0x4,
  FLAG_MALLOC = 0x8, // raw pointer that needs free() call
  FLAG_REFCOUNTED = 0x10
} VALUE_FLAGS;

// ===== heap memory =====

typedef struct {
  void *ptr;
  uint8_t flags;
} heap_value_t;

struct heap_node;
typedef struct heap_node heap_node_t;

struct heap_node {
  heap_value_t hv;
  heap_node_t *prev;
  heap_node_t *next;
};

heap_node_t *heap_node_create() {
  heap_node_t *node = (heap_node_t*)malloc(sizeof(heap_node_t));
  node->hv.ptr = NULL;
  node->hv.flags = 0;
  node->prev = NULL;
  node->next = NULL;
  return node;
}

void heap_node_destroy(heap_node_t *node) {
  // @TODO how to free internal obj? dtor pointer?
  free(node);
}


typedef struct {
  heap_node_t *head;
  size_t size;
} heap_t;

heap_t *heap_create() {
  heap_t *heap = (heap_t*)malloc(sizeof(heap_t));
  heap->head = NULL;
  heap->size = 0;
  return heap;
}

void heap_destroy(heap_t *heap) {
  while (heap->head) {
    heap_node_t *tmp = heap->head;
    heap->head = tmp->prev;

    heap_node_destroy(tmp);

    --heap->size;
  }
}

heap_value_t *heap_alloc(heap_t *heap) {
  pthread_mutex_lock(&heapMutex);

  heap_node_t *node = heap_node_create();

  if (heap->head != NULL) {
    heap->head->next = node;
  }

  // swap
  node->prev = heap->head;
  heap->head = node;

  ++heap->size;

  pthread_mutex_unlock(&heapMutex);

  return &heap->head->hv;
}

void heap_sweep(heap_t *heap) {
  heap_node_t *last = heap->head;

  while (last) {
    if (last->hv.flags & FLAG_MARKED) {
      // unmark
      last->hv.flags &= ~FLAG_MARKED;
      last = last->prev;
      continue;
    }

    // not marked; delete
    heap_node_t *prev = last->prev;
    heap_node_t *next = last->next;

    if (prev) {
      prev->next = next;
    }

    if (next) {
      // removing an item from the middle, so
      // make the nodes to the other sides now
      // point to each other
      next->prev = prev;
    } else {
      // since there are no nodes after this,
      // set the head to be this node here
      heap->head = prev;
    }

    heap_node_destroy(last);
    last = prev;

    --heap->size;
  }
}

// ===== Ref Counted Data =====
//! https://github.com/petewarden/c_hashmap/blob/master/hashmap.c

#define RC_INITIAL_SIZE (256)
#define RC_MAX_CHAIN_LENGTH (8)
#define RC_MAP_MISSING -3
#define RC_MAP_FULL -2
#define RC_MAP_OMEM -1
#define RC_MAP_OK 0

uint32_t hash6432shift(uint64_t key) {
  key = (~key) + (key << 18);
  key = key ^ (key >> 31);
  key = key * 21;
  key = key ^ (key >> 11);
  key = key + (key << 6);
  key = key ^ (key >> 22);
  return key;
}

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

int rcmap_put(rcmap_t *map, rcmap_key_t key, size_t count);

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


// ===== object =====


/*
typedef struct {
  char *key;
  bool used;
  value_t value; // payload
} objfield_t;

typedef struct {
  size_t tableSize;
  size_t size;
  objfield_t *data;
} objfieldmap_t;

objfieldmap_t *objfieldmap_create() {
  objfieldmap_t *map = (objfieldmap_t*)malloc(sizeof(objfieldmap_t));
  map->data = (objfield_t*)calloc(OBJFIELDMAP_INITIAL_SIZE, sizeof(objfield_t));
  map->tableSize = OBJFIELDMAP_INITIAL_SIZE;
  map->size = 0;
  return map;
}

void objfieldmap_destroy(objfieldmap_t *map) {
  free(map->data);
  free(map);
}

int objfieldmap_hash(objfieldmap_t *map, char *key) {
  int curr, i;

  if (map->size >= (map->tableSize / 2)) {
    return OBJFIELDMAP_FULL;
  }

  curr = objfieldmap_hashInt(map, key);

  for (i = 0; i < OBJFIELDMAP_CHAIN_LENGTH; i++) {
    if (!map->data[curr].used) {
      return curr;
    }

    if (map->data[curr].used && strcmp(map->data[curr].key, key) == 0) {
      return curr;
    }

    curr = (curr + 1) % map->tableSize;
  }

  return OBJFIELDMAP_FULL;
}

int objfieldmap_put(objfieldmap_t *map, char *key, value_t value);

int objfieldmap_rehash(objfieldmap_t *map) {
  int i, oldSize;
  objfield_t *curr, *tmp;

  tmp = (objfield_t*)calloc(2 * map->tableSize, sizeof(objfield_t));
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

    status = objfieldmap_put(map, curr[i].key, curr[i].value);

    if (status != OBJFIELDMAP_OK) {
      return status;
    }
  }

  free(curr);

  return OBJFIELDMAP_OK;
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

*/

typedef struct {
  void *ptr;
  // @TODO hashmap of fields?
} object_t;

object_t *object_create() {
  object_t *obj = (object_t*)malloc(sizeof(object_t));
  return obj;
}

void object_destroy(object_t *obj) {
  // @TODO delete fields, call destructor ?

  free(obj);
}




// ===== Value =====

struct value {
  union {
    int64_t i64;
    uint64_t u64;
    double dbl;
    bool b;
    void *raw;
    heap_value_t *hv;
    native_function_t fn;
    refcounted_t rc;
  } data;

  metadata_t metadata;
};




void value_destroy(runtime_t *rt, value_t *value) {
  /*if (value->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
    object_destroy((object_t*)value->data.ptr);
  } else*/
  if (value->metadata & (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) {
    runtime_release(rt, value->data.rc);
  } else if (value->metadata & (TYPE_POINTER | (FLAG_MALLOC << 8))) {
    free(value->data.raw);
  }
}

void value_copyValue(runtime_t *rt, value_t *v, value_t *other) {
  value_destroy(rt, v);

  if (other->metadata & (TYPE_POINTER | (FLAG_REFCOUNTED << 8))) {
    v->data.rc = runtime_claim(rt, (rcmap_key_t)other->data.rc);
  } else {
    v->data = other->data;
  }

  v->metadata = other->metadata;
}

void value_setInt(runtime_t *rt, value_t *v, int64_t i64) {
  value_destroy(rt, v);
  v->data.i64 = i64;
  v->metadata = TYPE_INT;
}

int64_t value_getInt(value_t *v) {
  return v->data.i64;
}

value_t value_fromInt(int64_t i64) {
  value_t v;
  v.data.i64 = i64;
  v.metadata = TYPE_INT;
  return v;
}

void value_setUint(runtime_t *rt, value_t *v, uint64_t u64) {
  value_destroy(rt, v);
  v->data.u64 = u64;
  v->metadata = TYPE_UINT;
}

uint64_t value_getUint(value_t *v) {
  return v->data.u64;
}

value_t value_fromUint(uint64_t u64) {
  value_t v;
  v.data.u64 = u64;
  v.metadata = TYPE_UINT;
  return v;
}

void value_setDouble(runtime_t *rt, value_t *v, double dbl) {
  value_destroy(rt, v);
  v->data.dbl = dbl;
  v->metadata = TYPE_DOUBLE;
}

double value_getDouble(value_t *v) {
  return v->data.dbl;
}

value_t value_fromDouble(double dbl) {
  value_t v;
  v.data.dbl = dbl;
  v.metadata = TYPE_DOUBLE;
  return v;
}

void value_setBoolean(runtime_t *rt, value_t *v, bool b) {
  value_destroy(rt, v);
  v->data.b = b;
  v->metadata = TYPE_BOOLEAN;
}

bool value_getBoolean(value_t *v) {
  return v->data.b;
}

value_t value_fromBoolean(bool b) {
  value_t v;
  v.data.b = b;
  v.metadata = TYPE_BOOLEAN;
  return v;
}

value_t value_createObject(heap_t *heap) {
  value_t v;
  v.data.hv = heap_alloc(heap);
  v.metadata = TYPE_POINTER | (FLAG_OBJECT << 8);
  return v;
}

void *value_getRawPointer(value_t *value) {
  return value->data.raw;
}

void value_setRawPointer(runtime_t *rt, value_t *v, void *raw, VALUE_FLAGS flags) {
  value_destroy(rt, v);
  v->data.raw = raw;
  v->metadata = TYPE_POINTER | (flags << 8);
}

value_t value_fromRawPointer(void *raw, VALUE_FLAGS flags) {
  value_t v;
  v.data.raw = raw;
  v.metadata = TYPE_POINTER | (flags << 8);
  return v;
}

void value_setRefCounted(runtime_t *rt, value_t *v, void *ptr) {
  value_destroy(rt, v);
  v->data.rc = runtime_claim(rt, (rcmap_key_t)ptr);
  v->metadata = TYPE_POINTER | (FLAG_REFCOUNTED << 8);
}

value_t value_fromFunction(native_function_t fn) {
  value_t v;
  v.data.fn = fn;
  v.metadata = TYPE_FUNCTION;
  return v;
}

void value_setFunction(runtime_t *rt, value_t *v, native_function_t fn) {
  value_destroy(rt, v);
  v->data.fn = fn;
  v->metadata = TYPE_FUNCTION;
}

VALUE_TYPE value_getType(value_t *value) {
  // type is stored in first 8 bits of metadata
  return (VALUE_TYPE)(value->metadata & 0xFF);
}

void value_setType(value_t *value, VALUE_TYPE type) {
  value->metadata = (value->metadata & 0xFF00) | type;
}

VALUE_FLAGS value_getFlags(value_t *value) {
  // flags stored in next 8 bits of metadata
  return (VALUE_FLAGS)(value->metadata & 0xFF00);
}

void value_setFlag(value_t *value, VALUE_FLAGS flag, int state) {
  value->metadata = (-state & (value->metadata | (flag << 8))) | ((state - 1) & (value->metadata & ~(flag << 8)));
}

uintptr_t value_getID(value_t *value) {
  VALUE_TYPE type = value_getType(value);
  VALUE_FLAGS flags = value_getFlags(value);

  if (type == TYPE_POINTER) {
    if (flags & FLAG_REFCOUNTED) {
      return (uintptr_t)value->data.rc;
    } else if (flags & FLAG_OBJECT) {
      // assert value->data.hv != NULL
      return (uintptr_t)value->data.hv->ptr;
    }

    return (uintptr_t)value->data.raw;
  } else if (type == TYPE_FUNCTION) {
    return (uintptr_t)value->data.fn;
  }

  return (uintptr_t)(&value->data);

  //int isptr = !!(value->metadata & TYPE_POINTER);
  //return (-isptr & (uintptr_t)value->data.ptr) | ((isptr - 1) & (uintptr_t)(&value->data));
}



// ===== Data storage =====

#define MB_TO_BYTES(mb) (mb*1000000)
#define DEFAULT_STATIC_DATA_SIZE_MB 5 // in MB
#define STATIC_DATA_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) % sizeof(value_t)))
#define DEFAULT_STACK_SIZE_MB 20 // in MB
#define STACK_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) % sizeof(value_t)))
#define NUM_REGISTERS 4

typedef struct {
  value_t *data;
  size_t len;
} storage_t;

typedef struct {
  storage_t storage[4];
} datatable_t;

datatable_t *datatable_create() {
  datatable_t *dt = (datatable_t*)malloc(sizeof(datatable_t));

  dt->storage[0].data = NULL;
  dt->storage[0].len = 0;

  dt->storage[AT_DATA].data = (value_t*)malloc(STATIC_DATA_SIZE_BYTES);
  memset(dt->storage[AT_DATA].data, 0, STATIC_DATA_SIZE_BYTES);
  dt->storage[AT_DATA].len = 0;

  dt->storage[AT_LOCAL].data = (value_t*)malloc(STACK_SIZE_BYTES);
  memset(dt->storage[AT_LOCAL].data, 0, STACK_SIZE_BYTES);
  dt->storage[AT_LOCAL].len = 0;

  dt->storage[AT_REG].data = (value_t*)malloc(sizeof(value_t) * NUM_REGISTERS);
  memset(dt->storage[AT_REG].data, 0, sizeof(value_t) * NUM_REGISTERS);
  dt->storage[AT_REG].len = 0;

  return dt;
}

void datatable_destroy(runtime_t *rt, datatable_t *dt) {
  int i;

  for (i = 0; i < 4; i++) {
    while (dt->storage[i].len) {
      value_destroy(rt, &dt->storage[i].data[--dt->storage[i].len]);
    }

    free(dt->storage[i].data);
  }

  free(dt);
}

void datatable_markTable(storage_t *s) {
  size_t len = s->len;

  while (len) {
    value_t *v = &s->data[len - 1];

    // @TODO partitioning to prevent branch prediction misses?
    if (v->metadata & (TYPE_POINTER | (FLAG_OBJECT << 8))) {
      v->data.hv->flags |= FLAG_MARKED;
    }

    --len;
  }
}

void datatable_mark(datatable_t *dt) {
  datatable_markTable(&dt->storage[AT_DATA]);
  datatable_markTable(&dt->storage[AT_LOCAL]);
}

value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at) {
  storage_t *s = &dt->storage[at & 0x3];

  int abs = ((at & 0xC) >> 2) - 2;
  size_t idx = (-abs & loc) | ((abs - 1) & (s->len - loc));

  return &s->data[idx];
}

// ===== Exceptions =====

typedef struct {
  object_t *base;
  value_t argument;
} exception_t;

exception_t exception_fromValue(value_t *argument) {
  exception_t e;
  e.base = NULL; // @TODO SimpleException
  e.argument = *argument;
  return e;
}

// ===== Runtime =====
struct runtime {
  datatable_t *dt;
  heap_t *heap;
  rcmap_t *rc;
};

runtime_t *runtime_create() {
  runtime_t *r = (runtime_t*)malloc(sizeof(runtime_t));

  r->heap = heap_create();
  r->dt = datatable_create();
  r->rc = rcmap_create();

  return r;
}

void runtime_destroy(runtime_t *r) {
  rcmap_destroy(r->rc);
  datatable_destroy(r, r->dt);
  heap_destroy(r->heap);
  free(r);
}

void runtime_gc(runtime_t *r) {
  pthread_mutex_lock(&heapMutex);

  datatable_mark(r->dt);
  heap_sweep(r->heap);

  pthread_mutex_unlock(&heapMutex);
}

void runtime_throwException(runtime_t *r, exception_t *e) {
  // @TODO internal VM handling.

}

#include <assert.h>

refcounted_t runtime_claim(runtime_t *rt, rcmap_key_t key) {
  size_t *cnt = NULL;

  int result = rcmap_getPtr(rt->rc, key, &cnt);

  if (result == RC_MAP_OK) {
    ++(*cnt);
    return (refcounted_t)key;
  } else if (result == RC_MAP_MISSING) {
    result = rcmap_put(rt->rc, key, 1);
  }

  if (result == RC_MAP_OK) {
    return (refcounted_t)key;
  }

  // uh oh. throw exception?
  return NULL;
}

void runtime_release(runtime_t *rt, refcounted_t rc) {
  size_t *cnt = NULL;

  int result = rcmap_getPtr(rt->rc, rc, &cnt);

  if (result != RC_MAP_OK) {
    // throw exception?
    return;
  }

  assert(*cnt > 0);

  if (--(*cnt) == 0) {
    free(rc);

    result = rcmap_remove(rt->rc, rc);
  }

  if (result != RC_MAP_OK) {
    // throw
    return;
  }
}

// ===== Interpreter =====
typedef uint8_t ubyte_t;

typedef struct {
  size_t pc;
  size_t len;
  uint8_t flags;
  ubyte_t *bc;
} interpreter_t;

typedef enum {
  INTERPRETER_FLAGS_NONE = 0x0,
  INTERPRETER_FLAGS_EQUAL = 0x1,
  INTERPRETER_FLAGS_GREATER = 0x2
} INTERPRETER_FLAGS;

interpreter_t *interpreter_create(ubyte_t *data, size_t len) {
  interpreter_t *it = (interpreter_t*)malloc(sizeof(interpreter_t));
  it->pc = 0;
  it->len = len;
  it->flags = 0;
  it->bc = malloc(sizeof(ubyte_t) * len);
  memcpy(it->bc, data, it->len);
  return it;
}

void interpreter_destroy(interpreter_t *it) {
  free(it->bc);
  free(it);
}

void interpreter_peek(interpreter_t *it, size_t size, void *out) {
  memcpy(out, it->bc + it->pc, size);
}

void interpreter_seek(interpreter_t *it, size_t loc) {
  // assert(loc < it->len);
  it->pc = loc;
}

void interpreter_read(interpreter_t *it, size_t size, void *out) {
  // assert(it->pc + n <= it->len);
  interpreter_peek(it, size, out);
  it->pc += size;
}

bool interpreter_atEnd(interpreter_t *it) {
  return it->pc >= it->len;
}

enum JUMP_FLAGS {
  JUMP_FLAGS_NONE = 0x0,
  JUMP_FLAGS_JE = 0x1,
  JUMP_FLAGS_JNE = 0x2,
  JUMP_FLAGS_JG = 0x3,
  JUMP_FLAGS_JGE = 0x4
};

enum CONST_FLAGS {
  CONST_FLAGS_NONE = 0x0,
  CONST_FLAGS_NULL = 0x1,
  CONST_FLAGS_I64 = 0x2,
  CONST_FLAGS_U64 = 0x3,
  CONST_FLAGS_F64 = 0x4,
  CONST_FLAGS_BOOL = 0x5,
  CONST_FLAGS_RAWDATA = 0x6
};

enum CMP_FLAG {
  CMP_FLAG_NONE = 0x0, // normal `cmp` -- compares .data to .data
  CMP_FLAG_F64_L = 0x1, // compare .data.f64 to .data
  CMP_FLAG_F64_R = 0x2, // compare .data to .data.f64
  CMP_FLAG_F64_LR = 0x3, // compare .data.f64 to .data.f64
};

//static size_t LOAD_CONST_SIZES[] = { 0, sizeof(int64_t), sizeof(uint64_t), sizeof(double), sizeof(bool) };

// instruction = 5 bits
// flags = 3 bits
enum INSTRUCTIONS { // max: 32 values
  OP_NOOP = 0,

  OP_LOAD = 1, // load value into register

  OP_MOV = 2, // move data from one place to another

  OP_CMP = 4, // compare and set compare flag

  OP_JMP = 5, // jump to location held in value

  OP_PUSH = 6, // push value to stack
  OP_POP = 7, // pop n values from stack

  // ===== binary operations
  OP_ADD = 8, // +
  OP_SUB = 9, // -
  OP_MUL = 10, // *
  OP_DIV = 11, // /
  OP_MOD = 12, // %

  // ===== bitwise (binary) operations
  OP_XOR = 13, // ^
  OP_AND = 14, // &
  OP_OR = 15, // |
  OP_SHL = 16, // <<
  OP_SHR = 17, // >>

  // ===== unary operations
  OP_NEG = 18, // - (mathematical negation)
  OP_NOT = 19, // ~

  OP_CALL = 20,
  OP_PLACEHOLDER_21 = 21,
  OP_PLACEHOLDER_22 = 22,
  OP_PLACEHOLDER_23 = 23,
  OP_PLACEHOLDER_24 = 24,
  OP_PLACEHOLDER_25 = 25,
  OP_PLACEHOLDER_26 = 26,
  OP_PLACEHOLDER_27 = 27,
  OP_PLACEHOLDER_28 = 28,
  OP_PLACEHOLDER_29 = 29,
  OP_JIT = 30,

  OP_HALT = 31, // exit program
};

#define JIT_MODE 0
#define INTERPRET_MODE 1

#define _(code) \
  if (INTERPRET_MODE) { \
    code \
  } else if (JIT_MODE) { \
    #str \
  } \

#define INTERPRET 0
#define JIT 1
#define INTERPRETER_MODE Jit
//#define DECODE(objLoc) \
#include <bcparse/vm/interpreter_loop.inc.h>
//#undef DECODE
#undef INTERPRETER_MODE
#undef JIT
#undef INTERPRET

#define INTERPRET 1
#define JIT 0
#define INTERPRETER_MODE Interpreter
//#define DECODE(objLoc)
#include <bcparse/vm/interpreter_loop.inc.h>
//#undef DECODE
#undef INTERPRETER_MODE
#undef JIT
#undef INTERPRET

void interpreter_run##INTERPRETER_MODE(interpreter_t *it, runtime_t *rt) {
  uint8_t data, opcode, flags, cache[64];

#if JIT
  printf("#define DECODE(loc) datatable_getValue(rt, loc >> 4, loc & 0xF)\n\n");

  printf("int main(int argc, char *argv[]) {\n");
  printf("runtime_t *rt = runtime_create();\n");
#endif

  while (!interpreter_atEnd(it)) {
    size_t pcBefore = it->pc;

    interpreter_read(it, sizeof(data), &data);

    opcode = data >> 3;
    flags = data & 0x7;

#if JIT
    printf("\n_lbl_%zu:\n", pcBefore);
#endif

    switch (opcode) {
      case OP_NOOP: break;
      case OP_LOAD: { // load
        obj_loc_t o;
        interpreter_read(it, sizeof(o), &o);

        loc_28_t loc;
        archtype_t at;
        obj_loc_parse(o, &loc, &at);

        value_t *v = datatable_getValue(rt->dt, loc, at);

#if JIT
        printf("value_t *v = DECODE(%#lx);\n", o);
#endif

        switch (flags) {
          case CONST_FLAGS_NONE: // ??
#if INTERPRET // @TODO macro to conditionally output JIT / Code
            v->data.i64 = 0;
            v->metadata = TYPE_NONE; // just zero out i guess
#endif

            break;
          case CONST_FLAGS_NULL: // loadnull
#if INTERPRET
            v->data.raw = NULL;
            v->metadata = TYPE_POINTER;
#endif

#if JIT
            printf("v->data.raw = NULL;\nv->metadata = TYPE_POINTER;\n");
#endif

            break;
          case CONST_FLAGS_I64: { // loadi4
            interpreter_read(it, sizeof(int64_t), cache);
            value_setInt(rt, v, *((int64_t*)cache));

            printf("value_setInt(rt, v, %d);\n", *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // loadu4
            interpreter_read(it, sizeof(uint64_t), cache);
            value_setUint(rt, v, *((uint64_t*)cache));

            printf("value_setUint(rt, v, %d);\n", *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // loadd
            interpreter_read(it, sizeof(double), cache);
            value_setDouble(rt, v, *((double*)cache));

            printf("value_setDouble(rt, v, %0.f);\n", *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // loadb
            interpreter_read(it, sizeof(uint8_t), cache);
            value_setBoolean(rt, v, (bool)cache[0]);

            printf("value_setDouble(rt, v, %d);\n", (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // loaddata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz); // managed by refcounter
            printf("void *data = malloc(%d);\n", sz);

            interpreter_read(it, sz, data);

            value_setRefCounted(rt, v, data);
            printf("value_setRefCounted(rt, v, data);\n");

            break;
          }
        }

        break;
      }

      case OP_MOV: { // mov
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);
        printf("value_t *left = DECODE(%#lx);\n", o);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);
        printf("value_t *right = DECODE(%#lx);\n", o);

        if (at & AT_REG) {
          // optimization
          *left = *right;
          printf("*left = *right;\n");
        } else {
          value_copyValue(rt, left, right);
          printf("value_copyValue(rt, left, right);\n");
        }

        break;
      }

      case OP_CMP: { // cmp
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        union { int i; double d; } cacheval;

        switch (flags) {
          case CMP_FLAG_F64_L: // cmpdl
            cacheval.d = left->data.dbl - right->data.i64;
            goto setDouble;
          case CMP_FLAG_F64_R: // cmpdr
            cacheval.d = left->data.i64 - right->data.dbl;
            goto setDouble;
          case CMP_FLAG_F64_LR: // cmpd
            cacheval.d = left->data.dbl - right->data.dbl;
            goto setDouble;
          default: // cmp
            cacheval.i = left->data.i64 - right->data.i64;
            goto setInt;
        }

      setDouble:
        it->flags = ((0 < cacheval.d) - (cacheval.d < 0)) + 1;
        break;
      setInt:
        it->flags = ((0 < cacheval.i) - (cacheval.i < 0)) + 1; // 0, 1, or 2. gets sign() of `val` and adds 1.
        break;
      }

      case OP_JMP: { // jmp
        value_t *v;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        v = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case JUMP_FLAGS_JE: // je
            if (~it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JNE: // jne
            if (it->flags & INTERPRETER_FLAGS_EQUAL) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JG: // jg
            if (~it->flags & INTERPRETER_FLAGS_GREATER) {
              goto noSeek;
            }
            break;
          case JUMP_FLAGS_JGE: // jge
            if (~it->flags & (INTERPRETER_FLAGS_GREATER | INTERPRETER_FLAGS_EQUAL)) {
              goto noSeek;
            }
            break;
        }

        interpreter_seek(it, value_getUint(v));
        //printf("goto _lbl_%zu;\n", value_getUint(v));

      noSeek:
        break;
      }

      case OP_PUSH: { // push
        storage_t *stack = &rt->dt->storage[AT_LOCAL];

        switch (flags) {
          case CONST_FLAGS_NONE: { // push -- load value_t to push to stack
            obj_loc_t o;
            loc_28_t loc;
            archtype_t at;

            interpreter_read(it, sizeof(o), &o);
            obj_loc_parse(o, &loc, &at);

            value_copyValue(rt, stack->data[stack->len], datatable_getValue(rt->dt, loc, at));

            break;
          }
          // shortcuts for pushing constants directly, rather than using multiple instructions
          // @TODO: make these values be copied from a constant pool, rather than by recreating.
          case CONST_FLAGS_NULL: { // pushnull
            value_t *v = &stack->data[stack->len];
            v->data.raw = NULL;
            v->metadata = TYPE_POINTER;

            break;
          }
          case CONST_FLAGS_I64: { // pushi4
            interpreter_read(it, sizeof(int64_t), cache);
            value_setInt(rt, &stack->data[stack->len], *((int64_t*)cache));

            printf("value_setInt(rt, &stack->data[stack->len], %d);\n", *((int64_t*)cache));

            break;
          }
          case CONST_FLAGS_U64: { // pushu4
            interpreter_read(it, sizeof(uint64_t), cache);
            value_setUint(rt, &stack->data[stack->len], *((uint64_t*)cache));

            printf("value_setUint(rt, &stack->data[stack->len], %d);\n", *((uint64_t*)cache));

            break;
          }
          case CONST_FLAGS_F64: { // pushd
            interpreter_read(it, sizeof(double), cache);
            value_setDouble(rt, &stack->data[stack->len], *((double*)cache));

            printf("value_setDouble(rt, &stack->data[stack->len], %0.f);\n", *((double*)cache));

            break;
          }
          case CONST_FLAGS_BOOL: { // pushb
            interpreter_read(it, sizeof(uint8_t), cache);
            value_setBoolean(rt, &stack->data[stack->len], (bool)cache[0]);

            printf("value_setDouble(rt, &stack->data[stack->len], %d);\n", (bool)cache[0]);

            break;
          }
          case CONST_FLAGS_RAWDATA: { // pushdata
            interpreter_read(it, sizeof(uint64_t), cache);
            uint64_t sz = *((uint64_t*)cache);

            void *data = malloc(sz); // managed by refcounter
            printf("void *data = malloc(%d);\n", sz);

            interpreter_read(it, sz, data);

            value_setRefCounted(rt, &stack->data[stack->len], data);
            printf("value_setRefCounted(rt, &stack->data[stack->len], data);\n");

            break;
          }
        }

        ++stack->len;

        printf("++stack->len;\n");

        break;
      }

      case OP_POP: {
        // assert(stack.len != 0);

        storage_t *s = &rt->dt->storage[AT_LOCAL];

        interpreter_read(it, sizeof(uint16_t), cache);
        uint16_t sz = *((uint16_t*)cache);

        //rt->dt->storage[AT_LOCAL].len -= sz;

        while (sz--) { // required to call free() on malloc'd objects
          value_destroy(rt, &s->data[--s->len]);
        }

        break;
      }

      case OP_ADD:
      case OP_SUB:
      case OP_MUL:
      case OP_DIV:
      case OP_MOD: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        switch (opcode) {
          case OP_ADD:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl + right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 + right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl + right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 + right->data.i64; break;
            }
            break;
          case OP_SUB:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl - right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 - right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl - right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 - right->data.i64; break;
            }
            break;
          case OP_MUL:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl * right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 * right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl * right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 * right->data.i64; break;
            }
            break;
          // @TODO: div by zero catch?
          case OP_DIV:
            switch (flags) {
              case CMP_FLAG_F64_L:  left->data.dbl = left->data.dbl / right->data.i64; break;
              case CMP_FLAG_F64_R:  left->data.i64 = left->data.i64 / right->data.dbl; break;
              case CMP_FLAG_F64_LR: left->data.dbl = left->data.dbl / right->data.dbl; break;
              default:              left->data.i64 = left->data.i64 / right->data.i64; break;
            }
            break;
          case OP_MOD:
            left->data.i64 = left->data.i64 % right->data.i64;
            break;
        }

        break;
      }

      case OP_XOR:
      case OP_AND:
      case OP_OR:
      case OP_SHL:
      case OP_SHR: {
        value_t *left, *right;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        right = datatable_getValue(rt->dt, loc, at);

        switch (opcode) {
          case OP_XOR: left->data.i64 = left->data.i64 ^ right->data.i64; break;
          case OP_AND: left->data.i64 = left->data.i64 & right->data.i64; break;
          case OP_OR:  left->data.i64 = left->data.i64 | right->data.i64; break;
          case OP_SHL: left->data.i64 = left->data.i64 << right->data.i64; break;
          case OP_SHR: left->data.i64 = left->data.i64 >> right->data.i64; break;
        }

        break;
      }

      case OP_NEG: {
        value_t *left;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);

        switch (flags) {
          case CMP_FLAG_F64_L:  left->data.dbl = -left->data.dbl; break;
          default:              left->data.i64 = -left->data.i64; break;
        }

        break;
      }

      case OP_NOT: {
        value_t *left;

        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        left = datatable_getValue(rt->dt, loc, at);
        left->data.i64 = ~left->data.i64;

        break;
      }

      case OP_CALL: {
        obj_loc_t o;
        loc_28_t loc;
        archtype_t at;

        interpreter_read(it, sizeof(o), &o);
        obj_loc_parse(o, &loc, &at);

        value_t result = value_invoke(rt, datatable_getValue(rt->dt, loc, at));

        rt->dt->storage[AT_REG].data[0] = result;

        // @NOTE: reason we are NOT doing value_copyValue() here, is because we want the register value to inherit
        // all responsibilities of `result` here ... including refcounts, free() obligations...

        break;
      }

      // ...

      case OP_JIT: {
        // @jit_begin("sum", memoized=true, args=1)
        // ... some instructions
        // @jit_end ; internally -- tags `sum` as $d6

        // -> OP_JIT JIT_FLAG_BEGIN | JIT_FLAG_MEMOIZE ; memoize for when fn is called ?? idk
        // ->   1 ; args to hash for memoize
        // ->   $d6 ; location of native fn to store at
        // -> ... some instructions
        // -> OP_JIT JIT_FLAG_END

        // ; raw, unoptimized code

        // -> OP_CALL #{_System_arrayCreate} ; construct an array -- now stored in $r0
        // -> OP_PUSH $r0 ; push new array to stack -- stored as local variable

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_LOAD | CONST_FLAG_U64 $r0 0
        // -> OP_PUSH $r0 ; push 0 to stack
        // -> OP_LOAD | CONST_FLAG_F64 $r0 3.5
        // -> OP_PUSH $r0 ; push 3.5 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 3.5
        // -> OP_POP 3

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_LOAD | CONST_FLAG_U64 $r0 0
        // -> OP_PUSH $r0 ; push 0 to stack
        // -> OP_LOAD | CONST_FLAG_F64 $r0 4.6
        // -> OP_PUSH $r0 ; push 4.6 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 4.6
        // -> OP_POP 3

        // -> OP_CALL #{sum} ; call native fn

        // =====
        // ; optimized, first pass

        // -> OP_CALL #{_System_arrayCreate} ; construct an array -- now stored in $r0
        // -> OP_PUSH $r0 ; push new array to stack -- stored as local variable

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_PUSH | CONST_FLAG_U64 0
        // -> OP_PUSH | CONST_FLAG_F64 3.5 ; push 3.5 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 3.5
        // -> OP_POP 3

        // -> OP_PUSH $s[-1] ; push last stack item to top, duplicating
        // -> OP_PUSH | CONST_FLAG_U64 0 ; push 0 to stack
        // -> OP_PUSH | CONST_FLAG_F64 4.6 ; push 4.6 to stack
        // -> OP_CALL #{_System_arraySetIndex} ; set array[0] to 4.6
        // -> OP_POP 3

        // -> OP_CALL #{sum} ; call native fn


        // built a native_function_t jitFunction_<addr>
        // @todo: function memoization? maybe have an argument for X number of stack parameters
        // to take a hash of and lookup a memoized function based on this?

        native_function_t jitFunction;

#if JIT
        return;
#else if INTERPRET

        if (flags & JIT_FLAG_BEGIN) { // begin
          char *buf; // C source code buffer?
          uint8_t jitOp = OP_JIT;
          uint8_t jitFlags = flags;


          // set hash to be hash of (it->pc)
          // as well as hashcode of X number of stack items
          // this could be useful for memoizing function arguments
          //char hash[64];

          // while (!interpreter_atEnd(it)) {
          //   interpreter_read(it, &jitOp, &jitFlags);

          //   if (flags & JIT_FLAG_MEMOIZE) {
          //     jit_buildMemoized(rt->jit, jitOp, jitFlags, &buf);
          //   } else {
          //     // default -- does not expand CMP and conditionals
          //     jit_build(rt->jit, jitOp, jitFlags, &buf);
          //   }
          // }
          interpreter_runJit(it);

          // first pass, run C compiler, load obj file, lookup function `jitFunction_<hash>`


          jitmap_set(rt->jit->map, )

        }
#endif

        // move value_t to static data that holds jitFunction

        break;
      }

      case OP_HALT:
        puts("\n\nProgram Terminated\n\n");
        break;
    }
  }

  printf("\nruntime_destroy(rt);\n");
  printf("return 0;\n}\n");
}

// ===== Native function arguments =====

struct args {
  storage_t *_stack;
};

value_t *args_getArg(args_t *args, size_t index) {
  return &args->_stack->data[args->_stack->len - 1 - index];
}

value_t value_invoke(runtime_t *r, value_t *value) {
  args_t args;
  args._stack = &r->dt->storage[AT_LOCAL];

  return value->data.fn(r, &args);
}

// ===== Builtin bindings =====
value_t _System_createObject(runtime_t *r, args_t *args) {
  return value_createObject(r->heap);
}

// ===== C Lib functions =====

value_t _System_C_exit(runtime_t *r, args_t *args) {
  exit(value_getInt(args_getArg(args, 0)));

  return value_fromRawPointer(NULL, 0);
}

#include <math.h>
value_t _System_C_fmod(runtime_t *r, args_t *args) {
  double a = value_getDouble(args_getArg(args, 0));
  double b = value_getDouble(args_getArg(args, 1));

  return value_fromDouble(fmod(a, b));
}

value_t _System_C_strlen(runtime_t *r, args_t *args) {
  return value_fromInt(strlen((char*)value_getRawPointer(args_getArg(args, 0))));
}

// ===== Utility functions =====

uint8_t makeInstruction(enum INSTRUCTIONS opcode, uint8_t flags) {
  uint8_t data = opcode;
  data <<= 3;
  data |= flags;
  return data;
}

void showArguments(int argc, char *argv[]) {
  printf("Arguments: %s <filename>\n", argv[0]);
  exit(EXIT_FAILURE);
}

// ===== threading functions =====

typedef struct {
  runtime_t *rt;
  ubyte_t *data;
  size_t len;
} interpreter_data_t;

void *interpreterThread(void *arg) {
  interpreter_data_t *iData = (interpreter_data_t*)arg;

  interpreter_t *it = interpreter_create(iData->data, iData->len);

  value_setFunction(iData->rt, datatable_getValue(iData->rt->dt, 0, AT_DATA | AT_ABS), _System_C_exit);

  interpreter_loop(it, iData->rt);
  interpreter_destroy(it);

  return NULL;
}

void *gcThread(void *arg) {
  runtime_t *rt = (runtime_t*)arg;

  // @TODO

  return NULL;
}

void openFile(interpreter_data_t *iData, int argc, char *argv[]) {
  FILE *fp = fopen(argv[1], "r");

  if (fp == NULL) {
    puts("Error while opening the file.");
    exit(EXIT_FAILURE);
  }

  /* Go to the end of the file. */
  if (fseek(fp, 0L, SEEK_END) == 0) {
    /* Get the size of the file. */
    long bufSize = ftell(fp);

    if (bufSize == -1) {
      puts("Error reading - bufSize == -1");
      exit(EXIT_FAILURE);
    }

    iData->data = malloc(bufSize);

    if (fseek(fp, 0L, SEEK_SET) != 0) {
      puts("Error reading - could not seek to end");
      exit(EXIT_FAILURE);
    }

    /* Read the entire file into memory. */
    iData->len = fread(iData->data, sizeof(ubyte_t), bufSize, fp);

    if (iData->len != bufSize) {
      printf("Error reading - iData->len (%zu) != bufSize (%ld)", iData->len, bufSize);
      exit(EXIT_FAILURE);
    }

    if (ferror(fp) != 0) {
      puts("Error reading file");
      exit(EXIT_FAILURE);
    }
  }

  fclose(fp);
}

#define BYTE_TO_BINARY_PATTERN "%c%c%c%c%c%c%c%c"
#define BYTE_TO_BINARY(byte)  \
  (byte & 0x80 ? '1' : '0'), \
  (byte & 0x40 ? '1' : '0'), \
  (byte & 0x20 ? '1' : '0'), \
  (byte & 0x10 ? '1' : '0'), \
  (byte & 0x08 ? '1' : '0'), \
  (byte & 0x04 ? '1' : '0'), \
  (byte & 0x02 ? '1' : '0'), \
  (byte & 0x01 ? '1' : '0')

// ===== Main driver =====
int main(int argc, char *argv[]) {
  interpreter_data_t iData;

  if (argc == 1) {
    iData.data = malloc(512);
    iData.len = 0;

    uint8_t u8;
    uint16_t u16;
    uint32_t u32;
    uint64_t u64;

    #define STR(s) s

    #define PUT_BYTES(sz, cmd) \
      do { \
        u##sz = STR(cmd); \
        memcpy((iData.data + iData.len), &u##sz, sizeof(u##sz)); \
        iData.len += sizeof(u##sz); \
      } while (0)

    PUT_BYTES(8, makeInstruction(OP_LOAD, CONST_FLAGS_I64));
    PUT_BYTES(32, obj_loc_make(1, AT_ABS | AT_DATA));
    PUT_BYTES(64, 255);

    PUT_BYTES(8, makeInstruction(OP_MOV, 0));
    PUT_BYTES(32, obj_loc_make(0, AT_ABS | AT_REG));
    PUT_BYTES(32, obj_loc_make(1, AT_ABS | AT_DATA));



    #undef PUT_BYTES
    #undef STR

    int i;
    for (i = 0; i < iData.len; i++) {
      printf(BYTE_TO_BINARY_PATTERN " ", BYTE_TO_BINARY(iData.data[i]));
    }

    puts("");

    // free(iData.data);

    // return 0;

  } else if (argc == 2) {
    openFile(&iData, argc, argv);
  } else {
    showArguments(argc, argv);
    return 1;
  }


  iData.rt = runtime_create();

  pthread_t interpreterThreadId, gcThreadId;

  pthread_create(&interpreterThreadId, NULL, interpreterThread, (void*)&iData);
  pthread_join(interpreterThreadId, NULL);

  pthread_create(&gcThreadId, NULL, gcThread, (void*)iData.rt);
  pthread_join(gcThreadId, NULL);

  runtime_gc(iData.rt);

  runtime_destroy(iData.rt);

  free(iData.data);

  return 0;
}
