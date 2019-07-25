#include <stdint.h>
#include <string.h>
#include <stdlib.h>

// ===== Instructions =====

enum ARCHETYPE {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
};

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

// ===== Values =====

typedef uint32_t metadata_t;

enum VALUE_TYPE {
  TYPE_NONE = 0x0,
  TYPE_INT = 0x1,
  TYPE_DOUBLE = 0x2,
  TYPE_BOOLEAN = 0x3,
  TYPE_POINTER = 0x4
};

typedef struct {
  union {
    int64_t i64;
    uint64_t u64;
    double dbl;
    void *ptr;
  };

  metadata_t metadata;
} value_t;

void value_destroy(value_t *value) {
  // @TODO 
}

VALUE_TYPE value_getType(value_t *value) {
  // type is stored in first 8 bits of metadata
  return (VALUE_TYPE)(value->metadata & 0xFF);
}

// ===== Data storage =====

#define MB_TO_BYTES(mb) (mb*1000000)
#define DEFAULT_STATIC_DATA_SIZE_MB 150 // in MB
#define STATIC_DATA_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STATIC_DATA_SIZE_MB) % sizeof(value_t)))
#define DEFAULT_STACK_SIZE_MB 256 // in MB
#define STACK_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) - (MB_TO_BYTES(DEFAULT_STACK_SIZE_MB) % sizeof(value_t)))
#define NUM_REGISTERS 4

typedef struct {
  value_t *data;
  size_t len;
} storage_t;

typedef struct {
  storage_t storage[4];
} datatable_t;

datatable_t *datatable_new() {
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

void datatable_destroy(datatable_t *dt) {
  int i;

  for (i = 0; i < 4; i++) {
    while (dt->storage[i].len) {
      value_destroy(&dt->storage[i].data[--dt->storage[i].len]);
    }

    free(dt->storage[i].data);
  }
  
  free(dt);
}

value_t *datatable_getValue(datatable_t *dt, loc_28_t loc, archtype_t at) {
  storage_t *s = &dt->storage[at & 0x3];

  int abs = ((at & 0xC) >> 2) - 2;
  size_t idx = (-abs & loc) | ((abs - 1) & (s->len - loc));

  return &s->data[idx];
}

// ===== VM =====

// ===== Main driver =====
int main() {
  obj_loc_t o = obj_loc_make(123/* 28-bit int */, AT_DATA | AT_ABS);

  loc_28_t loc;
  archtype_t at;
  obj_loc_parse(o, &loc, &at);
  
  datatable_t *dt = datatable_new();
  
  datatable_destroy(dt);
  return 0;
}
