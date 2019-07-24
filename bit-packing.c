#include <stdint.h>

enum ARCHETYPE {
  AT_DATA = 0x1,
  AT_LOCAL = 0x2,
  AT_REG = 0x3,

  AT_REL = 0x8,
  AT_ABS = 0xC
};

typedef int32_t obj_loc_t;
typedef uint8_t archtype_t;
typedef int32_t loc_28_t;

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

VALUE_TYPE value_getType(value_t *value) {
  // type is stored in first 8 bits of metadata
  return (VALUE_TYPE)(value->metadata & 0xFF);
}

typedef struct {
  value_t *storage[4];
} datatable_t;

#define MB_TO_BYTES(mb) (mb*1000000)

#define DEFAULT_STATIC_DATA_SIZE 150
#define DEFAULT_STACK_SIZE 256 // in MB
#define STACK_SIZE_BYTES (MB_TO_BYTES(DEFAULT_STACK_SIZE) - (MB_TO_BYTES(DEFAULT_STACK_SIZE) % sizeof(value_t)))
#define NUM_REGISTERS 4

void datatable_init(datatable_t *dt) {
  dt->storage[0] = NULL;

  dt->storage[AT_DATA] = malloc(sizeof(value_t) * DEFAULT_STATIC_DATA_SIZE);
  dt->storage[AT_LOCAL] = malloc(STACK_SIZE_BYTES);
  dt->storage[AT_REG] = malloc(sizeof(value_t) * NUM_REGISTERS);

  memset(dt->storage[AT_DATA], 0, sizeof(value_t) * DEFAULT_STATIC_DATA_SIZE);
  memset(dt->storage[AT_LOCAL], 0, STACK_SIZE_BYTES);
  memset(dt->storage[AT_REG], 0, sizeof(value_t) * NUM_REGISTERS);
}


// =====
int main() {
  obj_loc_t o = obj_loc_make(123/* 28-bit int */, AT_DATA | AT_ABS);

  loc_28_t loc;
  archtype_t at;
  obj_loc_parse(o, &loc, &at);

  

  return 0;
}
