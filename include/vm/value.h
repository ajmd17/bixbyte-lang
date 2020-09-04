#pragma once

#include <vm/rc.h>
#include <vm/types.h>

#include <stdint.h>
#include <stdbool.h>

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

typedef struct value {
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
} value_t;

void value_destroy(runtime_t *rt, value_t *value);
void value_copyValue(runtime_t *rt, value_t *v, value_t *other);
void value_setInt(runtime_t *rt, value_t *v, int64_t i64);
int64_t value_getInt(value_t *v);
value_t value_fromInt(int64_t i64);
void value_setUint(runtime_t *rt, value_t *v, uint64_t u64);
uint64_t value_getUint(value_t *v);
value_t value_fromUint(uint64_t u64);
void value_setDouble(runtime_t *rt, value_t *v, double dbl);
double value_getDouble(value_t *v);
value_t value_fromDouble(double dbl);
void value_setBoolean(runtime_t *rt, value_t *v, bool b);
bool value_getBoolean(value_t *v);
value_t value_fromBoolean(bool b);
value_t value_createObject(runtime_t *rt, heap_t *heap);
heap_value_t *value_getHeapNode(value_t *value);
void *value_getRawPointer(value_t *value);
void value_setRawPointer(runtime_t *rt, value_t *v, void *raw, VALUE_FLAGS flags);
value_t value_fromRawPointer(void *raw, VALUE_FLAGS flags);
void value_setRefCounted(runtime_t *rt, value_t *v, void *ptr);
value_t value_fromFunction(native_function_t fn);
void value_setFunction(runtime_t *rt, value_t *v, native_function_t fn);
VALUE_TYPE value_getType(value_t *value);
void value_setType(value_t *value, VALUE_TYPE type);
VALUE_FLAGS value_getFlags(value_t *value);
void value_setFlag(value_t *value, VALUE_FLAGS flag, int state);
uintptr_t value_getID(value_t *value);
value_t value_invoke(runtime_t *r, value_t *value);
